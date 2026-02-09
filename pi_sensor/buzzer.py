import time
import threading
import RPi.GPIO as GPIO
import config

class BuzzerController:
    """
    Manages the active buzzer for auditory feedback.
    Runs in a separate thread to handle non-blocking beeping patterns.
    """
    
    MODE_OFF = 0
    MODE_SLOW = 1   # Warning
    MODE_FAST = 2   # Critical
    
    def __init__(self):
        self.pin = config.BUZZER_PIN
        self.current_mode = self.MODE_OFF
        self._running = False
        self._thread = None
        self._setup_gpio()

    def _setup_gpio(self):
        """Configure the GPIO pin."""
        if config.SIMULATION:
            print(f"[SIM] Buzzer configured on GPIO {self.pin}")
            return
            
        try:
            GPIO.setmode(GPIO.BCM)
            GPIO.setup(self.pin, GPIO.OUT)
            GPIO.output(self.pin, GPIO.LOW)
        except Exception as e:
            print(f"Error setting up buzzer GPIO: {e}")

    def start(self):
        """Start the background buzzer thread."""
        self._running = True
        self._thread = threading.Thread(target=self._loop, daemon=True)
        self._thread.start()

    def stop(self):
        """Stop the background thread and cleanup."""
        self._running = False
        if self._thread:
            self._thread.join(timeout=1.0)
        
        if not config.SIMULATION:
            try:
                GPIO.output(self.pin, GPIO.LOW)
            except:
                pass

    def set_mode(self, mode: int):
        """Set the current beeping mode."""
        self.current_mode = mode

    def beep_once(self, duration=0.1):
        """Blocking single beep (useful for startup/status)."""
        if config.SIMULATION:
            print("[SIM] *BEEP*")
            return

        try:
            GPIO.output(self.pin, GPIO.HIGH)
            time.sleep(duration)
            GPIO.output(self.pin, GPIO.LOW)
        except Exception as e:
            print(f"Buzzer error: {e}")

    def startup_sequence(self):
        """Play a startup sound."""
        print("Playing startup sound...")
        self.beep_once(0.1)
        time.sleep(0.1)
        self.beep_once(0.1)

    def _loop(self):
        """Background loop to handle beeping patterns."""
        while self._running:
            if self.current_mode == self.MODE_OFF:
                if not config.SIMULATION:
                    GPIO.output(self.pin, GPIO.LOW)
                time.sleep(0.1)
                
            elif self.current_mode == self.MODE_SLOW:
                # Slow beep: 0.5s ON, 0.5s OFF
                self._tone_on()
                time.sleep(0.3)
                self._tone_off()
                time.sleep(0.7)
                
            elif self.current_mode == self.MODE_FAST:
                # Fast beep: 0.1s ON, 0.1s OFF
                self._tone_on()
                time.sleep(0.1)
                self._tone_off()
                time.sleep(0.1)

    def _tone_on(self):
        if config.SIMULATION:
            print("[SIM] *BEEP*")
        else:
            GPIO.output(self.pin, GPIO.HIGH)

    def _tone_off(self):
        if not config.SIMULATION:
            GPIO.output(self.pin, GPIO.LOW)
