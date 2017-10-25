#include <PID_v1.h>
#include <Wire.h>
#include <AFMotor.h>

// Werteberechnung
unsigned int raw_data[8], res_data[8], fil_data[8];
unsigned long weighted_sum, sum;
double error;
int t, linePosition, linePosition_leftMost, linePosition_rightMost;
bool setupDone = false;

// PID
double Setpoint, Input, Output;

// double consKp=0.001, consKi=0.02, consKd=0.19; // Sehr Gut!
double consKp=0.001, consKi=0.015, consKd=0.09;
int generalMotorSpeed = 60;

PID mainPID(&Input, &Output, &Setpoint, consKp, consKi, consKd, DIRECT);

// Motoren
#include <AFMotor.h>
AF_DCMotor motorL(4);
AF_DCMotor motorR(3);

/*
  Daten des Sensors einlesen.
*/
void readDataRaw()
{
  Wire.requestFrom(9, 16);
  for(int i = 0; i < 8; i++)
  {
    raw_data[i] = Wire.read() << 8 | Wire.read();
    fil_data[i] = raw_data[i] < 520 ? 1 : 0;
  }
}

/*
  Die Linienposition errechnen.
*/
unsigned int calcLinePosition()
{
  weighted_sum = 0;
  sum = 0;
  
  for (int i = 0; i < 8; i++)
  {
    weighted_sum += fil_data[i] * 100UL * (unsigned long)i;
    sum += fil_data[i];
  }

  // Wenn die Summe 0 ist, haben wir die Linie verloren und dürfen die Position nicht neu setzen.
  if (sum > 0 && sum < 4)
  {
    linePosition = map(weighted_sum / sum, 0, 700, 0, 255);
  }
}

/*
  Den Fehler berechnen.
*/
void calcError()
{
  error = linePosition - 127;
}

/*
  Daten des Sensors ausgeben
*/
void printDataToSerial(unsigned int data[8])
{
  for (int i = 0; i < 8; i++)
  {
    Serial.print(data[i]);
    Serial.print("");
  }
  
  Serial.println();
}

/*
  Das Setup.
*/
void setup()
{
  Serial.begin(9600);
  Serial.println("LineFollower v1");
  
  Wire.begin();
  t = 0;
  
  readDataRaw();
  calcLinePosition();
  calcError();
  
  //initialize the variables we're linked to
  Input = error;
  Setpoint = 0;

  //turn the PID on
  mainPID.SetOutputLimits(-255, 255);
  mainPID.SetMode(AUTOMATIC);
  
  setupDone = true;
}

/*
  Der Loop.
*/
void loop()
{
  readDataRaw();
  calcLinePosition();
  calcError();
  
  Input = error;
  mainPID.Compute();

  // printDataToSerial(fil_data);
  // printDataToSerial(raw_data);

  // Serial.println(Output);
  
  // Error negativ => Output positiv => nach links fahren
  // Error positiv => Output negativ => nach rechts fahren
  
  int leftMotorSpeed = generalMotorSpeed + Output;
  int rightMotorSpeed = generalMotorSpeed - Output;

  leftMotorSpeed = constrain(leftMotorSpeed, 0, 255);
  rightMotorSpeed = constrain(rightMotorSpeed, 0, 255);
  
  if (leftMotorSpeed < 0)
  {
    motorL.run(BACKWARD);
  }
  else
  {
    motorL.run(FORWARD);
  }
  
  if (rightMotorSpeed < 0)
  {
    motorR.run(BACKWARD);
  }
  else
  {
    motorR.run(FORWARD);
  }

  // Gucken ob linie da. Wenn nicht, rumdrehen, wenn ja, weiterfahren.´
  // int sum = 0;
  // for (int i = 0; i < 8; i++)
  // {
  //   sum += fil_data[i];
  // }

  // if (sum > 0)
  // {
    // Weiterfahren
  // }
  // else
  // {
    // Rumdrehen
    // motorL.setSpeed(0);
    // motorR.setSpeed(0);
  // } 
  
  motorL.setSpeed(abs(leftMotorSpeed));
  motorR.setSpeed(abs(rightMotorSpeed)); 
  
  // Serial.print("LinePos: ");
  // Serial.println(linePosition);
  // 
  // Serial.print("Error:   ");
  // Serial.println(error);
  // 
  // Serial.print("Output:  ");
  // Serial.println(Output);
  // Serial.println();
  
  delay(10);
  // delay(500);
}
