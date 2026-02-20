import asyncio
import hid

VID = 0x1209
PID = 0x88BF
REPORT_SIZE = 64


class RawMacroPad:
    def __init__(self):
        if hasattr(hid, 'Device'):
            self._hid = hid.Device(VID, PID)
        else:
            self._hid = hid.device()
            self._hid.open(VID, PID)
        self._buf = bytearray(REPORT_SIZE + 1)
        self._buf[0] = 0
        self._listeners = {}
        self._watchdog_task = None
        self._read_task = asyncio.get_running_loop().create_task(self._read_loop())

    async def _read_loop(self):
        loop = asyncio.get_running_loop()
        try:
            while True:
                data = await loop.run_in_executor(
                    None, lambda: self._hid.read(REPORT_SIZE, timeout_ms=100)
                )
                if data:
                    self._handle_data(data)
        except asyncio.CancelledError:
            pass

    def _handle_data(self, data):
        if data[0] == 0x01:
            self._emit("input", {
                "key": data[1],
                "press": bool(data[2]),
            })
        elif data[0] == 0x02:
            # Convert unsigned byte to signed (equivalent to JS's (data[2] << 24) >> 24)
            delta = data[2] if data[2] < 128 else data[2] - 256
            self._emit("input", {
                "encoder": data[1],
                "delta": delta,
            })

    def on(self, event, callback):
        if event not in self._listeners:
            self._listeners[event] = []
        self._listeners[event].append(callback)

    def _emit(self, event, *args):
        for cb in self._listeners.get(event, []):
            try:
                cb(*args)
            except Exception as e:
                print(f"Exception in '{event}' handler: {e}")

    def close(self):
        if self._watchdog_task:
            self._watchdog_task.cancel()
            self._watchdog_task = None
        if self._read_task:
            self._read_task.cancel()
            self._read_task = None
        self._hid.close()
        self._hid = None

    async def _watchdog_loop(self, interval_millis):
        try:
            while True:
                await asyncio.sleep(interval_millis / 2000)
                self._buf[1] = 0x01
                self._buf[2] = (interval_millis >> 8) & 0xFF
                self._buf[3] = interval_millis & 0xFF
                self._hid.write(bytes(self._buf))
        except asyncio.CancelledError:
            pass

    def set_watchdog(self, interval_millis):
        # Send initial ping
        self._buf[1] = 0x01
        self._buf[2] = (interval_millis >> 8) & 0xFF
        self._buf[3] = interval_millis & 0xFF
        self._hid.write(bytes(self._buf))

        # Cancel old watchdog
        if self._watchdog_task:
            self._watchdog_task.cancel()
            self._watchdog_task = None

        # Start new watchdog
        if interval_millis != 0:
            self._watchdog_task = asyncio.get_running_loop().create_task(
                self._watchdog_loop(interval_millis)
            )

    def enable_all_led(self, enable):
        self._buf[1] = 0x02
        self._buf[2] = 1 if enable else 0
        self._hid.write(bytes(self._buf))

    def enable_leds(self, chain, enable):
        self._buf[1] = 0x03
        self._buf[2] = chain
        self._buf[3] = 1 if enable else 0
        self._hid.write(bytes(self._buf))

    def set_leds(self, chain, color):
        if isinstance(color, (list, tuple)):
            self._buf[1] = 0x05
            self._buf[2] = chain
            for i, c in enumerate(color):
                self._buf[i * 3 + 3] = (c >> 16) & 0xFF
                self._buf[i * 3 + 4] = (c >> 8) & 0xFF
                self._buf[i * 3 + 5] = c & 0xFF
        else:
            self._buf[1] = 0x04
            self._buf[2] = chain
            self._buf[3] = (color >> 16) & 0xFF
            self._buf[4] = (color >> 8) & 0xFF
            self._buf[5] = color & 0xFF
        self._hid.write(bytes(self._buf))

    def set_led(self, chain, index, color):
        self._buf[1] = 0x04
        self._buf[2] = chain
        self._buf[3] = index
        self._buf[4] = (color >> 16) & 0xFF
        self._buf[5] = (color >> 8) & 0xFF
        self._buf[6] = color & 0xFF
        self._hid.write(bytes(self._buf))
