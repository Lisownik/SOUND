#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <format>

enum class SoundState {
  BAD,
  MEDIUM,
  GOOD
};

constexpr const char* to_cstr(SoundState state) {
  switch (state) {
    case SoundState::BAD:    return "BAD";
    case SoundState::MEDIUM: return "MEDIUM";
    case SoundState::GOOD:   return "GOOD";
  }

  return "UNKNOWN";
}

std::string to_string(SoundState state) {
  return to_cstr(state);
}

class LedRing{
  public:
    LedRing(uint32_t color, uint8_t pin, uint8_t ledCount): color_(color), pin_(pin), ledCount_(ledCount){ 
      strip_ = Adafruit_NeoPixel(ledCount_, pin_, NEO_GRB + NEO_KHZ800);
    }

    void show(){
      strip_.show();
    }

    void lightOn(){
      for(int i = 0; i < ledCount_; i++){
        strip_.setPixelColor(i, color_);
      }
      show();
    }

    void lightOff() {
      for(int i = 0; i < ledCount_; i++){
        strip_.setPixelColor(i, 0);
      }
      show();
    }

  private:
    uint32_t color_;
    uint8_t pin_;
    uint8_t ledCount_;
    Adafruit_NeoPixel strip_;
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

SoundState parseData(uint8_t value){
    if (value < 127){
      return SoundState::GOOD;
    }
    else if (value < 196){
      return SoundState::MEDIUM;
    }
    return SoundState::BAD;
}

void interpretResult(){

}


LedRing redRing{0xFF0000, 2, 16};
LedRing greenRing{0x00FF00, 3, 16};
LedRing blueRing{0x0000FF, 4, 16};

AnalogSensor microphone{5};

void setup(){
  Serial.begin(9600);
  Serial.println("S.O.U.N.D. Firmware Started!"); 
  
  redRing.lightOn();
  greenRing.lightOn();
  blueRing.lightOn();
  delay(500);
  
  redRing.lightOff();
  greenRing.lightOff();
  blueRing.lightOff();
  
  int value = microphone.read();
  uint8_t mapped = microphone.readMapped();

  Serial.println(
      std::format("Read Data: {}, and mapped: {}", value, mapped)
        .c_str()
      );
}



#define LOOP_DELAY 1000

void loop(){
 uint8_t data = microphone.readMapped();
 SoundState state = parseData(data);
 Serial.println(
     std::format("microphone data: {} and its interpreted state: {}", data, to_string(state))
     .c_str()
     );
 
 delay(LOOP_DELAY);
}

