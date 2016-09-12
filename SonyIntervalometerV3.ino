#include <Button.h>
#include <SoftwareSerial.h>
#include <EEPROM.h>

#define LEFT_PIN 5
#define RIGHT_PIN 4
#define ENTER_PIN 6
#define PULLUP false
#define INVERT false
#define DEBOUNCE_MS 20
#define REPEAT_FIRST 500   //ms required before repeating on long press
#define REPEAT_INCR 100    //repeat shotInterval for long press
#define MIN_COUNT 0
#define MAX_COUNT 999

#define blinkOn       LCD.write(0xFE); LCD.write(0x0D);
#define blinkOff      LCD.write(0xFE); LCD.write(0x0C);
#define clearScreen   LCD.write(0xFE); LCD.write(0x01);
#define bright        LCD.write(0x7C); LCD.write(156); delay(100);

Button btnUP(ENTER_PIN, PULLUP, INVERT, DEBOUNCE_MS);
Button LEFT_BUTTON(LEFT_PIN, PULLUP, INVERT, DEBOUNCE_MS);
Button RIGHT_BUTTON(RIGHT_PIN, PULLUP, INVERT, DEBOUNCE_MS);
Button ENTER_BUTTON(ENTER_PIN, PULLUP, INVERT, DEBOUNCE_MS);

// Attach the serial display's RX line to digital pin 2
SoftwareSerial LCD(3, 2); // pin 2 = TX, pin 3 = RX (unused)

int eepromAddress = 0;

struct dataObject {
  int startDelay;
  int shots;
  int shotLength;
  int shotInterval;
} myData;

boolean flipState = true;
unsigned long lastSecond;

enum {WAIT, INCR, DECR};              //The possible states for the state machine
enum {DELAY, SHOTS, LENGTH, INTERVAL, CHECK, RUN};
uint8_t STATE;                        //The current state machine state
int count = myData.startDelay;                            //The number that is adjusted
int lastCount = -1;                   //Previous value of count (initialized to ensure it's different when the sketch starts)
unsigned long rpt = REPEAT_FIRST;     //A variable time that is used to drive the repeats for long presses
uint8_t MENU_STATE = DELAY;

void setup()
{

  //EEPROM.put(10, myData); delay(100);
  
  EEPROM.get(10, myData); delay(100);

  Serial.begin(115200);
  LCD.begin(9600); // set up serial port for 9600 baud
  delay(500); // wait for display to boot up
  bright
  blinkOff
  clearScreen
  LCD.print("      Sony      ");
  LCD.print(" Intervalometer ");
  delay(1000);
  clearScreen

  menus();
  lastSecond = millis();
  AreYouSure();
}

void moveCursor(uint8_t position) {
  LCD.write(0xFE);
  LCD.write(position);
}

void menus() {
  while (1) {
    if (MENU_STATE >= DELAY && MENU_STATE <= INTERVAL) checkButtons();

    switch (MENU_STATE) {
      case DELAY:
        selectDelay();
        break;
      case SHOTS:
        selectShots();
        break;
      case LENGTH:
        selectshotLength();
        break;
      case INTERVAL:
        selectshotInterval();
        break;
      case CHECK:
        AreYouSure();
        return;
        break;
    }
  }
}

void AreYouSure() {
  int wait = 2000;
  lastSecond = millis();
  while (1) {
    if (flipState)
    {
      info();
      wait = 2000;
    }
    else
    {
      question();
      wait = 1000;
    }

    if ((millis() - lastSecond) >= wait) {
      lastSecond = millis();
      flipState = !flipState;
      clearScreen
    }
    LEFT_BUTTON.read();                             //read the buttons
    RIGHT_BUTTON.read();
    ENTER_BUTTON.read();
    if (LEFT_BUTTON.wasPressed() || RIGHT_BUTTON.wasPressed()) {
      MENU_STATE = DELAY;
      clearScreen
      menus();
    }
    if (ENTER_BUTTON.wasReleased()) {
      clearScreen
      MENU_STATE = RUN;
      EEPROM.put(10, myData);delay(100);
      return;
    }
    Serial.println(millis());
  }
}

void selectDelay() {
  moveCursor(128);
  LCD.print("Length of Delay?");
  moveCursor(196);
  LCD.print(myData.startDelay);
  LCD.print(" Secs ");
}

void selectShots() {
  moveCursor(128);
  LCD.print(" Num Exposures?");
  moveCursor(194);
  LCD.print(myData.shots);
  LCD.print(" Exposures ");
}

void selectshotLength() {
  moveCursor(128);
  LCD.print("Exposure Length?");
  moveCursor(196);
  LCD.print(myData.shotLength);
  LCD.print(" Secs ");
}

void selectshotInterval() {
  moveCursor(128);
  LCD.print(" Shot Interval?");
  moveCursor(196);
  LCD.print(myData.shotInterval);
  LCD.print(" Secs ");
}

void info() {
  moveCursor(128);
  LCD.print("Exp ");
  LCD.print(myData.shots);

  moveCursor(136);
  LCD.print("Len ");
  LCD.print(myData.shotLength);

  moveCursor(192);
  LCD.print("Del ");
  LCD.print(myData.startDelay);

  moveCursor(200);
  LCD.print("Int ");
  LCD.print(myData.shotInterval);
}

void question() {
  moveCursor(128);
  LCD.print("  Are You Sure?");
  moveCursor(192);
  LCD.print("  NO       YES");
}

void checkButtons() {
  LEFT_BUTTON.read();                             //read the buttons
  RIGHT_BUTTON.read();
  ENTER_BUTTON.read();

  if (count != lastCount) {                 //print the count if it has changed
    lastCount = count;
    Serial.println(count, DEC);
  }

  if (ENTER_BUTTON.wasPressed())
  {
    clearScreen
    MENU_STATE++;
    switch (MENU_STATE) {
      case DELAY:
        count = myData.startDelay;
        break;
      case SHOTS:
        count = myData.shots;
        break;
      case LENGTH:
        count = myData.shotLength;
        break;
      case INTERVAL:
        count = myData.shotInterval;
        break;
    }
  }

  switch (STATE) {

    case WAIT:                                //wait for a button event
      if (LEFT_BUTTON.wasPressed())
        STATE = INCR;
      else if (RIGHT_BUTTON.wasPressed())
        STATE = DECR;
      else if (LEFT_BUTTON.wasReleased())         //reset the long press shotInterval
        rpt = REPEAT_FIRST;
      else if (RIGHT_BUTTON.wasReleased())
        rpt = REPEAT_FIRST;
      else if (LEFT_BUTTON.pressedFor(rpt)) {     //check for long press
        rpt += REPEAT_INCR;               //increment the long press shotInterval
        STATE = INCR;
      }
      else if (RIGHT_BUTTON.pressedFor(rpt)) {
        rpt += REPEAT_INCR;
        STATE = DECR;
      }
      break;

    case INCR:                                //increment the counter
      count = min(count++, MAX_COUNT);      //but not more than the specified maximum
      STATE = WAIT;
      break;

    case DECR:                                //decrement the counter
      count = max(count--, MIN_COUNT);      //but not less than the specified minimum
      STATE = WAIT;
      break;
  }
  switch (MENU_STATE) {

    case DELAY:
      myData.startDelay = count;
      break;

    case SHOTS:
      myData.shots = count;
      break;

    case LENGTH:
      myData.shotLength = count;
      break;

    case INTERVAL:
      myData.shotInterval = count;
      break;
  }
}

void loop()
{
  moveCursor(128);
  LCD.print("RUNNING!!!");
  while (1) {}

}
