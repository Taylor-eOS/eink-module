#include <GxEPD2_BW.h>

#define EPD_BUSY 17
#define EPD_RST 16
#define EPD_DC 4
#define EPD_CS 5
#define EPD_SCK 18
#define EPD_MOSI 23

GxEPD2_BW<GxEPD2_154_D67, GxEPD2_154_D67::HEIGHT> display(GxEPD2_154_D67(EPD_CS, EPD_DC, EPD_RST, EPD_BUSY));

void setup() {
    display.init();
    display.setRotation(0);
    display.fillScreen(GxEPD_WHITE);
    //display.fillScreen(GxEPD_BLACK);
    display.display();
}

void loop() {
}
