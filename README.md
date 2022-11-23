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

