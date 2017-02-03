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
int left, right;

bool stop;

void setup()
{
  stop = false;
  
  motorL.setSpeed(0);
  motorL.run(FORWARD);
 
  motorR.setSpeed(0);
  motorR.run(FORWARD);
  
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
  
  if (line[0] && line[1] && line[2] && line[3] && line[4] && line[5] && line[6] && line[7])
  {
    tempRight = 0;
    tempLeft = 0;
  }
  else if (
    (line[0] == false) && 
    (line[1] == false) &&
    (line[2] == false) &&
    (line[3] == false) &&
    (line[4] == false) &&
    (line[5] == false) &&
    (line[6] == false) && 
    (line[7] == false))
  {
    tempLeft = left;
    tempRight = right;
  }
  else
  {
    if (line[0] || line[1])
    {
      tempRight = 250;
      tempLeft = 0;
    }
    
    if (line[6] || line[7])
    {
      tempRight = 0;
      tempLeft = 250;
    }
  
    if (line[3] || line[4])
    {
      tempRight = 250;
      tempLeft = 250;
    }
    
    if (line[2])
    {
      tempRight = 200;
      tempLeft = 0;
    }
    
    if (line[5])
    {
      tempRight = 0;
      tempLeft = 200;
    }
  }

  if (tempLeft < 0)
  {
    motorL.run(BACKWARD);
    tempLeft = (-1) * tempLeft;
  }
  else if (tempLeft > 0)
  {
    motorL.run(FORWARD);
  }
  else
  {
    motorL.run(RELEASE);
  }
  
  if (tempRight < 0)
  {
    motorR.run(BACKWARD);
    tempRight = (-1) * tempRight;
  }
  else if (tempRight > 0)
  {
    motorR.run(FORWARD);
  }
  else
  {
    motorR.run(RELEASE);
  }

  if (tempLeft == 0 && tempRight == 0)
  {
    delay(1000);
  }

  left = tempLeft;
  right = tempRight;
  
  motorL.setSpeed(tempLeft);
  motorR.setSpeed(tempRight);
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
