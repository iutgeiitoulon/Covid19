#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <splash.h>
#include <TimeLib.h>

#include "EEPROM.h"

//D4 est le bouton haut
//D5 est le bouton gauche
//D6 est le bouton DROIT
//Par defaut: SW1=L = 4

//#define BOUTON_BLEU4

// SW2=U = 5
//SW3=R = 6
#ifdef BOUTON_BLEU4
#define SW1 4   
#define SW2 5   //UP  
#define SW3 6   //RIGHT
#else
#define SW1 5   
#define SW2 4   //UP  
#define SW3 6   //RIGHT
#endif

//D11 commande de TIR
//D10 commande de demarrage hacheur boost
#define TIR_PIN 16
#define EN_BOOST 10
#define LED A2


Adafruit_SSD1306 display(4);
/** Énumération des boutons utilisables */
enum {
  BUTTON_U,    /*!< Button UP (haut) */
  BUTTON_L,  /*!< Button LEFT (gauche) */
  BUTTON_R, /*!< Button RIGHT (droite) */
  BUTTON_N, /* Button None*/
};




int etat_haut=10;
int etat_bas=10;


void setup() {
//  EEPROM.write(100,0);
//  EEPROM.write(101,5);
//  EEPROM.write(102,0);
//  EEPROM.write(103,0);
  EEPROM.write(110,0);
  EEPROM.write(111,1);
  EEPROM.write(112,1);
  EEPROM.write(113,0);
  EEPROM.write(114,0);
  EEPROM.write(115,0);
  EEPROM.write(120,0);
  EEPROM.write(121,1);
  EEPROM.write(122,1);
  EEPROM.write(123,0);
  EEPROM.write(124,0);
  EEPROM.write(125,0);
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(9600);
  delay(2000);
  Serial.println("init");
  Serial.println(now());
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  pinMode(EN_BOOST, OUTPUT);
  digitalWrite(EN_BOOST,HIGH);
  pinMode(TIR_PIN, OUTPUT);
  digitalWrite(TIR_PIN,LOW);
  pinMode(LED, OUTPUT);
  digitalWrite(LED,LOW);
  char message[19];
  int number ;

  pinMode(SW1, INPUT_PULLUP);
  digitalWrite(SW1,HIGH);
  pinMode(SW2, INPUT_PULLUP);
  digitalWrite(SW2,HIGH);
  pinMode(SW3, INPUT_PULLUP);
  digitalWrite(SW3,HIGH);

  for ( int count = 0; count < 2 ; count++)
  {
    display.clearDisplay();
    sprintf( message, "V Auto");
    display.print(message);
    display.display();
    delay(500);
    display.clearDisplay();
    display.setCursor(0, 0);
    display.display();
    delay(500);
  }

  for(int blinkCount=0;blinkCount<3;blinkCount++)
  {
    digitalWrite(LED,HIGH);
    delay(250);
    digitalWrite(LED,LOW);
    delay(250);
  }
}




void loop() {
  int s = select_mode();
  int t;
  long seconds;
  /////////////////////////// SELECTION OF TIME ////////////////////////////
  if(s == 1) t = select_time();
  else t = select_date();

  if (t == -1){
    aborting();
    return;
  }
  else if(t == -2)
  {
    //On ne fait rien, on passe a l'et^pe computation time
  }
  else
  {
    int v;
    /////////////////////////// VALIDATION OF TIME ////////////////////////////
    if (s == 1) v = validation_time();
    else v = validation_date();
  
    if (v == -1){
      aborting();
      return;
    }
  }
  /////////////////////////// COMPUTATION OF TIME ////////////////////////////
  if (s == 1) seconds = compute_sec_time();
  else seconds = compute_sec_date();

  Serial.print("Seconds before fire : ");
  Serial.println(seconds);

  /////////////////////////// COUNTING ///////////////////////////////////////
  if (seconds == -1){
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("TROP COURT\n 5\" mini");
    display.display();
    delay(5000);
    aborting();//less than 5 min
    return;
  }
  else{
    countdown(seconds);
    loading();//Fonction de generation pwm pour charge Boost
    sending();

    //On affiche mission finished a l'ecran, et on bloque l'application
    missionFinished();
  }


  //////////////////////////// LOADING ///////////////////////////////////////
  
  
  //////////////////////////// SENDING ///////////////////////////////////////
  
}




