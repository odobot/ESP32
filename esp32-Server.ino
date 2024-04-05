#include <WiFiNINA.h>

char ssid[] = "dekut";      //  your network SSID (name)
char pass[] = "dekut@ict2023";   // your network password
int status = WL_IDLE_STATUS;

WiFiClient client;

byte incomingbyte;

int startingAddress = 0x0000;  // Read Starting address
uint8_t MH, ML;
boolean EndFlag = false;

void SendResetCmd();
void SendTakePhotoCmd();
void SendReadDataCmd();
void StopTakePhotoCmd();
void sendDataToServer(byte *data, int length);

void setup() {
  Serial.begin(19200);
  Serial2.begin(38400);
  
  SendResetCmd();
  delay(3000);

  // Attempt to connect to WiFi network
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid, pass);
    delay(5000);  // Wait 5 seconds for connection
  }

  Serial.println("Connected to WiFi");
}

void loop() {
  SendTakePhotoCmd();
  
  Serial.println("Start pic"); 
  delay(100);

  while(Serial2.available() > 0) {
    incomingbyte = Serial2.read();
  }
  byte b[32];
      
  while(!EndFlag) {  
    int j = 0;
    int k = 0;
    int count = 0;
    SendReadDataCmd();
           
    delay(75); //try going up
    while(Serial2.available() > 0) {
      incomingbyte = Serial2.read();
      k++;
      if((k > 5) && (j < 32) && (!EndFlag)) {
        b[j] = incomingbyte;
        if((b[j-1] == 0xFF) && (b[j] == 0xD9)) {
          EndFlag = true;
        }
        j++;
        count++;
      }
    }
            
    for(j = 0; j < count; j++) {   
      if(b[j] < 0x10) {
        Serial.print("0");
      }
      Serial.print(b[j], HEX);
    }
    Serial.println();

    // Send data to server
    sendDataToServer(b, count);
  }
  
  delay(3000);
  StopTakePhotoCmd(); //stop this picture so another one can be taken
  EndFlag = false; //reset flag to allow another picture to be read
  Serial.println("End of pic");
  Serial.println(); 
}

//Send Reset command
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
    
  startingAddress = 0x0000; //reset so that another picture can taken
}

//Read data
void SendReadDataCmd() {
  MH = startingAddress / 0x100;
  ML = startingAddress % 0x100;
      
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

  startingAddress += 0x20; 
}

void StopTakePhotoCmd() {
  Serial2.write((byte)0x56);
  Serial2.write((byte)0x00);
  Serial2.write((byte)0x36);
  Serial2.write((byte)0x01);
  Serial2.write((byte)0x03);        
}

void sendDataToServer(byte *data, int length) {
  if (client.connect("yourflaskserver.com", 80)) {
    Serial.println("Connected to server");
    // Make a HTTP request
    client.println("POST /upload HTTP/1.1");
    client.println("Host: yourflaskserver.com");
    client.println("Content-Type: application/octet-stream");
    client.print("Content-Length: ");
    client.println(length);
    client.println();
    client.write(data, length);

    // Wait for response from the server
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.print(c);
      }
    }

    // Disconnect from server
    client.stop();
  } else {
    Serial.println("Unable to connect to server");
  }
}
