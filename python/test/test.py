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

    # Current color index for each LED group
    indicator = 1
    underglow = 0
    active_key = 0
    arr = [0] * 16

    # LED defaults
    km16.set_leds(2, map_color_index_to_rgb(indicator))
    km16.enable_leds(0, True)
    km16.enable_leds(1, True)

    def on_input(e):
        nonlocal active_key, underglow, indicator

        print(e)

        if e.get("key") and e.get("press"): # and e.get("key") > 0 and e.get("key") < 16:
            active_key = e["key"]

        # Main encoder controls key LEDs
        if e.get("encoder") == 16:
            arr[active_key] = (arr[active_key] + e["delta"] + 8) % 8
            km16.set_leds(0, [map_color_index_to_rgb(c) for c in arr])

        # Top left encoder controls underglow LEDs
        if e.get("encoder") == 17:
            underglow = (underglow + e["delta"] + 8) % 8
            km16.set_leds(1, map_color_index_to_rgb(underglow))

        # Top right encoder controls indicator LED
        if e.get("encoder") == 18:
            indicator = (indicator + e["delta"] + 8) % 8
            km16.set_leds(2, map_color_index_to_rgb(indicator))

    km16.on("input", on_input)

    print("Small Left Encoder = Underglow LEDs")
    print("Small Right Encoder = Indicator LED")
    print("Press key then main encoder to adjust that key's LED")
    print("Press Ctrl+C to exit")

    try:
        await asyncio.Event().wait()
    except (KeyboardInterrupt, asyncio.CancelledError):
        pass
    finally:
        km16.close()


asyncio.run(main())
