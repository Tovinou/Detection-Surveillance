import time
import board
import neopixel

pixel_pin = board.D18
num_pixels = 8
ORDER = neopixel.GRB

print("Initializing NeoPixels on Pin 18...")
try:
    pixels = neopixel.NeoPixel(
        pixel_pin, num_pixels, brightness=0.5, auto_write=False, pixel_order=ORDER
    )
    print("Initialization successful.")
except Exception as e:
    print(f"Initialization failed: {e}")
    exit(1)

while True:
    try:
        print("Color: Red")
        pixels.fill((255, 0, 0))
        pixels.show()
        time.sleep(1)
        
        print("Color: Green")
        pixels.fill((0, 255, 0))
        pixels.show()
        time.sleep(1)
        
        print("Color: Blue")
        pixels.fill((0, 0, 255))
        pixels.show()
        time.sleep(1)
        
        print("Color: White")
        pixels.fill((255, 255, 255))
        pixels.show()
        time.sleep(1)
        
        print("Color: Off")
        pixels.fill((0, 0, 0))
        pixels.show()
        time.sleep(1)
        
    except KeyboardInterrupt:
        print("\nExiting...")
        pixels.fill((0, 0, 0))
        pixels.show()
        break
    except Exception as e:
        print(f"Error during loop: {e}")
