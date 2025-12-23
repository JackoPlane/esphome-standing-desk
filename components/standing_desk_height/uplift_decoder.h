#pragma once

#include <stdint.h>
#include "decoder.h"
#include "esphome/core/log.h"

namespace esphome {
namespace standing_desk_height {

class UpliftDecoder : public Decoder {
  protected:
    enum state_t {
      SYNC1,   // waiting for 0x01
      SYNC2,   // waiting for 0x01 (second)
      HEIGHT1,  // waiting for first height arg, usually 0x01 but 0x00 if at the bottom
      HEIGHT2,  // waiting for second height arg
    } state_ = SYNC1;
    uint8_t buf_[2];
    
  public:
    UpliftDecoder() { }
    ~UpliftDecoder() { }

    bool put(uint8_t b);
    float decode();
};

struct Util {
  static unsigned int getword(unsigned char a, unsigned char b) {
    return (static_cast<unsigned>(a) << 8) + b;
  }

  static unsigned to_mm(unsigned h) {
    if (h < 600) {
      // Height in inches*10; convert to mm
      h *= 254; // convert to mm*100
      h += 50;  // round up to nearest whole mm
      h /= 100; // convert to mm
    }
    return h;
  }
};

}
}
