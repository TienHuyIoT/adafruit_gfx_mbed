/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */
#include "mbed.h"

// https://os.mbed.com/users/GlimwormBeacons/code/SEGGER_RTT/
#include "SEGGER_RTT.h"
#include "console_dbg.h"

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>             // Arduino SPI library
#include <pinmap_ex.h>

Timer t_system;

// ST7789 TFT module connections
#define TFT_EN p13 // define power enable pin
#define TFT_CS p17 // define chip select pin
#define TFT_DC p15 // define data/command pin
#define TFT_RST                                                                \
  p19 // define reset pin, or set to -1 and connect to Arduino RESET pin
#define TFT_SCLK p14 // set these to be whatever pins you like!
#define TFT_MOSI p16 // set these to be whatever pins you like!
#define TFT_MISO p42 // set these to be whatever pins you like!

const PinMapSPI PinMap_SPI[1] = {
    { TFT_MOSI, TFT_MISO, TFT_SCLK, 3 }
};

SPI tftspi(TFT_MOSI, TFT_MISO, TFT_SCLK); // mosi, miso, sclk

// Initialize Adafruit ST7789 TFT library
Adafruit_ST7789 tft = Adafruit_ST7789(&tftspi, TFT_CS, TFT_DC, TFT_RST);
// Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK,
// TFT_RST);

float p = 3.1415926;

#define MAIN_TAG_DBG(...) CONSOLE_TAG_LOGI("[MAIN]", __VA_ARGS__)
#define MAIN_CONSOLE(...) CONSOLE_LOGI(__VA_ARGS__)

uint32_t millis() {
  using namespace std::chrono;
  uint32_t m = duration_cast<milliseconds>(t_system.elapsed_time()).count();
  return m;
}

void aw9364_init(uint8_t pule) {
  // set the pin as an output:
  DigitalOut tft_enable(TFT_EN);
  tft_enable = 0;
  ThisThread::sleep_for(20ms);
  for (uint8_t i = 0; i < pule; ++i) {
    wait_us(50);
    tft_enable = 0;
    wait_us(50);
    tft_enable = 1;
  }
}

void testlines(uint16_t color) {
  tft.fillScreen(ST77XX_BLACK);
  for (int16_t x = 0; x < tft.width(); x += 6) {
    tft.drawLine(0, 0, x, tft.height() - 1, color);
    // ThisThread::sleep_for(0);
  }
  for (int16_t y = 0; y < tft.height(); y += 6) {
    tft.drawLine(0, 0, tft.width() - 1, y, color);
    // ThisThread::sleep_for(0);
  }

//   tft.fillScreen(ST77XX_BLACK);
//   for (int16_t x = 0; x < tft.width(); x += 6) {
//     tft.drawLine(tft.width() - 1, 0, x, tft.height() - 1, color);
//     // ThisThread::sleep_for(0);
//   }
//   for (int16_t y = 0; y < tft.height(); y += 6) {
//     tft.drawLine(tft.width() - 1, 0, 0, y, color);
//     // ThisThread::sleep_for(0);
//   }

//   tft.fillScreen(ST77XX_BLACK);
//   for (int16_t x = 0; x < tft.width(); x += 6) {
//     tft.drawLine(0, tft.height() - 1, x, 0, color);
//     // ThisThread::sleep_for(0);
//   }
//   for (int16_t y = 0; y < tft.height(); y += 6) {
//     tft.drawLine(0, tft.height() - 1, tft.width() - 1, y, color);
//     // ThisThread::sleep_for(0);
//   }

//   tft.fillScreen(ST77XX_BLACK);
//   for (int16_t x = 0; x < tft.width(); x += 6) {
//     tft.drawLine(tft.width() - 1, tft.height() - 1, x, 0, color);
//     // ThisThread::sleep_for(0);
//   }
//   for (int16_t y = 0; y < tft.height(); y += 6) {
//     tft.drawLine(tft.width() - 1, tft.height() - 1, 0, y, color);
//     // ThisThread::sleep_for(0);
//   }
}

void testdrawtext(char *text, uint16_t color) {
  tft.setCursor(0, 0);
  tft.setTextColor(color);
  tft.setTextWrap(true);
  tft.print(text);
}

void testfastlines(uint16_t color1, uint16_t color2) {
  tft.fillScreen(ST77XX_BLACK);
  for (int16_t y = 0; y < tft.height(); y += 5) {
    tft.drawFastHLine(0, y, tft.width(), color1);
  }
  for (int16_t x = 0; x < tft.width(); x += 5) {
    tft.drawFastVLine(x, 0, tft.height(), color2);
  }
}

void testdrawrects(uint16_t color) {
  tft.fillScreen(ST77XX_BLACK);
  for (int16_t x = 0; x < tft.width(); x += 6) {
    tft.drawRect(tft.width() / 2 - x / 2, tft.height() / 2 - x / 2, x, x,
                 color);
  }
}

