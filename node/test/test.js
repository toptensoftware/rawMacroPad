import { RawMacroPad } from "../index.js";

// Create KM16 instance
let km16 = new RawMacroPad();

// Start the watchdog timer with 2 second time out
km16.setWatchDog(2000);

// Current color number for each of LEDs
let indicator = 1;
let underglow = 0;
let keys = 0;

// LED defaults
km16.setLeds(2, mapColorIndexToRGB(indicator));
km16.enableLeds(0, true);
km16.enableLeds(1, true);

let activeKey = 0;
let arr = [ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ];

// Input handler
km16.on("input", (e) => {

    // Log it
    console.log(e);

    if (e.key && e.press)
        activeKey = e.key;

    // Main encoder controls key LEDs
    if (e.encoder == 16)
    {
        arr[activeKey] = (arr[activeKey] + e.delta + 8) % 8;
        km16.setLeds(0, arr.map(mapColorIndexToRGB));
    }

    // Top left encoder controls underglow LEDs
    if (e.encoder == 17)
    {
        underglow = (underglow + e.delta + 8) % 8;
        km16.setLeds(1, mapColorIndexToRGB(underglow));
    }

    // Top right encoder controls indicator LED
    if (e.encoder == 18)
    {
        indicator = (indicator + e.delta + 8) % 8;
        km16.setLeds(2, mapColorIndexToRGB(indicator));
    }
});

function mapColorIndexToRGB(index)
{
    return (index & 0x04 ? 0xFF0000 : 0) | 
            (index & 0x02 ? 0x00FF00 : 0) | 
            (index & 0x01 ? 0x0000FF : 0)
}

console.log("Small Left Encoder = Underglow LEDs");
console.log("Small Right Encoder = Indicator LED");
console.log("Press key then main encoder to adjust that key's LED");