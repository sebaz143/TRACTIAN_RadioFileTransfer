# FEATURES

This proyect has a firmware for full wireless communication system based on the ESP32-wroom-32u MCU. The system consist of two IoT devices. One has a 500KB file, this devices should be configured as a TRANSMITER and It has to send that file to a RECEIVER located 100 m away with LOS.

The TRANSMITER is powered by battery and it has low power features.


# PREREQUISITES
Before proceeding with this project, make sure you have installed the ESP32 add-on Arduino IDE, We’ll program the ESP32 using Arduino IDE, so before proceeding with this tutorial you should have the ESP32 add-on installed in your Arduino IDE.

##GETTING THE BOARDS MAC ADDRESS
To send messages between each board, we need to know their MAC address. Each board has a unique MAC address. Upload the following code to each of your boards to get their MAC address.

```cpp
#include "WiFi.h"
 
void setup(){
  Serial.begin(115200);
  WiFi.mode(WIFI_MODE_STA);
  Serial.println(WiFi.macAddress());
}
 
void loop(){
}
```
After uploading the code, press the RST button, and the MAC address should be displayed on the Serial Monitor.

#SET UP THE TRANSMITER DEVICE
Open the firmware with PLATFORM-IO. In the platform.ini file select the COM port of the transmiter device.

```
[env:esp32dev]
platform = espressif32
board = esp32dev
upload_port = COM4
framework = arduino
```

Then, in the main.cpp file located in the src folder, you have to choose the TRANSMITER mode by uncommenting the #define:

```cpp
#include "RadioFileTransfer.h"

#define _TRANSMITER 1
//#define _RECEIVER 1
...

```

If you want to see the output in a serial interface uncomment the DEBUG_FILE_TRANSFER flag in the RadioFileTransfer.h header file:

```cpp
#ifndef _RADIO_FILE_TRANSFER_H_INCLUDED
#define _RADIO_FILE_TRANSFER_H_INCLUDED

#define _DEBUG_FILE_TRANSFER
	....
```
The serial port is configured to 115200 bauds.

Before power on the transmiter device, you have to insert a SD memory with a file on it named test.txt with the content you want to trasmit to the receiver. Then place the SD memory on the SD card holder. 


#SET UP THE RECEIVER DEVICE

For the RECEIVER, use the same firmware, but this time you have to set up the RECEIVER flag in the main.cpp file:

```cpp
#include "RadioFileTransfer.h"

//#define _TRANSMITER 1
#define _RECEIVER 1
...

```

Also you can see the ouput in a serial terminal by uncommenting the DEBUG_FILE_TRANSFER flag in the RadioFileTransfer.h header file.

Before power on the device, remember to insert an SD card to save the incomming data.

The receiver devices acts like a gateway. The receiver is listening all the time for incomming packets. This devices should not be powered by battery.

