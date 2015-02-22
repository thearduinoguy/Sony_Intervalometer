// Sony Camera Intervalometer code
// by Mike McRoberts
// Feb 2015

#include <LiquidCrystal.h>
#include <Button.h>

#define DOWN_PIN 9           // Down button pin
#define UP_PIN 8            // Up button pin
#define ENTER_PIN 10        // Enter button pin

#define PULLUP false        
#define INVERT false       
#define DEBOUNCE_MS 20     //A debounce time of 20 milliseconds usually works well for tactile button switches.

#define LONG_PRESS 1000   //ms required before repeating on long press

Button btnUP(UP_PIN, PULLUP, INVERT, DEBOUNCE_MS);    //Declare the buttons
Button btnDOWN(DOWN_PIN, PULLUP, INVERT, DEBOUNCE_MS);
Button btnENTER(ENTER_PIN, PULLUP, INVERT, DEBOUNCE_MS);

int DELAY = 1; // How many seconds till we start
int SHOT = 1;  // How many shots to take
int GAP= 1; // How many seconds inbetween shots
int LENGTH = 1; // Length of shot
// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

int STATE = 0; 
enum {INITIAL, MENU};              //The possible states for the state machine
// 0 = Initial Mode
// 1 = Menu Mode


void menuMode(){
    STATE = MENU; // menu mode
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Menu Mode");
    while (btnENTER.wasReleased()); {}
}

void showInfo(){
    STATE = INITIAL; // info mode
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Del:"); lcd.print(wait); lcd.print("s Gap:"); 
    lcd.print(gap); lcd.print("s");
    lcd.setCursor(0,1);
    lcd.print("Shots:"); lcd.print(shots);
  
    while (btnENTER.wasReleased());{}
}

void setup() {
  Serial.begin(115200);
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.setCursor(4, 0);
  lcd.print("Welcome");
  lcd.setCursor(6,1);
  lcd.print("Doug");
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);
  delay(2000);
  
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Del:"); lcd.print(wait); lcd.print("s Gap:"); 
    lcd.print(gap); lcd.print("s");
    lcd.setCursor(0,1);
    lcd.print("Shots:"); lcd.print(shots);
    STATE = 0; // info mode
    
  Serial.print("Started");
}

void loop() {
  btnUP.read();                             //read the buttons
  btnDOWN.read();
  btnENTER.read();
  if (btnENTER.pressedFor(LONG_PRESS)) {
    Serial.println("Long Press.");
    switch(STATE){
      case 0:
        menuMode();
        break;
      case 1:
        showInfo();
        break;
    }
    delay(2000);
  }
}


