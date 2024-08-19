// original: ESPNOW sample code
// modified by Nyanyan

/**
   ESPNOW - Basic communication - Slave
   Date: 26th September 2017
   Author: Arvind Ravulavaru <https://github.com/arvindr21>
   Purpose: ESPNow Communication between a Master ESP32 and a Slave ESP32
   Description: This sketch consists of the code for the Slave module.
   Resources: (A bit outdated)
   a. https://espressif.com/sites/default/files/documentation/esp-now_user_guide_en.pdf
   b. http://www.esploradores.com/practica-6-conexion-esp-now/

   << This Device Slave >>

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

#include <Mozzi.h>
//#include <MozziGuts.h>
#include <ReverbTank.h>
#include <Oscil.h>
#include <tables/saw2048_int8.h>
#include <ADSR.h>

#define CHANNEL 1

#define N_FACES 6

#define CONTROL_RATE 128

#define W_TONE 329.628f
#define Y_TONE 261.626f
#define G_TONE 391.995f
#define B_TONE 523.251f
#define R_TONE 293.665f
#define O_TONE 440.000f

Oscil<SAW2048_NUM_CELLS, AUDIO_RATE> wOscil(SAW2048_DATA);
Oscil<SAW2048_NUM_CELLS, AUDIO_RATE> yOscil(SAW2048_DATA);
Oscil<SAW2048_NUM_CELLS, AUDIO_RATE> gOscil(SAW2048_DATA);
Oscil<SAW2048_NUM_CELLS, AUDIO_RATE> bOscil(SAW2048_DATA);
Oscil<SAW2048_NUM_CELLS, AUDIO_RATE> rOscil(SAW2048_DATA);
Oscil<SAW2048_NUM_CELLS, AUDIO_RATE> oOscil(SAW2048_DATA);
//ADSR <AUDIO_RATE, AUDIO_RATE> envelope;
//ReverbTank reverb;


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

// config AP SSID
void configDeviceAP() {
  const char *SSID = "Slave_1";
  bool result = WiFi.softAP(SSID, "Slave_1_Password", CHANNEL, 0);
  if (!result) {
    Serial.println("AP Config failed.");
  } else {
    Serial.println("AP Config Success. Broadcasting with AP: " + String(SSID));
    Serial.print("AP CHANNEL "); Serial.println(WiFi.channel());
  }
}

void setup() {
  startMozzi(CONTROL_RATE);
  //envelope.setADLevels(255, 128);
  //envelope.setTimes(10, 10, 100, 500);

  Serial.begin(115200);
  Serial.println("ESPNow/Basic/Slave Example");

  //Set device in AP mode to begin with
  WiFi.mode(WIFI_AP);
  // configure device AP mode
  configDeviceAP();
  // This is the mac address of the Slave in AP Mode
  Serial.print("AP MAC: "); Serial.println(WiFi.softAPmacAddress());
  // Init ESPNow with a fallback logic
  InitESPNow();
  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info.
  esp_now_register_recv_cb(OnDataRecv);
}

// callback when data is recv from Master
void OnDataRecv(const uint8_t *mac_addr, const uint8_t *data, int data_len) {
  if (1 & (data[0] >> 0)){
    wOscil.setFreq(W_TONE);
  } else{
    wOscil.setFreq(0);
  }
  if (1 & (data[0] >> 1)){
    yOscil.setFreq(Y_TONE);
  } else{
    yOscil.setFreq(0);
  }
  if (1 & (data[0] >> 2)){
    gOscil.setFreq(G_TONE);
  } else{
    gOscil.setFreq(0);
  }
  if (1 & (data[0] >> 3)){
    bOscil.setFreq(B_TONE);
  } else{
    bOscil.setFreq(0);
  }
  if (1 & (data[0] >> 4)){
    rOscil.setFreq(R_TONE);
  } else{
    rOscil.setFreq(0);
  }
  if (1 & (data[0] >> 5)){
    oOscil.setFreq(O_TONE);
  } else{
    oOscil.setFreq(0);
  }

  for (int i = 0; i < N_FACES; ++i){
    uint8_t bit = 1 & (data[0] >> i);
    Serial.print(bit);
    Serial.print(' ');
  }
  Serial.println("");
}

void updateControl() {
}

AudioOutput updateAudio(){
  int synth = (wOscil.next() + yOscil.next() + gOscil.next() + bOscil.next() + rOscil.next() + oOscil.next()) / 6;
  return MonoOutput::fromAlmostNBit(9, synth);
  //return MonoOutput::from8Bit(synth);
  //int arev = reverb.next(synth);
  //return MonoOutput::fromAlmostNBit(9, synth + (arev>>3));
}

void loop() {
  audioHook();
}
