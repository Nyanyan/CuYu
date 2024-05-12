// original: ESPNOW sample code
// modified by Nyanyan

/**
   ESPNOW - Basic communication - Master
   Date: 26th September 2017
   Author: Arvind Ravulavaru <https://github.com/arvindr21>
   Purpose: ESPNow Communication between a Master ESP32 and a Slave ESP32
   Description: This sketch consists of the code for the Master module.
   Resources: (A bit outdated)
   a. https://espressif.com/sites/default/files/documentation/esp-now_user_guide_en.pdf
   b. http://www.esploradores.com/practica-6-conexion-esp-now/

   << This Device Master >>

   Flow: Master
   Step 1 : ESPNow Init on Master and set it in STA mode
   Step 2 : Start scanning for Slave ESP32 (we have added a prefix of `slave` to the SSID of slave for an easy setup)
   Step 3 : Once found, add Slave as peer
   Step 4 : Register for send callback
   Step 5 : Start Transmitting data from Master to Slave

   Flow: Slave
   Step 1 : ESPNow Init on Slave
   Step 2 : Update the SSID of Slave with a prefix of `slave`
   Step 3 : Set Slave in AP mode
   Step 4 : Register for receive callback and wait for data
   Step 5 : Once data arrives, print it in the serial monitor

   Note: Master and Slave have been defined to easily understand the setup.
         Based on the ESPNOW API, there is no concept of Master and Slave.
         Any devices can act as master or salve.
*/

#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h> // only for esp_wifi_set_channel()

// Global copy of slave
esp_now_peer_info_t slave;
#define CHANNEL 1
#define PRINTSCANRESULTS 0
#define DELETEBEFOREPAIR 0

#define N_FACES 6
#define LED_PIN_R D7
#define LED_PIN_G D8
#define LED_PIN_B D10
// WYGBRO
uint8_t hall_data[N_FACES] = {0, 0, 0, 0, 0, 0};
const int hall_pin[N_FACES] = {D4, D2, D3, D1, D0, D5};
#define FACE_IDX_WHITE 0
#define FACE_IDX_YELLOW 1
#define FACE_IDX_GREEN 2
#define FACE_IDX_BLUE 3
#define FACE_IDX_RED 4
#define FACE_IDX_ORAGNE 5

// Init ESP Now with fallback
void InitESPNow() {
  WiFi.disconnect();
  if (esp_now_init() == ESP_OK) {
    Serial.println("ESPNow Init Success");
  }
  else {
    Serial.println("ESPNow Init Failed");
    // Retry InitESPNow, add a counte and then restart?
    // InitESPNow();
    // or Simply Restart
    ESP.restart();
  }
}

#define DATA_SIZE 6

void setup() {
  for (int i = 0; i < N_FACES; ++i){
    pinMode(hall_pin[i], INPUT);
  }
  pinMode(LED_PIN_R, OUTPUT);
  pinMode(LED_PIN_G, OUTPUT);
  pinMode(LED_PIN_B, OUTPUT);

  Serial.begin(115200);

  //Set device in STA mode to begin with
  WiFi.mode(WIFI_STA);
  esp_wifi_set_channel(CHANNEL, WIFI_SECOND_CHAN_NONE);
  // This is the mac address of the Master in Station Mode
  Serial.print("STA MAC: "); Serial.println(WiFi.macAddress());
  Serial.print("STA CHANNEL "); Serial.println(WiFi.channel());
  // Init ESPNow with a fallback logic
  InitESPNow();
  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet

  const uint8_t slave_mac_addr[6] = {0x48, 0x31, 0xb7, 0x3f, 0xbe, 0x01};
  memset(&slave, 0, sizeof(slave));
  for (int i = 0; i < 6; ++i) {
    slave.peer_addr[i] = slave_mac_addr[i];
  }
  while (true){
    esp_err_t addStatus = esp_now_add_peer(&slave);
    if (addStatus == ESP_OK) {
      // Pair success
      Serial.println("Pair success");
      break;
    } else{
      Serial.println("Cannot Pair");
    }
  }
}

