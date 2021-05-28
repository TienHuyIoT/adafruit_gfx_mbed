#ifndef _ADAFRUIT_LVGL_GLUE_H_
#define _ADAFRUIT_LVGL_GLUE_H_

#include <Adafruit_SPITFT.h>   // GFX lib for SPI and parallel displays
#include <lvgl.h>              // LittlevGL core lib
#include <Ticker.h>
#if defined(ARDUINO_ARCH_SAMD)
#include <Adafruit_ZeroTimer.h> // SAMD-specific timer lib
#elif defined(ESP32)
#include <Ticker.h> // ESP32-specific timer lib
#endif

typedef enum {
  LVGL_OK,
  LVGL_ERR_ALLOC,
  LVGL_ERR_TIMER,
} LvGLStatus;

/**
 * @brief Class to act as a "glue" layer between the LvGL graphics library and
 * most of Adafruit's TFT displays
 *
 */
class Adafruit_LvGL_Glue {
public:
  Adafruit_LvGL_Glue(void);
  ~Adafruit_LvGL_Glue(void);
  LvGLStatus begin(Adafruit_SPITFT *tft, bool debug = false);
  // These items need to be public for some internal callbacks,
  // but should be avoided by user code please!
  Adafruit_SPITFT *display; ///< Pointer to the SPITFT display instance
  void *touchscreen;        ///< Pointer to the touchscreen object to use
  bool is_adc_touch; ///< determines if the touchscreen controlelr is ADC based
  bool first_frame;  ///< Tracks if a call to `lv_flush_callback` needs to wait
                     ///< for DMA transfer to complete

private:
  LvGLStatus begin(Adafruit_SPITFT *tft, void *touch, bool debug);
  lv_disp_drv_t lv_disp_drv;
  lv_disp_buf_t lv_disp_buf;
  lv_color_t *lv_pixel_buf;
  lv_indev_drv_t lv_indev_drv;
  lv_indev_t *lv_input_dev_ptr;
#if defined(ARDUINO_ARCH_SAMD)
  Adafruit_ZeroTimer *zerotimer;
#elif defined(ESP32)
  Ticker tick;
#elif defined(NRF52840_XXAA)
  Ticker tick;
#endif
};

#endif // _ADAFRUIT_LVGL_GLUE_H_
