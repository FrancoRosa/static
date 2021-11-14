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

volatile float sensor_1 = 0;
volatile float sensor_2 = 0;

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

    Serial3.begin(9600);
    Serial.begin(115200);
    Serial3.println("...Start");
    Serial.println("...Start");

    scale1.set_scale(8800);
    scale1.tare();
    scale2.set_gain(32);
    scale2.set_scale(2200);
    scale2.tare();
}

void loop()
{
    // readScales();
    digitalWrite(ledpin, !digitalRead(ledpin));
    delay(200);
}

void readScales()
{
    sensor_1 = scale2.get_units(2);
    sensor_2 = scale1.get_units(2);
    Serial3.println(sensor_1);
    Serial3.println(sensor_2);
}
