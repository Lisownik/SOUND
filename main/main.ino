#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <driver/i2s.h>
#include <format>
#include <cmath>
#include "../lib/ringBuffer.h"

enum class SoundState {
  BAD,
  MEDIUM,
  GOOD
};

const char* to_cstr(SoundState state) {
  switch (state) {
    case SoundState::BAD:    return "BAD";
    case SoundState::MEDIUM: return "MEDIUM";
    case SoundState::GOOD:   return "GOOD";
  }

  return "UNKNOWN";
}

const char* to_string(SoundState state) {
  return to_cstr(state);
}

class LedRing{
  private:
    uint32_t color_;
    uint8_t pin_;
    uint8_t ledCount_;
    Adafruit_NeoPixel* strip_;
    
  public:
    LedRing(uint32_t color, uint8_t pin, uint8_t ledCount): color_(color), pin_(pin), ledCount_(ledCount){ 
      strip_ = new Adafruit_NeoPixel(ledCount_, pin_, NEO_GRB + NEO_KHZ800);
      strip_->begin();
      strip_->setBrightness(30);
    }

    void show(){
      strip_->show();
    }

    void lightOn(){
      for(int i = 0; i < ledCount_; i++){
        strip_->setPixelColor(i, color_);
      }
      show();
    }

    void lightOff() {
      for(int i = 0; i < ledCount_; i++){
        strip_->setPixelColor(i, 0);
      }
      show();
    }


};

struct AnalogSensor{
  uint8_t pin_;
  
  AnalogSensor(uint8_t pin){
    pin_ = pin;
  }

  int read(){
    return analogRead(pin_); 
  }

  uint8_t readMapped(){
    return map(
        read(),
        0, 4095,
        0, 255
        );
  }
};

SoundState parseData(double value){
    if (value < 75){
      return SoundState::GOOD;
    }
    else if (value < 100){
      return SoundState::MEDIUM;
    }
    return SoundState::BAD;
}



#define I2S_WS 25
#define I2S_SCK 32
#define I2S_SD 33

#define I2S_PORT  I2S_NUM_0
#define SAMPLE_RATE 16000
#define BUFFER_LEN  512

int32_t sBuffer[BUFFER_LEN];

i2s_config_t i2s_config = {
  .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
  .sample_rate = SAMPLE_RATE,
  .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
  .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
  .communication_format = I2S_COMM_FORMAT_STAND_I2S,
  .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
  .dma_buf_count = 8,
  .dma_buf_len = BUFFER_LEN,
  .use_apll = false
};

i2s_pin_config_t pin_config = {
  .bck_io_num = I2S_SCK,
  .ws_io_num = I2S_WS,
  .data_out_num = I2S_PIN_NO_CHANGE,
  .data_in_num = I2S_SD
};

double calculateLoudness(){
  size_t bytes_read = 0;

  i2s_read(I2S_PORT, sBuffer, sizeof(sBuffer), &bytes_read, portMAX_DELAY);

  int samples_read = bytes_read / sizeof(int32_t);
  double sum = 0;

  for (int i = 0; i < samples_read; i++) {
    int32_t sample = sBuffer[i] >> 8;
    sum += (double)sample * sample;
  }

  double rms = sqrt(sum / samples_read);

  return rms;
}

LedRing redRing{0xFF0000, 2, 24};
LedRing greenRing{0x00FF00, 2, 24};
LedRing yellowRing{0xAFFF00, 2, 24};

RingBuffer buffer{20, 50};

void setup(){
  Serial.begin(9600);
  Serial.println("S.O.U.N.D. Firmware Started!"); 
 
  i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_PORT, &pin_config);
  i2s_zero_dma_buffer(I2S_PORT);

  redRing.lightOn();
  delay(500);
  yellowRing.lightOn();
  delay(500);
  greenRing.lightOn();
  delay(500);
  redRing.lightOff();
  
}



#define LOOP_DELAY 100

void loop(){
  double loud = calculateLoudness();
  double dBFS = 20 * log10(loud / 8388608);
  double dB = 94 + 26 + dBFS;

  if (dB - buffer.getAvg() > 10)
    buffer.push(dB);
  buffer.push(dB);


  int avg = buffer.getAvg();
  SoundState state = parseData(avg);
  if(state == SoundState::BAD){
    yellowRing.lightOff();
    greenRing.lightOff();
    redRing.lightOn();
  }
  else if(state == SoundState::MEDIUM){
    redRing.lightOff();
    greenRing.lightOff();
    yellowRing.lightOn();
  }
  else if(state == SoundState::GOOD){
    redRing.lightOff();
    yellowRing.lightOff();
    greenRing.lightOn();
  }

  Serial.println(dB);
  Serial.println(avg);
  Serial.println((int)state);


  delay(LOOP_DELAY);
}