int select_mode(){
  Serial.println(hour());
  Serial.print("Minutes : ");
  Serial.println(minute(now()));
  bool continu = true;
  int message = 1;

  
//  while(continu){
//    display.clearDisplay();
//    display.setCursor(0, 0);
//    if (message == 0)
//      display.print("Countdwn\nDate<-");
//    else
//      display.print("Countdwn<-\nDate");
//    display.display();
//    delay(10);
//    switch (getPressedButton()) {
//      
//
//      
//      case BUTTON_N:
//      Serial.println("N");
//        break;
//  
//      case BUTTON_R:
//      Serial.println("R");
//      delay(100);
//        return message;
//        break;
//  
//      case BUTTON_U:
//      Serial.println("U");
//      delay(100);
//        message = (message + 1) % 2;
//        break;
//  
//      case BUTTON_L:
//      Serial.println("L");
//      delay(100);          
//        break;
//    }
//  }
  return 1;
}

int select_time(){
  int t2 = EEPROM.read(100);
  int t1 = EEPROM.read(101);
  int t0 = EEPROM.read(102)*256 + EEPROM.read(103);
  bool selected = false;
  
  char message[10];
  int cursorPos = 0;
  int cst = 12;
  int cursorTab[7] = {0, cst, cst*2, cst*4, cst*5, cst*7, cst*8};

  long timeOfEntry=millis();
  bool noActivity=true;
  while(selected == false){
    //Si on a pas d'appuis bouton pendant les 30 premieres sec, on demarre avec le dernier programme enregistré
    if(millis()-timeOfEntry>30000 && noActivity==true)
    {
      return -2;
    }
    display.clearDisplay();
    display.setCursor(0, 0);
    sprintf(message, "%.3d:%.2d:%.2d", t0, t1, t2);
    display.print(message);
    display.setCursor(cursorTab[cursorPos],16);
    display.print("^");
    display.display();


    switch (getPressedButton()) {
      

      
      case BUTTON_N:
      Serial.println("N");
        break;
  
      case BUTTON_R:
      noActivity=false;
      Serial.println("R");
      delay(100);
        if (cursorPos == 6){
          EEPROM.write(100,t2);
          EEPROM.write(101,t1);
          EEPROM.write(102,t0/256);
          EEPROM.write(103,t0%256);
          return t0+t1*60+t2*3600;
        }
        else{
          cursorPos++;
        }
        break;
  
      case BUTTON_U:
      noActivity=false;
      Serial.println("U");
      delay(100);
        switch(cursorPos){
          case 0:
          t0 = (t0 % 100 + (((t0 / 100) + 1)%10) *100 );break;
          case 1:
          t0 = (((t0/100)%10)*100 + (((t0/10+1))%10)*10 + (t0%10)) ;break;
          case 2:
          t0 = (((t0/100)%10)*100 + ((t0/10)%10)*10 + ((t0+1)%10));break;
          case 3:
          t1 = (t1 + 10) % 60;break;
          case 4:
          t1 = ((t1/10)*10 + (((t1%10)+1)%10) ) % 60;break;
          case 5:
          t2 = (t2 + 10) % 60;break;
          case 6:
          t2 = ((t2/10)*10 + (((t2%10)+1)%10) ) % 60;break;
          
        }
        break;
  
      case BUTTON_L:
      noActivity=false;
      Serial.println("L");
      delay(100);
        if (cursorPos == 0){
          EEPROM.write(100,t2);
          EEPROM.write(101,t1);
          EEPROM.write(102,t0/256);
          EEPROM.write(103,t0%256);
          return -1;
        }
        else{
          cursorPos--;
        }     
        break;
    }
  }
  
  return 0;
}

