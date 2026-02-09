import time
import threading
from typing import List, Tuple, Optional
import config

try:
    import neopixel
    _NEOPIXEL_AVAILABLE = True
except ImportError:
    _NEOPIXEL_AVAILABLE = False

# --- Communication Status Constants ---
COMM_IDLE = 0
COMM_SENDING = 1
COMM_ERROR = 2

class LEDController:
    """
    Manages the LED display, including different modes and status indicators.
    """

    def __init__(self):
        self.pixels: Optional[neopixel.NeoPixel] = None
        self._led_available = False
        
        # Initialize NeoPixel if available
        if _NEOPIXEL_AVAILABLE and config.LED_PIN is not None:
            try:
                self.pixels = neopixel.NeoPixel(
                    config.LED_PIN,
                    config.LED_COUNT,
                    brightness=config.BRIGHTNESS,
                    auto_write=False
                )
                self._led_available = True
                print("DEBUG: NeoPixel initialized successfully.")
            except Exception as e:
                print(f"Failed to initialize NeoPixel: {e}")
                self._led_available = False
        else:
            print(f"DEBUG: NeoPixel skipped (Available={_NEOPIXEL_AVAILABLE}, Pin={config.LED_PIN})")
            self._led_available = False

        # Shared Data
        self.latest_distance: float = 999.0
        self.distance_history: List[float] = []
        self.directional_data: List[Tuple[int, float]] = []
        self.comm_status: int = COMM_IDLE

        # Mode Management
        self.current_mode: int = config.MODE_NORMAL
        self.last_mode_switch: float = time.time()
        
        # Threading for auto mode switching
        self._running = False
        self._mode_thread: Optional[threading.Thread] = None

    def start(self):
        """Starts the mode switcher thread."""
        self._running = True
        self._mode_thread = threading.Thread(target=self._mode_switcher_loop, daemon=True)
        self._mode_thread.start()

    def stop(self):
        """Stops the mode switcher thread and cleans up."""
        self._running = False
        if self._mode_thread:
            self._mode_thread.join(timeout=2.0)
        self.shutdown_sequence()

    def _mode_switcher_loop(self):
        """Loop to automatically switch between display modes."""
        while self._running:
            time.sleep(1)
            if time.time() - self.last_mode_switch > config.MODE_SWITCH_INTERVAL:
                self.last_mode_switch = time.time()
                self.current_mode = (self.current_mode + 1) % 3
                
                # Flash white to indicate mode change
                self.set_all_leds(255, 255, 255)
                time.sleep(0.2)
                self.set_all_leds(0, 0, 0)
                time.sleep(0.2)

    def set_all_leds(self, r: int, g: int, b: int):
        """Set all LEDs to the same RGB color."""
        if not self._led_available:
            if config.SIMULATION:
                print(f"[SIM] LEDs all -> ({r},{g},{b})")
            return
        
        if self.pixels:
            self.pixels.fill((r, g, b))
            self.pixels.show()

    def system_status_leds(self):
        """Show a startup sequence to indicate the system is ready."""
        if not self._led_available:
            if config.SIMULATION:
                print("[SIM] LED startup sequence")
            return
            
        if self.pixels:
            for i in range(config.LED_COUNT):
                self.pixels[i] = (0, 0, 255)  # Blue
                self.pixels.show()
                time.sleep(0.1)
            
            time.sleep(0.5)
            self.set_all_leds(0, 255, 0)  # Green
            time.sleep(1)
            self.set_all_leds(0, 0, 0)    # Off

    def alert_pattern(self):
        """Flash red three times to indicate an alert."""
        if not self._led_available:
            if config.SIMULATION:
                print("[SIM] LED alert pattern")
            return
            
        for _ in range(3):
            self.set_all_leds(255, 0, 0)
            time.sleep(0.2)
            self.set_all_leds(0, 0, 0)
            time.sleep(0.2)

    def update_comm_status(self, status: int):
        """Updates the communication status."""
        self.comm_status = status
        # In a real event-driven system we might update the LED immediately,
        # but here it's updated in the next update_display cycle or immediately if we want.
        # The original code called communication_indicator immediately.
        self._display_comm_indicator()

    def _display_comm_indicator(self):
        """Show communication status on the last LED."""
        if not self._led_available:
            if config.SIMULATION:
                status_map = {COMM_IDLE: "idle", COMM_SENDING: "sending", COMM_ERROR: "error"}
                print(f"[SIM] COMM {status_map.get(self.comm_status, 'unknown')}")
            return

        if self.pixels:
            if self.comm_status == COMM_IDLE:
                self.pixels[config.LED_COUNT - 1] = (0, 0, 255)  # Blue
            elif self.comm_status == COMM_SENDING:
                self.pixels[config.LED_COUNT - 1] = (255, 255, 0) # Yellow
            elif self.comm_status == COMM_ERROR:
                self.pixels[config.LED_COUNT - 1] = (255, 0, 0)   # Red
            self.pixels.show() # Make sure to show the change

    def update_display(self):
        """Updates the LED display based on the current mode."""
        if self.current_mode == config.MODE_NORMAL:
            self._display_normal_mode()
        elif self.current_mode == config.MODE_HISTORY:
            self._display_history_mode()
        elif self.current_mode == config.MODE_DIRECTIONAL:
            self._display_directional_mode()

    def _display_normal_mode(self):
        """Visualize the current distance as a bar graph."""
        if not self._led_available:
            if config.SIMULATION:
                print(f"[SIM] Normal mode, distance={self.latest_distance}")
            return
            
        if self.pixels:
            # Clear all LEDs first (except possibly the comm indicator if we want to be careful, 
            # but usually we redraw everything)
            # The original code preserved the last LED.
            for i in range(config.LED_COUNT - 1):
                self.pixels[i] = (0, 0, 0)
            
            # Map distance to LED count
            # Ensure we don't go out of bounds
            max_leds = config.LED_COUNT - 1
            if self.latest_distance < config.DIST_THRESHOLD:
                # Closer = more LEDs
                ratio = 1 - (self.latest_distance / config.DIST_THRESHOLD)
                led_count = int(ratio * max_leds)
                led_count = max(1, min(max_leds, led_count))
            else:
                led_count = 0
            
            for i in range(led_count):
                # Gradient from Green to Red? Or just colors.
                # Original: r increases, g decreases.
                r = int(255 * (i / max_leds))
                g = int(255 * (1 - i / max_leds))
                self.pixels[i] = (r, g, 0)
            
            self._display_comm_indicator() # Re-apply comm status
            self.pixels.show()

    def _display_history_mode(self):
        """Display historical distance data."""
        if not self._led_available:
            if config.SIMULATION:
                # Use slice for display
                history_slice = self.distance_history[-config.LED_COUNT:]
                print(f"[SIM] History mode, history={history_slice}")
            return

        if self.pixels:
            for i in range(config.LED_COUNT - 1):
                self.pixels[i] = (0, 0, 0)

            if not self.distance_history:
                self._display_comm_indicator()
                self.pixels.show()
                return
            
            # Display last N readings
            history_to_show = self.distance_history[-(config.LED_COUNT - 1):]
            
            for i, dist in enumerate(history_to_show):
                if dist < config.DIST_THRESHOLD:
                    intensity = max(0, min(255, int(255 * (1 - dist / config.DIST_THRESHOLD))))
                    # r = int(intensity * (i / config.LED_COUNT)) # Logic from original
                    # b = intensity
                    self.pixels[i] = (0, 0, intensity) # Simplify to Blue for history? 
                    # Original: r based on index, b based on intensity.
                    # Let's keep original logic roughly but cleaner
                    self.pixels[i] = (0, int(intensity/2), intensity) 
                else:
                    self.pixels[i] = (0, 0, 0)
                    
            self._display_comm_indicator()
            self.pixels.show()

    def _display_directional_mode(self):
        """Display directional detection from multiple sensors."""
        if not self._led_available:
            if config.SIMULATION:
                print(f"[SIM] Directional mode, data={self.directional_data}")
            return

        if self.pixels:
            for i in range(config.LED_COUNT - 1):
                self.pixels[i] = (0, 0, 0)
                
            for sensor_id, distance in self.directional_data:
                if distance < config.DIST_THRESHOLD:
                    # Map sensor_id to LED position. 
                    # Assuming sensor_id corresponds to direction.
                    led_pos = sensor_id % (config.LED_COUNT - 1)
                    intensity = max(0, min(255, int(255 * (1 - distance / config.DIST_THRESHOLD))))
                    self.pixels[led_pos] = (intensity, 0, 255 - intensity) # Purple-ish
                    
            self._display_comm_indicator()
            self.pixels.show()

    def shutdown_sequence(self):
        """Runs a shutdown sequence and turns off LEDs."""
        if not self._led_available:
            return
        self.set_all_leds(0, 0, 0)
