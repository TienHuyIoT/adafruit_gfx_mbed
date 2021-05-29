/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */
#include "mbed.h"

// https://os.mbed.com/users/GlimwormBeacons/code/SEGGER_RTT/
#include "SEGGER_RTT.h"
#include "console_dbg.h"

#include "Adafruit_GFX.h"    // Core graphics library
#include "Adafruit_ST7789.h" // Hardware-specific library for ST7789
#include "Adafruit_LvGL_Glue.h"
#include <lvgl.h>    // LittlevGL header
#include "SPIMode.h" // Arduino SPI library
#include <pinmap_ex.h>

#include "Fonts/FreeSans24pt7b.h"
#include "Fonts/FreeSans18pt7b.h"
#include "Fonts/FreeSans12pt7b.h"
#include "Fonts/FreeSans9pt7b.h"
#include "Fonts/SansSerifSize28.h"
#include "schedule_event_add_32.h"
#include "schedule_event_add_64.h"
#include "schedule_event_add_40.h"

Timer t_system;

// ST7789 TFT module connections
#define TFT_EN p13 // define power enable pin
#define TFT_CS p17 // define chip select pin
#define TFT_DC p15 // define data/command pin
#define TFT_RST \
    p19              // define reset pin, or set to -1 and connect to Arduino RESET pin
#define TFT_SCLK p14 // set these to be whatever pins you like!
#define TFT_MOSI p16 // set these to be whatever pins you like!
#define TFT_MISO p42 // set these to be whatever pins you like!

#if (0) /* Using SPI lib mbed */
const PinMapSPI PinMap_SPI[1] = {
    {TFT_MOSI, TFT_MISO, TFT_SCLK, 3}};
SPI tftspi(TFT_MOSI, TFT_MISO, TFT_SCLK); // mosi, miso, sclk
#endif

/* Using SPI lib Arduino Porting */
// Initialize Adafruit ST7789 TFT library
Adafruit_ST7789 tft = Adafruit_ST7789(&SPIMode, TFT_CS, TFT_DC, TFT_RST);

#define COLOR_BLACK0 (rgb_hex565(0x0A0A0A))

Adafruit_GFX_Button gfx_button;

uint16_t rgb_hex565(uint32_t rgb888)
{
  uint16_t rgb565;
  uint16_t r, g, b;
  r = (rgb888 & 0xFF0000) >> 16;
  g = (rgb888 & 0xFF00) >> 8;
  b = rgb888 & 0xFF;

  rgb565 = (uint16_t(r * 31 / 255) << 11) | (uint16_t(g * 63 / 255) << 5) | (uint16_t(b * 31 / 255));

  return rgb565;
}

float p = 3.1415926;

#define MAIN_TAG_DBG(...) CONSOLE_TAG_LOGI("[MAIN]", __VA_ARGS__)
#define MAIN_CONSOLE(...) CONSOLE_LOGI(__VA_ARGS__)

uint32_t millis()
{
    using namespace std::chrono;
    uint32_t m = duration_cast<milliseconds>(t_system.elapsed_time()).count();
    return m;
}

Adafruit_LvGL_Glue glue;

lv_obj_t *tabview,         // LittlevGL tabview object
    *gauge,                // Gauge object (on first of three tabs)
    *chart,                // Chart object (second tab)
    *canvas;               // Canvas object (third tab)
uint8_t active_tab = 0,    // Index of currently-active tab (0-2)
    prev_tab = 0;          // Index of previously-active tab
lv_chart_series_t *series; // 'Series' data for the bar chart

// Canvas object is an image in memory. Although LittlevGL supports several
// internal image formats, only the TRUE_COLOR variety allows drawing
// operations. Fortunately nRF52840 has gobs of RAM for this. The canvas
// buffer is global because it's referenced both in the setup and main loop.
#define CANVAS_WIDTH 200 // Dimensions in pixels
#define CANVAS_HEIGHT 150
lv_color_t canvas_buffer[LV_CANVAS_BUF_SIZE_TRUE_COLOR(CANVAS_WIDTH, CANVAS_HEIGHT)];
lv_draw_line_dsc_t draw_dsc; // Drawing style (for canvas) is similarly global

