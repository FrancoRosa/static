# Arduino-cli Template for STM32 Modules
> This repo contains snipeds to work with stm32f1 from arduino-cli

## Install board
Create a `arduino-cli.yaml` file with the additional board
```code
board_manager:
  additional_urls:
    - https://github.com/stm32duino/BoardManagerFiles/raw/main/package_stmicroelectronics_index.json

```
Then update the index and install the board support

```code
arduino-cli core update-index
arduino-cli core search stm32
arduino-cli core install STMicroelectronics:stm32
```
Verify if board was installed successfully
```code
arduino-cli core list
```



## Compile
arduino-cli compile --fqbn STMicroelectronics:stm32:genericSTM32F103C6:upload_method=STLinkMethod,cpu_speed=speed_48mhz,opt=osstd stm32 --build-path build  

## Flash board with STLink
```code
st-flash write build/stm32.ino.bin 0x08000000
st-flash reset
```
