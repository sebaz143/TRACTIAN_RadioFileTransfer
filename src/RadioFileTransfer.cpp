/*
 * File:   RadioFileTransfer.cpp
 * Author: Sebastian Arboleda Duque
 * Processor: ESP32-wroom-32u
 * Program: Source file for application
 * Compiler: Arduino Framework
 * Program Version 0.89
 * Program Description: This file contains source code for transfer a file between two ESP32
 *
 * Modified From: None
 *
 * Change History:
 *
 * Author                Rev     Date          Description
 *
 * Updated on 20/03/2022
 */

#include "RadioFileTransfer.h"

RadioFileTransfer tx_file;
/****************************************************************************************/
/*                               CALLBACK FUNCTIONS                                     */
/****************************************************************************************/
// Callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status){
    //#ifdef _DEBUG_FILE_TRANSFER
    //    Serial.print("\r\nLast Packet Send Status:\t");
    //    Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
    //#endif
    
    if (status == 0){
        tx_file.done_tx = 1;
        tx_file.success = 1;
    } else {
        tx_file.done_tx = 1;
        tx_file.success = 0;
    }
}

// Callback when data is received
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len){

    memcpy(&tx_file.receivedMsg, incomingData, sizeof(tx_file.receivedMsg));
    
    #ifdef _DEBUG_FILE_TRANSFER
        Serial.print("Bytes received: ");
        Serial.println(len);
        Serial.println(tx_file.receivedMsg.data_buffer);
        Serial.println(tx_file.receivedMsg.frame);
        Serial.println(tx_file.receivedMsg.packet_size);
    #endif
    
    if (tx_file.receivedMsg.frame == 0){
        tx_file.writeFile(SD, "/receivedFile.txt", tx_file.receivedMsg.data_buffer);
    }
    else if (tx_file.receivedMsg.frame > 0){
        tx_file.appendFile(SD, "/receivedFile.txt", tx_file.receivedMsg.data_buffer);
    }
}


/****************************************************************************************/
/*                               INIT FUNCTIONS                                         */
/****************************************************************************************/
uint8_t RadioFileTransfer::init(uint8_t mac1, uint8_t mac2, uint8_t mac3, uint8_t mac4, uint8_t mac5, uint8_t mac6){
    macAddress[0] = mac1;
    macAddress[1] = mac2;
    macAddress[2] = mac3;
    macAddress[3] = mac4;
    macAddress[4] = mac5;
    macAddress[5] = mac6;

    if (!initSD())
        return 0;
    if (!initESPNow())
        return 0;

    return 1;
}

uint8_t RadioFileTransfer::initSD(){
    
    if (!SD.begin()){

        #ifdef _DEBUG_FILE_TRANSFER
            Serial.println("Card Mount Failed");
            while (1);
        #endif

        return 0;
    }

    uint8_t cardType = SD.cardType();

    if (cardType == CARD_NONE){
        #ifdef _DEBUG_FILE_TRANSFER
            Serial.println("No SD card attached");
            while (1);
        #endif

        return 0;
    }

    #ifdef _DEBUG_FILE_TRANSFER
        Serial.print("SD Card Type: ");

        if (cardType == CARD_MMC){
            Serial.println("MMC");
        }
        else if (cardType == CARD_SD){
            Serial.println("SDSC");
        }
        else if (cardType == CARD_SDHC){
            Serial.println("SDHC");
        }
        else{
            Serial.println("UNKNOWN");
        }

        uint64_t cardSize = SD.cardSize() / (1024 * 1024);
        Serial.printf("SD Card Size: %lluMB\r\n", cardSize);
        RadioFileTransfer::listDir(SD, "/", 0);
    #endif

    return 1;
}

uint8_t RadioFileTransfer::initESPNow()
{
    // Set device as a Wi-Fi Station
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    if (esp_now_init() == ESP_OK){
        
        #ifdef _DEBUG_FILE_TRANSFER
            Serial.println("ESPNow Init Success");
        #endif

        // Set the send CallBack Function to get the status of Trasnmitted packet
        esp_now_register_send_cb(OnDataSent);

        memcpy(peerInfo.peer_addr, macAddress, 6);
        peerInfo.channel = 0;
        peerInfo.encrypt = false;

        // Add peer
        if (esp_now_add_peer(&peerInfo) != ESP_OK){
            #ifdef _DEBUG_FILE_TRANSFER
                Serial.println("Failed to add peer");
            #endif

            return 0;
        }

        // Register for a callback function that will be called when data is received
        esp_now_register_recv_cb(OnDataRecv);
    }
    else{
        #ifdef _DEBUG_FILE_TRANSFER
            Serial.println("ESPNow Init Failed");
        #endif

        return 0;
    }

    return 1;
}

/****************************************************************************************/
/*                            DATA MANAGMENT & TX DATA                                  */
/****************************************************************************************/