int select_date(){
  int t2 = EEPROM.read(110)%30;
  int t1 = EEPROM.read(111)%30;
  int t0 = EEPROM.read(112)%99;
  bool selected = false;

  ////////////////////////////// SETTING REFERENCE DATE ///////////////////////////////
  char message[15];
  int cursorPos = 0;
  int cst = 12;
  int cursorTab[6] = {0, cst, cst*3, cst*4, cst*6, cst*7};

  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Ref Date");
  display.display();
  delay(500);
  while(selected == false){
    display.clearDisplay();
    display.setCursor(0, 0);
    sprintf(message, "%.2d/%.2d/%.2d R", t0, t1, t2);
    display.print(message);
    display.setCursor(cursorTab[cursorPos],16);
    display.print("^");
    display.setCursor(9*12,16);
    display.print("D");
    display.display();

    switch (getPressedButton()) {
      

      
      case BUTTON_N:
      Serial.println("N");
        break;
  
      case BUTTON_R:
      Serial.println("R");
        if (cursorPos == 5){
          EEPROM.write(110,t2);
          EEPROM.write(111,t1);
          EEPROM.write(112,t0);
          selected = true;
        }
        else{
          cursorPos++;
        }
        break;
  
      case BUTTON_U:
      Serial.println("U");
        switch(cursorPos){
          case 0:
          t0 = (t0 + 10)%40;
          if (t0 > 31) t0 = 30; 
          if (t0 == 0) t0 = 1;break;
          case 1:
          t0 = ((t0/10) * 10 + ((t0%10) + 1)%10)%40;
          if (t0 > 31) t0 = 30; 
          if (t0 == 0) t0 = 1;break;
          
          case 2:
          t1 = (t1 + 10)%20;
          if (t1 > 12) t1 = 10; 
          if(t1 == 0) t1 = 1;break;
          case 3:
          t1 = ((t1/10) * 10 + ((t1%10) + 1)%10)%20;
          if (t1 > 12) t1 = 10; 
          if(t1 == 0) t1 = 1;break;
          
          case 4:
          t2 = (t2 + 10) % 100;break;
          case 5:
          t2 = ((t2/10)*10 + (((t2%10)+1)%10)) % 100;break;
          
        }
        break;
  
      case BUTTON_L:
      Serial.println("L");
        if (cursorPos == 0){
          EEPROM.write(110,t2);
          EEPROM.write(111,t1);
          EEPROM.write(112,t0);
          return -1;
        }
        else{
          cursorPos--;
        }     
        break;
    }
  }
  ///////////////////////////////////// SETTING REFERENCE HOUR //////////////////
  t2 = EEPROM.read(113)%24;
  t1 = EEPROM.read(114)%60;
  t0 = EEPROM.read(115)%60;
  selected = false;
  cursorPos = 0;
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Ref Hour");
  display.display();
  delay(500);
  while(selected == false){
    display.clearDisplay();
    display.setCursor(0, 0);
    sprintf(message, "%.2d:%.2d:%.2d R", t0, t1, t2);
    display.print(message);
    display.setCursor(cursorTab[cursorPos],16);
    display.print("^");
    display.setCursor(9*12,16);
    display.print("H");
    display.display();


    switch (getPressedButton()) {
      

      
      case BUTTON_N:
      Serial.println("N");
        break;
  
      case BUTTON_R:
      Serial.println("R");
        if (cursorPos == 5){
          EEPROM.write(113,t2);
          EEPROM.write(114,t1);
          EEPROM.write(115,t0);
          selected = true;
        }
        else{
          cursorPos++;
        }
        break;
  
      case BUTTON_U:
      Serial.println("U");
        switch(cursorPos){
          case 0:
          t0 = (t0 + 10)%30;
          if (t0 > 23) t0 = 20;break;
          case 1:
          t0 = ((t0/10) * 10 + ((t0%10) + 1)%10)%30;
          if (t0 > 23) t0 = 20;break;
          
          case 2:
          t1 = (t1 + 10) % 60;break;
          case 3:
          t1 = ((t1/10)*10 + (((t1%10)+1)%10) ) % 60;break;
          
          case 4:
          t2 = (t2 + 10) % 60;break;
          case 5:
          t2 = ((t2/10)*10 + (((t2%10)+1)%10) ) % 60;break;
          
        }
        break;
  
      case BUTTON_L:
      Serial.println("L");
        if (cursorPos == 0){
          EEPROM.write(113,t2);
          EEPROM.write(114,t1);
          EEPROM.write(115,t0);
          return -1;
        }
        else{
          cursorPos--;
        }     
        break;
    }
  }

  ////////////////////////////// SETTING FIRE DATE ///////////////////////////////
  t2 = EEPROM.read(120)%24;
  t1 = EEPROM.read(121)%60;
  t0 = EEPROM.read(122)%60;
  selected = false;
  cursorPos = 0;
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Fire Date");
  display.display();
  delay(500);
  while(selected == false){
    display.clearDisplay();
    display.setCursor(0, 0);
    sprintf(message, "%.2d/%.2d/%.2d F", t0, t1, t2);
    display.print(message);
    display.setCursor(cursorTab[cursorPos],16);
    display.print("^");
    display.setCursor(9*12,16);
    display.print("D");
    display.display();

    
    switch (getPressedButton()) {
      

      
      case BUTTON_N:
      Serial.println("N");
        break;
  
      case BUTTON_R:
      Serial.println("R");
        if (cursorPos == 5){
          EEPROM.write(120,t2);
          EEPROM.write(121,t1);
          EEPROM.write(122,t0);
          selected = true;
        }
        else{
          cursorPos++;
        }
        break;
  
      case BUTTON_U:
      Serial.println("U");
        switch(cursorPos){
          case 0:
          t0 = (t0 + 10)%40;
          if (t0 > 31) t0 = 30; 
          if (t0 == 0) t0 = 1;break;
          case 1:
          t0 = ((t0/10) * 10 + ((t0%10) + 1)%10)%40;
          if (t0 > 31) t0 = 30; 
          if (t0 == 0) t0 = 1;break;
          
          case 2:
          t1 = (t1 + 10)%20;
          if (t1 > 12) t1 = 10; 
          if(t1 == 0) t1 = 1;break;
          case 3:
          t1 = ((t1/10) * 10 + ((t1%10) + 1)%10)%20;
          if (t1 > 12) t1 = 10; 
          if(t1 == 0) t1 = 1;break;
          
          case 4:
          t2 = (t2 + 10) % 100;break;
          case 5:
          t2 = ((t2/10)*10 + (((t2%10)+1)%10)) % 100;break;
          
        }
        break;
  
      case BUTTON_L:
      Serial.println("L");
        if (cursorPos == 0){
          EEPROM.write(120,t2);
          EEPROM.write(121,t1);
          EEPROM.write(122,t0/256);
          return -1;
        }
        else{
          cursorPos--;
        }     
        break;
    }
  }
  ///////////////////////////////////// SETTING FIRE HOUR //////////////////
  t2 = EEPROM.read(123)%24;
  t1 = EEPROM.read(124)%60;
  t0 = EEPROM.read(125)%60;
  selected = false;
  cursorPos = 0;
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Fire Hour");
  display.display();
  delay(500);
  while(selected == false){
    display.clearDisplay();
    display.setCursor(0, 0);
    sprintf(message, "%.2d:%.2d:%.2d F", t0, t1, t2);
    display.print(message);
    display.setCursor(cursorTab[cursorPos],16);
    display.print("^");
    display.setCursor(9*12,16);
    display.print("H");
    display.display();


    switch (getPressedButton()) {
      

      
      case BUTTON_N:
      Serial.println("N");
        break;
  
      case BUTTON_R:
      Serial.println("R");
        if (cursorPos == 5){
          selected = true;
          EEPROM.write(123,t2);
          EEPROM.write(124,t1);
          EEPROM.write(125,t0);
        }
        else{
          cursorPos++;
        }
        break;
  
      case BUTTON_U:
      Serial.println("U");
        switch(cursorPos){
          case 0:
          t0 = (t0 + 10)%30;
          if (t0 > 23) t0 = 20;break;
          case 1:
          t0 = ((t0/10) * 10 + ((t0%10) + 1)%10)%30;
          if (t0 > 23) t0 = 20;break;
          
          case 2:
          t1 = (t1 + 10) % 60;break;
          case 3:
          t1 = ((t1/10)*10 + (((t1%10)+1)%10) ) % 60;break;
          
          case 4:
          t2 = (t2 + 10) % 60;break;
          case 5:
          t2 = ((t2/10)*10 + (((t2%10)+1)%10) ) % 60;break;
          
        }
        break;
  
      case BUTTON_L:
      Serial.println("L");
        if (cursorPos == 0){
          EEPROM.write(123,t2);
          EEPROM.write(124,t1);
          EEPROM.write(125,t0);
          return -1;
        }
        else{
          cursorPos--;
        }     
        break;
    }
  }
  
  return 0;

}

