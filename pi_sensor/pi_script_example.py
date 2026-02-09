import serial
import time
import json
import random

# Configure Serial Port (ensure this matches your Pi's setup)
# On Pi 3/4/Zero W, /dev/ttyS0 is often the mini-UART on GPIO 14/15
# You might need to disable console on serial via raspi-config
SERIAL_PORT = '/dev/ttyS0' 
BAUD_RATE = 115200

def main():
    print(f"Starting Sensor Simulation on {SERIAL_PORT} at {BAUD_RATE} baud...")
    
    try:
        ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
    except Exception as e:
        print(f"Error opening serial port: {e}")
        return

    history = [50.0] * 5
    
    while True:
        try:
            # Simulate data
            distance = 50.0 + random.uniform(-5, 5)
            mode = 0 # 0: Normal
            alert = 1 if distance < 40 else 0
            
            # Update history
            history.pop(0)
            history.append(round(distance, 1))
            
            # Create JSON packet
            data = {
                "d": round(distance, 1),
                "m": mode,
                "a": alert,
                "h": history
            }
            
            json_str = json.dumps(data)
            
            # Send via UART (with newline for ESP32 readline)
            ser.write((json_str + "\n").encode('utf-8'))
            print(f"Sent: {json_str}")
            
            time.sleep(0.5)
            
        except KeyboardInterrupt:
            print("\nStopping...")
            break
        except Exception as e:
            print(f"Error: {e}")
            time.sleep(1)

if __name__ == "__main__":
    main()