void RadioFileTransfer::sendFile(fs::FS &fs, const char *path){

    #ifdef _DEBUG_FILE_TRANSFER
        Serial.printf("Reading file: %s\r\n", path);
    #endif

    File file = fs.open(path);
    
    if (!file){
        #ifdef _DEBUG_FILE_TRANSFER
            Serial.println("Failed to open file for reading");
        #endif
        
        return;
    }

    int fileSize = file.size();

    int tx_status = 0;
    int chunk = 0;
    int bytesCounter = 0;
    int frameCounter = 0;

    char tmp_buffer[200];
    char singleByte[10];

    sprintf(tmp_buffer, "");

    while (file.available()){

        if (chunk < 50){
            sprintf(singleByte, "%c", file.read());
            strcat(tmp_buffer, singleByte);
            bytesCounter++;
            chunk++;
        } else if (chunk == 50){

            #ifdef _DEBUG_FILE_TRANSFER
                Serial.println(tmp_buffer);
            #endif

            if (!transmitChunk(tmp_buffer, frameCounter, fileSize)){
                return;
            }

            chunk = 0;
            frameCounter++;
            sprintf(tmp_buffer, "");
        }

        if (bytesCounter == fileSize){

            #ifdef _DEBUG_FILE_TRANSFER
                Serial.println(tmp_buffer);
            #endif

            if (!transmitChunk(tmp_buffer, frameCounter, fileSize)){
                return;
            }
            sprintf(tmp_buffer, "");
            frameCounter++;
            chunk = 0;
        }
    }

    file.close();
}

uint8_t RadioFileTransfer::transmitChunk(const char frame_data[], int frame_Id, int file_size)
{

    int ack_window = 0;
    int tx_retransmision = 5;

    // Set values to send
    strcpy(sendMsg.data_buffer, frame_data);
    sendMsg.frame = frame_Id;
    sendMsg.packet_size = file_size;

    while (tx_retransmision-- != 0)
    {

        // Send message via ESP-NOW
        success = 0;
        done_tx = 0;
        ack_window = 0;

        esp_err_t result = esp_now_send(macAddress, (uint8_t *)&sendMsg, sizeof(sendMsg));

        while (!(done_tx && success) && (ack_window < 500))
        {
            ack_window++;
            delay(1);
        }

#ifdef _DEBUG_FILE_TRANSFER
        Serial.print("ack_window: ");
        Serial.println(ack_window, DEC);
#endif

        if ((result == ESP_OK) && (success == 1))
        {
#ifdef _DEBUG_FILE_TRANSFER
            Serial.println("Sent with success");
#endif
            return 1;
        }
        else
        {
#ifdef _DEBUG_FILE_TRANSFER
            Serial.println("Error sending the data");
#endif
        }
    }

#ifdef _DEBUG_FILE_TRANSFER
    Serial.println("FATAL ERROR: Packet TX Fail...");
#endif
    return 0;
}

/****************************************************************************************/
/*                                  SD MANAGMENT                                        */
/****************************************************************************************/

void RadioFileTransfer::writeFile(fs::FS &fs, const char *path, const char *message){

    //#ifdef _DEBUG_FILE_TRANSFER
    //    Serial.printf("Writing file: %s\r\n", path);
    //#endif

    File file = fs.open(path, FILE_WRITE);
    
    if (!file){
        #ifdef _DEBUG_FILE_TRANSFER
            Serial.println("Failed to open file for writing");
        #endif
        
        return;
    }

    if (file.print(message)){
        
        #ifdef _DEBUG_FILE_TRANSFER
            Serial.println("File written");
        #endif
    } else {
        #ifdef _DEBUG_FILE_TRANSFER
            Serial.println("Write failed");
        #endif
    }

    file.close();
}

void RadioFileTransfer::appendFile(fs::FS &fs, const char *path, const char *message){

    //#ifdef _DEBUG_FILE_TRANSFER
    //    Serial.printf("Appending to file: %s\r\n", path);
    //#endif

    File file = fs.open(path, FILE_APPEND);

    if (!file){
        #ifdef _DEBUG_FILE_TRANSFER
            Serial.println("Failed to open file for appending");
        #endif
    
        return;
    }
    
    if (file.print(message)){
        
        #ifdef _DEBUG_FILE_TRANSFER
            Serial.println("Message appended");
        #endif
    } else {
        #ifdef _DEBUG_FILE_TRANSFER
            Serial.println("Append failed");
        #endif
    }

    file.close();
}

void RadioFileTransfer::listDir(fs::FS &fs, const char *dirname, uint8_t levels){

    Serial.printf("Listing directory: %s\r\n", dirname);

    File root = fs.open(dirname);
    
    if (!root){
        Serial.println("Failed to open directory");
        return;
    }

    if (!root.isDirectory()){
        Serial.println("Not a directory");
        return;
    }

    File file = root.openNextFile();
    
    while (file){
        
        if (file.isDirectory()){
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if (levels){
                listDir(fs, file.name(), levels - 1);
            }
        }
        else{
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("  SIZE: ");
            Serial.println(file.size());
        }
        
        file = root.openNextFile();
    }
}

