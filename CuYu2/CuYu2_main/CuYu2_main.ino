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
// WYGBRO
uint8_t hall_data[N_FACES] = {0, 0, 0, 0, 0, 0};
const int hall_pin[N_FACES] = {D4, D2, D3, D1, D7, D5};
#define FACE_IDX_WHITE 0
#define FACE_IDX_YELLOW 1
#define FACE_IDX_GREEN 2
#define FACE_IDX_BLUE 3
#define FACE_IDX_RED 4
#define FACE_IDX_ORAGNE 5

#define STATUS_SEND_SUCCESS 0
#define STATUS_SEND_FAILED 1

#define DEEP_SLEEP_TIME_THRESHOLD 30000 // ms

uint8_t last_hall_data_bit = 0b11111111;
uint8_t hall_data_bit = 0;
int data_status = STATUS_SEND_FAILED;
unsigned long last_turned = 0;

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

void deep_sleep() {
  Serial.println("sleep...");
  esp_wifi_stop();
  esp_deep_sleep_enable_gpio_wakeup(
    BIT(GPIO_NUM_2) | BIT(GPIO_NUM_3) | BIT(GPIO_NUM_4) | BIT(GPIO_NUM_5), 
    ESP_GPIO_WAKEUP_GPIO_LOW);
  esp_deep_sleep_start();
  Serial.println("wake");
}

void setup() {
  for (int i = 0; i < N_FACES; ++i){
    pinMode(hall_pin[i], INPUT_PULLUP);
  }
  pinMode(D0, INPUT_PULLUP); // for sleep
  pinMode(D8, INPUT_PULLUP); // for boot
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

  last_hall_data_bit = 0b11111111;
  hall_data_bit = 0;
  data_status = STATUS_SEND_FAILED;
  last_turned = millis();
}

void loop() {
  // update data
  for (int i = 0; i < N_FACES; ++i){
    hall_data[i] = !digitalRead(hall_pin[i]);
  }

  // send data
  last_hall_data_bit = hall_data_bit;
  hall_data_bit = 0;
  for (int i = 0; i < N_FACES; ++i){
    hall_data_bit |= hall_data[i] << i;
  }
  if (hall_data_bit != last_hall_data_bit || data_status == STATUS_SEND_FAILED){
    last_turned = millis();
    for (int i = 0; i < N_FACES; ++i){
      Serial.print(hall_data[i]);
    }
    Serial.print(" ");
    Serial.println(hall_data_bit);
    esp_err_t result = esp_now_send(slave.peer_addr, &hall_data_bit, sizeof(hall_data_bit));
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
  } else if (hall_data_bit == 0 && millis() - last_turned > DEEP_SLEEP_TIME_THRESHOLD){ // sleep
    deep_sleep();
  }

  // wait for 3seconds to run the logic again
  delay(10);
}
