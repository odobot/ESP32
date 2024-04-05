//______________HTTTP__________________
#include <WiFi.h>
#include <HTTPClient.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>
//#include <BLE2a05.h>
#include <Wire.h>

//___________JPEG CAMERA_____________
String dataToSend = "88";
String hexData = "";
byte incomingbyte;
int data_index = 0;

int a = 0x0000,  //Read Starting address
  j = 0,
    k = 0,
    count = 0;
uint8_t MH, ML;
boolean EndFlag = 0;
int Camera_data[10000];

//___________BLUETOOTH________________

#define SERVICE_UUID "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_RX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define BLEName "ESP32_BLE"
bool deviceConnected = false;
bool oldDeviceConnected = false;
BLECharacteristic *pCharacteristicTX;

class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *pServer) {
    Serial.println("Client Connected!");
    digitalWrite(13, HIGH);
  };

  void onDisconnect(BLEServer *pServer) {
    Serial.println("Client disconnecting... Waiting for new connection");
    digitalWrite(13, LOW);
    pServer->startAdvertising();  // restart advertising
  }
};


//BLE RX callback
class MyCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristicTX) {
    Serial.print("Receiving.........");
    // uint8_t *data = pCharacteristicTX->getData();
    // //std::string rxValue = pCharacteristicTX->getValue();
    // int size = pCharacteristicTX->getLength();

    // if (size > 0) {

    // }
  }
};

//___________ULTRASONIC________________
#define SOUND_SPEED 0.034

const int trigPins[3] = { 25, 26, 27 };
const int echoPins[3] = { 34, 35, 32 };
float distance[3] = { 0.0, 0.0, 0.0 };
String direction[3] = { "LEFT", "FRONT", "RIGHT" };

long duration;
float distanceCm;
float distanceInch;

void ultrasonic_setup() {
  pinMode(trigPins[0], OUTPUT);  // Sets the trigPin as an Output
  pinMode(echoPins[0], INPUT);   // Sets the echoPin as an Input

  pinMode(trigPins[1], OUTPUT);  // Sets the trigPin as an Output
  pinMode(echoPins[1], INPUT);   // Sets the echoPin as an Input

  pinMode(trigPins[2], OUTPUT);  // Sets the trigPin as an Output
  pinMode(echoPins[2], INPUT);   // Sets the echoPin as an Input
}

String Closest_Object() {
  for (int i = 0; i < 3; i++) {
    distance[i] = read_ultrasonic(i);
    delay(100);
    // Serial.println(String(distance[i]) + "cm" +" "+ direction[i]);
  }
  String Output;

  if (distance[0] < distance[1] && distance[0] < distance[2] && distance[0] < 100) {
    Serial.println("LEFT" + String(distance[0]) + "cm");
    Output = "OBSTACLE LEFT" + String(distance[0]) + "CENTIMETRES";
    return Output;

  } else if (distance[1] < distance[0] && distance[1] < distance[2] && distance[1] < 100) {
    Serial.println("FRONT" + String(distance[1]) + "cm");
    Output = "OBSTACLE INFRONT" + String(distance[1]) + "CENTIMETRES";
    return Output;


  } else if (distance[2] < distance[1] && distance[2] < distance[0] && distance[2] < 100) {
    Serial.println("RIGHT" + String(distance[2]) + "cm");
    Output = "OBSTACLE RIGHT" + String(distance[2]) + "CENTIMETRES";
    return Output;


  } else {
    Serial.println("NOT IN RANGE");
    Output = "NO OBSTACLE";
    return Output;
  }
}

void disance_measure() {
  for (int i = 0; i < 3; i++) {
    distance[i] = read_ultrasonic(i);
    delay(100);
    // Serial.println(String(distance[i]) + "cm" +" "+ direction[i]);
  }

  if (distance[0] < distance[1] && distance[0] < distance[2] && distance[0] < 100) {
    Serial.println("LEFT" + String(distance[0]) + "cm");
    pCharacteristicTX->setValue("obstacle on the left");

  } else if (distance[1] < distance[0] && distance[1] < distance[2] && distance[1] < 100) {
    Serial.println("FRONT" + String(distance[1]) + "cm");
    pCharacteristicTX->setValue("obstacle upfront ");


  } else if (distance[2] < distance[1] && distance[2] < distance[0] && distance[2] < 100) {
    Serial.println("RIGHT" + String(distance[2]) + "cm");
    pCharacteristicTX->setValue("obstacle on the right");

  } else {
    Serial.println("NOT IN RANGE");
    pCharacteristicTX->setValue("no obstacle in range");
  }
}

