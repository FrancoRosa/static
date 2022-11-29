#include "src/HX711/HX711.h"

// Leds
#define l_pin PC13
#define l_man PB14
#define l_mov PB13

// buttons
#define b_man PA2
#define b_up PA1
#define b_down PA0

// end of course
#define s_bottom PA15
#define s_top PB4
#define s_origin PB12

// motor controller
#define m_up PA8
#define m_down PA9
#define m_en PA10

// Scale
#define sda PB0
#define sck PB1

const int buffersize = 40;
volatile char cbuffer;
char buffer[buffersize];

volatile int left_counter, right_counter = 0;
const int left_limit = 1000;
const int right_limit = 1000;
volatile int conf_t_total = 60;
volatile int conf_t_eff = 5;
volatile int conf_t_rest = 5;
volatile int conf_effort = 7;
volatile bool conf_alt = false;

volatile int t_total = 0;
volatile int t_eff = 0;
volatile int t_rest = 0;
volatile float effort = 0;

// counters
volatile int disp_count = 0; // each 500ms to show values on display
volatile bool disp_flag = false;

volatile int ms_count = 0;
volatile bool dir_flag = false;

enum state_t
{
    st_idle,
    st_running,
    st_wait,
    st_origin,
    st_tare,
    st_right,
    st_left
};

enum running_t
{
    run_effort,
    run_rest,
};

enum origin_t
{
    going_up,
    going_mid,
};

enum command_t
{
    cm_reset,
    cm_start,
    cm_pause,
    cm_config,
    cm_right,
    cm_left,
    cm_zero,
    cm_exit,
    cm_none,
    cm_tare
};

command_t command;
origin_t origin;
state_t state = st_origin;
running_t running;

HX711 scale(sda, sck);

void gpio_config()
{
    pinMode(l_pin, OUTPUT);
    pinMode(l_man, OUTPUT);
    pinMode(l_mov, OUTPUT);
    pinMode(m_down, OUTPUT);
    pinMode(m_up, OUTPUT);
    pinMode(m_en, OUTPUT);

    pinMode(b_man, INPUT_PULLUP);
    pinMode(b_up, INPUT_PULLUP);
    pinMode(b_down, INPUT_PULLUP);
    pinMode(s_bottom, INPUT_PULLUP);
    pinMode(s_top, INPUT_PULLUP);
    pinMode(s_origin, INPUT_PULLUP);
}

void interrupt_config()
{
    Timer2.setChannel1Mode(TIMER_OUTPUTCOMPARE);
    Timer2.setPeriod(1000); // in microseconds
    Timer2.setCompare1(1);  // overflow might be small
    Timer2.attachCompare1Interrupt(one_ms);
}

void serial_config()
{
    Serial3.begin(9600);
    Serial.begin(19200);
    Serial3.println("... start");
    Serial.println("... start");
}

void scale_config()
{
    scale.begin(sda, sck, 128);
    scale.set_scale(8800);
    scale.tare();
}

int sensor_active(int pin)
{
    return !digitalRead(pin);
}

void setup()
{
    gpio_config();
    interrupt_config();
    serial_config();
    disable_motor();
    scale_config();
}

void read_commands()
{
    while (Serial3.available())
    {
        processingRemoteData(Serial3.read());
    }
    while (Serial.available())
    {
        processingRemoteData(Serial.read());
    }
}

void tic()
{
    digitalWrite(l_pin, !digitalRead(l_pin));
}

void loop()
{
    tic();
    read_commands();
    read_scales();
    display_values();
    if (disp_flag)
    {
        disp_flag = 0;
    }
}

void read_scales()
{
    effort = scale.get_units(5);
}

void display_values()
{
    Serial3.print(t_total);
    Serial3.print(" ");
    Serial3.print(t_eff);
    Serial3.print(" ");
    Serial3.print(t_rest);
    Serial3.print(" ");
    Serial3.print(effort, 1);
    Serial3.print(" ");
    if ((state == st_running) && (running == run_effort))
    {
        if (dir_flag)
            Serial3.print(">>>"); // alternancy
        else
            Serial3.print("<<<"); //! alternancy
    }
    else
    {

        Serial3.print("---");
    }
    Serial3.println();

    Serial.print(effort);
    Serial.print(", ");
    switch (state)
    {
    case 0:
        Serial.print("st_idle");
        break;
    case 1:
        Serial.print("st_running");
        break;
    case 2:
        Serial.print("st_wait");
        break;
    case 3:
        Serial.print("st_origin");
        break;
    case 4:
        Serial.print("st_tare");
        break;
    case 5:
        Serial.print("st_right");
        break;
    case 6:
        Serial.print("st_left");
        break;
    default:
        break;
    }
    Serial.print(", ");

    switch (running)
    {
    case 0:
        Serial.print("run_effort");
        break;
    case 1:
        Serial.print("run_rest");
        break;
    default:
        break;
    }
    Serial.print(", total: ");
    Serial.print(t_total);
    Serial.print(", eff:");
    Serial.print(t_eff);
    Serial.print(", rest:");
    Serial.print(t_rest);
    Serial.println("");
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
    const char reset_str[] = "reset";
    const char start_str[] = "start";
    const char pause_str[] = "pause";
    const char config_str[] = "config";
    const char right_str[] = "right";
    const char left_str[] = "left";
    const char zero_str[] = "zero";
    const char exit_str[] = "exit";
    const char tare_str[] = "tare";

    if (findCommand(start_str))
    {
        command = cm_start;
    }
    if (findCommand(pause_str))
    {
        command = cm_pause;
    }
    if (findCommand(reset_str))
    {
        command = cm_reset;
    }
    if (findCommand(config_str))
    {
        command = cm_config;
    }
    if (findCommand(zero_str))
    {
        command = cm_zero;
    }
    if (findCommand(left_str))
    {
        command = cm_left;
    }
    if (findCommand(right_str))
    {
        command = cm_right;
    }
    if (findCommand(exit_str))
    {
        command = cm_exit;
    }

    if (findCommand(tare_str))
    {
        command = cm_tare;
    }
    Serial.println();
    Serial.print(">> ");
    switch (command)
    {
    case cm_reset:
        Serial.println("reset");
        {
            running = run_effort;
            t_total = 0;
            t_eff = 0;
            t_rest = 0;
        }
        state = st_origin;
        break;
    case cm_start:
        Serial.println("start");
        if (state != st_wait)
        {
            running = run_effort;
            t_total = 0;
            t_eff = 0;
            t_rest = 0;
        }
        state = st_running;
        break;
    case cm_pause:
        Serial.println("pause");
        if (state == st_running)
        {
            state = st_wait;
        }
        break;
    case cm_config:
        Serial.println("config");
        getConfig();
        break;
    case cm_right:
        state = st_right;
        right_counter = 0;
        Serial.println("right");
        break;
    case cm_left:
        state = st_left;
        left_counter = 0;
        Serial.println("left");
        break;
    case cm_zero:
        state = st_origin;
        origin = going_up;
        Serial.println("zero");
        break;
    case cm_exit:
        Serial.println("exit");
        break;
    case cm_tare:
        Serial.println("tare");
        scale_config();
        break;
    default:
        break;
    }
    command = cm_none;
}