void tft_lvgl_setup(void)
{
    // Create a tabview object, by default this covers the full display.
    tabview = lv_tabview_create(lv_disp_get_scr_act(NULL), NULL);
    // The CLUE display has a lot of pixels and can't refresh very fast.
    // To show off the tabview animation, let's slow it down to 1 second.
    lv_tabview_set_anim_time(tabview, 1000);

    // Because they're referenced any time an object is drawn, styles need
    // to be permanent in scope; either declared globally (outside all
    // functions), or static. The styles used on tabs are never modified after
    // they're used here, so let's use static on those...
    static lv_style_t tab_style, tab_background_style, indicator_style;

    // This is the background style "behind" the tabs. This is what shows
    // through for "off" (inactive) tabs -- a vertical green gradient,
    // minimal padding around edges (zero at bottom).
    lv_style_init(&tab_background_style);
    lv_style_set_bg_color(&tab_background_style, LV_STATE_DEFAULT, lv_color_hex(0x408040));
    lv_style_set_bg_grad_color(&tab_background_style, LV_STATE_DEFAULT, lv_color_hex(0x304030));
    lv_style_set_bg_grad_dir(&tab_background_style, LV_STATE_DEFAULT, LV_GRAD_DIR_VER);
    lv_style_set_pad_top(&tab_background_style, LV_STATE_DEFAULT, 2);
    lv_style_set_pad_left(&tab_background_style, LV_STATE_DEFAULT, 2);
    lv_style_set_pad_right(&tab_background_style, LV_STATE_DEFAULT, 2);
    lv_style_set_pad_bottom(&tab_background_style, LV_STATE_DEFAULT, 0);
    lv_obj_add_style(tabview, LV_TABVIEW_PART_TAB_BG, &tab_background_style);

    // Style for tabs. Active tab is white with opaque background, inactive
    // tabs are transparent so the background shows through (only the white
    // text is seen). A little top & bottom padding reduces scrunchyness.
    lv_style_init(&tab_style);
    lv_style_set_pad_top(&tab_style, LV_STATE_DEFAULT, 3);
    lv_style_set_pad_bottom(&tab_style, LV_STATE_DEFAULT, 10);
    lv_style_set_bg_color(&tab_style, LV_STATE_CHECKED, LV_COLOR_WHITE);
    lv_style_set_bg_opa(&tab_style, LV_STATE_CHECKED, LV_OPA_100);
    lv_style_set_text_color(&tab_style, LV_STATE_CHECKED, LV_COLOR_GRAY);
    lv_style_set_bg_opa(&tab_style, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    lv_style_set_text_color(&tab_style, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_add_style(tabview, LV_TABVIEW_PART_TAB_BTN, &tab_style);

    // Style for the small indicator bar that appears below the active tab.
    lv_style_init(&indicator_style);
    lv_style_set_bg_color(&indicator_style, LV_STATE_DEFAULT, LV_COLOR_RED);
    lv_style_set_size(&indicator_style, LV_STATE_DEFAULT, 5);
    lv_obj_add_style(tabview, LV_TABVIEW_PART_INDIC, &indicator_style);

    // Back to creating widgets...

    // Add three tabs to the tabview
    lv_obj_t *tab1 = lv_tabview_add_tab(tabview, "Gauge");
    lv_obj_t *tab2 = lv_tabview_add_tab(tabview, "Chart");
    lv_obj_t *tab3 = lv_tabview_add_tab(tabview, "Canvas");

    // And then add stuff in each tab...

    // The first tab holds a gauge. To keep the demo simple, let's just use
    // the default style and range (0-100). See LittlevGL docs for options.
    gauge = lv_gauge_create(tab1, NULL);
    lv_obj_set_size(gauge, 186, 186);
    lv_obj_align(gauge, NULL, LV_ALIGN_CENTER, 0, 0);

    // Second tab, make a chart...
    chart = lv_chart_create(tab2, NULL);
    lv_obj_set_size(chart, 200, 180);
    lv_obj_align(chart, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_chart_set_type(chart, LV_CHART_TYPE_COLUMN);
    // For simplicity, we'll stick with the chart's default 10 data points:
    series = lv_chart_add_series(chart, LV_COLOR_RED);
    lv_chart_init_points(chart, series, 0);
    // Make each column shift left as new values enter on right:
    lv_chart_set_update_mode(chart, LV_CHART_UPDATE_MODE_SHIFT);

    // Third tab is a canvas, which we'll fill with random colored lines.
    // LittlevGL draw functions only work on TRUE_COLOR canvas.
    canvas = lv_canvas_create(tab3, NULL);
    lv_canvas_set_buffer(canvas, canvas_buffer,
                         CANVAS_WIDTH, CANVAS_HEIGHT, LV_IMG_CF_TRUE_COLOR);
    lv_obj_align(canvas, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_canvas_fill_bg(canvas, LV_COLOR_WHITE, LV_OPA_100);

    // Set up canvas line-drawing style based on defaults.
    // Later we'll change color settings when drawing each line.
    lv_draw_line_dsc_init(&draw_dsc);
}

uint32_t prev_time = -1;
void tft_lvgl_loop(void)
{
    // Read left/right button inputs. Debouncing could be done better,
    // but this'll do for a quick simple demo...
    if(digitalRead(43) == LOW) {
      if(active_tab > 0) {
        active_tab--;
      }
      while(digitalRead(43) == LOW); // Wait for button release
    } else if(digitalRead(44) == LOW) {
      if(active_tab < 2) {
        active_tab++;
      }
      while(digitalRead(44) == LOW); // Wait for button release
    }

    // Change active tab if button pressings happened
    if (active_tab != prev_tab)
    {
        lv_tabview_set_tab_act(tabview, active_tab, true);
        prev_tab = active_tab;
    }

    // Make the gauge sweep a full sine wave over time
    lv_gauge_set_value(gauge, 0, (int)(50.5 + sin(millis() / 1000.0) * 50.0));

    // About 2X a second, things happen on the other two tabs...
    uint32_t new_time = millis() / 500; // Current half-second
    if (new_time != prev_time)
    { // freshly elapsed
        prev_time = new_time;

        // Add a new random item to the bar chart (old value shift left)
        lv_chart_set_next(chart, series, rand() % 100);
        lv_chart_refresh(chart);

        // Add a rand line to the canvas
        lv_point_t points[2];
        points[0].x = rand() % CANVAS_WIDTH;
        points[0].y = rand() % CANVAS_HEIGHT;
        points[1].x = rand() % CANVAS_WIDTH;
        points[1].y = rand() % CANVAS_HEIGHT;
        draw_dsc.color.ch.red = rand();
        draw_dsc.color.ch.green = rand();
        draw_dsc.color.ch.blue = rand();
        lv_canvas_draw_line(canvas, points, 2, &draw_dsc);
        // This forces the canvas to update (otherwise changes aren't
        // seen unless leaving and returning to the canvas tab):
        lv_canvas_set_buffer(canvas, canvas_buffer,
                             CANVAS_WIDTH, CANVAS_HEIGHT, LV_IMG_CF_TRUE_COLOR);
    }

    lv_task_handler(); // Call LittleVGL task handler periodically
    ThisThread::sleep_for(5ms);
}

void aw9364_init(uint8_t pule)
{
    // set the pin as an output:
    DigitalOut tft_enable(TFT_EN);
    tft_enable = 0;
    ThisThread::sleep_for(20ms);
    for (uint8_t i = 0; i < pule; ++i)
    {
        wait_us(50);
        tft_enable = 0;
        wait_us(50);
        tft_enable = 1;
    }
}

void testlines(uint16_t color)
{
    tft.fillScreen(ST77XX_BLACK);
    for (int16_t x = 0; x < tft.width(); x += 6)
    {
        tft.drawLine(0, 0, x, tft.height() - 1, color);
        // ThisThread::sleep_for(0);
    }
    for (int16_t y = 0; y < tft.height(); y += 6)
    {
        tft.drawLine(0, 0, tft.width() - 1, y, color);
        // ThisThread::sleep_for(0);
    }

    tft.fillScreen(ST77XX_BLACK);
    for (int16_t x = 0; x < tft.width(); x += 6)
    {
        tft.drawLine(tft.width() - 1, 0, x, tft.height() - 1, color);
        // ThisThread::sleep_for(0);
    }
    for (int16_t y = 0; y < tft.height(); y += 6)
    {
        tft.drawLine(tft.width() - 1, 0, 0, y, color);
        // ThisThread::sleep_for(0);
    }

    tft.fillScreen(ST77XX_BLACK);
    for (int16_t x = 0; x < tft.width(); x += 6)
    {
        tft.drawLine(0, tft.height() - 1, x, 0, color);
        // ThisThread::sleep_for(0);
    }
    for (int16_t y = 0; y < tft.height(); y += 6)
    {
        tft.drawLine(0, tft.height() - 1, tft.width() - 1, y, color);
        // ThisThread::sleep_for(0);
    }

    tft.fillScreen(ST77XX_BLACK);
    for (int16_t x = 0; x < tft.width(); x += 6)
    {
        tft.drawLine(tft.width() - 1, tft.height() - 1, x, 0, color);
        // ThisThread::sleep_for(0);
    }
    for (int16_t y = 0; y < tft.height(); y += 6)
    {
        tft.drawLine(tft.width() - 1, tft.height() - 1, 0, y, color);
        // ThisThread::sleep_for(0);
    }
}

void testdrawtext(char *text, uint16_t color)
{
    tft.setCursor(0, 0);
    tft.setTextColor(color);
    tft.setTextWrap(true);
    tft.print(text);
}

void testfastlines(uint16_t color1, uint16_t color2)
{
    tft.fillScreen(ST77XX_BLACK);
    for (int16_t y = 0; y < tft.height(); y += 5)
    {
        tft.drawFastHLine(0, y, tft.width(), color1);
    }
    for (int16_t x = 0; x < tft.width(); x += 5)
    {
        tft.drawFastVLine(x, 0, tft.height(), color2);
    }
}

void testdrawrects(uint16_t color)
{
    tft.fillScreen(ST77XX_BLACK);
    for (int16_t x = 0; x < tft.width(); x += 6)
    {
        tft.drawRect(tft.width() / 2 - x / 2, tft.height() / 2 - x / 2, x, x,
                     color);
    }
}

void testfillrects(uint16_t color1, uint16_t color2)
{
    tft.fillScreen(ST77XX_BLACK);
    for (int16_t x = tft.width() - 1; x > 6; x -= 6)
    {
        tft.fillRect(tft.width() / 2 - x / 2, tft.height() / 2 - x / 2, x, x,
                     color1);
        tft.drawRect(tft.width() / 2 - x / 2, tft.height() / 2 - x / 2, x, x,
                     color2);
    }
}

void testfillcircles(uint8_t radius, uint16_t color)
{
    for (int16_t x = radius; x < tft.width(); x += radius * 2)
    {
        for (int16_t y = radius; y < tft.height(); y += radius * 2)
        {
            tft.fillCircle(x, y, radius, color);
        }
    }
}

void testdrawcircles(uint8_t radius, uint16_t color)
{
    for (int16_t x = 0; x < tft.width() + radius; x += radius * 2)
    {
        for (int16_t y = 0; y < tft.height() + radius; y += radius * 2)
        {
            tft.drawCircle(x, y, radius, color);
        }
    }
}

void testtriangles()
{
    tft.fillScreen(ST77XX_BLACK);
    int color = 0xF800;
    int t;
    int w = tft.width() / 2;
    int x = tft.height() - 1;
    int y = 0;
    int z = tft.width();
    for (t = 0; t <= 15; t++)
    {
        tft.drawTriangle(w, y, y, x, z, x, color);
        x -= 4;
        y += 4;
        z -= 4;
        color += 100;
    }
}

void testroundrects()
{
    tft.fillScreen(ST77XX_BLACK);
    int color = 100;
    int i;
    int t;
    for (t = 0; t <= 4; t += 1)
    {
        int x = 0;
        int y = 0;
        int w = tft.width() - 2;
        int h = tft.height() - 2;
        for (i = 0; i <= 16; i += 1)
        {
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

void tftPrintTest()
{
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

void mediabuttons()
{
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

void tft_setup(void)
{
    aw9364_init(10);
    SPIMode.setPins(TFT_MISO, TFT_SCLK, TFT_MOSI);

    MAIN_TAG_DBG("Hello! ST77xx TFT Test");

    tft.init(240, 240, 0); // Init ST7789 display 240x240 pixel
    tft.setSPISpeed(32E6);

    ThisThread::sleep_for(20ms);
    // if the screen is flipped, remove this command
    tft.setRotation(0);
}

void tft_gfx_graphicstest(void)
{
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
    ThisThread::sleep_for(2000ms);

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

void tft_lvgl_widgetsTest()
{
    // Initialize glue, passing in address of display
    LvGLStatus status = glue.begin(&tft);
    if (status != LVGL_OK)
    {
        MAIN_TAG_DBG("Glue error %d", (int)status);
        for (;;)
            ;
    }

    tft_lvgl_setup(); // Call UI-building function above

    // Enable "A" and "B" buttons as inputs
    pinMode(43, INPUT_PULLUP);
    pinMode(44, INPUT_PULLUP);
}

void tft_gfx_loop()
{
    tft.invertDisplay(true);
    ThisThread::sleep_for(500ms);
    tft.invertDisplay(false);
    ThisThread::sleep_for(500ms);
}


void watch_not_int_duration_page()
{
  tft.fillScreen(ST77XX_BLACK);

  tft.setFont(&FreeSans24pt7b);
  tft.setTextWrap(false);
  tft.setCursor(40, 100);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(2);
  tft.print("7:00");

  /* Schedule icon */
  tft.drawCircle(120, 170, 40, COLOR_BLACK0);
  tft.fillCircle(120, 170, 40, COLOR_BLACK0);
  
  tft.drawXBitmap(120 - 32, 170 - 32, schedule_event_add_64_bits, 64, 64, ST77XX_BLUE);
  
}

void watch_int_duration_page()
{
  tft.fillScreen(ST77XX_BLACK);
  
  /* Place A */
  tft.setFont(&FreeSans18pt7b);
  tft.setTextWrap(false);
  tft.setCursor(0, 30);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(1);
  tft.print("Place A");

  /* From To */
  tft.setFont(&FreeSans12pt7b);
  tft.setTextWrap(false);
  tft.setTextColor(COLOR_BLACK0);
  tft.setTextSize(1);

  tft.setCursor(0, 60);
  tft.print("From");
  tft.setCursor(150, 60);
  tft.print("To");

  /* Time From To */
  tft.setFont(&FreeSans18pt7b);
  tft.setTextWrap(false);
  tft.setTextColor(ST77XX_ORANGE);
  tft.setTextSize(1);

  tft.setCursor(0, 100);
  tft.print("8:00");
  tft.setCursor(150, 100);
  tft.print("9:30");

  /* Activity */
  tft.setFont(&FreeSans12pt7b);
  tft.setTextWrap(false);
  tft.setTextColor(COLOR_BLACK0);
  tft.setTextSize(1);

  tft.setCursor(0, 140);
  tft.print("Activity");

  /* Lorem ipsu... */
  tft.setFont(&FreeSans18pt7b);
  tft.setTextWrap(false);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(1);

  tft.setCursor(0, 180);
  tft.print("Lorem ipsu ...");
}

void watch_event_page()
{
  tft.fillScreen(ST77XX_BLACK);

  /* Schedule icon */
  tft.drawCircle(25, 25, 22, ST77XX_BLUE);
  tft.fillCircle(25, 25, 22, ST77XX_BLUE);
  tft.drawXBitmap(25 -20, 25 - 20, schedule_event_add_40_bits, 40, 40, ST77XX_WHITE);
  
  /* Schedule */
  tft.setFont(&SansSerif_plain_28);
  tft.setTextWrap(false);
  tft.setCursor(53, 40);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(1);
  tft.print("Schedule");

  /* 10:20 */
  tft.setFont(&FreeSans9pt7b);
  tft.setTextWrap(false);
  tft.setTextColor(COLOR_BLACK0);
  tft.setTextSize(1);
  tft.setCursor(195, 40);
  tft.print("10:30");

  /* In 5 minutes */
  tft.setFont(&FreeSans12pt7b);
  tft.setTextWrap(true);
  tft.setCursor(0, 80);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(1);
  tft.print("   In 5 minutes, start\n   activity 2 at place B");

  /* Button */
  gfx_button.initButton(&tft, 120, 170, 200, 60, 
                        COLOR_BLACK0, COLOR_BLACK0, ST77XX_WHITE,
                        nullptr, 1);
  gfx_button.drawButton();

  tft.setFont(&FreeSans18pt7b);
  tft.setCursor(120 - 30, 170 + 10);
  tft.setTextColor(ST77XX_WHITE);
  tft.print("Back");
}

void watch_full_message_page()
{
  tft.fillScreen(ST77XX_BLACK);

  tft.setFont(&FreeSans18pt7b);
  tft.setTextWrap(true);
  tft.setCursor(0, 30);
  tft.setTextColor(ST77XX_WHITE);
  tft.print("Lorem ipsum\ndolor sit amet,\nconsectetur\nadipiscing elit.");
}

// bool inverttft = true;
void watchMeloop() {
  watch_not_int_duration_page();

  ThisThread::sleep_for(2000ms);

  watch_int_duration_page();

  ThisThread::sleep_for(2000ms);

  watch_event_page();

  ThisThread::sleep_for(2000ms);

  watch_full_message_page();

  ThisThread::sleep_for(2000ms);
//   inverttft = !inverttft;
//   tft.invertDisplay(inverttft);
}

int main()
{
    t_system.start();
    MAIN_TAG_DBG("system clock %d\r\n", SystemCoreClock);
    tft_setup();

    // tft_gfx_graphicstest();
    tft_lvgl_widgetsTest();

    while (true)
    {
        // tft_gfx_loop();
        // tft_lvgl_loop();
        watchMeloop();
    }
}