#define STATUS_SEND_SUCCESS 0
#define STATUS_SEND_FAILED 1

int led_status = 0;
uint8_t last_hall_data = 0b11111111;
uint8_t data = 0;

void loop() {
  // update data
  for (int i = 0; i < N_FACES; ++i){
    hall_data[i] = !digitalRead(hall_pin[i]);
  }

  // send data
  int data_status = STATUS_SEND_FAILED;
  const uint8_t *peer_addr = slave.peer_addr;
  last_hall_data = data;
  data = 0;
  for (int i = 0; i < N_FACES; ++i){
    data <<= 1;
    data |= hall_data[i];
    Serial.print(hall_data[i]);
  }
  Serial.print(" ");
  Serial.println(data);
  uint8_t send_data[DATA_SIZE] = {'C', 'u', 'Y', 'u', '2', data};
  esp_err_t result = esp_now_send(peer_addr, send_data, sizeof(send_data[0]) * DATA_SIZE);
  //Serial.print("Send Status: ");
  if (result == ESP_OK) {
    data_status = STATUS_SEND_SUCCESS;
    Serial.println("Success");
  } else {
    data_status = STATUS_SEND_FAILED;
    if (result == ESP_ERR_ESPNOW_NOT_INIT) {
      // How did we get so far!!
      Serial.println("ESPNOW not Init.");
    } else if (result == ESP_ERR_ESPNOW_ARG) {
      Serial.println("Invalid Argument");
    } else if (result == ESP_ERR_ESPNOW_INTERNAL) {
      Serial.println("Internal Error");
    } else if (result == ESP_ERR_ESPNOW_NO_MEM) {
      Serial.println("ESP_ERR_ESPNOW_NO_MEM");
    } else if (result == ESP_ERR_ESPNOW_NOT_FOUND) {
      Serial.println("Peer not found.");
    } else {
      Serial.println("Not sure what happened");
    }
  }

  if (data_status == STATUS_SEND_FAILED){
    digitalWrite(LED_PIN_R, HIGH);
    digitalWrite(LED_PIN_G, LOW);
    digitalWrite(LED_PIN_B, LOW);
  } else{
    if (data != 0){ // turning
      if (data != last_hall_data){
        ++led_status;
        led_status %= 3;
      }
      bool led_red = false;
      bool led_green = false;
      bool led_blue = false;
      if (1 & (data >> (5 - FACE_IDX_WHITE))){
        led_red = true;
        led_green = true;
        led_blue = true;
      }
      if (1 & (data >> (5 - FACE_IDX_YELLOW))){
        led_red = true;
        led_green = true;
      }
      if (1 & (data >> (5 - FACE_IDX_GREEN))){
        led_green = true;
      }
      if (1 & (data >> (5 - FACE_IDX_BLUE))){
        led_blue = true;
      }
      if (1 & (data >> (5 - FACE_IDX_RED))){
        led_red = true;
      }
      if (1 & (data >> (5 - FACE_IDX_ORAGNE))){
        led_red = true;
        led_blue = true;
      }
      digitalWrite(LED_PIN_R, led_red);
      digitalWrite(LED_PIN_G, led_green);
      digitalWrite(LED_PIN_B, led_blue);
      /*
      if (led_status == 0){
        digitalWrite(LED_PIN_R, HIGH);
        digitalWrite(LED_PIN_G, LOW);
        digitalWrite(LED_PIN_B, LOW);
      } else if (led_status == 1){
        digitalWrite(LED_PIN_R, LOW);
        digitalWrite(LED_PIN_G, HIGH);
        digitalWrite(LED_PIN_B, LOW);
      } else{
        digitalWrite(LED_PIN_R, LOW);
        digitalWrite(LED_PIN_G, LOW);
        digitalWrite(LED_PIN_B, HIGH);
      }
      */
    } else{ // not turning
      digitalWrite(LED_PIN_R, LOW);
      digitalWrite(LED_PIN_G, LOW);
      digitalWrite(LED_PIN_B, LOW);
    }
  }

  // wait for 3seconds to run the logic again
  delay(10);
}