int validation_time(){
  int t2 = EEPROM.read(100);
  int t1 = EEPROM.read(101);
  int t0 = EEPROM.read(102)*256+EEPROM.read(103);
  bool selected = false;
  bool m = false; 
  char message[10];
  bool valid1 = true;
  bool sure = false;

  while(valid1){

    display.clearDisplay();
    display.setCursor(0, 0);
    sprintf(message, "%.3d:%.2d:%.2d", t0, t1, t2);
    display.print(message);
    display.setCursor(0,16);
    if(sure){
      if(m) display.print("Sure? Yes");
      else display.print("Sure? No");
    }
    else{
      if(m) display.print("Valid? Yes");
      else display.print("Valid? No");
    }
    display.display();
    switch (getPressedButton()) {
      case BUTTON_N:
        break;

      case BUTTON_R:
        if (m == false) return -1;
        if(sure == true) return 0;
        m = false;
        sure = true;
        delay(100);
        break;

      case BUTTON_U:
        m = !m;
        delay(100);
        break;

      case BUTTON_L:
        return -1;
        delay(100);
        break;
        
    }
  }

  return 0;
}

int validation_date(){
  int t2 = EEPROM.read(110);
  int t1 = EEPROM.read(111);
  int t0 = EEPROM.read(112);
  bool selected = false;
  bool m = false; 
  char message[10];
  bool valid1 = true;
  bool sure = false;

  while(valid1){

    display.clearDisplay();
    display.setCursor(0, 0);
    sprintf(message, "%.2d/%.2d/%.2d R", t0, t1, t2);
    display.print(message);
    display.setCursor(0,16);
    if(sure){
      if(m) display.print("Sure? Yes");
      else display.print("Sure? No");
    }
    else{
      if(m) display.print("Valid? Yes");
      else display.print("Valid? No");
    }
    display.display();
    switch (getPressedButton()) {
      case BUTTON_N:
        break;

      case BUTTON_R:
        if (m == false) return -1;
        if(sure == true) valid1 = false;
        m = false;
        sure = true;
        break;

      case BUTTON_U:
        m = (m + 1) % 2;
        break;

      case BUTTON_L:
        return -1;
        break;
        
    }
  }

  t2 = EEPROM.read(113);
  t1 = EEPROM.read(114);
  t0 = EEPROM.read(115);
  m = false; 
  valid1 = true;
  sure = false;

  while(valid1){

    display.clearDisplay();
    display.setCursor(0, 0);
    sprintf(message, "%.2d:%.2d:%.2d R", t0, t1, t2);
    display.print(message);
    display.setCursor(0,16);
    if(sure){
      if(m) display.print("Sure? Yes");
      else display.print("Sure? No");
    }
    else{
      if(m) display.print("Valid? Yes");
      else display.print("Valid? No");
    }
    display.display();
    switch (getPressedButton()) {
      case BUTTON_N:
        break;

      case BUTTON_R:
        if (m == false) return -1;
        if(sure == true) valid1 = false;
        m = false;
        sure = true;
        break;

      case BUTTON_U:
        m = (m + 1) % 2;
        break;

      case BUTTON_L:
        return -1;
        break;
        
    }
  }

  

  t2 = EEPROM.read(120);
  t1 = EEPROM.read(121);
  t0 = EEPROM.read(122);
  bool valid2 = true;
  sure = false;
  m = false;
  while(valid2){

    display.clearDisplay();
    display.setCursor(0, 0);
    sprintf(message, "%.2d/%.2d/%.2d F", t0, t1, t2);
    display.print(message);
    display.setCursor(0,16);
    if(sure){
      if(m) display.print("Sure? Yes");
      else display.print("Sure? No");
    }
    else{
      if(m) display.print("Valid? Yes");
      else display.print("Valid? No");
    }
    display.display();
    switch (getPressedButton()) {
      case BUTTON_N:
        break;

      case BUTTON_R:
        if (m == false) return -1;
        if(sure == true) valid2 = false;
        m = false;
        sure = true;
        break;

      case BUTTON_U:
        m = (m + 1) % 2;
        break;

      case BUTTON_L:
        return -1;
        break;
        
    }
  }

  t2 = EEPROM.read(123);
  t1 = EEPROM.read(124);
  t0 = EEPROM.read(125);
  m = false; 
  valid1 = true;
  sure = false;

  while(valid1){

    display.clearDisplay();
    display.setCursor(0, 0);
    sprintf(message, "%.2d:%.2d:%.2d R", t0, t1, t2);
    display.print(message);
    display.setCursor(0,16);
    if(sure){
      if(m) display.print("Sure? Yes");
      else display.print("Sure? No");
    }
    else{
      if(m) display.print("Valid? Yes");
      else display.print("Valid? No");
    }
    display.display();
    switch (getPressedButton()) {
      case BUTTON_N:
        break;

      case BUTTON_R:
        if (m == false) return -1;
        if(sure == true) valid1 = false;
        m = false;
        sure = true;
        break;

      case BUTTON_U:
        m = (m + 1) % 2;
        break;

      case BUTTON_L:
        return -1;
        break;
        
    }
  }
  return 0;
}


