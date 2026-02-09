import sys
import os

try:
    import board
    LED_PIN = board.D18
except Exception:
    board = None
    LED_PIN = None
SIMULATION = bool(board is None or sys.platform.startswith("win") or os.environ.get("SIMULATION") == "1")

# --- Hardware Pin Configuration ---
ULTRASONIC_TRIG_PIN = 17
ULTRASONIC_ECHO_PIN = 27
BUZZER_PIN = 23
# For future expansion with directional detection
# ULTRASONIC_TRIG_PIN_2 = 22
# ULTRASONIC_ECHO_PIN_2 = 23

# --- LED Module Settings ---
LED_COUNT = 8
BRIGHTNESS = 0.5  # Keep at 0.5 or lower to avoid power issues

# --- Sensor & Logic Settings ---
DIST_THRESHOLD = 50  # in cm
HISTORY_SIZE = 8     # Number of readings to store for visualization

# --- Communication Settings ---
UART_PORT = '/dev/serial0'
UART_BAUD_RATE = 115200
SPI_BUS = 0
SPI_DEVICE = 0
SPI_MAX_SPEED_HZ = 1000000

# --- File Paths ---
LOG_FILE = 'distance_log.txt'

# --- Display Modes ---
MODE_NORMAL = 0
MODE_HISTORY = 1
MODE_DIRECTIONAL = 2
MODE_SWITCH_INTERVAL = 10  # seconds
