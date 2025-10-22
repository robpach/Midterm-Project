#include <Arduino.h>
#include <P1AM.h>
#include <MotorEncoder.h>

int modInput = 1;
int modOutput = 2;

// input pins
int vertRef = 1;
int horizRef = 2;
int turnRef = 3;
int vertPulse1 = 5;
int vertPulse2 = 6;
int horizPulse1 = 7;
int horizPulse2 = 8;
int turnPulse1 = 9;
int turnPulse2 = 10;
int whiteAvail = 11;
int redAvail = 12;
int blueAvail = 13;
int warehouseAvail = 14;

// output pins
int motorUp = 1;
int motorDown = 2;
int motorForward = 3;
int motorBackward = 4;
int motorCW = 5;
int motorCCW = 6;
int compressor = 7;
int suction = 8;
int warehouse = 9;

enum ProcessingStates
{
  Home,
  Waiting,
  Pickup,
  DropOff,
  blank
};

ProcessingStates currentState = Home;

MotorEncoder vertMotor(modInput, modOutput, motorDown, motorUp, vertPulse1, vertRef);
MotorEncoder horizMotor(modInput, modOutput, motorBackward, motorForward, horizPulse1, horizRef);
MotorEncoder turnMotor(modInput, modOutput, motorCCW, motorCW, turnPulse1, turnRef);

void setup()
{
  delay(500);
  Serial.begin(9600);
  delay(500);

  while (!P1.init())
  {
    delay(100);
    Serial.println("Waiting for Connection");
  }
  Serial.println("Successfully Connected");

  P1.writeDiscrete(true, modOutput, compressor);
}

bool Turn(int d)
{
  return turnMotor.MoveTo(d);
}

bool Extend(int d)
{
  return horizMotor.MoveTo(d);
}

bool Raise(int d)
{
  return vertMotor.MoveTo(d);
}

void loop()
{
  switch (currentState)
  {
  case Home:
    vertMotor.Home();
    horizMotor.Home();
    turnMotor.Home();
    P1.writeDiscrete(false, modOutput, warehouse);
    currentState = Waiting;
    break;
  case Waiting:
    if ((P1.readDiscrete(modInput, whiteAvail) || P1.readDiscrete(modInput, redAvail) || P1.readDiscrete(modInput, blueAvail)))
    {
      currentState = Pickup;
    }
    break;
  case Pickup:
    Serial.println("Picking Up");
    if (P1.readDiscrete(modInput, whiteAvail))
    {
      while (!Turn(221))
        ;
      while (!Extend(180))
        ;
      while (!Raise(390))
        ;
      delay(500);
      P1.writeDiscrete(true, modOutput, suction);
      delay(500);
      currentState = DropOff;
      
    }
    else if (P1.readDiscrete(modInput, redAvail))
    {
      while (!Turn(185))
        ; 
      while (!Extend(210))
        ;
      while (!Raise(390))
        ;
      delay(500);
      P1.writeDiscrete(true, modOutput, suction);
      delay(500);
      currentState = DropOff;
    }
    else if (P1.readDiscrete(modInput, blueAvail))
    {
      while (!Turn(150))
        ;
      while (!Extend(280))
        ;
      while (!Raise(390))
        ;
      delay(500);
      P1.writeDiscrete(true, modOutput, suction);
      delay(500);
      currentState = DropOff;
    }

    break;
  case DropOff:

    vertMotor.Home();
    horizMotor.Home();
    while (!Turn(673))
      ;
    while (!Extend(77))
      ;

    while (!P1.readDiscrete(modInput, warehouseAvail))
    {
      delay(1);
    }
  
    while (!Raise(60))
      ;
    P1.writeDiscrete(false, modOutput, suction);
    while (!Raise(10));
    P1.writeDiscrete(true, modOutput, warehouse);
    currentState = Home;


    break;
  case blank:
    break;
  }
}
