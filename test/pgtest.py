import PyTango
import numpy
import time
import random

nframes = 50
exp_time = 0.01

cam=PyTango.DeviceProxy('pointgrey/tango/1')
lima=PyTango.DeviceProxy('limaccd/tango/1')

for i in range(2):
    # do acquisition
    lima.write_attribute("acq_nb_frames",nframes)
    lima.write_attribute("latency_time",0.01)
    lima.write_attribute("acq_expo_time",exp_time)
    lima.write_attribute("acq_trigger_mode", "INTERNAL_TRIGGER")
    camera.write_attribute("frame_rate", 10)

    lima.command_inout("prepareAcq")
    start = time.time()
    lima.command_inout("startAcq")
    print "Running "
    while lima.read_attribute("acq_status").value == "Running" :
        time.sleep(.2)

    print "Completed run in ", time.time()-start, "secs"
print "Done"
