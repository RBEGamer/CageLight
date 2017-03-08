# CageLight
A smart light control for pet cages with simple and easy to use WebUI and API.

## See the images below




# FEATURES
* (currently) 2 channels
* Low cost ESP8266 powered
* WebUI
* RTC for automatic mode
* easy to use http-get api

# PARTS
* ESP8266 module (I used the NodeMCU V1.1 board)
* 2 channel relais card
* DS1307 RTC module
* 12V power supply (12V for the led strips, min 5V)
* some wood/plastic odds for mounting the stuff
* luster terminals
* [OPTIONAL] I used a small 5V step down converter for the esp and relays
* [OPTIONAL] the 3d printed case (located at `documentation/cage_light.skp` - Sketchup)
* [OPTIONAL] 2 push buttons for on off switching
* [OPTIONAL] 2 pullup resistors for the 2 buttons
* [IF REQUIRED] CR2032 Battery for the RTC Module
### OPTIONAL STUFF
* a dyndns provider if you have not a static ip adress and you want to access the system from outside of your lan

# TOOLS
* soldering stuff
* hot glue
* wires, ....
* a 3D Printer for the case (optional)


# BUILD

## 3D PRINT
Print the both stl-files (`/documentation/cage_light_case_box.stl` and `/documentation/cage_light_case_top.stl`). Good settings are 0.2mm layerheight,  0.4mm nozzle, no support, no raft, heatplate is recommented
![Gopher image](/documentation/images/cg_box.png)

## HARDWARE
* Open the downloaded sketch (`src/cage_light`) and see the pin config or change it!
* connect the input of the step down converter to the power supply
* connect the output of the step down converter to the `VI` and `GND` of the ESP8266 board
* connect the step down converter to `VCC` and `GND` of the relays board
* connect `3.3V` and `GND` from the ESP8266 board to the power pins of the RTC module
* connect the I2C interface from the RTC module to the in the config set pins of the ESP8266 board
* connect the in the config set pins for the relay outputs of the ESP8266 board to the channel input of the relais


# SOFTWARE SETUP
#### Please note the software is not really clean at this time because it was a one day project and WIP!
* add the esp8266 board url to the additional board in your arduino ide :  http://arduino.esp8266.com/stable/package_esp8266com_index.json
* download the sketch located at `src/cage_light/`
* [ edit the pin config to your connected pins (first lines) ]
* upload the sketch to the ESP8266 board
* show at startup over the serial port the given IP of the board (or check your router)

# IMAGES
### FINAL UNIT WITH NO CASE
![Gopher image](/documentation/images/final_build.jpeg)
### BASIC WEB UI
![Gopher image](/documentation/images/webui.png)

# TODO
See the TODO List located at `/TODO.MD`
