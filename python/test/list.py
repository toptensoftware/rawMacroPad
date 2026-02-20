import hid

VID = 0x1209
PID = 0x88BF

devices = hid.enumerate(VID, PID)
for d in devices:
    print(d)
