#include "pb_LEDLight.h"
#include <Wire.h>
#include <PID_v1.h>

// Linienberachnung
unsigned int raw_data[8], fil_data[8];
unsigned long weighted_sum, sum;
double error;
int linePosition;
uint8_t readIterationsPerCycle = 10;

// PID Regler
double Setpoint, Input, Output;

// double Kp=0.09, Ki=0, Kd=0.07; Extrem gut. Fährt bereits > 15 Minuten ohne Proleme!
// double Kp=0.09, Ki=0, Kd=0.062; Auch gut
// double Kp=0.09, Ki=0, Kd=0.060; Auch gut

double Kp = 0.09, Ki = 0, Kd = 0.07;

int generalMotorSpeed = 45;
bool driveOn = true, sawEndpoint = false;

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

  Serial.println("Setup");

  digitalWrite(modePin, HIGH);

  readData();
  calcLinePosition();

  Input = error;
  Setpoint = 0;

  mainPID.SetOutputLimits(-255, 255);
  mainPID.SetMode(AUTOMATIC);
}

uint8_t counter = 0, counterMax = 8;
void loop()
{
  readData();
  calcLinePosition();
  writeSensorDataToLEDS();
  
  Input = error;
  mainPID.Compute();
  
  int leftMotorSpeed = generalMotorSpeed + Output;
  int rightMotorSpeed = generalMotorSpeed - Output;

  if (sumSensorValues() >= 4)
  {
    sawEndpoint = true;
  }

  if (sawEndpoint == true)
  {
    counter += 1;

    if (counter > counterMax)
    {
      // readData();
      if (sumSensorValues() <= 0)
      {
        driveMotors(0, 0);
        delay(700);
        driveOn = false;
        driveMotors(55, -40);
        do
        {
          delay(50);
          readData();
        } while (sumSensorValues() <= 0);
        
        driveMotors(0, 0);
        driveOn = true;
      }
      
      sawEndpoint = false;
      counter = 0;
    }
  }

  if (counter > counterMax)
  {
    counter = 0;
  }

  if (driveOn == true)
  {
    driveMotors(leftMotorSpeed, rightMotorSpeed);
  }

  delay(10);
}

void driveMotors(int leftMotorSpeed, int rightMotorSpeed)
{
  leftMotorSpeed = constrain(leftMotorSpeed, -255, 255);
  rightMotorSpeed = constrain(rightMotorSpeed, -255, 255);

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

  analogWrite(motorBEnablePin, abs(rightMotorSpeed));
  analogWrite(motorAEnablePin, abs(leftMotorSpeed));
}

void writeSensorDataToLEDS()
{
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
}

/*
 * Sensorwerte summieren.
 */
uint8_t sumSensorValues()
{
  uint8_t sensorSum = 0;
  for (uint8_t i = 0; i < 8; i++)
  {
    sensorSum += fil_data[i];
  }

  Serial.println(sensorSum);
  return sensorSum;
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
  double avg_data[8];
  for (int i = 0; i < 8; i++)
  {
    avg_data[i] = 0;
  }

  for (uint8_t iteration = 0; iteration < readIterationsPerCycle; iteration++)
  {
    Wire.requestFrom(9, 16);
    for (int i = 0; i < 8; i++)
    {
      raw_data[i] = Wire.read() << 8 | Wire.read();
      fil_data[i] = raw_data[i] < 540 ? 1 : 0;
      avg_data[i] = ((double)iteration * avg_data[i] + (double)fil_data[i]) / (double)(iteration + 1) ;
    }
  }

  for (int i = 0; i < 8; i++)
  {
    fil_data[i] = avg_data[i] < 0.5 ? 0 : 1;
  }
}
