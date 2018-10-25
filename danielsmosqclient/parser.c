//mosquitto_pub -d -t /test/topic -m "hello from terminal 2"
//mosquitto_sub -d -t /test/topic
#include <mongoc.h>

#include <mosquitto.h>
#include <signal.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <errno.h>

#define PORT 1883
#define HOST "localhost"
#define SUBTOPIC "/readings"

struct mosquitto *mosq = NULL;
static int run = 1;

int send_to_mongo(char *data)
{
	const char *uri_string = "mongodb://localhost:27017";
	mongoc_uri_t *uri;
	mongoc_client_t *client;
	mongoc_collection_t *collection;
	bson_t *insert;
	bson_error_t error;

	/*
	* Required to initialize libmongoc's internals
	*/
	mongoc_init ();

	/*
	* Safely create a MongoDB URI object from the given string
	*/
	uri = mongoc_uri_new_with_error (uri_string, &error);
	if (!uri) {
		fprintf (stderr,
		       "failed to parse URI: %s\n"
		       "error message:       %s\n",
		       uri_string,
		       error.message);
		return EXIT_FAILURE;
	}

	/*
	* Create a new client instance
	*/
	client = mongoc_client_new_from_uri (uri);
	if (!client) {
		return EXIT_FAILURE;
	}

	/*
	* Register the application name so we can track it in the profile logs
	* on the server. This can also be done from the URI (see other examples).
	*/
	mongoc_client_set_appname (client, "connect-example");

	/*
	* Get a handle on the database "db_name" and collection "coll_name"
	*/
	collection = mongoc_client_get_collection (client, "readings", "temperature");
	insert = bson_new_from_json((const uint8_t *)data, -1, &error);
	if (!insert)
	{
		fprintf(stderr, "bsonfromjson: %s\n", error.message);
		return EXIT_FAILURE;
	}

	if (!mongoc_collection_insert_one (collection, insert, NULL, NULL, &error)) {
		fprintf (stderr, "collection: %s\n", error.message);
	}
	//should insert some error logging somewhere near here

   /*
    * Release our handles and clean up libmongoc
    */
	bson_destroy (insert);
	mongoc_collection_destroy (collection);

	mongoc_uri_destroy (uri);
	mongoc_client_destroy (client);
	mongoc_cleanup ();

	return EXIT_SUCCESS;
}

void handle_signal(int s)
{
	run = 0;
	mosquitto_destroy(mosq);
	mosquitto_lib_cleanup();
}

void connect_callback(struct mosquitto *mosq, void *obj, int result)
{
	printf("connect callback, rc=%d\n", result);
}

void message_callback(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message)
{
	//printf("got message '%.*s' for topic '%s'\n", message->payloadlen, (char*) message->payload, message->topic);
	//printf("got a message callback");
	send_to_mongo((char*) message->payload);
}

int main(int argc, char *argv[])
{
	int err;
	//char *str = "this is a test string";

	signal(SIGINT, handle_signal);
	signal(SIGTERM, handle_signal);

	mosquitto_lib_init();

	mosq = mosquitto_new("mongoparser", true, NULL);
	if (!mosq)
	{
		printf("failed to make mosquitto struct");
		mosquitto_lib_cleanup();
		return 1;
	}
	mosquitto_connect_callback_set(mosq, connect_callback);
	mosquitto_message_callback_set(mosq, message_callback);

	err = mosquitto_connect(mosq, HOST, PORT, 60);
	if (err != MOSQ_ERR_SUCCESS)
	{
		switch(err)
		{
			case MOSQ_ERR_INVAL:
				printf("connect err: %s\n", mosquitto_strerror(err));
				break;
			case MOSQ_ERR_ERRNO:
				printf("%s\n", strerror(errno));
				break;
			default:
				printf("broken: %d\n", err);
				break;
		}
		mosquitto_lib_cleanup();
		return 1;
	}
	else
		printf("connected\n");

	/*err = mosquitto_publish(mosq, NULL, SUBTOPIC, strlen(str), str, 0, false);
	if (err != MOSQ_ERR_SUCCESS)
	{
		printf("publish err: %s\n", mosquitto_strerror(err));
		mosquitto_disconnect(mosq);
		mosquitto_lib_cleanup();
		return 1;
	}*/
	mosquitto_subscribe(mosq, NULL, SUBTOPIC, 0);
	while(run)
	{
		err = mosquitto_loop(mosq, -1, 1);
		if(run && err)
		{
			printf("connection error!\n");
			sleep(10);
			mosquitto_reconnect(mosq);
		}
		//printf("loop");
	}
	return 1;
}

