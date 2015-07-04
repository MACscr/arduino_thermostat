#include <DHT.h>
#include <LiquidCrystal.h>
#include <EEPROM.h>

// Initialize the library with the numbers of the interface pins
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

int max_panels = 4;
int panel_num = 1;
int last_panel = 0;
int press_count = 0;

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
int read_LCD_buttons()
{
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
  //Set the characters and column numbers.
  lcd.begin(16, 2);
}

void loop() {
// high to turn on backlight and low to turn off
//digitalWrite(BACKLIGHT, HIGH);
  int state = 0;
  //Refresh the button pressed.
  lcd_key = read_LCD_buttons();  // read the buttons
  //Set the Row 0, Col 0 position.
  lcd.setCursor(0,0);

  //Check values from LCD Keypad
  if (lcd_key == 3) {
    //Left
    state = 1;
  } else if (lcd_key == 0){
    //Right
    state = 2;
  } else if (lcd_key == 1){
    //Up
    state = 3;
  } else if (lcd_key == 2){
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
  if (panel_num == 1 || lcd_key != 5) {
    displayPanel(panel_num,lcd_key);
  }
  //Small delay to hopefully help with bouncing
  delay(150);
}

void humidity() {
  // code to get DHT reading coming soon
  int humidity_lvl = 44;
  lcd.print("Humidity: ");
  lcd.print(humidity_lvl);
  lcd.print("%");
}

void temperature(int lcd_key) {
  // code to get DHT reading coming soon
  // current temperature  
  int ctemp = 73;
  lcd.setCursor(0,0);
  lcd.print("Cur Temp: ");
  lcd.print(ctemp);
  // degree symbol
  lcd.print((char)223);

  // requested temperature
  int rtemp = 69;

  // add or increase temp based on up/down arrow
  if (lcd_key == 1) {
    rtemp = rtemp + 1;
  }else if (lcd_key ==2) {
    rtemp = rtemp - 1;
  }
  
  lcd.setCursor(0,1);
  lcd.print("Req Temp: ");
  lcd.print(rtemp);
  // degree symbol
  lcd.print((char)223);
}

void mode(int lcd_key) {
  Serial.println(press_count);
  lcd.print("Mode: ");

  press_count++;
  lcd.setCursor(6,0);
  switch (press_count%3) {
    case 0:
      lcd.print("Cool");
      break;
    case 1:
      lcd.print("Heat");
      break;
    case 2:
      lcd.print("Off ");
      break;
  }
  // when selection above is changed, wait a second
  // (this way it doesnt send the command while cycling through options),
  // then send do the following:
}

void fan(int lcd_key) {
  Serial.print("Key Pressed: ");
  Serial.println(lcd_key);
  lcd.print("Fan: ");

  press_count++;
  lcd.setCursor(5,0);
  switch (press_count%2) {
    case 0:
      lcd.print("Auto");
      break;
    case 1:
      lcd.print("Off ");
      break;
  }
}

//Display Panel Option based on Index.
void displayPanel(int panel_num,int lcd_key) {
     switch (panel_num) {
      case 1:
          temperature(lcd_key);
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
