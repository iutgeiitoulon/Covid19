/**************************************************************************
 This is an example for our Monochrome OLEDs based on SSD1306 drivers

 Pick one up today in the adafruit shop!
 ------> http://www.adafruit.com/category/63_98

 This example is for a 128x32 pixel display using I2C to communicate
 3 pins are required to interface (two I2C and one reset).

 Adafruit invests time and resources providing this open
 source code, please support Adafruit and open-source
 hardware by purchasing products from Adafruit!

 Written by Limor Fried/Ladyada for Adafruit Industries,
 with contributions from the open source community.
 BSD license, check license.txt for more information
 All text above, and the splash screen below must be
 included in any redistribution.
 **************************************************************************/

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SW1 4   //+
#define SW2 5   //-
#define SW3 6   //Bouton valider
#define ON_REG 10
#define TRIGGER 16

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define NUMFLAKES     10 // Number of snowflakes in the animation example

#define LOGO_HEIGHT   16
#define LOGO_WIDTH    16
static const unsigned char PROGMEM logo_bmp[] =
{ B00000000, B11000000,
  B00000001, B11000000,
  B00000001, B11000000,
  B00000011, B11100000,
  B11110011, B11100000,
  B11111110, B11111000,
  B01111110, B11111111,
  B00110011, B10011111,
  B00011111, B11111100,
  B00001101, B01110000,
  B00011011, B10100000,
  B00111111, B11100000,
  B00111111, B11110000,
  B01111100, B11110000,
  B01110000, B01110000,
  B00000000, B00110000 };

void setup() {

  //Init des pin
  pinMode(SW1, INPUT_PULLUP);
  digitalWrite(SW1, HIGH);      //Disable boost
  pinMode(SW2, INPUT_PULLUP);
  digitalWrite(SW2, HIGH);      //Disable boost
  pinMode(SW3, INPUT_PULLUP);
  digitalWrite(SW3, HIGH);      //Disable boost
  pinMode(ON_REG, OUTPUT);
  pinMode(TRIGGER, OUTPUT);
  digitalWrite(ON_REG, LOW);      //Disable boost
  digitalWrite(TRIGGER, LOW);     //Disable TRIGGER

  
  Serial.begin(9600);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
 // display.display();
 // delay(2000); // Pause for 2 seconds

  // Clear the buffer


for(int i=0;i<10;i++)
{
  display.clearDisplay();
  display.setTextSize(2); // Draw 2X-scale text
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 0);
  char str[20];
  sprintf(str,"00:00:%02d",9-i);
  display.println(str);
  //display.setCursor(10, 70);display.print(10-i);
  display.display();      // Show initial text
  delay(1000);
}
display.clearDisplay();
  display.setTextSize(2); // Draw 2X-scale text
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 0);
display.println("BOOM");
display.display();      // Show initial text
}

unsigned char state=0;
unsigned char menuState=0;
int heureUnit=0;
int heureDiz=0;
int minuteUnit=0;
int minuteDiz=0;
int secondeUnit=0;
int secondeDiz=0;
unsigned char selectedDigit=0;
void loop() {

  

  if(digitalRead(SW2)==LOW)
  {
    delay(1000);
  }

  

  switch(state)
  {
    //Menu
    case 0:
      switch(menuState)
      {
          //Affichage selection
          case 0:  display.clearDisplay();
                  display.setCursor(10, 0);
                   display.println("Demo");
                    if(digitalRead(SW2)==LOW)
                    {
                      menuState=1;
                      delay(1000);
                    }
          break;
          case 1:  display.setCursor(10, 0);
                   display.println("Decompte");
          break;
      }
      if(digitalRead(SW3)==LOW)
      {
        //On regle le decompte
        if(menuState==0)
          state=1;      //State reglage decompte
        else if(menuState==1)
          state=3;      //State Demo

          delay(1000);
      }
      
    break;

    //Reglage decompte
    case 1:
    {
      display.clearDisplay();
      display.setCursor(10, 0);
      char str[20];
      sprintf(str,"%02d%02d:%02d%02d:%02d",heureDiz,heureUnit,minuteDiz,minuteUnit,secondeDiz,secondeUnit);
      display.println(str);
      if(digitalRead(SW1)==LOW)
      {
        delay(1000);
        switch(selectedDigit)
        {
          case 0: //Seconde unit
           secondeUnit++;
            if(secondeUnit>9)
            secondeUnit=0;
          break;
          case 1: //Seconde diz
           secondeDiz++;
           if(secondeDiz>5)
            secondeDiz=0;
          break;
          case 2: //minutes unit
           minuteUnit++;
           if(minuteUnit>9)
            minuteUnit=0;
          break;
          case 3: //minutes Diz
           minuteDiz++;
           if(minuteDiz>5)
            minuteDiz=0;
          break;
          case 4: //hour unit
          heureUnit++;
          if(heureUnit>5)
           heureUnit=0;
          break;
          case 5: //hour Diz
           heureDiz++;
           if(heureDiz>2)
              heureDiz=0;
          break;
        }
      }

      if(digitalRead(SW3)==LOW)
      {
        //On valide le digit selectionné
        if(selectedDigit<5)
        {
          selectedDigit++;
        }
        else
        {
          state=10;
          // on demarre le hacheur Boost
        }

          delay(1000);
      }
    }//End case 1
        break;

    case 10://Case running
    {
    display.setCursor(10, 0);
    char str[20];
    sprintf(str,"%02d%02d:%02d%02d:%02d",heureDiz,heureUnit,minuteDiz,minuteUnit,secondeDiz,secondeUnit);
    display.println(str);
    display.println(str);
    }
    break;
  }
  display.display();      // Show initial text
}
