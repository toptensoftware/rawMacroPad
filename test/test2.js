// rawhid-listen.mjs
import HID from 'node-hid';

const VID = 0x1209;  // Your device VID
const PID = 0x88BF;  // Your device PID
const REPORT_SIZE = 64; // Must match your device report size

const sleep = ms => new Promise(r => setTimeout(r, ms));

console.log("Opening device")
const device = new HID.HID(VID, PID);
console.log("Device opened")

try {

  await sleep(50);

  // Prepare a 64-byte buffer to send
  let outReport = Buffer.alloc(REPORT_SIZE + 1, 0);

  outReport[0] = 0x00;
  outReport[1] = 0x11; // set underglow
  outReport[2] = 0xFF; // R
  outReport[3] = 0x00; // G
  outReport[4] = 0x00; // B

  for (let i=1; i<=2; i++)
  {
    outReport[2] = (i & 4) ? 0xFF : 0; // R
    outReport[3] = (i & 2) ? 0xFF : 0; // G
    outReport[4] = (i & 1) ? 0xFF : 0; // B

    console.log("Writing...")
    device.write(Array.from(outReport));
    console.log('Sent report:', outReport.slice(0, 10));

    await sleep(500);
  }


  await sleep(1000);

  device.close()

  await sleep(1000);

  console.log("finished");
} catch (err) {
  console.error('Error:', err);
  device.close();
}
