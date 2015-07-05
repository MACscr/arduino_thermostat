#include <DHT.h>
#include <LiquidCrystal.h>
#include <EEPROM.h>

// Initialize the library with the numbers of the interface pins
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

// initialize a few values
int last_panel = 0;
int timer = 0;
int rtemp = 0;
int system_mode = 0;
int fan_mode = 0;
int ctemp = 0;
int humidity_lvl = 0;

int max_panels = 4;
int panel_num = 1;

// address of where were going to store the value in eeprom
int rtemp_address = 0;
int mode_address = 4;
int fan_address = 8;

// define some values used by the panel and buttons
int lcd_key     = 0;
int adc_key_in  = 0;
#define btnRIGHT  0
#define btnUP     1
#define btnDOWN   2
#define btnLEFT   3
#define btnSELECT 4
#define btnNONE   5
#define BACKLIGHT 10

// read the buttons
int read_LCD_buttons() {
 adc_key_in = analogRead(0);      // read the value from the sensor

 // my buttons when read are centered at these valies: 0, 144, 329, 504, 741
 // we add approx 50 to those values and check to see if we are close
 if (adc_key_in > 1000) return btnNONE; // We make this the 1st option for speed reasons since it will be the most likely result

 if (adc_key_in < 50)   return btnRIGHT;  
 if (adc_key_in < 195)  return btnUP; 
 if (adc_key_in < 380)  return btnDOWN; 
 if (adc_key_in < 555)  return btnLEFT; 
 if (adc_key_in < 790)  return btnSELECT;

 return btnNONE;  // when all others fail, return this...
}

void setup() {

  Serial.begin(9600);
  pinMode(BACKLIGHT, OUTPUT);
  //Set the characters and column numbers.
  lcd.begin(16, 2);
  // turn on backlight
  digitalWrite(BACKLIGHT, HIGH);

  // reset rtemp if we have som bizare value by default
  //eeprom_write(rtemp_address, 69);
  //eeprom_write(mode_address, 0);
  //eeprom_write(fan_address, 0);

  // now lets read what we already have saved
  rtemp = eeprom_read(rtemp_address);
  system_mode = eeprom_read(mode_address);
  fan_mode = eeprom_read(fan_address);

  Serial.println("Initial Values");
  Serial.print("rtemp "); 
  Serial.println(rtemp);
  Serial.print("system_mode "); 
  Serial.println(system_mode);
  Serial.print("fan_mode "); 
  Serial.println(fan_mode);
}

void loop() {

  int state = 0;
  // Refresh the button pressed.
  lcd_key = read_LCD_buttons();  // read the buttons

  // check for button activity, if not, turn off backlight
  if (lcd_key < btnNONE) {
    digitalWrite(BACKLIGHT, HIGH);
    timer = 0;
  // if its been awhile since anyone touched a button, lets turn off the backlight
  }else if (timer > 160) {
    digitalWrite(BACKLIGHT, LOW);
  }
  // Set the Row 0, Col 0 position of cursor on LCD
  lcd.setCursor(0,0);

  // Check values from LCD Keypad
  if (lcd_key == btnLEFT) {
    //Left
    state = 1;
  } else if (lcd_key == btnRIGHT){
    //Right
    state = 2;
  } else if (lcd_key == btnUP){
    //Up
    state = 3;
  } else if (lcd_key == btnDOWN){
    //Down
    state = 4;
  }

  // if pretty much ran for the first time, last_panel will be zero, so lets set one
  if (last_panel == 0) {
    last_panel = 1;
  }

  //if last panel set was 1 and left key is pressed, cycle to max panel
  if (last_panel == 1 && state == 1) {
    panel_num = max_panels;
    last_panel = panel_num;
    lcd.clear();

  // if at max panel and right button is pressed, reset to first panel
  }else if (last_panel == max_panels && state == 2) {
    panel_num = 1;
    last_panel = panel_num;
    lcd.clear();

  // if above logic is not met and right button is pressed, add 1 to last_panel
  }else if (state == 2) {
    panel_num = last_panel + 1;
    last_panel = panel_num;
    lcd.clear();

  // if above logic is not met and left button is pressed, subtract 1 from last_pane
  }else if (state == 1) {
    panel_num = last_panel - 1;
    last_panel = panel_num;
    lcd.clear();
  }

  //display panel, but dont keep recreating it if no button is pressed
  if (panel_num == 1 || lcd_key != btnNONE) {
    displayPanel(panel_num,lcd_key);
  }

  // add to timer count (will need adjusted above if delay is adjusted below)
  if (digitalRead(BACKLIGHT) == LOW) {
    timer++;
  }
  //Small delay to hopefully help with bouncing
  delay(150);
}

