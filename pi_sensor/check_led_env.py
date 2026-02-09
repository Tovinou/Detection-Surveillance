import sys
import os

print("--- Environment Diagnostic ---")
print(f"Python Executable: {sys.executable}")
print(f"Platform: {sys.platform}")

print("\n--- Checking 'board' (adafruit-blinka) ---")
try:
    import board
    print(f"SUCCESS: board imported. D18 available? {hasattr(board, 'D18')}")
    if hasattr(board, 'D18'):
        print(f"board.D18: {board.D18}")
except ImportError as e:
    print(f"FAILURE: Could not import board. Error: {e}")
except Exception as e:
    print(f"FAILURE: Error checking board: {e}")

print("\n--- Checking 'neopixel' ---")
try:
    import neopixel
    print("SUCCESS: neopixel imported.")
except ImportError as e:
    print(f"FAILURE: Could not import neopixel. Error: {e}")

print("\n--- Checking Permissions ---")
try:
    import RPi.GPIO as GPIO
    print("SUCCESS: RPi.GPIO imported.")
    try:
        GPIO.setmode(GPIO.BCM)
        print("SUCCESS: RPi.GPIO setmode(BCM) worked (Permissions OK).")
        GPIO.cleanup()
    except Exception as e:
        print(f"FAILURE: RPi.GPIO permission check failed: {e}")
except ImportError:
    print("WARNING: RPi.GPIO not found (might be normal if using libgpiod, but check requirements).")

print("\n--- Checking config.py ---")
try:
    import config
    print(f"config.LED_PIN: {config.LED_PIN}")
    print(f"config.SIMULATION: {config.SIMULATION}")
except ImportError:
    print("FAILURE: Could not import config.py")

print("\n--- End Diagnostic ---")
