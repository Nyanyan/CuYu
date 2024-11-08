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
//#include <ReverbTank.h>
#include <Oscil.h>
#include <tables/saw2048_int8.h>
#include <ADSR.h>
#include <ResonantFilter.h>

#define CHANNEL 1

#define N_FACES 6
#define N_TONES 5
#define CONTROL_RATE 128

#define DATA_CHARGING 0b01000000
#define CHARGING_LED 33

#define DATA_DEEPSLEEP 0b10000000
#define AWAKE_LED 5
bool awake_state = true;

const int rgb_led[3] = {27, 14, 32}; // RGB
const int tone_led_colors[N_TONES][3] = { // RGB, false: off
  { true,  true,  true}, // 呂旋法 (黒->白)
  { true, false, false}, // 律旋法 (橙->赤)
  {false,  true, false}, // 民謡音階 (緑->緑)
  { true,  true, false}, // 都節音階 (黄->黄)
  {false, false,  true}, // 琉球音階 (青->青)
};

int led_blink_state = 1;

int tone_idx = 0;

const float tones[N_TONES][N_FACES] = {
  {
    // 呂旋法
    391.995f, // W G4 4
    261.626f, // Y C4 1
    329.628f, // G E4 3
    523.251f, // B C5 6
    293.665f, // R D4 2
    440.000f  // O A4 5
  },
  {
    // 律旋法
    /*
    391.995f, // W G4 4
    261.626f, // Y C4 1
    349.228f, // G F4 3
    523.251f, // B C5 6
    293.665f, // R D4 2
    440.000f  // O A4 5
    */
    // key -5
    293.665f, // W D4 4
    195.998f, // Y G3 1
    261.626f, // G C4 3
    391.995f, // B G4 6
    220.000f, // R A3 2
    329.628f  // O E4 5
  },
  {
    // 民謡音階
    391.995f, // W G4 4
    261.626f, // Y C4 1
    349.228f, // G F4 3
    523.251f, // B C5 6
    311.127f, // R Eb4 2
    466.164f  // O Bb4 5
  },
  {
    // 都節音階
    /*
    391.995f, // W G4 4
    261.626f, // Y C4 1
    349.228f, // G F4 3
    523.251f, // B C5 6
    277.183f, // R Db4 2
    415.305f  // O Ab4 5
    */
    /*
    // key -3
    329.628f, // W E4 4
    220.000f, // Y A3 1
    293.665f, // G D4 3
    440.000f, // B A4 6
    233.082f, // R Bb3 2
    349.228f  // O F4 5
    */
    // key -7
    261.616f, // W C4 4
    174.614f, // Y F3 1
    233.082f, // G Bb3 3
    349.228f, // B F4 6
    184.997f, // R Gb3 2
    277.183f  // O Db4 5
  },
  {
    // 琉球音階
    /*
    391.995f, // W G4 4
    261.626f, // Y C4 1
    349.228f, // G F4 3
    523.251f, // B C5 6
    329.628f, // R E4 2
    493.883f  // O B4 5
    */
    // key +3
    466.164f, // W Bb4 4
    311.127f, // Y Eb4 1
    415.305f, // G Ab4 3
    622.254f, // B Eb5 6
    391.995f, // R G4 2
    587.330f  // O D5 5
  }
};

const int tone_buttons[N_TONES] = {23, 22, 21, 19, 18};

Oscil<SAW2048_NUM_CELLS, AUDIO_RATE> wOscil(SAW2048_DATA);
Oscil<SAW2048_NUM_CELLS, AUDIO_RATE> yOscil(SAW2048_DATA);
Oscil<SAW2048_NUM_CELLS, AUDIO_RATE> gOscil(SAW2048_DATA);
Oscil<SAW2048_NUM_CELLS, AUDIO_RATE> bOscil(SAW2048_DATA);
Oscil<SAW2048_NUM_CELLS, AUDIO_RATE> rOscil(SAW2048_DATA);
Oscil<SAW2048_NUM_CELLS, AUDIO_RATE> oOscil(SAW2048_DATA);
Oscil<2048, 32768> *Oscils[N_FACES];

ADSR <AUDIO_RATE, AUDIO_RATE> wenvelope;
ADSR <AUDIO_RATE, AUDIO_RATE> yenvelope;
ADSR <AUDIO_RATE, AUDIO_RATE> genvelope;
ADSR <AUDIO_RATE, AUDIO_RATE> benvelope;
ADSR <AUDIO_RATE, AUDIO_RATE> renvelope;
ADSR <AUDIO_RATE, AUDIO_RATE> oenvelope;
ADSR<32768, 32768> *envelopes[N_FACES];

//ReverbTank reverb;

LowPassFilter lpf;

int f_values[N_FACES];
int values[N_FACES];

bool charging_led_state = false;

uint8_t received_data = 0;

// multi threading
//TaskHandle_t threads[1];

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

void set_freq(int idx){
  for (int i = 0; i < N_FACES; ++i){
    Oscils[i]->setFreq(tones[idx][i]);
  }
}