bool findCommand(void const *commandString)
{
    return (memcmp(buffer, commandString, 2) == 0) ? true : false;
}

void getConfig()
{
    // config: 0065, 0078, 8184, 0987, true
    //         Ttot  Tesf  Tdes  Esf   alt
    // 01234567890123456789012345678901234
    conf_t_total = string2int(8, 4);
    conf_t_eff = string2int(14, 4);
    conf_t_rest = string2int(20, 4);
    conf_effort = string2int(26, 4);
    conf_alt = buffer[32] == 't' ? true : false;

    Serial.println("times: ");
    Serial.print("       total: ");
    Serial.println(conf_t_total);
    Serial.print("       effort: ");
    Serial.println(conf_t_eff);
    Serial.print("       rest: ");
    Serial.println(conf_t_rest);
    Serial.print("effort:  ");
    Serial.println(conf_effort);
    Serial.print("alternant:  ");
    Serial.println(conf_alt ? "true" : "false");
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

void disable_motor()
{
    digitalWrite(m_en, LOW);
}

void enable_motor()
{
    digitalWrite(m_en, HIGH);
}

void move_up()
{
    if (!sensor_active(s_top))
    {

        digitalWrite(m_down, LOW);
        digitalWrite(m_up, HIGH);
        enable_motor();
    }
    else
    {
        disable_motor();
    }
}

void move_down()
{
    if (!sensor_active(s_bottom))
    {
        digitalWrite(m_down, HIGH);
        digitalWrite(m_up, LOW);
        enable_motor();
    }
    else
    {
        disable_motor();
    }
}

void one_ms()
{
    switch (state)
    {
    case st_idle:
        disable_motor();
        break;

    case st_origin:
        switch (origin)
        {
        case going_up:
            move_up();
            if (sensor_active(s_top))
            {
                origin = going_mid;
            }
            if (sensor_active(s_origin))
            {
                state = st_idle;
            }
            break;
        case going_mid:
            move_down();
            if (sensor_active(s_origin))
            {
                state = st_idle;
            }
            break;
        default:
            break;
        }
        break;

    case st_right:
        move_up();
        right_counter++;
        if (right_counter > right_limit)
        {
            state = st_idle;
        }
        break;

    case st_left:
        move_down();
        left_counter++;
        if (left_counter > left_limit)
        {
            state = st_idle;
        }
        break;

    case st_running:
        if (t_total < conf_t_total)
        {
            switch (running)
            {
            case run_effort:
                if (t_eff > conf_t_eff)
                {
                    running = run_rest;
                    t_rest = 0;
                    t_eff = 0;
                }
                else
                {

                    if (dir_flag)
                    {
                        if (effort < conf_effort)
                        {
                            move_up();
                        }
                        else
                        {
                            disable_motor();
                        }
                    }
                    else
                    {
                        if (-effort < conf_effort)
                        {
                            move_down();
                        }
                        else
                        {
                            disable_motor();
                        }
                    }
                }
                break;
            case run_rest:
                if (t_rest > conf_t_rest)
                {
                    running = run_effort;
                    t_rest = 0;
                    t_eff = 0;
                    if (conf_alt)
                    {
                        dir_flag = !dir_flag;
                    }
                }
                disable_motor();
                break;
            default:
                break;
            }
        }
        else
        {
            state = st_idle;
        }
        break;

    case st_wait:
        disable_motor();
        break;

    default:
        break;
    }

    ms_count++;
    if (ms_count > 999)
    {
        if (state == st_running)
        {
            t_total++;
            if (running == run_effort)
            {
                t_eff++;
            }
            if (running == run_rest)
            {
                t_rest++;
            }
        }
        ms_count = 0;
    }
    disp_count++;
    if (disp_count > 249)
    {
        disp_flag = 1;
        disp_count = 0;
    }
}