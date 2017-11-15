#include "pb_LEDLight.h"
#include <Wire.h>
#include <PID_v1.h>

// Linienberachnung
unsigned int raw_data[8], fil_data[8];
unsigned long weighted_sum, sum;
double error;
int linePosition;

// PID Regler
double Setpoint, Input, Output;

// double Kp=0.09, Ki=0, Kd=0.07; Extrem gut. Fährt bereits > 15 Minuten ohne Proleme!
// double Kp=0.09, Ki=0, Kd=0.062; Auch gut
// double Kp=0.09, Ki=0, Kd=0.060; Auch gut

double Kp=0.09, Ki=0, Kd=0.07;

int generalMotorSpeed=45;
bool driveOn = true;

PID mainPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT);

// Motoren
// Richtung
const int motorBPhasePin = 11;
const int motorAPhasePin = 12;
// Geschwindigkeit
const int motorBEnablePin = 9;
const int motorAEnablePin = 10;

const int modePin = 8;

void setup()
{
  Serial.begin(9600);
  Wire.begin();
  pb_init();

  digitalWrite(modePin, HIGH);
  
  readData();
  calcLinePosition();

  Input = error;
  Setpoint = 0;
  
  mainPID.SetOutputLimits(-255, 255);
  mainPID.SetMode(AUTOMATIC);
}

void loop()
{
  readData();
  calcLinePosition();
  
  Input = error;
  mainPID.Compute();
  
  double b = 0;
  for (uint8_t i = 0; i < 8; i++)
  {
    if (fil_data[i] == 1)
    {
      b += pow(2, i);
    }
  }

  b += 0.1;

  writeNumber((byte)b);
  showNumber();

  // Wenn er eine Querlinie sieht, muss er schauen, ob die Fahrlinie danach weitergeht.
  // Wenn keine mehr da ist, rumdrehen.
  // Wenn noch eine da ist, weiter fahren.

  int leftMotorSpeed = generalMotorSpeed + Output;
  int rightMotorSpeed = generalMotorSpeed - Output;

  leftMotorSpeed = constrain(leftMotorSpeed, 0, 255);
  rightMotorSpeed = constrain(rightMotorSpeed, 0, 255);
  
  if (rightMotorSpeed < 0)
  {
    digitalWrite(motorBPhasePin, HIGH);
  }
  else
  {
    digitalWrite(motorBPhasePin, LOW);
  }
  
  if (leftMotorSpeed < 0)
  {
    digitalWrite(motorAPhasePin, HIGH);
  }
  else
  {
    digitalWrite(motorAPhasePin, LOW);
  }

  if (driveOn == true)
  {
    analogWrite(motorBEnablePin, abs(rightMotorSpeed));
    analogWrite(motorAEnablePin, abs(leftMotorSpeed)); 
  }
  
  delay(150);
}

/*
 * Linienposition und Fehlerwert  berechnen.
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

  error = linePosition - 127;
}

/*
 * Daten des Liniensensors lesen und verarbeiten.
 */
void readData()
{
  Wire.requestFrom(9, 16);
  for(int i = 0; i < 8; i++)
  {
    raw_data[i] = Wire.read() << 8 | Wire.read();
    fil_data[i] = raw_data[i] < 540 ? 1 : 0;
  }
}
