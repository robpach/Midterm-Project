#include <Arduino.h>
#include <P1AM.h>
#include <MotorEncoder.h>

int modInput = 1;
int modOutput = 2;

// input pins
int turnSuction = 1;
int turnConvey = 2;
int lbConveyor = 3;
int turnSaw = 4;
int transferSaw = 5;  // limit switch for transfer gripper at saw
int inOven = 6;       // limit switch in oven
int outOven = 7;      // limit switch out of oven
int transferKiln = 8; // limit switch for transfer gripper at kiln
int lbKiln = 9;       // initial kiln light barrier

// output pins
int motorCW = 1;
int motorCCW = 2;
int convey = 3;
int saw = 4;
int retractOven = 5;
int extendOven = 6;
int transferToOven = 7;
int trasferToSaw = 8;
int kilnLamp = 9;
int compressor = 10;
int transferSuction = 11;
int lowerTransfer = 12;
int kilnDoor = 13;
int ejector = 14;

enum ProcessingStates
{
    Home,
    Waiting,
    Heating, // 3 seconds
    Transferring,
    Cutting, // 5 seconds
    Conveying
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

    P1.writeDiscrete(true, modOutput, compressor);
}

// Functions

bool KilnTriggered()
{
    return !P1.readDiscrete(modInput, lbKiln);
}

void loop()
{
    switch (currentState)
    {
    case Home:
        while (!P1.readDiscrete(modInput, outOven))
        {
            P1.writeDiscrete(true, modOutput, extendOven);
            P1.writeDiscrete(false, modOutput, retractOven);
            P1.writeDiscrete(true, modOutput, kilnDoor);
            delay(1);
        }
        P1.writeDiscrete(false, modOutput, extendOven);
        P1.writeDiscrete(false, modOutput, kilnDoor);
        currentState = Waiting;
        break;

    case Waiting:
        if (KilnTriggered())
        {
            currentState = Heating;
        }
        break;
    case Heating:
        P1.writeDiscrete(true, modOutput, kilnLamp);
        P1.writeDiscrete(true, modOutput, kilnDoor);
        while (!P1.readDiscrete(modInput, inOven))
        {
            P1.writeDiscrete(true, modOutput, retractOven);
            P1.writeDiscrete(false, modOutput, extendOven);
            delay(1);
        }
        P1.writeDiscrete(false, modOutput, retractOven);
        P1.writeDiscrete(false, modOutput, kilnDoor);
        delay(3000);
        P1.writeDiscrete(true, modOutput, kilnDoor);
        while (!P1.readDiscrete(modInput, outOven))
        {
            P1.writeDiscrete(true, modOutput, extendOven);
            P1.writeDiscrete(false, modOutput, retractOven);
            delay(1);
        }
        P1.writeDiscrete(false, modOutput, extendOven);
        P1.writeDiscrete(false, modOutput, kilnLamp);
        P1.writeDiscrete(false, modOutput, kilnDoor);
        currentState = Transferring;
        break;
    case Transferring:
        while (!P1.readDiscrete(modInput, transferKiln))
        {
            P1.writeDiscrete(true, modOutput, transferToOven);
            P1.writeDiscrete(false, modOutput, trasferToSaw);
            delay(1);
        }
        P1.writeDiscrete(false, modOutput, transferToOven);
        P1.writeDiscrete(true, modOutput, lowerTransfer);
        delay(500);
        P1.writeDiscrete(true, modOutput, transferSuction);
        delay(500);
        P1.writeDiscrete(false, modOutput, lowerTransfer);
        while (!P1.readDiscrete(modInput, transferSaw))
        {
            P1.writeDiscrete(true, modOutput, trasferToSaw);
            P1.writeDiscrete(false, modOutput, transferToOven);
            delay(1);
        }
        P1.writeDiscrete(false, modOutput, trasferToSaw);
        while (!P1.readDiscrete(modInput, turnSuction))
        {
            P1.writeDiscrete(true, modOutput, motorCCW);
            P1.writeDiscrete(false, modOutput, motorCW);
            delay(1);
        }
        P1.writeDiscrete(false, modOutput, motorCCW);
        P1.writeDiscrete(false, modOutput, transferSuction);
        delay(500);
        currentState = Cutting;
        // make sure saw is in right position
        break;
    case Cutting:
        while (!P1.readDiscrete(modInput, turnSaw))
        {
            P1.writeDiscrete(true, modOutput, motorCW);
            P1.writeDiscrete(false, modOutput, motorCCW);
            delay(1);
        }
        P1.writeDiscrete(false, modOutput, motorCW);
        P1.writeDiscrete(true, modOutput, saw);
        delay(5000);
        P1.writeDiscrete(false, modOutput, saw);
        while (!P1.readDiscrete(modInput, turnConvey))
        {
            P1.writeDiscrete(true, modOutput, motorCW);
            P1.writeDiscrete(false, modOutput, motorCCW);
            delay(1);
        }
        P1.writeDiscrete(false, modOutput, motorCW);
        P1.writeDiscrete(true, modOutput, ejector);
        delay(1000);
        P1.writeDiscrete(false, modOutput, ejector);
        currentState = Conveying;
        break;
    case Conveying:
        P1.writeDiscrete(true, modOutput, convey);
        delay(3000);
        P1.writeDiscrete(false, modOutput, convey);
        currentState = Home;
        break;
    }
}