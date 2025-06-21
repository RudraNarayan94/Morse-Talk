/*
 * MorseTalk - Arduino Morse Code Decoder
 *
 * This sketch reads IR sensor interruptions as Morse code input,
 * decodes them into text, and sends the decoded messages through
 * serial communication to a Python script for text-to-speech conversion.
 *
 * Hardware connections:
 * - IR sensor: GPIO pin 33 (detects finger movements)
 * - Button: GPIO pin 32 (emergency trigger)
 * - LCD: I2C connection (SDA, SCL pins)
 *
 * Created: May 2023
 * Last updated: July 2023
 */

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <map>

// Pin definitions and timing constants
#define IR_SENSOR_PIN 33
#define BUTTON_PIN 32     // Emergency button on GPIO 32
#define TAP_WINDOW 300    // Threshold for dot vs dash (milliseconds)
#define CHAR_GAP 2000     // Gap between characters
#define WORD_GAP 5000     // Gap between words
#define SENTENCE_GAP 7000 // Gap to end sentence

LiquidCrystal_I2C lcd(0x27, 16, 2);

// Morse code lookup table - maps morse code to characters
std::map<String, char> morseToChar = {
    {".-", 'A'}, {"-...", 'B'}, {"-.-.", 'C'}, {"-..", 'D'}, {".", 'E'}, {"..-.", 'F'}, {"--.", 'G'}, {"....", 'H'}, {"..", 'I'}, {".---", 'J'}, {"-.-", 'K'}, {".-..", 'L'}, {"--", 'M'}, {".", 'N'}, {"---", 'O'}, {".--.", 'P'}, {"--.-", 'Q'}, {".-.", 'R'}, {"...", 'S'}, {"-", 'T'}, {"..-", 'U'}, {"...-", 'V'}, {".--", 'W'}, {"-..-", 'X'}, {"-.--", 'Y'}, {"--..", 'Z'}, {".-.-", ' '}, {".----", '1'}, {"..---", '2'}, {"...--", '3'}, {"....-", '4'}, {".....", '5'}, {"-....", '6'}, {"--...", '7'}, {"---..", '8'}, {"----.", '9'}, {"-----", '0'}};

// Special code for backspace functionality
const String BACKSPACE_CODE = ".-..-";

// Global variables
String morseCode, decodedMessage;
unsigned long lastSignalTime = 0, tapStartTime = 0;
bool measuringTap = false;
int tapCount = 0;

// Debounce variables
int prevSensorValue = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;

// Button state tracking
bool lastButtonState = HIGH;

void setup()
{
  // Initialize pins
  pinMode(IR_SENSOR_PIN, INPUT_PULLUP);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Start serial communication
  Serial.begin(115200);

  // Initialize LCD
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
  handleButtonPress(); // Check emergency button first
  handleMorseInput();  // Process IR sensor input
}

// Triggers emergency call/SMS
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

// Detects button press and triggers emergency call
void handleButtonPress()
{
  bool btn = digitalRead(BUTTON_PIN);
  if (btn == LOW && lastButtonState == HIGH)
  {
    // Button just pressed (falling edge)
    triggerCall("Btn Pressed");
  }
  lastButtonState = btn;
}

// Processes IR sensor input as Morse code
void handleMorseInput()
{
  int sensorValue = digitalRead(IR_SENSOR_PIN);
  unsigned long currentTime = millis();

  // Debounce logic
  if (sensorValue != prevSensorValue)
  {
    lastDebounceTime = currentTime;
  }

  if ((currentTime - lastDebounceTime) > debounceDelay)
  {
    // Start measuring tap when finger blocks IR
    if (sensorValue == LOW && !measuringTap)
    {
      tapStartTime = currentTime;
      measuringTap = true;
    }

    // End measuring tap when finger is removed
    if (sensorValue == HIGH && measuringTap)
    {
      unsigned long dur = currentTime - tapStartTime;
      measuringTap = false;
      morseCode += (dur < TAP_WINDOW ? "." : "-"); // Short = dot, Long = dash
      lastSignalTime = currentTime;
      updateDisplay();
    }

    // Handle gaps between characters and words
    if (sensorValue == HIGH && morseCode.length() > 0 && !measuringTap)
    {
      unsigned long gap = currentTime - lastSignalTime;

      // Character gap - decode current morse sequence
      if (gap >= CHAR_GAP && gap < WORD_GAP)
      {
        decodeMorse();
        morseCode = "";
      }
      // Word gap - add space after decoding
      else if (gap >= WORD_GAP && gap < SENTENCE_GAP)
      {
        decodeMorse();
        decodedMessage += " ";
        morseCode = "";
        updateDisplay();
      }
    }

    // End of sentence - trigger call if "Y" or reset
    if ((currentTime - lastSignalTime) >= SENTENCE_GAP && decodedMessage.length() > 0)
    {
      if (decodedMessage == "Y")
      {
        triggerCall("Sent: Y");
      }
      else
      {
        // Complete sentence - send to Python for TTS
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

      // Reset for next sentence
      decodedMessage = "";
      morseCode = "";
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Start Input...");
    }
  }
  prevSensorValue = sensorValue;
}

// Converts morse code to character and adds to message
void decodeMorse()
{
  // Handle backspace code
  if (morseCode == BACKSPACE_CODE)
  {
    if (decodedMessage.length() > 0)
    {
      decodedMessage.remove(decodedMessage.length() - 1);
      Serial.println("[DEL]");
    }
  }
  // Look up character in morse code map
  else if (morseToChar.count(morseCode))
  {
    decodedMessage += morseToChar[morseCode];
  }
  // Handle invalid morse code
  else
  {
    Serial.print("[Invalid Morse]: ");
    Serial.println(morseCode);
  }
  updateDisplay();
}

// Updates the LCD display with current morse code and message
void updateDisplay()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Morse: " + morseCode);
  lcd.setCursor(0, 1);

  // Show last 16 characters if message is longer than display
  if (decodedMessage.length() > 16)
    lcd.print(decodedMessage.substring(decodedMessage.length() - 16));
  else
    lcd.print(decodedMessage);
}
