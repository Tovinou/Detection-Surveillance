import struct
import json
from typing import Optional, Any, List, Dict
import config

# Import LEDController for type hinting if needed, but we'll pass the instance
if False:
    from led_display import LEDController

try:
    import serial
    _SERIAL_AVAILABLE = True
except ImportError:
    serial = None
    _SERIAL_AVAILABLE = False
    
try:
    import spidev
    _SPI_AVAILABLE = True
except ImportError:
    spidev = None
    _SPI_AVAILABLE = False

class CommunicationManager:
    """
    Manages UART and SPI communication.
    """
    def __init__(self, led_controller=None):
        self.ser: Optional[Any] = None
        self.spi: Optional[Any] = None
        self.led_controller = led_controller

    def setup(self) -> bool:
        """Initializes UART and SPI communication."""
        try:
            self.ser = serial.Serial(config.UART_PORT, config.UART_BAUD_RATE, timeout=1) if _SERIAL_AVAILABLE else None
            
            if _SPI_AVAILABLE:
                self.spi = spidev.SpiDev()
                self.spi.open(config.SPI_BUS, config.SPI_DEVICE)
                self.spi.max_speed_hz = config.SPI_MAX_SPEED_HZ
            else:
                self.spi = None
            return True
        except Exception as e:
            if config.SIMULATION:
                print(f"[SIM] Communication setup fallback: {e}")
                self.ser = None
                self.spi = None
                return True
            print(f"Error setting up communication: {e}")
            return False

    def send_status_packet(self, distance: float, mode: int, history: List[float], alert: bool) -> bool:
        """Sends a full status packet via UART (JSON) and SPI (Distance only)."""
        success = True
        
        # 1. Send JSON via UART
        if self.ser or config.SIMULATION:
            data = {
                "d": round(distance, 2),
                "m": mode,
                "a": int(alert),
                "h": [round(x, 1) for x in history[-5:]] # Send last 5 readings to save bandwidth
            }
            json_str = json.dumps(data) + "\n"
            
            if self.ser:
                try:
                    if self.led_controller:
                        self.led_controller.update_comm_status(1) # COMM_SENDING
                    self.ser.write(json_str.encode('utf-8'))
                except Exception as e:
                    print(f"UART Error: {e}")
                    success = False
                    if self.led_controller:
                        self.led_controller.update_comm_status(2) # COMM_ERROR
            elif config.SIMULATION:
                 print(f"[SIM] UART JSON: {json_str.strip()}")

        # 2. Send Distance via SPI (Fast, real-time)
        if self.spi or config.SIMULATION:
            # Re-use the existing SPI logic here or call it
            self.send_spi_distance(distance)

        if self.led_controller and success:
             self.led_controller.update_comm_status(0) # COMM_IDLE
             
        return success

    def send_uart_alert(self, message: str) -> bool:
        """Sends a text alert via UART."""
        if self.ser:
            try:
                self.ser.write(message.encode('utf-8'))
                return True
            except Exception as e:
                print(f"UART Error: {e}")
                if self.led_controller:
                    # We need to import COMM_ERROR from somewhere or define it. 
                    # Better to use constants from led_display or just integers if we want to decouple.
                    # For now, let's assume led_controller has constants or we use integers matching led_display.
                    # led_display.COMM_ERROR is 2.
                    self.led_controller.update_comm_status(2) 
                return False
        elif config.SIMULATION:
            print(f"[SIM] UART send: {message.strip()}")
            return True
        return False

    def send_spi_distance(self, distance: float) -> bool:
        """Sends a float distance value via SPI."""
        if self.spi:
            try:
                if self.led_controller:
                    self.led_controller.update_comm_status(1) # COMM_SENDING
                
                dist_bytes = bytearray(struct.pack("f", distance))
                self.spi.xfer2([0x01])  # Start signal
                self.spi.xfer2(dist_bytes)
                self.spi.xfer2([0xFF])  # End signal
                
                if self.led_controller:
                    self.led_controller.update_comm_status(0) # COMM_IDLE
                return True
            except Exception as e:
                print(f"SPI Error: {e}")
                if self.led_controller:
                    self.led_controller.update_comm_status(2) # COMM_ERROR
                return False
        elif config.SIMULATION:
            if self.led_controller:
                self.led_controller.update_comm_status(1)
            print(f"[SIM] SPI send distance: {distance}")
            if self.led_controller:
                self.led_controller.update_comm_status(0)
            return True
        return False

    def cleanup(self):
        """Closes the communication ports."""
        if self.ser:
            self.ser.close()
        if self.spi:
            self.spi.close()
