#include <GxEPD2_BW.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold24pt7b.h>

#define EPD_BUSY 17
#define EPD_RST 16
#define EPD_DC 4
#define EPD_CS 5
#define EPD_SCK 18
#define EPD_MOSI 23

#define SCREEN_W 200
#define SCREEN_H 200

GxEPD2_BW<GxEPD2_154_D67, GxEPD2_154_D67::HEIGHT> display(GxEPD2_154_D67(EPD_CS, EPD_DC, EPD_RST, EPD_BUSY));

struct ClockTime {
    int hour;
    int minute;
    int second;
};

ClockTime getTime() {
    ClockTime t;
    t.hour = 12;
    t.minute = 0;
    t.second = 0;
    return t;
}

void drawClock(ClockTime t) {
    char timeBuf[6];
    char secBuf[10];
    snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d", t.hour, t.minute);
    snprintf(secBuf, sizeof(secBuf), ":%02d", t.second);
    int16_t x1, y1;
    uint16_t tw, th, sw, sh;
    display.setFont(&FreeMonoBold24pt7b);
    display.getTextBounds(timeBuf, 0, 0, &x1, &y1, &tw, &th);
    int timeX = (SCREEN_W - tw) / 2 - x1;
    int timeY = (SCREEN_H / 2) - (th / 2) - y1;
    display.setFont(&FreeMonoBold9pt7b);
    display.getTextBounds(secBuf, 0, 0, &x1, &y1, &sw, &sh);
    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);
    display.setFont(&FreeMonoBold24pt7b);
    display.setCursor(timeX, timeY);
    display.print(timeBuf);
    display.setFont(&FreeMonoBold9pt7b);
    display.setCursor((SCREEN_W - sw) / 2 - x1, timeY + th + 4);
    display.print(secBuf);
}

void setup() {
    display.init();
    display.setRotation(0);
    ClockTime t = getTime();
    display.setFullWindow();
    display.firstPage();
    do {
        drawClock(t);
    } while (display.nextPage());
}

void loop() {
    delay(60000);
    ClockTime t = getTime();
    display.setPartialWindow(0, 0, SCREEN_W, SCREEN_H);
    display.firstPage();
    do {
        drawClock(t);
    } while (display.nextPage());
}
