// rawhid-listen.mjs
import HID from 'node-hid';

const VID = 0x1209;  // Your device VID
const PID = 0x88BF;  // Your device PID
const REPORT_SIZE = 64; // Must match your device report size

try {
  const device = new HID.HID(VID, PID);
  console.log('Device opened, waiting for packets...');

  // Read in a loop
  while (true) {
    const report = device.readTimeout(1000); // 1000ms timeout
    if (report && report.length > 0) {
      console.log('Received:', Buffer.from(report));
    }
  }

} catch (err) {
  console.error('Error:', err);
}
