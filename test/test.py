import hid
import time

VID = 0x1209
PID = 0x88BF
REPORT_SIZE = 64

print("Opening device")
device = hid.device()
device.open(VID, PID)
print("Device opened")

try:
    time.sleep(0.05)

    for i in range(1, 3):
        outReport = [0x00] * (REPORT_SIZE + 1)

        outReport[0] = 0x00  # report id
        outReport[1] = 0x11  # set underglow
        outReport[2] = 0xFF if (i & 4) else 0x00  # R
        outReport[3] = 0xFF if (i & 2) else 0x00  # G
        outReport[4] = 0xFF if (i & 1) else 0x00  # B

        print("Writing...")
        device.write(outReport)
        print(f"Sent report: {list(outReport[:10])}")

        time.sleep(0.5)

    time.sleep(1.0)

    device.close()

    time.sleep(1.0)

    print("finished")

except Exception as err:
    print(f"Error: {err}")
    device.close()