void set_rgb_led_tone(int idx) {
  for (int i = 0; i < 3; ++i) {
    digitalWrite(rgb_led[i], !tone_led_colors[idx][i]);
  }
}

void rgb_led_off() {
  for (int i = 0; i < 3; ++i) {
    digitalWrite(rgb_led[i], HIGH);
  }
}

void rgb_led_light_bit(int bits) {
  for (int i = 0; i < 3; ++i) {
    bool bit = (bits >> i) & 1;
    digitalWrite(rgb_led[i], !bit);
  }
}

void setup() {
  for (int i = 0; i < N_FACES; ++i){
    f_values[i] = 0;
    values[i] = 0;
  }

  for (int i = 0; i < N_TONES; ++i){
    pinMode(tone_buttons[i], INPUT_PULLUP);
  }
  pinMode(CHARGING_LED, OUTPUT);
  pinMode(AWAKE_LED, OUTPUT);
  for (int i = 0; i < 3; ++i) {
    pinMode(rgb_led[i], OUTPUT);
  }
  digitalWrite(AWAKE_LED, HIGH);
  digitalWrite(CHARGING_LED, LOW);
  rgb_led_off();

  startMozzi(CONTROL_RATE);
  Oscils[0] = &wOscil;
  Oscils[1] = &yOscil;
  Oscils[2] = &gOscil;
  Oscils[3] = &bOscil;
  Oscils[4] = &rOscil;
  Oscils[5] = &oOscil;
  envelopes[0] = &wenvelope;
  envelopes[1] = &yenvelope;
  envelopes[2] = &genvelope;
  envelopes[3] = &benvelope;
  envelopes[4] = &renvelope;
  envelopes[5] = &oenvelope;
  for (int i = 0; i < N_FACES; ++i){
    envelopes[i]->setADLevels(255, 128);
    envelopes[i]->setTimes(10, 10, 1000000, 200);
  }
  lpf.setCutoffFreqAndResonance(150, 100);
  set_freq(tone_idx);
  set_rgb_led_tone(tone_idx);

  Serial.begin(115200);

  
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
  


  // multi threading
  //xTaskCreatePinnedToCore(Core0a, "Core0a", 4096, NULL, 3, &threads[0], 0);
}

/*
void Core0a(void *args) {
  for (;;) {
    audioHook();
  }
}
*/

// callback when data is recv from Master
void OnDataRecv(const uint8_t *mac_addr, const uint8_t *data, int data_len) {
  if (data[0] == 'C' && data[1] == 'u' && data[2] == 'Y' && data[3] == 'u'){
    uint8_t datum = data[4];
    if (datum == DATA_CHARGING) { // charging
      for (int i = 0; i < N_FACES; ++i) {
        if (f_values[i]) {
          envelopes[i]->noteOff();
          values[i] = 0;
          f_values[i] = 0;
        }
      }
      rgb_led_light_bit(led_blink_state);
      ++led_blink_state;
      if (led_blink_state >= 8) {
        led_blink_state = 1;
      }
      charging_led_state = !charging_led_state;
      digitalWrite(CHARGING_LED, charging_led_state);
      awake_state = false;
      digitalWrite(AWAKE_LED, awake_state);
      Serial.println("Charging");
    } else if (datum == DATA_DEEPSLEEP) { // sleep
      awake_state = false;
      digitalWrite(AWAKE_LED, awake_state);
      rgb_led_off();
      Serial.println("Sleep");
    } else { // sound
      if (charging_led_state) {
        charging_led_state = false;
        digitalWrite(CHARGING_LED, LOW);
      }
      if (!awake_state) {
        awake_state = true;
        digitalWrite(AWAKE_LED, awake_state);
        set_rgb_led_tone(tone_idx);
      }
      for (int i = 0; i < N_FACES; ++i){
        values[i] = (1 & (datum >> i));
        if (values[i] == 1 && f_values[i] == 0){
          envelopes[i]->noteOn();
        } else if (values[i] == 0 && f_values[i] == 1){
          envelopes[i]->noteOff();
        }
        f_values[i] = values[i];
      }
      
      for (int i = 0; i < N_FACES; ++i){
        uint8_t bit = 1 & (datum >> i);
        Serial.print(bit);
        Serial.print(' ');
      }
      Serial.println("");
    }
  }
}

void updateControl() {
  for (int i = 0; i < N_TONES; ++i){
    if (!digitalRead(tone_buttons[i])){
      tone_idx = i;
      set_freq(i);
      set_rgb_led_tone(i);
      Serial.print("tone set ");
      Serial.println(i);
    }
  }
}

AudioOutput updateAudio(){
  int synth = 0;
  for (int i = 0; i < N_FACES; ++i){
    envelopes[i]->update();
    int gain = envelopes[i]->next();
    synth += gain * Oscils[i]->next();
  }
  synth >>= 9;
  synth = lpf.next(synth) >> 1;
  return MonoOutput::fromAlmostNBit(8, synth);
}

void loop() {
  audioHook();
}
