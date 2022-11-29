# Static

> Static load controller

## Process variables

Name|Description|type
-----|-----|-----
t_total|Total time to complete|number
t_effort|time to activate motor|number
t_rest|time to deactivate motor|number
effort|load goal|number
alternant|motor moves in both directions|bool

## Process Status
title|title|title
-----|-----|-----
conte|conte|conteF

## Process Commands
Name|Effect
-----|-----
start|Starts or continues
pause|pauses the Process
reset|Stops current status and goes back to origin
tare|Sets the effort to zero (disabled when running)
zero| moves the motor to origin position (disabled when running)

# 
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