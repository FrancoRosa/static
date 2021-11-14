/*
Arduino_STM32-master
STM32F1
Generic STM32F103C
48Mhz
Smallest
STLink
STM32F103C8

Bluetooth MAC 20:16:07:19:32:00
Bluetooth Commands:
    From APP:
        reset
        start
        config: 0065, 0078, 8184, 0987, true
        config: 0065, 0078, 8184, 0987, false
        // config: 0065,   0078,      8184,       0987, true
        //         TTotal  TEsfuerzo  TDescanso   Esf   alternancia
    
    From uC
        1 2 3 4
        // 1     2     3    4
        // TTotal TEsfuerzo TDescanso Esfuerzo
*/

/*
1.- On boot or "reset"
        Move left to start position
        returntohome()
        gotomedium()

2.- Read all config parameters
           config: 0065, 0078, 8184, 0987, false
        // config: 0065,   0078,      8184,       0987, true
        //         TTotal  TEsfuerzo  TDescanso   Esf   alternancia

3.- All time send excercise report
        print counteers and sensor values
        1- Alternancy mode
        2- One direction mode



*/

#include "src/HX711/HX711.h"

// Leds
#define ledpin PC13
#define ledaut PB14
#define ledsts PB13

// Buttons
#define bpause PB12
#define bman PA2
#define bup PA1
#define bdwn PA0

// EndOfCourse
#define fcLeft PA15
#define fcRight PB4

// StepperController
#define stpul PA8
#define stdir PA9
#define sten PA10

// Scale
#define sda PB0
#define sck PB1

const int buffersize = 40;
volatile char cbuffer;
char buffer[buffersize];

const char resetCommand[] = "reset";
const char startCommand[] = "start";
const char pauseCommand[] = "pause";
const char configCommand[] = "config";
const char rightCommand[] = "right";
const char leftCommand[] = "left";
const char zeroCommand[] = "zero";
const char exitCommand[] = "exit";

volatile bool resetFlag = false;
volatile bool startFlag = false;
volatile bool efforFlag = false;
volatile bool configFlag = false;
volatile bool rightFlag = false;
volatile bool leftFlag = false;
volatile bool zeroFlag = false;
volatile bool exitFlag = false;
volatile bool flagCommand = false;
volatile bool flagMoveLeft = false;
volatile bool flagMoveRight = false;

volatile int confTTotal = 60;
volatile int confTEsfz = 5;
volatile int confTDesc = 5;
volatile int confEsfz = 7;
volatile bool confAlt = false;

volatile int curntTTotal = 0;
volatile int curntTEsfz = 0;
volatile int curntTDesc = 0;
volatile bool curntAlt = false;

// ms counters
volatile int dispCount = 0; //each 500ms to show values on display
volatile int stepSpeed = 5; //each speed of step
volatile int secCount = 0;  //each 1000ms to seconds in counter
volatile bool dispFlag = false;

volatile float curntEsfz = 0;

volatile int stepCounter = 0;
volatile int uCounter = 0;
// Values to be modified by user
const int stepsmedium = 5500;
//const int stepsmedium = 0;
const int confVel = 10;

HX711 scale1(sda, sck);
HX711 scale2(sda, sck);

void setup()
{
    pinMode(ledpin, OUTPUT);
    pinMode(ledaut, OUTPUT);
    pinMode(ledsts, OUTPUT);
    pinMode(stdir, OUTPUT);
    pinMode(stpul, OUTPUT);
    pinMode(sten, OUTPUT);

    pinMode(bpause, INPUT_PULLUP);
    pinMode(bman, INPUT_PULLUP);
    pinMode(bup, INPUT_PULLUP);
    pinMode(bdwn, INPUT_PULLUP);
    pinMode(fcLeft, INPUT_PULLUP);
    pinMode(fcRight, INPUT_PULLUP);

    Timer2.setChannel1Mode(TIMER_OUTPUTCOMPARE);
    Timer2.setPeriod(1000); // in microseconds
    Timer2.setCompare1(1);  // overflow might be small
    Timer2.attachCompare1Interrupt(one_ms);

    Serial3.begin(9600);
    Serial.begin(19200);
    Serial3.println("...Start");
    Serial.println("...Start");

    disableStep();

    scale1.set_scale(8800);
    scale1.tare();
    scale2.set_gain(32);
    scale2.set_scale(2200);
    scale2.tare();

    // returntohome();
    // gotomedium();
}

void loop()
{

    digitalWrite(ledpin, !digitalRead(ledpin));
    while (Serial3.available())
    {
        processingRemoteData(Serial3.read());
    }
    readScales();
    if (dispFlag)
    {
        displayValues();
        dispFlag = false;
    }
}

