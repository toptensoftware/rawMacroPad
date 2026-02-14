import { KM16 } from "../km16.js";

// Create KM16 instance
let km16 = new KM16();

// Start the watchdog timer with 2 second time out
km16.setWatchDog(2000);

// Current color number for each of LEDs
let indicator = 1;
let underglow = 0;
let keys = 0;

// LED defaults
km16.setIndicator(indicator);
km16.enableUnderglow(true);
km16.enableKeyLeds(true);

let activeKey = 0;
let arr = [ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ];

// Input handler
km16.on("input", (e) => {

    // Log it
    console.log(e);

    if (e.key && e.press)
        activeKey = e.key;

    // Main encoder controls key LEDs
    if (e.encoder == 0)
    {
        arr[activeKey] = (arr[activeKey] + e.delta + 8) % 8;
        km16.setKeyLeds(arr.map(mapColorIndexToRGB));
    }

    // Top left encoder controls underglow LEDs
    if (e.encoder == 1)
    {
        underglow = (underglow + e.delta + 8) % 8;
        km16.setUnderglow(mapColorIndexToRGB(underglow));
    }

    // Top right encoder controls indicator LED
    if (e.encoder == 2)
    {
        indicator = (indicator + e.delta + 8) % 8;
        km16.setIndicator(indicator);
    }
});

function mapColorIndexToRGB(index)
{
    return (index & 0x04 ? 0xFF0000 : 0) | 
            (index & 0x02 ? 0x00FF00 : 0) | 
            (index & 0x01 ? 0x0000FF : 0)
}
