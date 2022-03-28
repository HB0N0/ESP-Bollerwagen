# ESP Bollerwagen
[![Build](https://github.com/HB0N0/ESP-Bollerwagen/actions/workflows/build_on_commit.yml/badge.svg?branch=master)](https://github.com/HB0N0/ESP-Bollerwagen/actions/workflows/build_on_commit.yml)

ESP8266 controlling [HoverboradFirmwareHack-FOC](https://github.com/HB0N0/HoverBoardFirmwareHack-FOC/) over UART Protocol. Received Data is used to control status led, WS2812B indicator lights, brake lights, ...

The ESP is used as Auxilarity-input additionally to the normal ADC input of the hoverboard module. The input is used to brake the vehicle when an emergency (NC) switch gets triggered. 

