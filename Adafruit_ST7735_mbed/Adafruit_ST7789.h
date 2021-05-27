#ifndef _ADAFRUIT_ST7789H_
#define _ADAFRUIT_ST7789H_

#include "Adafruit_ST77xx.h"

/// Subclass of ST77XX type display for ST7789 TFT Driver
class Adafruit_ST7789 : public Adafruit_ST77xx {
public:
#if defined(ARDUINO)
  Adafruit_ST7789(int8_t cs, int8_t dc, int8_t mosi, int8_t sclk,
                  int8_t rst = -1);
  Adafruit_ST7789(int8_t cs, int8_t dc, int8_t rst);
#elif defined(__MBED__)
  Adafruit_ST7789(PinName cs, PinName dc, PinName mosi, PinName sclk,
                  PinName rst = NC);
  Adafruit_ST7789(PinName cs, PinName dc, PinName rst);
#endif
#if !defined(ESP8266)
#if defined(ARDUINO)
  Adafruit_ST7789(SPIClass *spiClass, int8_t cs, int8_t dc, int8_t rst);
#elif defined(__MBED__)
  Adafruit_ST7789(SPI *spiClass, PinName cs, PinName dc, PinName rst);
#endif
#endif // end !ESP8266

  void setRotation(uint8_t m);
  void init(uint16_t width, uint16_t height, uint8_t spiMode = 0);

protected:
  uint8_t _colstart2 = 0, ///< Offset from the right
      _rowstart2 = 0;     ///< Offset from the bottom

private:
  uint16_t windowWidth;
  uint16_t windowHeight;
};

#endif // _ADAFRUIT_ST7789H_
