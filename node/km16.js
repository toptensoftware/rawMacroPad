import { EventEmitter } from "node:events";

import HID from 'node-hid';

const VID = 0x1209;
const PID = 0x88BF;
const REPORT_SIZE = 64;

let ledmap = [ 0, 1, 2, 3, 7, 6, 5, 4, 8, 9, 10, 11, 15, 14, 13, 12];

export class KM16 extends EventEmitter
{
    constructor()
    {
        super();
        this.#hid = new HID.HID(VID, PID);
        this.#buf[0] = 0;
        this.#hid.on('data', (data) => {
            switch (data[0])
            {
                case 0x01:
                    this.emit("input", {
                        key: data[1],
                        press: !!data[2],
                    });
                    break;

                case 0x02:
                    this.emit("input", {
                        encoder: data[1],
                        delta: (data[2] << 24) >> 24,   // Signed byte to integer
                    });
            }
        });
    }


    close()
    {
        this.#hid.close();
        this.#hid = null;

        if (this.#watchDogTimer != null)
        {
            clearInterval(this.#watchDogTimer);
            this.#watchDogTimer = null;
        }
    }

    setWatchDog(intervalMillis)
    {
        // Store interval
        this.#watchDogInterval = intervalMillis;

        // Ping helper
        let ping = () => {
            this.#buf[1] = 0x02;
            this.#buf[2] = (this.#watchDogInterval >> 8) & 0xFF;
            this.#buf[3] = this.#watchDogInterval & 0xFF;
            this.#hid.write(this.#buf);
        }

        // Send initial ping
        ping();

        // Clear old timer
        if (this.#watchDogTimer)
        {
            clearInterval(this.#watchDogTimer);
            this.#watchDogTimer = null;
        }

        // Setup new timer
        if (intervalMillis != 0)
        {
            this.#watchDogTimer = setInterval(ping, intervalMillis / 2);
        }
    }

    setIndicator(color)
    {
        this.#buf[1] = 0x01;
        this.#buf[2] = color;
        this.#hid.write(this.#buf);
    }

    enableUnderglow(enable)
    {
        this.#buf[1] = 0x10;
        this.#buf[2] = enable ? 1 : 0;
        this.#hid.write(this.#buf);
    }

    setUnderglow(color)
    {
        this.#buf[1] = 0x11;
        this.#buf[2] = (color >> 16) & 0xFF;
        this.#buf[3] = (color >> 8) & 0xFF;
        this.#buf[4] = color & 0xFF;
        this.#hid.write(this.#buf);
    }
    
    enableKeyLeds(enable)
    {
        this.#buf[1] = 0x20;
        this.#buf[2] = enable ? 1 : 0;
        this.#hid.write(this.#buf);
    }

    setKeyLeds(color)
    {
        if (!Array.isArray(color))
        {
            this.#buf[1] = 0x21;
            this.#buf[2] = (color >> 16) & 0xFF;
            this.#buf[3] = (color >> 8) & 0xFF;
            this.#buf[4] = color & 0xFF;
        }
        else
        {
            if (color.length != 16)
                throw new Error("Key LED array must have 16 entries");

            this.#buf[1] = 0x22;
            for (let i=0; i<16; i++)
            {
                let pos = ledmap[i] * 3;
                this.#buf[pos + 2] = (color[i] >> 16) & 0xFF;
                this.#buf[pos + 3] = (color[i] >> 8) & 0xFF;
                this.#buf[pos + 4] = color[i] & 0xFF;
            }
        }

        this.#hid.write(this.#buf);
    }

    #hid;
    #buf = Buffer.alloc(REPORT_SIZE + 1, 0);
    #watchDogInterval = 0;
    #watchDogTimer = null;
}
