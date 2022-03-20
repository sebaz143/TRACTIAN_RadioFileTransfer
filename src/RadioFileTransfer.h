/*
 * RadioFileTransfer.h
 *
 * Created: 20/03/2022 10:35:29 a. m.
 * Author: Sebastian Arboleda
 */ 

#ifndef _RADIO_FILE_TRANSFER_H_INCLUDED
#define _RADIO_FILE_TRANSFER_H_INCLUDED

#define _DEBUG_FILE_TRANSFER


#define CHUNK_SIZE 20
#define NUM_OF_RETRANSMISION 5

#include "Arduino.h"
#include "FS.h"
#include "SD.h"
#include "SPI.h"

#include <esp_now.h>
#include <WiFi.h>

//Structure  to send data
//Must match the receiver structure
typedef struct payload {
    
    char data_buffer[200];
    uint32_t frame;
    uint32_t packet_size;

} struct_message;

class RadioFileTransfer{

    public:
        // Variable to store if sending data was successful
        uint8_t success = 0;
        uint8_t done_tx = 0;

        // REPLACE WITH THE MAC Address of your Devices
        uint8_t macAddress[6]; 

        // Create a struct_message called BME280Readings to hold sensor readings
        payload sendMsg;

        // Create a struct_message to hold incoming sensor readings
        payload receivedMsg;

        uint8_t init(uint8_t mac1, uint8_t mac2, uint8_t mac3, uint8_t mac4, uint8_t mac5, uint8_t mac6);
        
        uint8_t initESPNow();
        esp_now_peer_info_t peerInfo;

        void sendFile(fs::FS &fs, const char * path);
        uint8_t transmitChunk(const char frame_data[], int frame_Id, int file_size);

        uint8_t initSD();
        void writeFile(fs::FS &fs, const char * path, const char * message);
        void appendFile(fs::FS &fs, const char * path, const char * message);
        void listDir(fs::FS &fs, const char * dirname, uint8_t levels);
};

extern RadioFileTransfer tx_file;
#endif