void humidity() {
  // code to get DHT reading coming soon
  humidity_lvl = 44;
  lcd.print("Humidity: ");
  lcd.print(humidity_lvl);
  lcd.print("%");
}

void dht_temperature() {
  ctemp = 77;
  lcd.setCursor(0,0);
  lcd.print("Cur Temp: ");
  lcd.print(ctemp);
  // degree symbol
  lcd.print((char)223);
}

void requested_temperature(int lcd_key) {

  //rtemp

  // add or increase temp based on up/down arrow
  if (lcd_key == btnUP) {
    rtemp = rtemp + 1;
  }else if (lcd_key == btnDOWN) {
    rtemp = rtemp - 1;
  }

  lcd.print("Req Temp: ");
  lcd.print(rtemp);
  // degree symbol (doesnt print right in openhab apple app, but we are only using for lcd here, so should be fine)
  lcd.print((char)223);

  // whene select button pressed, we save the data to eeprom and print SAVED to lcd
  if (lcd_key == btnSELECT) {
    lcd.clear();
    eeprom_write(rtemp_address, rtemp);
  }
}

void mode(int lcd_key) {

  lcd.print("Mode: ");
  lcd.setCursor(6,0);
  
  // Now change the unsaved mode
  if (lcd_key == btnUP) {
    if (system_mode < 2) system_mode++;
  }
  else if (lcd_key == btnDOWN) {
    if (system_mode > 0) system_mode--;
  }
  switch(system_mode) {
    case 0:
      lcd.print("Off ");
      break;
    case 1:
      lcd.print("Cool");
      break;
    case 2:
      lcd.print("Heat");
      break;
    default:
      // if somehow, mode gets out of range, do something
      // about it here. Sensible fail-safe default may be OFF?
      system_mode = 0;
      break;
  }
  // whene select button pressed, we save the data to eeprom and print SAVED to lcd
  if (lcd_key == btnSELECT) {
    lcd.clear();
    eeprom_write(mode_address, system_mode);
  } 
}

void fan(int lcd_key) {

  //example fan_mode value from eeprom
  //fan_mode = 1;

  lcd.print("Fan: ");
  lcd.setCursor(5,0);
  
  // Now change the unsaved mode
  if (lcd_key == btnUP) {
    if (fan_mode < 1) fan_mode++;
  }
  else if (lcd_key == btnDOWN) {
    if (fan_mode > 0) fan_mode--;
  }
  switch (fan_mode) {
    case 0:
      lcd.print("Auto");
      fan_mode = 0;
      break;
    case 1:
      lcd.print("On  ");
      fan_mode = 1;
      break;
  }

  // whene select button pressed, we save the data to eeprom and print SAVED to lcd
  if (lcd_key == btnSELECT) {
    lcd.clear();
    eeprom_write(fan_address, fan_mode);
  }  
}

void eeprom_write(int address, long value) {
    //Decomposition from a long to 4 bytes by using bitshift.
    //One = Most significant -> Four = Least significant byte
    byte four = (value & 0xFF);
    byte three = ((value >> 8) & 0xFF);
    byte two = ((value >> 16) & 0xFF);
    byte one = ((value >> 24) & 0xFF);
    //Write the 4 bytes into the eeprom memory.
    EEPROM.write(address, four);
    EEPROM.write(address + 1, three);
    EEPROM.write(address + 2, two);
    EEPROM.write(address + 3, one);

    // prit some feedback to the lcd
    lcd.setCursor(6,0);
    lcd.print("SAVED");
    delay(500);
}
long eeprom_read(long address) {
    //Read the 4 bytes from the eeprom memory.
    long four = EEPROM.read(address);
    long three = EEPROM.read(address + 1);
    long two = EEPROM.read(address + 2);
    long one = EEPROM.read(address + 3);
    //Return the recomposed long by using bitshift.
    return ((four << 0) & 0xFF) + ((three << 8) & 0xFFFF) + ((two << 16) & 0xFFFFFF) + ((one << 24) & 0xFFFFFFFF);
}


//Display Panel Option based on Index.
void displayPanel(int panel_num,int lcd_key) {
   switch (panel_num) {
    case 1:
        dht_temperature();
        // set to second row
        lcd.setCursor(0,1);
        requested_temperature(lcd_key);
      break;
    case 2:
        humidity();
      break;
    case 3:
        mode(lcd_key);
      break;
    case 4:
        fan(lcd_key);
      break;
  }
}
