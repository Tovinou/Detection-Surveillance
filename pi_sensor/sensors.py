import time
import math
from typing import Optional, Any
import config

try:
    import RPi.GPIO as GPIO
    _GPIO_AVAILABLE = True
except ImportError:
    GPIO = None
    _GPIO_AVAILABLE = False

class SensorManager:
    """
    Manages sensor hardware and readings.
    """
    def __init__(self):
        self._gpio_available = _GPIO_AVAILABLE

    def setup(self):
        """Initializes GPIO pins for the ultrasonic sensor."""
        if not self._gpio_available:
            if config.SIMULATION:
                print("[SIM] Sensors setup")
            return
        
        if GPIO:
            GPIO.setmode(GPIO.BCM)
            GPIO.setup(config.ULTRASONIC_TRIG_PIN, GPIO.OUT)
            GPIO.setup(config.ULTRASONIC_ECHO_PIN, GPIO.IN)
            
            # Ensure Trigger is low initially
            GPIO.output(config.ULTRASONIC_TRIG_PIN, False)
            time.sleep(0.5) # Allow sensor to settle

    def get_distance(self, trig_pin: int, echo_pin: int) -> float:
        """
        Measures distance from a specific ultrasonic sensor.
        Returns distance in cm, or 999.0 on timeout.
        """
        if not self._gpio_available:
            if config.SIMULATION:
                t = time.time()
                # Simulate a sine wave distance
                return round(30 + 20 * (1 + math.sin(t)) / 2, 2)
            return 100.0
        
        if GPIO:
            # 1. Trigger Pulse
            GPIO.output(trig_pin, True)
            time.sleep(0.00001) # 10us pulse
            GPIO.output(trig_pin, False)
            
            # 2. Wait for Echo Start (Signal goes High)
            timeout_start = time.time()
            while GPIO.input(echo_pin) == 0:
                if time.time() - timeout_start > 0.1: # 100ms timeout waiting for echo
                    print(f"DEBUG: Timeout waiting for Echo START (Pin {echo_pin} is Low)")
                    return 999.0
            
            pulse_start = time.time()
            
            # 3. Wait for Echo End (Signal goes Low)
            timeout_end = time.time()
            while GPIO.input(echo_pin) == 1:
                if time.time() - timeout_end > 0.1: # 100ms timeout during echo
                    print(f"DEBUG: Timeout waiting for Echo END (Pin {echo_pin} is High)")
                    return 999.0
            
            pulse_end = time.time()
            
            # 4. Calculate Distance
            pulse_duration = pulse_end - pulse_start
            distance = pulse_duration * 17150  # Speed of sound conversion (34300 cm/s / 2)
            
            # Filter noise (max range of HC-SR04 is usually ~400cm)
            if distance > 400 or distance < 2:
                 # Return previous reading or a specific error code if we had history, 
                 # but for now just cap it or ignore. 
                 # Let's cap at 400 for consistency or mark as invalid?
                 # Returning the raw value is okay but main loop might want to filter.
                 # Let's just return it but note the outlier.
                 pass
            
            return round(distance, 2)
            
        return 999.0

    def cleanup(self):
        """Resets GPIO settings."""
        if not self._gpio_available:
            if config.SIMULATION:
                print("[SIM] Sensors cleanup")
            return
        if GPIO:
            GPIO.cleanup()
