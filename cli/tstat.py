from bw2python.bwtypes import PayloadObject
import msgpack
import time
from bluepy import btle

class MyDelegate(btle.DefaultDelegate):
    def __init__(self):
        btle.DefaultDelegate.__init__(self)
    def handleNotification(self, cHandle, data):
        print "got notification!" 
        data = [int(x) for x in bytearray(data)]
        print int(data[0]), int(data[1])
        temp_in = (int(data[0]) + (int(data[1]) << 8)) / 10.
        hsp = (int(data[2]) + (int(data[3]) << 8)) / 10.
        csp = (int(data[4]) + (int(data[5]) << 8)) / 10.
        state = data[6]
        is_heating = (state & 0x08) > 0
        is_cooling = (state & 0x04) > 0
        is_fan_on = (state & 0x02) > 0
        is_tstat_on = (state & 0x01) > 0
        print temp_in, hsp, csp, is_heating, is_cooling, is_fan_on, is_tstat_on
        if is_heating:
            state = 1
        elif is_cooling:
            state = 2
        elif not is_tstat_on:
            state = 0
        else:
            state = 3
        msg = {
            'temperature': float(temp_in),
            'heating_setpoint': float(hsp),
            'cooling_setpoint': float(csp),
            'override': True,
            'fan': is_fan_on,
            'mode': 3,
            'state': state,
            'time': int(time.time()*1e9),
        }
        print msg

p = btle.Peripheral("C0:98:E5:80:80:80")
p.setDelegate(MyDelegate())

svc = p.getServiceByUUID("7bad890f-2883-4587-e64e-ea96a0dea487")
ch = svc.getCharacteristics("7bad8911-2883-4587-e64e-ea96a0dea487")
print svc
print ch
print "h1", ch[0].getHandle()
notifyhandle = ch[0].getHandle()+1
p.writeCharacteristic(notifyhandle, b"\x01\x00", withResponse=True)

while True:
    if p.waitForNotifications(10):
        continue
    print "Waiting"
    #ch[0].write(
    #print([hex(x) for x in ch[0].read()])