long compute_sec_date(){
  tmElements_t T1,T2;
  T1.Hour = EEPROM.read(115);
  T1.Minute = EEPROM.read(114);
  T1.Second = EEPROM.read(113);
  T1.Day = EEPROM.read(112);
  T1.Month = EEPROM.read(111);
  T1.Year = EEPROM.read(110) + 30;

  T2.Hour = EEPROM.read(125);
  T2.Minute = EEPROM.read(124);
  T2.Second = EEPROM.read(123);
  T2.Day = EEPROM.read(122);
  T2.Month = EEPROM.read(121);
  T2.Year = EEPROM.read(120) + 30;

  time_t T1sec = makeTime(T1);
  time_t T2sec = makeTime(T2);

  Serial.print("Time 1 : ");
  Serial.println(T1.Day);

  Serial.print("Time 2 : ");
  Serial.println(T2.Day);
  
  Serial.print("Diff time : ");
  Serial.println(T2sec - T1sec);
  if (T2sec - T1sec < 300){
    return -1;
  }
  return T2sec - T1sec;
  
}

long compute_sec_time(){
  long t2 = EEPROM.read(100);
  long t1 = EEPROM.read(101);
  long t0 = EEPROM.read(102)*256+EEPROM.read(103);

  if (t0 * 3600 + t1 * 60 + t2 < 300){
    return -1;//less than 5 min
  }

  return t0 * 3600 + t1 * 60 + t2;
}