float read_ultrasonic(int num) {

  digitalWrite(trigPins[num], LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPins[num], HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPins[num], LOW);

  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPins[num], HIGH);

  // Calculate the distance
  distanceCm = duration * SOUND_SPEED / 2;
  return distanceCm;
}

void SendResetCmd() {
  Serial2.write((byte)0x56);
  Serial2.write((byte)0x00);
  Serial2.write((byte)0x26);
  Serial2.write((byte)0x00);
}

//Send take picture command
void SendTakePhotoCmd() {
  Serial2.write((byte)0x56);
  Serial2.write((byte)0x00);
  Serial2.write((byte)0x36);
  Serial2.write((byte)0x01);
  Serial2.write((byte)0x00);

  a = 0x0000;  //reset so that another picture can taken
}

void FrameSize() {
  Serial2.write((byte)0x56);
  Serial2.write((byte)0x00);
  Serial2.write((byte)0x34);
  Serial2.write((byte)0x01);
  Serial2.write((byte)0x00);
}

//Read data
void SendReadDataCmd() {
  MH = a / 0x100;
  ML = a % 0x100;

  Serial2.write((byte)0x56);
  Serial2.write((byte)0x00);
  Serial2.write((byte)0x32);
  Serial2.write((byte)0x0c);
  Serial2.write((byte)0x00);
  Serial2.write((byte)0x0a);
  Serial2.write((byte)0x00);
  Serial2.write((byte)0x00);
  Serial2.write((byte)MH);
  Serial2.write((byte)ML);
  Serial2.write((byte)0x00);
  Serial2.write((byte)0x00);
  Serial2.write((byte)0x00);
  Serial2.write((byte)0x20);
  Serial2.write((byte)0x00);
  Serial2.write((byte)0x0a);

  a += 0x20;
}

void StopTakePhotoCmd() {
  Serial2.write((byte)0x56);
  Serial2.write((byte)0x00);
  Serial2.write((byte)0x36);
  Serial2.write((byte)0x01);
  Serial2.write((byte)0x03);
}

void Read_Save_Image() {
  while (Serial2.available() > 0) {
    incomingbyte = Serial2.read();
  }
  byte b[32];

  while (!EndFlag) {
    j = 0;
    k = 0;
    count = 0;
    SendReadDataCmd();

    delay(75);  //try going up
    while (Serial2.available() > 0) {
      incomingbyte = Serial2.read();
      k++;
      if ((k > 5) && (j < 32) && (!EndFlag)) {
        b[j] = incomingbyte;
        if ((b[j - 1] == 0xFF) && (b[j] == 0xD9))
          EndFlag = 1;
        j++;
        count++;
      }
    }

    Serial.print("COUNT VALUE:" + String(count));
    for (j = 0; j < count; j++) {
      if (b[j] < 0x10)
        //Serial.print("0");
        //Serial.print(b[j], HEX);
        dataToSend = String(b[j], HEX);
    }
    Serial.println();
  }

  delay(3000);
  StopTakePhotoCmd();  //stop this picture so another one can be taken
  EndFlag = 0;         //reset flag to allow another picture to be read
  Serial.println("End of pic");
  Serial.println();
  Serial.println("Data stored");
}

void setup() {
  Serial.begin(115200);
  ultrasonic_setup();
  Serial.println("Starting BLE work");

  // create BLE DEVICE
  BLEDevice::init(BLEName);
  Serial.printf("BLE Server Mac Address: %s\n", BLEDevice::getAddress().toString().c_str());



  //CREATE BLE Server
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());



  //CREATE A SERVICE
  BLEService *pService = pServer->createService(SERVICE_UUID);
  //Serial.print(pService);

  //CREATE a BLE CHARACTERISTIC FOR SENDING
  pCharacteristicTX = pService->createCharacteristic(
    CHARACTERISTIC_UUID_TX,
    BLECharacteristic::PROPERTY_READ);
  // pCharacteristicTX->addDescriptor(new BLE2902());
  //pCharacteristicTX->setValue("Hello Steve");

  //Create a BLE CHARACTERISTIC FOR RECEIVING
  BLECharacteristic *pCharacteristicRX = pService->createCharacteristic(
    CHARACTERISTIC_UUID_RX,
    BLECharacteristic::PROPERTY_WRITE);
  pCharacteristicRX->setCallbacks(new MyCallbacks());
  // pCharacteristicRX->addDescriptor(new BLE2902());



  //START THE SERVICE
  pService->start();

  // //START ADVERTISING
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x00);  // set value to 0x00 to not advertise this parameter
  BLEDevice::startAdvertising();
  Serial.println("Waiting a client connection to notify...");
  SendResetCmd();
  delay(6000);
}

void loop() {
  // disance_measure();
  Serial.println(dataToSend);
}
