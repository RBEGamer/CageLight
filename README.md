# CageLight
A smart light control for pet cages with simple and easy to use WebUI.


#### Please not the software is not really clean at this time because it was a one day project and WIP!


# FEATURES
* (currently) 2 channels
* Low cost ESP8266 powered
* WebUI
* RTC for automatic mode


# PARTS
* ESP8266 module (I used the NodeMCU V1 board with usb and power converter)
* 2 channel relais card
* DS1307 RTC module with backup battery 
* 12V power supply (12V for the led strips)
* some wood/plastic odds for mounting the stuff
* luster terminals

### OPTIONAL STUFF
* a dyndns provider if you have not a static ip adress and you want to access the system from outside of you lan


# TOOLS
* soldering stuff
* hot glue
* wires, ....



# BUILD
* TODO ADD

# SOFTWARE SETUP
* add the esp8266 board url to the additional board in your arduino ide :  http://arduino.esp8266.com/stable/package_esp8266com_index.json
* download the sketch located atd `src/cage_light/`
* [ edit the pin config to your connected pins (first lines) ]
* upload the sketch to the ESP8266 board
* show at startu over the serial port the given IP of the board

# IMAGES
### FINAL UNIT WITH NO CASE
![Gopher image](/documentation/images/final_build.jpeg)