void readinputs()
{
    Serial3.print("fcLeft:");
    Serial3.print(digitalRead(fcLeft));
    Serial3.print(" ");
    Serial3.print("fcRight:");
    Serial3.print(digitalRead(fcRight));
    Serial3.print(" ");
    Serial3.print("bpause:");
    Serial3.print(digitalRead(bpause));
    Serial3.print(" ");
    Serial3.print("stepCounter:");
    Serial3.print(stepCounter);
    Serial3.print(" ");
    Serial3.println();
}

void readScales()
{
    if (curntAlt)
        curntEsfz = scale2.get_units(2);
    else
        curntEsfz = scale1.get_units(2);
}

void displayValues()
{
    Serial3.print(curntTTotal);
    Serial3.print(" ");
    Serial3.print(curntTEsfz);
    Serial3.print(" ");
    Serial3.print(curntTDesc);
    Serial3.print(" ");
    Serial3.print(curntEsfz, 1);
    Serial3.print(" ");
    if (efforFlag)
    {
        if (curntAlt)
            Serial3.print(">>>"); //curntAlt
        else
            Serial3.print("<<<"); //!curntAlt
    }
    else
        Serial3.print("---");
    Serial3.println();
}

void processingRemoteData(char c)
{
    Serial.print(c);
    buffer[cbuffer] = c;
    cbuffer++;
    if ((c == '\n') || (c == '\r'))
    {
        if (cbuffer > 2)
        {
            comparator();
        }
        cbuffer = 0;
    }
    if (cbuffer >= buffersize)
        cbuffer = 0;
}

void comparator(void)
{
    Serial.println(" ");
    if (findCommand(startCommand))
    {
        startFlag = true;
        efforFlag = true;
        Serial.println(">> startFlag");
    }
    if (memcmp(buffer, pauseCommand, 2) == 0)
    {
        startFlag = false;
        Serial.println(">> pauseFlag");
    }
    if (memcmp(buffer, resetCommand, 2) == 0)
    {
        resetFlag = true;
        Serial.println(">> resetFlag");
    }
    if (memcmp(buffer, configCommand, 2) == 0)
    {
        configFlag = true;
        Serial.println(">> configFlag");
        getConfig();
    }
    if (memcmp(buffer, zeroCommand, 2) == 0)
    {
        zeroFlag = true;
        Serial.println(">> zeroFlag");
    }
    if (memcmp(buffer, leftCommand, 2) == 0)
    {
        leftFlag = true;
        Serial.println(">> leftFlag");
    }
    if (memcmp(buffer, rightCommand, 2) == 0)
    {
        rightFlag = true;
        Serial.println(">> rightFlag");
    }
    if (memcmp(buffer, exitCommand, 2) == 0)
    {
        exitFlag = true;
        Serial.println(">> exitFlag");
    }
    if (resetFlag)
    {
        resetFlag = false;
        curntTDesc = 0;
        curntTEsfz = 0;
        curntTTotal = 0;
        curntAlt = false;
        efforFlag = false;
        stepSpeed = 5;
        gotomedium();
    }
}

bool findCommand(void const *commandString)
{
    if (memcmp(buffer, commandString, 2) == 0)
        return true;
    else
        return false;
}
// config: 0065, 0078, 8184, 0987, true
//         Ttot  Tesf  Tdes  Esf   alt
// 01234567890123456789012345678901234

void getConfig()
{
    confTTotal = string2int(8, 4);
    confTEsfz = string2int(14, 4);
    confTDesc = string2int(20, 4);
    confEsfz = string2int(26, 4);
    confAlt = buffer[32] == 't' ? true : false;

    Serial.println("Results:");
    Serial.print("TTot: ");
    Serial.println(confTTotal);
    Serial.print("TEsf: ");
    Serial.println(confTEsfz);
    Serial.print("TDes: ");
    Serial.println(confTDesc);
    Serial.print("Esfz: ");
    Serial.println(confEsfz);
    Serial.print("Alt:  ");
    Serial.println(confAlt);
}

int string2int(int start, int lenght)
{
    int value = 0;
    for (int i = 0; i < lenght; i++)
    {
        value = value * 10 + buffer[start + i] - 48;
    }
    return value;
}

/*
void moveLeft( int sleep){
    if(digitalRead(fcLeft) && digitalRead(bpause)) {
        stepCounter--;
        dirLeft();
        enableStep();
        step(sleep);
    }
    else
        disableStep();
}

void moveRight( int sleep){
    if(digitalRead(fcRight) && digitalRead(bpause)) {
        stepCounter++;
        dirRight();
        enableStep();
        step(sleep);   
    }
    else
        disableStep();
}

void step(int sleep){
    digitalWrite(stpul, HIGH); delay(sleep);
    digitalWrite(stpul, LOW);  //delayMicroseconds(100);
}
*/

