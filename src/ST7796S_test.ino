// Basic full-color example for Adafruit_ST7796S
// test with arduino nano

#include <Adafruit_GFX.h>
#include <Adafruit_ST7796S.h>
#include <Fonts/FreeSansBold18pt7b.h> // A custom font
#include <Fonts/FreeSansBold24pt7b.h> // A custom font
#include "font.h"

// Define display pin connections
#define TFT_CS        10
#define TFT_RST        9 // Or set to -1 and connect to Arduino RESET pin
#define TFT_DC         8

// Initialize the display
Adafruit_ST7796S display(TFT_CS, TFT_DC, TFT_RST);

#define PAUSE 3000  // Delay (milliseconds) between examples
uint8_t rotate = 0; // Current screen orientation (0-3)
#define CORNER_RADIUS 0

#define DISPLAY_COL_TEXT 0x000000
#define DISPLAY_COL_BG   0xFFFFFF
#define DISPLAY_COL_DIV  0xFFFFFF

int k = 0; 
int l = 0; 
int m = 0; 
int n = 0; 

unsigned long  timeCount; 
unsigned long  timeRun; 
unsigned long  distCount; 

void setup() {
  Serial.begin(115200);
  Serial.println("ST7796S graphics demo");

  display.init(320, 480, 0, 0, ST7796S_RGB);
  display.setRotation(1); // landscape
  display.fillScreen(DISPLAY_COL_BG);  // Clear screen
  show_init();
}

void loop() {

  if (timeCount < millis()) {
    timeCount = millis() + 1000;
    timeRun += 1; 
    show_time();
  }
  show_speed();
  show_inclination();
  show_heart();
  show_energy();
  show_distance();
  delay(50);
}

// BASIC SHAPES EXAMPLE ----------------------------------------------------

void show_shapes() {
  display.fillScreen(0); // Start by clearing the screen; color 0 = black

  // Draw same shapes, same positions, but filled this time.
//display.fillRoundRect(x, y, dx, dy, round, color);
  display.fillRoundRect(20, 20, 80, 60, 15, display.color565(0,255,255));
  display.fillRoundRect(20, 100, 80, 60, 15, display.color565(255,0,255));
  display.fillRoundRect(20, 180, 80, 60, 15, display.color565(255,255,0));
  display.fillRoundRect(120, 20, 80, 60, 15, display.color565(255,255,255));
  display.fillRoundRect(120, 100, 80, 60, 15, display.color565(0,0,0));
} // END SHAPE EXAMPLE

/*
typedef enum { // Alignment options passed to functions below
  GFX_ALIGN_LEFT,
  GFX_ALIGN_CENTER,
  GFX_ALIGN_RIGHT
} GFXalign;
*/
void print_aligned(Adafruit_GFX &gfx, const char *str,
                   int16_t x, int16_t y) {
  uint16_t x1, y1, w, h;
  gfx.getTextBounds(str, x, y, &x1, &y1, &w, &h);
  gfx.setCursor(x, y);  // Center/right align
  gfx.println(str);
}

// Equivalent function for strings in flash memory (e.g. F("Foo")). Body
// appears identical to above function, but with C++ overloading it it works
// from flash instead of RAM. Any changes should be made in both places.
void print_aligned(Adafruit_GFX &gfx, const __FlashStringHelper *str,
                   int16_t x, int16_t y) {
  gfx.setCursor(x, y);  
  gfx.println(str);
}

void print_aligned_deci(Adafruit_GFX &gfx, int value,
                   int16_t x, int16_t y) {
  float j = (float)(value)/10;                    
  String str = String(j,1);
  const uint8_t l = 26; 
  const uint8_t digit = 4; 
  uint16_t w = 0, h = 35;
  // quirk formatting
  if (value >= 100)
    w = l; 
  if (value >= 1000)
    w = 2 * l; 
  if (value >= 10000)
    w = 3 * l; 

  gfx.setTextColor(DISPLAY_COL_TEXT, DISPLAY_COL_BG);
  gfx.fillRect(x - (l * (digit - 2)), y - h, l * digit + 15, h + 2, DISPLAY_COL_DIV);        
  gfx.setCursor(x - w, y);  
  gfx.println(str);
}

