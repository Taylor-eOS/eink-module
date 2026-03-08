#include <GxEPD2_BW.h>
#include "BigClock.h"

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
};

ClockTime getTime() {
    ClockTime t;
    t.hour = 17;
    t.minute = 25;
    return t;
}

void drawClock(ClockTime t) {
    char hourBuf[3];
    char minBuf[3];
    snprintf(hourBuf, sizeof(hourBuf), "%02d", t.hour);
    snprintf(minBuf, sizeof(minBuf), "%02d", t.minute);
    int16_t x1, y1;
    uint16_t tw, th;
    display.setFont(&BigClock);
    display.getTextBounds("00", 0, 0, &x1, &y1, &tw, &th);
    int gap = 10;
    int totalH = th * 2 + gap;
    int startY = (SCREEN_H - totalH) / 2 - y1;
    int startX = (SCREEN_W - tw) / 2 - x1;
    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);
    display.setCursor(startX, startY);
    display.print(hourBuf);
    display.setCursor(startX, startY + th + gap);
    display.print(minBuf);
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
