//mosquitto_pub -d -t /test/topic -m "hello from terminal 2"
//mosquitto_sub -d -t /test/topic
#include <mosquitto.h>
#include <signal.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#define PORT 1883
#define HOST "localhost"
#define SUBTOPIC "/test/topic"

struct mosquitto *mosq = NULL;
static int run = 1;
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
	//process data

}

int main(int argc, char *argv[])
{
        int err;
        //char *str = "this is a test string";

        signal(SIGINT, handle_signal);
        signal(SIGTERM, handle_signal);

        mosquitto_lib_init();
	system("python3 /home/pi/Desktop/upload.py");
        mosq = mosquitto_new("firstclient", true, NULL);
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
        }
        return 1;
}