void print_aligned(Adafruit_GFX &gfx, int value,
                   int16_t x, int16_t y, uint8_t digit) {
  String str = String(value);
  const uint8_t l = 27; 
//  const uint8_t digit = 4; 
  uint16_t w = 0, h = 35;
  // quirk formatting
  if (value >= 1)
    w = -l; 
  if (value >= 10)
    w = 0; 
  if (value >= 100)
    w = l; 
  if (value >= 1000)
    w = 2 * l; 
  w-= 13;

  gfx.fillRect(x - (l * (digit - 2)), y - h, l * digit + 15, h + 2, DISPLAY_COL_DIV);        
  gfx.setCursor(x - w, y);  
  gfx.println(str);
}


void show_speed() {
  display.setTextSize(1);
  display.setFont(&Open_Sans_Regular_45);
  l+= 3; 
  if (l > 2000)
    l = 0; 
  print_aligned_deci(display, l, 150, 190);
}

void show_inclination() {
  display.setTextSize(1);
  display.setFont(&Open_Sans_Regular_45);
  m+= 1; 
  if (m > 20)
    m = 3; 
  print_aligned(display, m, 350, 190, 2);
}
  
void show_time() {

  char buf[4]; 
  unsigned long timeSec = timeRun;  
  unsigned long timeMin = timeRun / 60; 
  unsigned long timeHour = timeMin / 60; 
  timeSec %= 60; 
  timeMin %= 60;
  timeHour %= 10;

  display.setTextSize(1);
  display.setFont(&Open_Sans_Regular_45);
  display.fillRect(60, 215, 160, 37, DISPLAY_COL_DIV);

  sprintf(buf, "%01d:", timeHour);
  display.setCursor(60, 250);  
  display.println(buf);
  
  sprintf(buf, "%02d:", timeMin);
  display.setCursor(100, 250);  
  display.println(buf);
  
  sprintf(buf, "%02d", timeSec);
  display.setCursor(165, 250);  
  display.println(buf);
}

void show_distance() {
  display.setTextSize(1);
  display.setFont(&Open_Sans_Regular_45);
  distCount+= 17; 
  if (distCount > 50000)
    distCount = 0; 
  print_aligned(display, distCount, 130, 300, 5);
}


void show_heart() {
  display.setTextSize(1);
  display.setFont(&Open_Sans_Regular_45);
  n+= 1; 
  if (n > 200)
    n = 0; 
  print_aligned(display, n, 340, 250, 3);
}

void show_energy() {
  display.setTextSize(1);
  display.setFont(&Open_Sans_Regular_45);
  k+= 3; 
  if (k > 5000)
    k = 0; 
  print_aligned(display, k, 340, 300, 4);
}

void show_init() {
  display.setTextSize(1);
  display.setTextColor(DISPLAY_COL_TEXT, DISPLAY_COL_BG);
  display.setFont(&Open_Sans_Regular_24);

  display.setCursor(20, 40);  
  display.println("mode: ");
  display.setCursor(135, 40);  
  display.println("standard");

  display.setCursor(20, 65);  
  display.println("runner: ");
  display.setCursor(135, 65);  
  display.println("martin");

//  display.setCursor(280, 40);  
//  display.println("mode: ");
//  display.setCursor(360, 40);  
//  display.println("continous");

  display.setFont(&Open_Sans_Regular_24);

  display.setCursor(220, 190);  
  display.println("km/h");

  display.setCursor(423, 190);  
  display.println("%");

  display.setCursor(200, 300);  
  display.println("m");

  display.setCursor(411, 250);  
  display.println("BPM");

  display.setCursor(411, 300);  
  display.println("kcal");


}
  
