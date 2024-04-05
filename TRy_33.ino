byte incomingbyte;

int startingAddress = 0x0000;  // Read Starting address
uint8_t MH, ML;
boolean EndFlag = false;

void SendResetCmd();
void SendTakePhotoCmd();
void SendReadDataCmd();
void StopTakePhotoCmd();

void setup() {
  Serial.begin(19200);
  Serial2.begin(38400);
  
  SendResetCmd();
  delay(3000);
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