void disableStep()
{
    digitalWrite(sten, LOW);
}

void enableStep()
{
    digitalWrite(sten, HIGH);
}

void dirLeft()
{
    digitalWrite(stdir, HIGH);
}

void dirRight()
{
    digitalWrite(stdir, LOW);
}

void returntohome()
{
    stepSpeed = 5;
    //Serial3.println(">>> returntohome");
    while (true)
    {
        flagMoveLeft = true;
        delay(250);
        //readinputs();
        if (!digitalRead(fcLeft))
        {
            break;
        }
    }
    flagMoveLeft = false;
    stepCounter = 0;
    disableStep();
}

void gotomedium()
{
    stepSpeed = 5;
    //Serial3.println(">>> gotomedium");
    if (stepsmedium > stepCounter)
    {
        while (stepsmedium > stepCounter)
        {
            flagMoveRight = true;
        }
        flagMoveRight = false;
    }

    if (stepCounter > stepsmedium)
    {
        while (stepCounter > stepsmedium)
        {
            flagMoveLeft = true;
        }
        flagMoveLeft = false;
    }
    disableStep();
    //Serial3.println(">>> medium");
}

void one_ms()
{
    // start block //
    // this counter triggers an event every 500ms or 1/2 sec
    dispCount++;
    if (dispCount >= 499)
    {
        dispFlag = true;
        dispCount = 0;
    }
    // end block //

    // start block //
    // this counter triggers an event every 1000ms or 1sec
    secCount++;
    if (secCount >= 999)
    {
        secCount = 0;
        if (startFlag)
        {
            curntTTotal++;
            if (efforFlag)
                curntTEsfz++;
            if (!efforFlag)
                curntTDesc++;
            if (curntTTotal >= confTTotal)
            {
                startFlag = false;
                efforFlag = false;
                curntTTotal = 0;
                curntTEsfz = 0;
                curntTDesc = 0;
                disableStep();
                flagMoveLeft = false;
                flagMoveRight = false;
            }
            if (curntTEsfz >= confTEsfz)
            {
                curntTEsfz = 0;
                efforFlag = false;
            }
            if (curntTDesc >= confTDesc)
            {
                curntTDesc = 0;
                efforFlag = true;
                if (confAlt)
                {
                    curntAlt = !curntAlt;
                }
                else
                {
                    if (curntTTotal > (confTTotal / 2))
                        curntAlt = !curntAlt;
                }
            }
        }
    }
    // end block //

    // start block //
    if (flagMoveLeft)
    {
        if (digitalRead(fcLeft) && digitalRead(bpause))
        {
            uCounter++;
            dirLeft();
            enableStep();
            digitalWrite(stpul, HIGH);
            digitalWrite(ledpin, HIGH);
            if (uCounter > stepSpeed)
            {
                stepCounter--;
                digitalWrite(stpul, LOW);
                digitalWrite(ledpin, LOW);
                uCounter = 0;
            }
        }
        else
        {
            disableStep();
            uCounter = 0;
        }
    }

    if (flagMoveRight)
    {
        if (digitalRead(fcRight) && digitalRead(bpause))
        {
            uCounter++;
            dirRight();
            enableStep();
            digitalWrite(stpul, HIGH);
            digitalWrite(ledpin, HIGH);
            if (uCounter > stepSpeed)
            {
                stepCounter++;
                digitalWrite(stpul, LOW);
                digitalWrite(ledpin, LOW);
                uCounter = 0;
            }
        }
        else
        {
            disableStep();
            uCounter = 0;
        }
    }

    // end block //

    // start block //

    if (efforFlag)
    {
        enableStep();
        stepSpeed = 10;
        if ((int)curntEsfz < confEsfz)
        {
            if (!curntAlt)
            { //<<<<<<<<<
                flagMoveRight = false;
                flagMoveLeft = true;
                digitalWrite(ledsts, HIGH);
                digitalWrite(ledaut, LOW);
            }
            if (curntAlt)
            { //>>>>>>>>>
                flagMoveRight = true;
                flagMoveLeft = false;
                digitalWrite(ledsts, LOW);
                digitalWrite(ledaut, HIGH);
            }
        }
        else
        {
            flagMoveRight = false;
            flagMoveLeft = false;
            digitalWrite(ledaut, LOW);
            digitalWrite(ledsts, LOW);
            disableStep();
        }
    }
    else
    {
        if (startFlag)
        {
            disableStep();
            flagMoveLeft = false;
            flagMoveRight = false;
        }
    }
    // end block //
}