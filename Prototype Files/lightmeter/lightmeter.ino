  #include <Adafruit_GFX.h>
  #include <Adafruit_SSD1306.h>
  #include <Wire.h>
  #include <EEPROM.h>
  #define SCREEN_WIDTH 128 // OLED display width, in pixels
  #define SCREEN_HEIGHT 64 // OLED display height, in pixels
  Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

  // Button pins
  const int measureb = 15;
  const int plusb = 14;
  const int minusb = 16;

  // EEPROM addresses for remembered settings
  const int aindexaddr = 0;
  const int sindexaddr = 1;
  const int isoindexaddr = 2;

  // Variables
  float volts;
  float microamps;
  float lux;
  float aperture[] = {1.4,2.0,2.8,4.0,5.6,8.0,11.0,16.0,22.0};
  int aindex = 0; // Aperture set in Aperture Priority
  int shutterspd[] = {30,15,8,4,2,1,2,4,8,15,30,60,125,250,500,1000};
  int sindex = 12; // Shutter speed set in Shutter Priority (1/125s)
  int iso[] = {50,100,200,400,800,1600,3200,6400};
  int isoindex = 0;
  int ev = 0;
  int x;
  int count; // Shutter speed reading in Aperture Priority
  int acount; // Aperture reading in Shutter Priority
  int firstcheck;
  bool shuttermode; // false = Aperture Priority, true = Shutter Priority

void setup() {
  Serial.begin(115200);

  pinMode(measureb, INPUT_PULLUP);
  pinMode(plusb, INPUT_PULLUP);
  pinMode(minusb, INPUT_PULLUP);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }

  display.clearDisplay();
  display.setRotation(2);

  // Load remembered settings, keeping defaults if EEPROM is blank (0xFF) or invalid
  if (EEPROM.read(aindexaddr) <= 8){
    aindex = EEPROM.read(aindexaddr);
  }
  if (EEPROM.read(sindexaddr) <= 15){
    sindex = EEPROM.read(sindexaddr);
  }
  if (EEPROM.read(isoindexaddr) <= 7){
    isoindex = EEPROM.read(isoindexaddr);
  }

  // Mode select: - for Shutter Priority, + for Aperture Priority
  display.drawRect(0, 0, 128, 64, WHITE); // Container frame
  display.drawFastHLine(0, 13, 128, WHITE); // Title divider
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(34, 3); // Centered in title bar, 2px gap above and below
  display.print("lightmeter");
  display.setTextSize(4);
  display.setCursor(17, 24);
  display.print("S");
  display.setCursor(91, 24);
  display.print("A");
  display.setTextSize(3);
  display.setCursor(45, 28);
  display.print("<");
  display.setCursor(68, 28);
  display.print(">");
  display.display();

  while (digitalRead(minusb) == HIGH && digitalRead(plusb) == HIGH){
    delay(10);
  }
  shuttermode = (digitalRead(minusb) == LOW);

  while (digitalRead(minusb) == LOW || digitalRead(plusb) == LOW){ // Wait for release so the pick doesn't adjust settings
    delay(10);
  }
  display.clearDisplay();
}

void printShutter(int index) {
  if (index <= 5){
    display.print(shutterspd[index]);
    display.print("s");
  }
  else{
    display.print("1/");
    display.print(shutterspd[index]);
    display.print("s");
  }
}

void loop() {
  volts = analogRead(A1) * 5.0 / 1024.0;
  microamps = 1000000 * (volts / 10000.0); // 10k resistor onboard
  lux = microamps * 2.0;

  x = 2.5;
  ev = 0;
  while (lux > x + (x/2)){
    x = x * 2;
    ev++;
  }

  //Button Commands
  if (digitalRead(measureb) == LOW){// Take Measurement
    if (digitalRead(plusb) == LOW){ // +1 stop ISO
      if (isoindex < 7){
        isoindex++;
      }
    }

    if (digitalRead(minusb) == LOW){ // -1 stop ISO
      if (isoindex > 0){
        isoindex--;
      }
    }

    firstcheck = 1; // To show lines on first boot
    if (shuttermode){
      acount = ev - 1; // First ISO entry is 50 iso, 1 stop under normal
      acount = acount + isoindex; // Account for ISO
      acount = acount - sindex; // Shutter 5 is 1s | F/1.4 at EV1 with ISO 100
      acount = acount + 4; // Inverse of the Aperture Priority formula
    }
    else{
      count = ev - 1; // First ISO entry is 50 iso, 1 stop under normal
      count = count + isoindex; // Account for ISO
      count = count - aindex; // Aperture 0 is F/1.4 | 1 sec exp. at EV1 with ISO 100
      count = count + 4; // Because 1s is in Array[5]
    }
  }

  if (digitalRead(measureb) == HIGH){// Only increase if measure button isn't pressed
    // Before the first reading there's nothing to compensate, so settings move freely
    if (shuttermode){
      if (digitalRead(plusb) == LOW){
        if (sindex < 15){
          if (firstcheck == 0 || acount > 0){
          sindex++;
          acount--;
          }
        }
      }

      if (digitalRead(minusb) == LOW){
        if (sindex > 0){
          if (firstcheck == 0 || acount < 8){
          sindex--;
          acount++;
          }
        }
      }
    }
    else{
      if (digitalRead(plusb) == LOW){
        if (aindex < 8){
          if (firstcheck == 0 || count > 0){
          aindex++;
          count--;
          }
        }
      }

      if (digitalRead(minusb) == LOW){
        if (aindex > 0){
          if (firstcheck == 0 || count < 15){
          aindex--;
          count++;
          }
        }
      }
    }
  }

  // Remember settings through reboots (update only writes changed bytes)
  EEPROM.update(aindexaddr, aindex);
  EEPROM.update(sindexaddr, sindex);
  EEPROM.update(isoindexaddr, isoindex);

  // Header text
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  if (shuttermode){
    display.print("Aperture:");
  }
  else{
    display.print("Shutter Speed:");
  }
  display.setTextSize(2);
  display.setCursor(112, 0);
  if (shuttermode){
    display.print("S");
  }
  else{
    display.print("A");
  }

  // Reading (Aperture or Shutter Speed)
  display.setTextSize(3);
  display.setCursor(0, 12);
  if (firstcheck == 0) {
    display.print("----");
  }
  else if (shuttermode){
    if (acount < 0){ // Needs wider than F/1.4
      display.print("LOW");
    }
    else if (acount > 8){ // Needs narrower than F/22
      display.print("HIGH");
    }
    else {
      display.print("f/");
      display.print(aperture[acount],1);
    }
  }
  else{
    if (count < 0){ // Needs longer than 30s
      display.print("LOW");
    }
    else if (count > 15){ // Needs faster than 1/1000s
      display.print("HIGH");
    }
    else {
      printShutter(count);
    }
  }

  // Display Aperture/Shutter Speed, ISO and EV/LUX
  display.setTextSize(1);
  display.setCursor(0,40);
  if (shuttermode){
    display.print("SHUTTER: ");
    printShutter(sindex);
  }
  else{
    display.print("APERTURE: ");
    display.print(aperture[aindex],1);
  }
  display.setCursor(0, 48);
  display.print("ISO: ");
  display.print(iso[isoindex]);
  display.setCursor(0, 56);
  display.print("IEV/LUX: ");
  display.print(ev);
  display.print(" / ");
  display.print(lux);
  display.display();
  display.clearDisplay();
  delay(75);
}
