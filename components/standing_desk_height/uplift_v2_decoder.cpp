#include "uplift_v2_decoder.h"

namespace esphome {
namespace standing_desk_height {

static const char *const TAG = "uplift_v2_decoder";

const uint8_t JARVIS_ADDR = 0x01; // 0xF2;

// Implementation based off of: https://github.com/phord/Jarvis
void UpliftV2Decoder::reset(uint8_t ch) {
  // state = SYNC;
  state = static_cast<state_t>(SYNC + (ch == JARVIS_ADDR));
  cmd = NONE;
  argc = 0;
  memset(argv, 0U, sizeof(argv));

  // NOTE: jarvis_uplift_decoder default:
  // state = SYNC;
  // cmd = 0;
  // argc = 0;
  // memset(argv, 0U, sizeof(argv));

  // NOTE: jarvis_decoder default:
  // state = static_cast<state_t>(SYNC + (ch == JARVIS_ADDR));
  // cmd = NONE;
  // argc = 0;
  // memset(argv, 0U, sizeof(argv));
}

// Compensating handler for error bytes.
// If we get an unexpected char, reset our state and clear any accumulated
// arguments. But we want to resync with the start of the next possible
// message as soon as possible. So, after an error we set the state back to
// SYNC to begin waiting for a new packet.  But if the error byte itself was a
// sync byte (matches our address), then we should already advance to SYNC2.
// returns "false" to simplify returning from "put"
bool UpliftV2Decoder::error(unsigned char ch) {
  reset(ch);
  return false;
}

// Implementation based off of: https://github.com/rmcgibbo/Jarvis
// Which, despite the name, works for Uplift desks too
bool UpliftV2Decoder::put(uint8_t b) {
  bool complete = false;

  switch (state) {
    case SYNC:
      if (b != JARVIS_ADDR) {
        ESP_LOGD(TAG, "Bad Sync: %u", b);
        return error(b);
      }
      break;

    case CMD:
      if (b == 5) // end of stream
        return error(b);
      if (b != 1 && b != 2 && b != 4 && b != 6) {
        ESP_LOGD(TAG, "Bad cmd: %u", b);
        return error(b);
      }
      cmd = static_cast<command_byte>(b); // was checksum = b
      break;

    default:                        // ARGS, state increased by 2 each time
      if (state < 4 || state > 6) { // only 2 args seen
        ESP_LOGD(TAG, "Arg mismatch, cmd: %d", (int)cmd);
        for (int i = 0; i <= 2; i++)
          ESP_LOGD(TAG, "\t |-> argv[%d]: %u", i, argv[i]);

        return error(b);
      }
      argv[argc++] = b;

      if (argc == 2)
        complete = true;

      break;
  }

  switch (state_) {
  case SYNC1:
    if (b == 0x01) {
      state_ = SYNC2;
      return false;
    } else {
      state_ = SYNC1;
      return false;
    }
  case SYNC2:
    if (b == 0x01) {
      state_ = HEIGHT1;
      return false;
    } else {
      state_ = SYNC1;
      return false;
    }
  case HEIGHT1:
    if (b == 0x00 || b == 0x01) {
      buf_[0] = b;
      state_ = HEIGHT2;
      return false;
    } else {
      state_ = SYNC1;
      return false;
    }
  case HEIGHT2:
    buf_[1] = b;
    state_ = SYNC1;
    return true;
  default:
    return false;
  }
  return false;
}

float UpliftV2Decoder::decode() {
  const float currentMethod = ((buf_[0] << 8) | (buf_[1] & 0xFF)) / 10.0;

  const unsigned int jarvisMethod = Util::getword(buf_[0], buf_[1]);
  // const unsigned int jarvisMethod = (static_cast<unsigned>(buf_[0]) << 8) + buf_[1];
  const unsigned int jarvisMM = Util::to_mm(jarvisMethod);

  // ESP_LOGD(TAG, "Uplift Decoder - Jarvis Method: %d, Jarvis MM: %d", jarvisMethod, jarvisMM);
  ESP_LOGD(TAG, "Uplift Decoder - Current Method: %f, Jarvis Method: %d, Jarvis MM: %d", currentMethod, jarvisMethod, jarvisMM);

  return currentMethod;
  // return (float)jarvisMM / 10.0;
}

}
}