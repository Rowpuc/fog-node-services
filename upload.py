import pymongo
import time
from pymongo import MongoClient
from pymongo import CursorType
from pprint import pprint
import json
import firebase_admin
from firebase_admin import credentials
from firebase_admin import db
from firebase_admin import firestore
import time
import random


client = MongoClient("localhost", 27017)

db = client["IoT-Dashboard"]

collection = db["readings"]


cred = credentials.Certificate('/home/pi/fog-node-services/fbakey.json')
firebase_admin.initialize_app(cred, {
    'projectId': 'iot-dashboard-d1f74'
})
cloudDb = firestore.client()

cursor = collection.find({"uploaded":{"$exists": True}},cursor_type = CursorType.TAILABLE_AWAIT)
while cursor.alive:
    try:
        doc = cursor.next()
        if doc['uploaded'] == False:
        
          sensors = cloudDb.collection('sensors').document(str(doc['sensorId']))
          push_sensor = sensors.set({
            'id': doc['sensorId'],
            'type': doc['type'],
            'location': "Unknown",
            'maxValue': -1,
            'minValue': -1
          }, firestore.CreateIfMissingOption(True))
        
          readings = cloudDb.collection('readings').document(str(doc['_id']))
          push_reading = readings.set({
            'createdDate': firestore.SERVER_TIMESTAMP,
            'sensorId': doc['sensorId'],
            'type': doc['type'],
            'value': doc['value'],
             })
          print('Uploaded Mongo document: '+ str(doc['_id']))
          collection.update_one({'_id':doc['_id']},{'$set': {'uploaded': True}})
          
          
        else:
          print('Already uploaded '+ str(doc['_id']))
    except StopIteration:
        time.sleep(1)