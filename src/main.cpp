#include <Arduino.h>
#include "RadioFileTransfer.h"

#define _TRANSMITER 1
//#define _RECEIVER 1

#define TIME_IN_DEEP_SLEEP_MODE  5            /* Time ESP32 will go to sleep (in seconds) */

RTC_DATA_ATTR int bootCount = 0;

void setup() {
  
  Serial.begin(115200);
  delay(500);
  Serial.print("\n\r\n\r****************************************************"); //Used to declare a static pointer to a string in program space.
  Serial.print("\n\r         IoT TRACTIAN System V0.89 PLATFORMIO IDE ");
  Serial.print("\n\r****************************************************\n\r");
  // This is the mac address of the Master in Station Mode
  Serial.print("DEVICE MAC: "); Serial.println(WiFi.macAddress());

  uint8_t systemStatus = 0;
  
  #if _TRANSMITER == 1
    Serial.print("\n\rTRANSMITER MODE!\n\r");
    systemStatus = tx_file.init(0x94, 0xB9, 0x7E, 0xFA, 0xCF, 0x74);
  #elif _RECEIVER == 1
    Serial.print("\n\rRECEIVER MODE!\n\r");
    systemStatus = tx_file.init(0x94, 0xB9, 0x7E, 0xF9, 0x95, 0x90);
  #endif
  
  if(systemStatus){
    Serial.print("INIT Completed\n\r");
  }else{
    Serial.print("FATAL ERROR: Fail Init, restarting ESP32.....\n\r");
    delay(5000);
    ESP.restart();
  }
  
  #ifdef _TRANSMITER
    //Setup the ESP32 to wake up every 5 seconds
    esp_sleep_enable_timer_wakeup(TIME_IN_DEEP_SLEEP_MODE * 1000000ULL);
    tx_file.sendFile(SD, "/test.txt");
    Serial.println("Going to sleep now");
    esp_deep_sleep_start();
  #endif

}

void loop() {
}