void testfillrects(uint16_t color1, uint16_t color2) {
  tft.fillScreen(ST77XX_BLACK);
  for (int16_t x = tft.width() - 1; x > 6; x -= 6) {
    tft.fillRect(tft.width() / 2 - x / 2, tft.height() / 2 - x / 2, x, x,
                 color1);
    tft.drawRect(tft.width() / 2 - x / 2, tft.height() / 2 - x / 2, x, x,
                 color2);
  }
}

void testfillcircles(uint8_t radius, uint16_t color) {
  for (int16_t x = radius; x < tft.width(); x += radius * 2) {
    for (int16_t y = radius; y < tft.height(); y += radius * 2) {
      tft.fillCircle(x, y, radius, color);
    }
  }
}

void testdrawcircles(uint8_t radius, uint16_t color) {
  for (int16_t x = 0; x < tft.width() + radius; x += radius * 2) {
    for (int16_t y = 0; y < tft.height() + radius; y += radius * 2) {
      tft.drawCircle(x, y, radius, color);
    }
  }
}

void testtriangles() {
  tft.fillScreen(ST77XX_BLACK);
  int color = 0xF800;
  int t;
  int w = tft.width() / 2;
  int x = tft.height() - 1;
  int y = 0;
  int z = tft.width();
  for (t = 0; t <= 15; t++) {
    tft.drawTriangle(w, y, y, x, z, x, color);
    x -= 4;
    y += 4;
    z -= 4;
    color += 100;
  }
}

void testroundrects() {
  tft.fillScreen(ST77XX_BLACK);
  int color = 100;
  int i;
  int t;
  for (t = 0; t <= 4; t += 1) {
    int x = 0;
    int y = 0;
    int w = tft.width() - 2;
    int h = tft.height() - 2;
    for (i = 0; i <= 16; i += 1) {
      tft.drawRoundRect(x, y, w, h, 5, color);
      x += 2;
      y += 3;
      w -= 4;
      h -= 6;
      color += 1100;
    }
    color += 100;
  }
}

void tftPrintTest() {
  tft.setTextWrap(false);
  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(0, 30);
  tft.setTextColor(ST77XX_RED);
  tft.setTextSize(1);
  tft.println("Hello World!");
  tft.setTextColor(ST77XX_YELLOW);
  tft.setTextSize(2);
  tft.println("Hello World!");
  tft.setTextColor(ST77XX_GREEN);
  tft.setTextSize(3);
  tft.println("Hello World!");
  tft.setTextColor(ST77XX_BLUE);
  tft.setTextSize(4);
  tft.print(1234.567);
  ThisThread::sleep_for(500ms);
  tft.setCursor(0, 0);
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(0);
  tft.println("Hello World!");
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_GREEN);
  tft.print(p, 6);
  tft.println(" Want pi?");
  tft.println(" ");
  tft.print(8675309, HEX); // print 8,675,309 out in HEX!
  tft.println(" Print HEX!");
  tft.println(" ");
  tft.setTextColor(ST77XX_WHITE);
  tft.println("Sketch has been");
  tft.println("running for: ");
  tft.setTextColor(ST77XX_MAGENTA);
  tft.print(millis() / 1000);
  tft.setTextColor(ST77XX_WHITE);
  tft.print(" seconds.");
}

void mediabuttons() {
  // play
  tft.fillScreen(ST77XX_BLACK);
  tft.fillRoundRect(25, 10, 78, 60, 8, ST77XX_WHITE);
  tft.fillTriangle(42, 20, 42, 60, 90, 40, ST77XX_RED);
  ThisThread::sleep_for(500ms);
  // pause
  tft.fillRoundRect(25, 90, 78, 60, 8, ST77XX_WHITE);
  tft.fillRoundRect(39, 98, 20, 45, 5, ST77XX_GREEN);
  tft.fillRoundRect(69, 98, 20, 45, 5, ST77XX_GREEN);
  ThisThread::sleep_for(500ms);
  // play color
  tft.fillTriangle(42, 20, 42, 60, 90, 40, ST77XX_BLUE);
  ThisThread::sleep_for(50ms);
  // pause color
  tft.fillRoundRect(39, 98, 20, 45, 5, ST77XX_RED);
  tft.fillRoundRect(69, 98, 20, 45, 5, ST77XX_RED);
  // play color
  tft.fillTriangle(42, 20, 42, 60, 90, 40, ST77XX_GREEN);
}