//Fonction d'attente
void countdown(long seconds){
  Serial.print("Waiting for seconds : ");
  Serial.println(seconds - 60L );
  Serial.print("Waiting for milis : ");
  Serial.println((seconds - 60 )*1000L);
  int h = seconds / 3600;
  int m = (seconds - 3600 * h) / 60;
  int s = (seconds - 3600 * h - 60 * m);
  char message[50];
  for (int i = 0; i < 15 and false; i++){
    delay(1000);
    display.clearDisplay();
    display.setCursor(0, 0);
    if (s - i < 0){
      sprintf(message, "%.3d:%.2d:%.2d", h, m - 1, s - i + 60);
    }
    else{
      sprintf(message, "%.3d:%.2d:%.2d", h, m, s - i);
    }
    display.print(message);
    display.display();    
  }
  display.clearDisplay();
  display.display();

  long curTime = millis();
  //Etape attente
  while ((millis() - curTime)/1000 < seconds - 60L){
    display.setCursor(0, 0);
    display.clearDisplay();
       int h = (seconds-(int)((millis() - curTime)/1000)) / 3600;
       int m = (int)((seconds-(int)((millis() - curTime)/1000)) - 3600 * h) / 60;
       int s = (seconds-(int)((millis() - curTime)/1000)) - 3600 * h - 60 * m;
      sprintf(message, "%.3d:%.2d:%.2d", h, m, s);
      Serial.println("=======");
      Serial.println(h);
      Serial.println(m);
      Serial.println(s);
      Serial.println("Waiting");
      display.print(message);
      display.display();
      //On fait clignoter la LED
      digitalWrite(LED,HIGH);
      delay(250);
      digitalWrite(LED,LOW);
      delay(250);    
  }
}

