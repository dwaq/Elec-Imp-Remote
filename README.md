# Elec-Imp-Remote
Project to control [Elec-Imp-Relay](https://github.com/dwaq/Elec-Imp-Relay) from [32F429IDISCOVERY board](http://www.st.com/web/catalog/tools/FM116/SC959/SS1532/LN1848/PF259090).

This code is a work in progress.

####Peripherals
* Uses LCD to display information
* Uses USART to talk to ESP8266 module to connect to Wi-Fi
* Uses the MEMS sensor to detect motion and turn the LCB backlight on accordingly

####Modifications to 32F429IDISCOVERY board
1. Remove resistors R47, R48, R49, R50
2. Add MOSFET circuit (to be detailed later) to pin PA5
3. Close connection points SB11 and SB15 

####How to build
Built on top of the Touch_Panel example project from [ST](http://www.st.com/web/en/catalog/tools/PF259429). The majority of that code is copywritten to prohibit from sharing, so I can only share my own file. Down the road, I can write instructions on how to compile it using Keil uVision and ST's library files.
