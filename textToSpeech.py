import time
import pyttsx3
import serial
from twilio.rest import Client
from concurrent.futures import ThreadPoolExecutor, as_completed

# ===== List of different Twilio trial accounts and their verified recipients =====
recipients = [
    {
        'account_sid': 'AC1c35d0ac4f9e5427975422f365bXXXXX',
        'auth_token': '03f2e4dc009f5a1b5e2725527ae47a93',
        'twilio_number': '+1585620XXXXX',
        'recipient_number': '+9189174XXXXX'
    },
    {
        'account_sid': 'AC0447a88d6b27d70603c8a4b13a8XXXXX',
        'auth_token': 'ad31ea66db6257c1c3ba65788df7a03b',
        'twilio_number': '+122335XXXXX',
        'recipient_number': '+9176848XXXXX'
    },
]

# ===== Serial port configuration =====
PORT = 'COM7'
BAUD = 115200

ser = serial.Serial(PORT, BAUD, timeout=1)
time.sleep(2)  # ESP32 reset

# ===== Initialize TTS engine =====
engine = pyttsx3.init()
engine.setProperty('rate', 150)

def notify_recipient(r):
    """Make the call and send SMS for one recipient dict r."""
    client = Client(r['account_sid'], r['auth_token'])
    # call
    call = client.calls.create(
        twiml='<Response>'
          '<Say>' \
          'This is an emergency. I am in urgent need of help. ' \
          'I am currently in a dangerous situation and unable to get to safety on my own.' \
          ' Please send immediate assistance to my location.'
          '</Say>'
        '</Response>',
        from_=r['twilio_number'],
        to=r['recipient_number']
    )
    print(f"✅ Call sent to {r['recipient_number']} (SID: {call.sid})")
    # SMS
    msg = client.messages.create(
        body="🚨 Alert: Your ESP32 device triggered an emergency!",
        from_=r['twilio_number'],
        to=r['recipient_number']
    )
    print(f"✅ SMS sent to {r['recipient_number']} (SID: {msg.sid})")

print("Listening for decoded Morse, call trigger, and text for TTS...")

while True:
    try:
        line = ser.readline().decode('utf-8', errors='ignore').strip()
        if (
            not line
            or line == "[DEL]"
            or line == "."
            or line.startswith("Button state:")
        ):
            continue

        print("Received:", line)

        if line == "CALL_TRIGGERED":
            # notify all recipients in parallel
            with ThreadPoolExecutor(max_workers=len(recipients)) as executor:
                futures = {executor.submit(notify_recipient, r): r for r in recipients}
                for fut in as_completed(futures):
                    r = futures[fut]
                    try:
                        fut.result()
                    except Exception as e:
                        print(f"❌ Failed for {r['recipient_number']}: {e}")

        else:
            # normal messages
            engine.say(line)
            engine.runAndWait()

    except KeyboardInterrupt:
        print("\nStopped by user.")
        break
    except Exception as e:
        print(f"Error during processing: {e}")