//fonction de charge de la capacité
int loading(){
  long curTime = millis();
  Serial.print(curTime);
  Serial.println("      cc");
  int onOff = 1;
  //On active le hacheur boost
  digitalWrite(EN_BOOST,LOW);
  while (millis() - curTime < 60000 ){
   //On attend 60 sec que la capa se charge
   digitalWrite(LED, HIGH);
   delay(250);
   digitalWrite(LED, LOW);
   delay(250);
   display.setCursor(0, 0);
    display.clearDisplay();
    char message[40];
    int s=(60000L-(millis() - curTime))/1000;
    sprintf(message, "00:00:%.2d",s);
    Serial.println("Waiting");
    display.print(message);
    display.display();
  }
  //Desactivation Hacheur
  digitalWrite(EN_BOOST,HIGH);
  return 0;
}

//Fonction de tir
int sending(){
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("FIRE");
  display.display(); 
  //delay(2000); 
  digitalWrite(TIR_PIN,HIGH);//
  digitalWrite(LED, HIGH);
  delay(10000);
  digitalWrite(TIR_PIN,LOW);
  digitalWrite(LED, LOW);
  return 0;
}

void missionFinished()
{
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Mission");
  display.setCursor(0, 15);
  display.print("Terminee");
  display.display(); 
  
  //On bloque le system jusqu'au prochain cycle d'alimentation
  while(1);
}


void aborting(){
  Serial.println("ABORTING");
}


//Gestion des boutons actif a l'etat bas
int getPressedButton()
{
  if (digitalRead(SW1)== LOW){
    delay(200);
    return BUTTON_L;
  }
  else if (digitalRead(SW2)== LOW){
    delay(200);
    return BUTTON_U;
  }
  else if (digitalRead(SW3)== LOW){
    delay(200);
    return BUTTON_R;
  }
  else
  return BUTTON_N;
}
