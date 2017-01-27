#include <AFMotor.h>
#include <Wire.h>

//AF_DCMotor motorL(3);
//AF_DCMotor motorR(4);

#define uchar unsigned char
uchar t;
uchar data[16];

byte dataByte[8];
byte dataByteOffset[8];
bool line[8];
bool sensorInitialized;
byte threshhold;
uchar tempData;

void setup()
{
  //motorL.setSpeed(160);
  //motorL.run(FORWARD);
  
  //motorR.setSpeed(160);
  //motorR.run(FORWARD);
  
  //delay(1000);
  
  //motorL.setSpeed(0);
  //motorL.run(RELEASE);
 
  //motorR.setSpeed(0);
  //motorR.run(RELEASE);
  
  Serial.begin(9600);
  Serial.println("HALLO WELT");
  t = 0;
  threshhold = 100;

  sensorInitialized = false;
  initSensor();
}

void initSensor()
{
  Serial.println("Sensor initialisieren");

  byte data1[8];
  byte data2[8];
  byte data3[8];
  byte data4[8];

  Serial.println("1/4");
  readSensorData(data1);
  Serial.println("2/4");
  delay(1000);
  readSensorData(data2);
  Serial.println("3/4");
  delay(1000);
  readSensorData(data3);
  Serial.println("4/4");
  delay(1000);
  readSensorData(data4);

  for(int i = 0; i < 8; i++)
  {
    int temp = data1[i];
    temp += data2[i];
    temp += data3[i];
    temp += data4[i];

    temp = byte((temp - 40) / 4);
    
    dataByteOffset[i] = temp;

    Serial.print(String(i) + ": " + String(dataByteOffset[i]) + " | ");
  }

  delay(3000);
  Serial.println("");
  
  Serial.println("Sensor initialisiert");
  sensorInitialized = true;
}

void printSensorDataToSerial()
{
  for (int i = 0; i < 8; i++)
  {
    Serial.print(String(i) + "\t");
  }
  Serial.println("");
  
  for (int i = 0; i < 8; i++)
  {
    Serial.print(String(line[i]) + "\t");
  }
  Serial.println("");
  
  for (int i = 0; i < 8; i++)
  {
    Serial.print(String(dataByte[i]) + "\t");
  }
  Serial.println("");
  Serial.println("");
}

void readSensorData(byte data[])
{
  Wire.begin();
  Serial.println("TEST1");
  Wire.requestFrom(9, 16);
  Serial.println("TEST2");
  while (Wire.available())
  {
    tempData = Wire.read();
    
    if (t % 2 == 0)
    {
      if (sensorInitialized == true)
      {
        tempData = tempData - dataByteOffset[t/2];
      }
      
      data[t/2] = tempData;
      line[t/2] = tempData < threshhold;
      
      // Serial.println(String(t) + "\t" + String(tempData) + "\t" + line[t/2]);
    }
    
    if (t < 15)
      t++;
    else
      t = 0;
  }
}

void loop()
{
  readSensorData(dataByte);
  printSensorDataToSerial();
  Serial.println("");
  Serial.println("");
  delay(500);
}




