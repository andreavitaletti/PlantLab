import u6
from time import sleep
from datetime import datetime
import struct
import traceback
import ConfigParser
import thread
import os

################################################################################
## U6
## Uncomment these lines to stream from a U6
################################################################################
## At high frequencies ( >5 kHz), the number of samples will be MAX_REQUESTS times 48 (packets per request) times 25 (samples per packet).
d = u6.U6()
#
## For applying the proper calibration to readings.
d.getCalibrationData()

print "configuring U6 stream"

CONF_FILE = "config.ini"

Config = ConfigParser.ConfigParser()
Config.read(CONF_FILE)

samfreq = int(Config.get('ExperimentalSetUp','sampling_frequency'))

d.streamConfig( NumChannels = 8, ChannelNumbers = [ 0,1,2,3,4,5,6,7 ], ChannelOptions = [ 8,8,8,8,8,8,8,8 ], ResolutionIndex = 2, ScanFrequency = samfreq)  


chunk = 1
start = datetime.now()
file_data_name = start.strftime("%Y-%m-%d-%H-%M-%S")+'_CHUNK_'+str(chunk)+'.dat'
file_report_name = start.strftime("%Y-%m-%d-%H-%M-%S")+'.rep'

print "CURRENT DATA FILE IS:"+file_data_name

folder = Config.get('ExperimentalSetUp','folder')

if not os.path.exists(folder):
    os.makedirs(folder)

fout = open(folder+'/'+file_data_name, "w")
fout_plot = open("plot.dat", "w")

#Copy the config file into the experiment report file
#shutil.copyfile(CONF_FILE, file_report_name)

repfile = open(folder+'/'+file_report_name,'w')

Config.add_section('Event')
Config.set('Event','Started',start.strftime("%Y-%m-%d-%H-%M-%S"))
Config.set('Event','Chunk 1 Start',start.strftime("%Y-%m-%d-%H-%M-%S"))

from Tkinter import *

ST = 1
quit_pressed = False

class App:
    def __init__(self, master):

        frame = Frame(master)
        frame.pack()

        self.button = Button(
            frame, text="QUIT", fg="red", command=frame.quit
			)
        self.button.pack(side=LEFT)

        self.hi_there = Button(frame, text="Register Event", command=self.reg_event)
        self.hi_there.pack(side=LEFT)
        self.textarea = Text(frame, height=2, width=30)
        self.textarea.pack(side=LEFT)
        
    def quit_event(self):
		global quit_pressed
		quit_pressed = True

    def reg_event(self):
        global ST
        global Config
        Config.set('Event','Stimulus '+ str(ST)+" time",datetime.now().strftime("%Y-%m-%d-%H-%M-%S"))
        Config.set('Event','Stimulus '+ str(ST)+" type",self.textarea.get(1.0, END))
        ST = ST + 1
def threadmain():
	root = Tk()

	app = App(root)

	root.mainloop()
	root.destroy() # optional; see description below

thread.start_new_thread(threadmain, ())

try:
    d.streamStart()
    new_start=start
    
    missed = 0
    dataCount = 0
    packetCount = 0
    duration_in_seconds = int(Config.get('ExperimentalSetUp','duration'))* 60
    chunk_duration_in_seconds = int(Config.get('ExperimentalSetUp','chunk_size')) * 60 

    for r in d.streamData():
        if r is not None:
            # Our stop condition
            if (datetime.now()-start).seconds > duration_in_seconds:
                break
                
            if (datetime.now()-new_start).seconds > chunk_duration_in_seconds:
				fout.close()
				new_start = datetime.now()
				chunk = chunk + 1
				Config.set('Event','Chunk '+str(chunk)+' start',new_start.strftime("%Y-%m-%d-%H-%M-%S"))
				file_data_name = start.strftime("%Y-%m-%d-%H-%M-%S")+'_CHUNK_'+str(chunk)+'.dat'
				fout = open(folder+'/'+file_data_name, "w")
            
            if r['errors'] != 0:
                print "Error: %s ; " % r['errors'], datetime.now()

            if r['numPackets'] != d.packetsPerRequest:
                print "----- UNDERFLOW : %s : " % r['numPackets'], datetime.now()

            if r['missed'] != 0:
                missed += r['missed']
                print "+++ Missed ", r['missed']
  
			# the number of samples for each channel are not necessarily the same. Here we take the shortest 
            shortest = min(len(r['AIN0']),len(r['AIN1']),len(r['AIN2']),len(r['AIN3']),len(r['AIN4']),len(r['AIN5']),len(r['AIN6']),len(r['AIN7']))
           
            for a,b,c,d1,e,f,g,h in zip(r['AIN0'][:shortest],r['AIN1'][:shortest],r['AIN2'][:shortest],r['AIN3'][:shortest],r['AIN4'][:shortest],r['AIN5'][:shortest],r['AIN6'][:shortest],r['AIN7'][:shortest]):
				item = ",".join([str(x) for x in [a,b,c,d1,e,f,g,h]])
				fout.write("%s\n" % item)
				fout_plot.write("%s\n" % item)

except KeyboardInterrupt:
    print "END"

except:
    print "".join(i for i in traceback.format_exc())
    
finally:
	stop = datetime.now()
	Config.set('Event','Stop',stop.strftime("%Y-%m-%d-%H-%M-%S"))
	Config.write(repfile)
	d.streamStop()
	d.close()
	fout.close()
	fout_plot.close()
	repfile.close()


