import HID from 'node-hid';
const VID = 0x1209;  // Your device VID
const PID = 0x88BF;  // Your device PID
const devices = HID.devices(VID, PID);
console.log(devices);