void tft_setup(void) {
  aw9364_init(10);  

  MAIN_TAG_DBG("Hello! ST77xx TFT Test");

//   tftspi.frequency(32E6);
//   tftspi.format(8, 0);
  // highspeed SPIM should set SCK and MOSI to high drive
    // nrf_gpio_cfg(TFT_SCLK,
    //             NRF_GPIO_PIN_DIR_OUTPUT,
    //             NRF_GPIO_PIN_INPUT_CONNECT,
    //             NRF_GPIO_PIN_NOPULL,
    //             NRF_GPIO_PIN_H0H1,
    //             NRF_GPIO_PIN_NOSENSE);

    // nrf_gpio_cfg(TFT_MOSI,
    //             NRF_GPIO_PIN_DIR_OUTPUT,
    //             NRF_GPIO_PIN_INPUT_DISCONNECT,
    //             NRF_GPIO_PIN_NOPULL,
    //             NRF_GPIO_PIN_H0H1,
    //             NRF_GPIO_PIN_NOSENSE);
  // if the display has CS pin try with SPI_MODE0
  tft.init(240, 240, 0); // Init ST7789 display 240x240 pixel
  // tft.setSPISpeed(32E6); // Init SPI speed

  ThisThread::sleep_for(20ms);

  // if the screen is flipped, remove this command
  tft.setRotation(2);

  MAIN_TAG_DBG("Initialized");

  MAIN_TAG_DBG("fillScreen ST77XX_BLACK");
  uint16_t time = millis();
  tft.fillScreen(ST77XX_BLACK);
  time = millis() - time;
  MAIN_TAG_DBG("Time fill: %u", time);

  ThisThread::sleep_for(500ms);

  // large block of text
  MAIN_TAG_DBG("testdrawtext");
  testdrawtext(
      (char
           *)"Lorem ipsum dolor sit amet, consectetur adipiscing elit. "
             "Curabitur adipiscing ante sed nibh tincidunt feugiat. Maecenas "
             "enim massa, fringilla sed malesuada et, malesuada sit amet "
             "turpis. Sed porttitor neque ut ante pretium vitae malesuada nunc "
             "bibendum. Nullam aliquet ultrices massa eu hendrerit. Ut sed "
             "nisi lorem. In vestibulum purus a tortor imperdiet posuere. ",
      ST77XX_WHITE);
  ThisThread::sleep_for(1000ms);

  // tft print function!
  MAIN_TAG_DBG("tftPrintTest");
  tftPrintTest();
  ThisThread::sleep_for(4000ms);

  // a single pixel
  MAIN_TAG_DBG("drawPixel ST77XX_GREEN");
  tft.drawPixel(tft.width() / 2, tft.height() / 2, ST77XX_GREEN);
  ThisThread::sleep_for(500ms);

  // line draw test
  MAIN_TAG_DBG("testlines(ST77XX_YELLOW);");
  testlines(ST77XX_YELLOW);
  ThisThread::sleep_for(500ms);

  // optimized lines
  MAIN_TAG_DBG("testfastlines(ST77XX_RED, ST77XX_BLUE);");
  testfastlines(ST77XX_RED, ST77XX_BLUE);
  ThisThread::sleep_for(500ms);

  MAIN_TAG_DBG("testdrawrects(ST77XX_GREEN);");
  testdrawrects(ST77XX_GREEN);
  ThisThread::sleep_for(500ms);

  MAIN_TAG_DBG("testfillrects(ST77XX_YELLOW, ST77XX_MAGENTA);");
  testfillrects(ST77XX_YELLOW, ST77XX_MAGENTA);
  ThisThread::sleep_for(500ms);

  MAIN_TAG_DBG("tft.fillScreen(ST77XX_BLACK);");
  tft.fillScreen(ST77XX_BLACK);

  MAIN_TAG_DBG("testfillcircles(10, ST77XX_BLUE);");
  testfillcircles(10, ST77XX_BLUE);

  MAIN_TAG_DBG("testdrawcircles(10, ST77XX_WHITE);");
  testdrawcircles(10, ST77XX_WHITE);
  ThisThread::sleep_for(500ms);

  MAIN_TAG_DBG("testroundrects();");
  testroundrects();
  ThisThread::sleep_for(500ms);

  MAIN_TAG_DBG("testtriangles();");
  testtriangles();
  ThisThread::sleep_for(500ms);

  MAIN_TAG_DBG("mediabuttons();");
  mediabuttons();
  ThisThread::sleep_for(500ms);

  MAIN_TAG_DBG("done");
  ThisThread::sleep_for(1000ms);
}

void tft_loop() {
  tft.invertDisplay(true);
  ThisThread::sleep_for(500ms);
  tft.invertDisplay(false);
  ThisThread::sleep_for(500ms);
}

int main() {
  t_system.start();

  MAIN_TAG_DBG("system clock %d\r\n", SystemCoreClock);

  tft_setup();

  while (true) {
    tft_loop();
  }
}
