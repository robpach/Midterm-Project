#include <Arduino.h>
#include <P1AM.h>
#include <MotorEncoder.h>

int modInput = 1;
int modOutput = 2;

// input pins
int horizRef = 1;
int lbIn = 2;
int lbOut = 3;
int vertRef = 4;
int horizPulse = 5;
int vertPulse = 8;
int armFront = 9;
int armRear = 10;
int robotIn = 11;

// output pins
int conveyForward = 1;
int conveyBack = 2;
int moveLeft = 3;
int moveRight = 4;
int motorDown = 5;
int motorUp = 6;
int moveForward = 7;
int moveBackward = 8;
int robotOut = 13;

// position array
int c1 = 956;
int c2 = 662;
int c3 = 365;
int r1 = 70;
int r2 = 420;
int r3 = 820;
int xPositions[] = {c1, c2, c3, c1, c2, c3, c1, c2, c3};
int yPositions[] = {r1, r1, r1, r2, r2, r2, r3, r3, r3};
int position = 0;
int offset = 50;

MotorEncoder vertMotor(modInput, modOutput, motorDown, motorUp, vertPulse, vertRef);
MotorEncoder horizMotor(modInput, modOutput, moveLeft, moveRight, horizPulse, horizRef);
MotorEncoder armMotor(modInput, modOutput, moveForward, moveBackward, armFront, armRear);

enum ProcessingStates
{
  Home,
  Waiting,
  GetEmpty,
  PlaceEmpty,
  PickupFull,
  DropOff,
};

ProcessingStates currentState = Home;

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
}

// Functions

bool Lower(int d)
{
  return vertMotor.MoveTo(d);
}

bool Extend(int d)
{
  return horizMotor.MoveTo(d);
}

bool Arm(char p)
{
  // if front else if back
  // return false until position reached
  if (p == 'f')
  {
    while (!P1.readDiscrete(modInput, armFront))
    {
      P1.writeDiscrete(true, modOutput, moveForward);
      P1.writeDiscrete(false, modOutput, moveBackward);
      delay(1);
    }
    P1.writeDiscrete(false, modOutput, moveForward);
    return true;
  }
  else if (p == 'b')
  {
    while (!P1.readDiscrete(modInput, armRear))
    {
      P1.writeDiscrete(true, modOutput, moveBackward);
      P1.writeDiscrete(false, modOutput, moveForward);
      delay(1);
    }
    P1.writeDiscrete(false, modOutput, moveBackward);
    return true;
  }
}

void loop()
{
  switch (currentState)
  {
  case Home:
    armMotor.Home();
    vertMotor.Home();
    horizMotor.Home();
    currentState = Waiting;
    break;
  case Waiting:

    if (P1.readDiscrete(modInput, lbIn) && P1.readDiscrete(modInput, lbOut))
    {
      currentState = GetEmpty;
      P1.writeDiscrete(false, modOutput, robotOut);
    }
    // if there is a vox at lbOut, case = PickupFull
    // this means the robot output pin must be changed
    break;
  case GetEmpty:
    // create position arrays of x and y positions for each bay, then use counter i to iterate through
    while (!Lower(yPositions[position]))
      ;
    while (!Extend(xPositions[position]))
      ;
    while (!Arm('f'))
      ;
    while (!Lower(yPositions[position] - offset))
      ;
    while (!Arm('b'))
      ;
    horizMotor.Home();
    while (!Lower(670))
      ;
    while (!Arm('f'))
      ;
    while (!Lower(680))
      ;
    while (!P1.readDiscrete(modInput, lbIn))
    {
      P1.writeDiscrete(true, modOutput, conveyForward);
      delay(1);
    }
    while (P1.readDiscrete(modInput, lbOut))
    {
      delay(1);
    }
    delay(250);
    P1.writeDiscrete(false, modOutput, conveyForward);
    P1.writeDiscrete(true, modOutput, robotOut);
    while (!P1.readDiscrete(modInput, robotIn))
    {
      delay(1);
    }
    currentState = PickupFull;
    break;
  case PlaceEmpty:
    // yposition[position] + 10 ish then retract arm, go to extend value to place on belt
    break;
  case PickupFull:
    P1.writeDiscrete(false, modOutput, robotOut);
    while (!Lower(680))
      ;
    while (!Arm('f'))
      ;

    while (P1.readDiscrete(modInput, lbIn))
    {
      P1.writeDiscrete(true, modOutput, conveyBack);
      delay(1);
    }
    delay(250);
    P1.writeDiscrete(false, modOutput, conveyBack);
    while (!Lower(660))
      ;
    while (!Arm('b'))
      ;
    while (!Lower(yPositions[position] - offset))
      ;
    horizMotor.Home();
    while (!Extend(xPositions[position]))
      ;
    while (!Arm('f'))
      ;
    while (!Lower(yPositions[position]))
      ;
    while (!Arm('b'))
      ;
    // there are 9 positions so it should reset to 0 after 8
    position = (position + 1) % 9;
    currentState = Home;
    break;
  case DropOff:
    break;
  }
}