#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <map>

#define IR_SENSOR_PIN 33
#define BUTTON_PIN 32 // <— your added button on GPIO 32
#define TAP_WINDOW 300
#define CHAR_GAP 2000
#define WORD_GAP 5000
#define SENTENCE_GAP 7000

LiquidCrystal_I2C lcd(0x27, 16, 2);

std::map<String, char> morseToChar = {
    {".-", 'A'}, {"-...", 'B'}, {"-.-.", 'C'}, {"-..", 'D'}, {".", 'E'}, {"..-.", 'F'}, {"--.", 'G'}, {"....", 'H'}, {"..", 'I'}, {".---", 'J'}, {"-.-", 'K'}, {".-..", 'L'}, {"--", 'M'}, {".", 'N'}, {"---", 'O'}, {".--.", 'P'}, {"--.-", 'Q'}, {".-.", 'R'}, {"...", 'S'}, {"-", 'T'}, {"..-", 'U'}, {"...-", 'V'}, {".--", 'W'}, {"-..-", 'X'}, {"-.--", 'Y'}, {"--..", 'Z'}, {".-.-", ' '}, {".----", '1'}, {"..---", '2'}, {"...--", '3'}, {"....-", '4'}, {".....", '5'}, {"-....", '6'}, {"--...", '7'}, {"---..", '8'}, {"----.", '9'}, {"-----", '0'}};

const String BACKSPACE_CODE = ".-..-";

String morseCode, decodedMessage;
unsigned long lastSignalTime = 0, tapStartTime = 0;
bool measuringTap = false;
int tapCount = 0;

int prevSensorValue = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;

// remember last button state for edge‐detect
bool lastButtonState = HIGH;

void setup()
{
  pinMode(IR_SENSOR_PIN, INPUT_PULLUP);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  Serial.begin(115200);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Morse Decoder");
  delay(1000);
  lcd.clear();
  lcd.print("Start Input...");
}

void loop()
{
  handleButtonPress(); // check your push button first
  handleMorseInput();
}

// Fires the call/SMS trigger
void triggerCall(const String &reason)
{
  Serial.println("CALL_TRIGGERED");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Calling...");
  lcd.setCursor(0, 1);
  lcd.print(reason);
  delay(1500);
  updateDisplay();
}

// Detect a button‐down event and trigger immediately
void handleButtonPress()
{
  bool btn = digitalRead(BUTTON_PIN);
  if (btn == LOW && lastButtonState == HIGH)
  {
    // on the falling edge (pressed)
    triggerCall("Btn Pressed");
  }
  lastButtonState = btn;
}

// Your existing morse logic
void handleMorseInput()
{
  int sensorValue = digitalRead(IR_SENSOR_PIN);
  unsigned long currentTime = millis();

  if (sensorValue != prevSensorValue)
  {
    lastDebounceTime = currentTime;
  }
  if ((currentTime - lastDebounceTime) > debounceDelay)
  {
    // measure tap
    if (sensorValue == LOW && !measuringTap)
    {
      tapStartTime = currentTime;
      measuringTap = true;
    }
    if (sensorValue == HIGH && measuringTap)
    {
      unsigned long dur = currentTime - tapStartTime;
      measuringTap = false;
      morseCode += (dur < TAP_WINDOW ? "." : "-");
      lastSignalTime = currentTime;
      updateDisplay();
    }

    // char / word gap
    if (sensorValue == HIGH && morseCode.length() > 0 && !measuringTap)
    {
      unsigned long gap = currentTime - lastSignalTime;
      if (gap >= CHAR_GAP && gap < WORD_GAP)
      {
        decodeMorse();
        morseCode = "";
      }
      else if (gap >= WORD_GAP && gap < SENTENCE_GAP)
      {
        decodeMorse();
        decodedMessage += " ";
        morseCode = "";
        updateDisplay();
      }
    }

    // end‐of‐sentence gap: now trigger if Y or button was just pressed
    if ((currentTime - lastSignalTime) >= SENTENCE_GAP && decodedMessage.length() > 0)
    {
      if (decodedMessage == "Y")
      {
        triggerCall("Sent: Y");
      }
      else
      {
        Serial.println("Sentence complete: " + decodedMessage);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Speaking...");
        lcd.setCursor(0, 1);
        lcd.print(
            decodedMessage.length() > 16
                ? decodedMessage.substring(decodedMessage.length() - 16)
                : decodedMessage);
        delay(1500);
        updateDisplay();
      }
      // reset
      decodedMessage = "";
      morseCode = "";
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Start Input...");
    }
  }
  prevSensorValue = sensorValue;
}

void decodeMorse()
{
  if (morseCode == BACKSPACE_CODE)
  {
    if (decodedMessage.length() > 0)
    {
      decodedMessage.remove(decodedMessage.length() - 1);
      Serial.println("[DEL]");
    }
  }
  else if (morseToChar.count(morseCode))
  {
    decodedMessage += morseToChar[morseCode];
  }
  else
  {
    Serial.print("[Invalid Morse]: ");
    Serial.println(morseCode);
  }
  updateDisplay();
}

void updateDisplay()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Morse: " + morseCode);
  lcd.setCursor(0, 1);
  if (decodedMessage.length() > 16)
    lcd.print(decodedMessage.substring(decodedMessage.length() - 16));
  else
    lcd.print(decodedMessage);
}