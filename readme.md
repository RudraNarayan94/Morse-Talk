# MorseTalk 🔊👆

## An Assistive Communication System for Partially Paralyzed Patients

MorseTalk is an IoT-based solution that enables partially paralyzed patients who cannot speak to communicate through finger movements. The system detects Morse code input via an IR sensor, translates it into text, and converts it to speech, with emergency calling capabilities.

## 🌟 Features

- 👆 Finger movement detection using IR sensor
- 📝 Real-time Morse code decoding
- 🔊 Text-to-speech conversion
- 📱 Emergency calling and SMS alerts
- 📊 LCD display for visual feedback
- 🆘 Dedicated emergency button

## 🛠️ Hardware Requirements

- Arduino/ESP32 board
- IR sensor module
- 16x2 I2C LCD display
- Push button (for emergency trigger)
- Connecting wires
- Computer with Python installed

## 📋 Software Requirements

- Arduino IDE
- Python 3.x
- Required Python libraries:
  - `pyttsx3` (text-to-speech)
  - `pyserial` (serial communication)
  - `twilio` (for emergency calls/SMS)

## 📥 Installation

1. **Arduino Setup**

   - Connect the IR sensor to GPIO pin 33
   - Connect the emergency button to GPIO pin 32
   - Connect the I2C LCD display (SDA, SCL pins)
   - Upload the `morseCodeDecoder.ino` sketch to your Arduino/ESP32

2. **Python Setup**

   ```bash
   pip install pyttsx3 pyserial twilio
   ```

3. **Twilio Configuration**
   - Create a Twilio account at [twilio.com](https://www.twilio.com)
   - Get your Account SID and Auth Token
   - Update the `recipients` list in `textToSpeech.py` with your Twilio credentials and phone numbers

## 🚀 Usage

1. Connect the Arduino/ESP32 to your computer via USB
2. Note the COM port (e.g., COM7) and update it in `textToSpeech.py` if needed
3. Run the Python script:
   ```bash
   python textToSpeech.py
   ```
4. Use finger movements to create Morse code patterns:

   - Short interruption = dot (.)
   - Long interruption = dash (-)
   - Pause between characters
   - Longer pause for word separation

5. For emergency help:
   - Input "Y" in Morse code (-.--) or
   - Press the dedicated emergency button

## 🎬 Demonstration

Watch our demonstration video: [MorseTalk Demo](https://youtu.be/your-video-link)

## 🔄 How It Works

1. **Input Detection**: The IR sensor detects finger movements, interpreting them as Morse code dots and dashes
2. **Decoding**: The Arduino/ESP32 decodes the Morse code into characters and words
3. **Communication**: Decoded text is sent via serial communication to the Python script
4. **Output**:
   - Regular messages are converted to speech using pyttsx3
   - Emergency signals trigger calls and SMS to predefined contacts using Twilio

## 📊 Morse Code Reference

| Letter | Morse Code | Letter | Morse Code |
| ------ | ---------- | ------ | ---------- |
| A      | .-         | N      | -.         |
| B      | -...       | O      | ---        |
| C      | -.-.       | P      | .--.       |
| D      | -..        | Q      | --.-       |
| E      | .          | R      | .-.        |
| F      | ..-.       | S      | ...        |
| G      | --.        | T      | -          |
| H      | ....       | U      | ..-        |
| I      | ..         | V      | ...-       |
| J      | .---       | W      | .--        |
| K      | -.-        | X      | -..-       |
| L      | .-..       | Y      | -.--       |
| M      | --         | Z      | --..       |

## 🤝 Contributors

- Rudra
- Nagen

## 📄 License

This project is licensed under the MIT License - see the LICENSE file for details.
