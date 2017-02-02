#include <AFMotor.h>
#include <Wire.h>

AF_DCMotor motorL(3);
AF_DCMotor motorR(4);

#define uchar unsigned char
uchar t;
uchar data[16];

byte dataByte[8];
bool line[8];
byte threshhold;
uchar tempData;

bool stop;

void setup()
{
  stop = false;
  
  motorL.setSpeed(0);
  motorL.run(RELEASE);
 
  motorR.setSpeed(0);
  motorR.run(RELEASE);
  
  Serial.begin(9600);
  Serial.println("HALLO WELT");
  
  t = 0;
  threshhold = 10;
  
  motorL.run(FORWARD);
  motorR.run(FORWARD);
}

void loop()
{
  readSensorData();
  printSensorDataToSerial();
  setMotorspeed();
  delay(10);
}

// SENSOREN:
// 00000000
// L______R

// 0 010 255
// 1 060 220
// 2 090 190
// 3 130 160
// 4 160 130
// 5 190 090
// 6 220 060
// 7 255 010

void setMotorspeed()
{
  int tempLeft = 0;
  int tempRight = 0;
  int count = 0;

  if(line[0] && line[1] && line[2] && line[3] && line[4] && line[5] && line[6] && line[7])
  {
    stop = true;
  }
  else
  {
    stop = false;
  }
  
  if (line[0])
  {
    tempLeft += 10;
    tempRight += 255;
    count++;
  }

  if (line[1])
  {
    tempLeft += 60;
    tempRight += 220;
    count++;
  }

  if (line[2])
  {
    tempLeft += 90;
    tempRight += 190;
    count++; 
  }

  if (line[3])
  {
    tempLeft += 130;
    tempRight += 160;
    count++;
  }

  if (line[4])
  {
    tempLeft += 160;
    tempRight += 130;
    count++;
  }

  if (line[5])
  {
    tempLeft += 190;
    tempRight += 90;
    count++; 
  }

  if (line[6])
  {
    tempLeft += 220;
    tempRight += 60;
    count++; 
  }

  if (line[7])
  {
    tempLeft += 255;
    tempRight += 10;
    count++;
  }

  if (count == 0)
  {
    tempLeft = 0;
    tempRight = 0;
  }

  tempLeft = tempLeft / count;
  tempRight = tempRight / count;

  if (stop == false)
  {
    motorL.setSpeed(0);
    motorR.setSpeed(0);
  }
  else
  {
    motorL.setSpeed(250);
    delay(10);
    motorL.setSpeed(tempLeft);
    
    motorR.setSpeed(250);
    delay(10);
    motorR.setSpeed(tempRight);
  }
}

void printSensorDataToSerial()
{
  for (int i = 0; i < 8; i++)
  {
    Serial.print(String(line[i]) + "");
  }
  Serial.println("");
}

void readSensorData()
{
  Wire.begin();
  Wire.requestFrom(9, 16);
  while (Wire.available())
  {
    tempData = Wire.read();
    
    if (t % 2 == 0)
    {
      dataByte[t/2] = tempData;
      line[t/2] = tempData < threshhold;
    }
    
    if (t < 15)
      t++;
    else
      t = 0;
  }
}
