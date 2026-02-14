import { KM16 } from "../km16.js";

// Create KM16 instance
let km16 = new KM16();

function sleep(ms) { return new Promise((resolve) => setTimeout(resolve, ms)) }

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

let arr = [ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ];

function write_leds()
{
    km16.setLeds(0, arr.map(mapColorIndexToRGB));
}


// Input handler
km16.on("input", (e) => {

    // Log it
    console.log(e);

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

function clear()
{
    for (let i=0; i<16; i++)
    {
        arr[i] = 0;
    }
}

function setHBar(y,color)
{
    for (let i=0; i<4; i++)
    {
        arr[y * 4 + i] = color;
    }
}

function setVBar(x, color)
{
    for (let i=0; i<4; i++)
    {
        arr[i * 4 + x] = color;
    }
}


while (true)
{
    let c = 1;
    for (let i=0; i<2; i++)
    {
        for (let col = 0; col < 4; col++)
        {
            clear();
            setVBar(col, c + 1);
            write_leds();   
            await sleep(200);
            c = (c + 1) % 7
        }

        for (let row = 0; row < 4; row++)
        {
            clear();
            setHBar(row, c + 1);
            write_leds();   
            await sleep(200);
            c = (c + 1) % 7
        }
    }

    for (let i=0; i<2; i++)
    {
        for (let i = 0; i < 16; i++)
        {
            clear();
            arr[i] = c + 1;
            write_leds();   
            await sleep(100);
            c = (c + 1) % 7
        }

        for (let i = 14; i > 0; i--)
        {
            clear();
            arr[i] = c + 1;
            write_leds();   
            await sleep(100);
            c = (c + 1) % 7
        }
    }

}