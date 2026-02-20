import asyncio
import sys
sys.path.insert(0, "..")
from raw_macro_pad import RawMacroPad


def map_color_index_to_rgb(index):
    return ((0xFF0000 if index & 0x04 else 0) |
            (0x00FF00 if index & 0x02 else 0) |
            (0x0000FF if index & 0x01 else 0))


async def main():
    km16 = RawMacroPad()

    # Start the watchdog timer with 2 second timeout
    km16.set_watchdog(2000)

    # LED defaults
    km16.enable_leds(0, True)
    km16.enable_leds(1, True)

    arr = [0] * 16

    def write_leds():
        km16.set_leds(0, [map_color_index_to_rgb(c) for c in arr])

    def clear():
        for i in range(16):
            arr[i] = 0

    def set_h_bar(y, color):
        for i in range(4):
            arr[y * 4 + i] = color

    def set_v_bar(x, color):
        for i in range(4):
            arr[i * 4 + x] = color

    print("Running LED pattern. Press Ctrl+C to exit.")

    try:
        while True:
            c = 1
            for _ in range(2):
                for col in range(4):
                    clear()
                    set_v_bar(col, c + 1)
                    write_leds()
                    await asyncio.sleep(0.2)
                    c = (c + 1) % 7

                for row in range(4):
                    clear()
                    set_h_bar(row, c + 1)
                    write_leds()
                    await asyncio.sleep(0.2)
                    c = (c + 1) % 7

            for _ in range(2):
                for i in range(16):
                    clear()
                    arr[i] = c + 1
                    write_leds()
                    await asyncio.sleep(0.1)
                    c = (c + 1) % 7

                for i in range(14, 0, -1):
                    clear()
                    arr[i] = c + 1
                    write_leds()
                    await asyncio.sleep(0.1)
                    c = (c + 1) % 7

    except (KeyboardInterrupt, asyncio.CancelledError):
        pass
    finally:
        km16.close()


asyncio.run(main())
