import time
import config
import logger
from sensors import SensorManager
from led_display import LEDController
from communication import CommunicationManager
from buzzer import BuzzerController

def main():
    """Main application loop."""
    # --- Initialization ---
    print("Initializing system...")
    
    # Instantiate managers
    sensor_mgr = SensorManager()
    led_ctrl = LEDController()
    buzzer_ctrl = BuzzerController()
    comm_mgr = CommunicationManager(led_controller=led_ctrl)
    
    # Setup hardware
    sensor_mgr.setup()
    
    if not comm_mgr.setup():
        print("Failed to initialize communication. Exiting.")
        return
        
    led_ctrl.system_status_leds()
    led_ctrl.start() # Start mode switcher thread
    buzzer_ctrl.start() # Start buzzer thread
    buzzer_ctrl.startup_sequence()
    
    print("System ready. Starting main loop.")

    try:
        while True:
            # --- Data Acquisition ---
            raw_dist = sensor_mgr.get_distance(config.ULTRASONIC_TRIG_PIN, config.ULTRASONIC_ECHO_PIN)
            
            # Simple noise filter: Ignore huge jumps or unrealistic values
            if raw_dist > 400: # Max range
                # print(f"DEBUG: Ignored outlier distance: {raw_dist} cm")
                continue # Skip this loop iteration
                
            dist = raw_dist
            print(f"Distance: {dist} cm")
            
            # --- Update Shared Data for LED Display ---
            led_ctrl.latest_distance = dist
            led_ctrl.distance_history.append(dist)
            if len(led_ctrl.distance_history) > config.HISTORY_SIZE:
                led_ctrl.distance_history.pop(0)
            
            # For now, simulate directional data with one sensor
            led_ctrl.directional_data = [(0, dist)]
            
            # --- Update LED Display ---
            led_ctrl.update_display()

            # --- Logic and Actions ---
            is_alert = dist < config.DIST_THRESHOLD
            
            # Buzzer Logic
            if dist < 10:
                buzzer_ctrl.set_mode(BuzzerController.MODE_FAST) # Critical
            elif dist < config.DIST_THRESHOLD:
                buzzer_ctrl.set_mode(BuzzerController.MODE_SLOW) # Warning
            else:
                buzzer_ctrl.set_mode(BuzzerController.MODE_OFF)

            if is_alert:
                print(f"ALERT: Object close at {dist} cm")
                logger.log_event(f"Object detected at {dist} cm")
                
                # Trigger alert pattern
                # led_ctrl.alert_pattern()
                pass
            
            # --- Send Full Status to ESP32 (Always) ---
            # Sends: Distance, Mode, History, Alert Status
            comm_mgr.send_status_packet(
                distance=dist,
                mode=led_ctrl.current_mode,
                history=led_ctrl.distance_history,
                alert=is_alert
            )
            
            time.sleep(0.5)

    except KeyboardInterrupt:
        print("\nShutting down gracefully...")
    finally:
        # --- Cleanup ---
        if 'buzzer_ctrl' in locals():
            buzzer_ctrl.stop()
        if 'led_ctrl' in locals():
            led_ctrl.stop()
            led_ctrl.shutdown_sequence()
        if 'comm_mgr' in locals():
            comm_mgr.cleanup()
        if 'sensor_mgr' in locals():
            sensor_mgr.cleanup()
        print("Shutdown complete.")

if __name__ == "__main__":
    main()
