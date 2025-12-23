#include "uplift_decoder.h"

namespace esphome {
namespace standing_desk_height {

static const char *const TAG = "standing_desk_height";

// Implementation based off of: https://github.com/rmcgibbo/Jarvis
// Which, despite the name, works for Uplift desks too
bool UpliftDecoder::put(uint8_t b) {
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

float UpliftDecoder::decode() {
  const float currentMethod = ((buf_[0] << 8) | (buf_[1] & 0xFF)) / 10.0;

  const unsigned int jarvisMethod = Util::getword(buf_[0], buf_[1]);
  // const unsigned int jarvisMethod = (static_cast<unsigned>(buf_[0]) << 8) + buf_[1];
  const unsigned int jarvisMM = Util::to_mm(jarvisMethod);

  ESP_LOGD(TAG, "Uplift Decoder - Current Method: %f, Jarvis Method: %d, Jarvis MM: %d", currentMethod, jarvisMethod, jarvisMM);

  return jarvisMM;
}

}
}