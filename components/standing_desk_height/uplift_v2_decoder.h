#pragma once

#include <stdint.h>
#include "decoder.h"
#include "esphome/core/log.h"

namespace esphome {
namespace standing_desk_height {

class UpliftV2Decoder : public Decoder {
  private:
    enum command_byte {
      // FAKE
      NONE        = 0x00,  // Unused/never seen; used as default for "Uninitialized"

      // CONTROLLER
      HEIGHT      = 0x01,  // Height report; P0=4 (mm?)
      ERROR       = 0x02,  // Error reporting and desk lockout
      RESET       = 0x04,  // Indicates desk in RESET mode; Displays "RESET"
      PRGM        = 0x06, // Programming code. Used when memory is pressed and in start up.
    };

    command_byte cmd = NONE;
    uint8_t checksum = 99;
    uint8_t argc = 0;
    uint8_t argv[5];

    enum state_t {
      SYNC,   // waiting for addr
      SYNC2,  // waiting for addr2
      CMD,    // waiting for cmd
      LENGTH, // waiting for argc
      // ARGS4,3,2,1   // collecting args
      ARGS = LENGTH + sizeof(argv), // collecting args
      CHKSUM,                       // waiting for checksum
      ENDMSG,                       // waiting for EOM
    } state = SYNC;

    void reset(uint8_t ch);
    bool error(unsigned char ch);
    
  public:
    UpliftV2Decoder() { }
    ~UpliftV2Decoder() { }

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
