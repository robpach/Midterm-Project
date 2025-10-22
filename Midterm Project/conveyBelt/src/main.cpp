#include <Arduino.h>
#include <P1AM.h>

enum MachineStates
{
  Waiting,
  ColorSensing,
  CountedMove,
  ejectState
};

MachineStates curState = Waiting;

// Modules
int modInput = 1;
int modOutput = 2;
int modAnalog = 3;
int detectW = 4;
int detectR = 5;
int detectB = 6;

// Inputs
int pulse = 1;
int lbIn = 2;
int lbOut = 3;

// Outputs
int conv = 1;
int compressor = 2;
int ejectW = 3;
int ejectR = 4;
int ejectB = 5;
int whiteAvailable = 6;
int redAvailable = 7;
int blueAvailable = 8;

// Analog Inputs
int color = 1;

// Vars
int colorValue = 10000;
int distToEject = 0;
bool prevKeyState = false;
int distMoved = 0;
bool curKey = false;
char targetColor = 'b';
char w = 'f';
char r = 'f';
char b = 'f';

void setup()
{

  delay(1000);
  Serial.begin(9600);
  delay(1000);

  // Start up P1AM modules!
  while (!P1.init())
  {
    delay(1);
  }
}

bool InputTriggered()
{
  return !P1.readDiscrete(modInput, lbIn);
}

bool OuputTriggered()
{
  return !P1.readDiscrete(modInput, lbOut);
}

void ToggleConveyor(bool s)
{
  P1.writeDiscrete(s, modOutput, conv);
}

int GetColor()
{
  return P1.readAnalog(modAnalog, color);
}

bool GetPulseKey()
{
  return P1.readDiscrete(modInput, pulse);
}

void ToggleCompressor(bool s)
{
  P1.writeDiscrete(s, modOutput, compressor);
}

void UseEjector(char c)
{
  int tempPin;
  if (c == 'w')
  {
    tempPin = ejectW;
  }
  else if (c == 'r')
  {
    tempPin = ejectR;
  }
  else
  {
    tempPin = ejectB;
  }
  P1.writeDiscrete(true, modOutput, tempPin);
  delay(1500);
  P1.writeDiscrete(false, modOutput, tempPin);
}

void loop()
{
  switch (curState)
  {
  case Waiting:
    // wait for light barrier to be tripped
    // After tripped, switch state and turn on conveyor
    if (InputTriggered())
    {
      curState = ColorSensing;
      ToggleConveyor(true);
      colorValue = 10000;
      ToggleCompressor(true);
    }

    break;
  case ColorSensing:
    // get color and find min
    colorValue = min(GetColor(), colorValue);
    // keep on going until second light barrier
    // Then switch states
    if (OuputTriggered())
    {
      distMoved = 0;
      curState = CountedMove;
      // Decide how far to move
      if (colorValue < 2500)
      {
        distToEject = 3;
        targetColor = 'w';
      }
      else if (colorValue < 5000)
      {
        distToEject = 9;
        targetColor = 'r';
      }
      else
      {
        distToEject = 14;
        targetColor = 'b';
      }
    }
    break;
  case CountedMove:
    // Watch pulse key
    // Switch States
    curKey = GetPulseKey();
    if (curKey && !prevKeyState)
    {
      distMoved++;
    }
    prevKeyState = curKey;
    // switch states and turn of conveyor
    if (distMoved >= distToEject)
    {
      curState = ejectState;
      ToggleConveyor(false);
    }
    break;
  case ejectState:
    UseEjector(targetColor);
    curState = Waiting;
    break;
  default:
    break;
  }
  if (P1.readDiscrete(modInput, detectW) == false)
  {
    P1.writeDiscrete(true, modOutput, whiteAvailable);
    w = 't';
  }
  else
  {
    P1.writeDiscrete(false, modOutput, whiteAvailable);
    w = 'f';
  }

  if (P1.readDiscrete(modInput, detectR) == false)
  {
    P1.writeDiscrete(true, modOutput, redAvailable);
    r = 't';
  }
  else
  {
    P1.writeDiscrete(false, modOutput, redAvailable);
    r = 'f';
  }

  if (P1.readDiscrete(modInput, detectB) == false)
  {
    P1.writeDiscrete(true, modOutput, blueAvailable);
    b = 't';
  }
  else
  {
    P1.writeDiscrete(false, modOutput, blueAvailable);
    b = 'f';
  }

  Serial.print("White: ");
  Serial.print(w);
  Serial.print(" Red: ");
  Serial.print(r);
  Serial.print(" Blue: ");
  Serial.println(b);
}
