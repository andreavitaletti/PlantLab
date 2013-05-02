import wx
import wx.lib.newevent
from numpy import arange, array, hstack, random

from traits.api import Array, Bool, Callable, Enum, Float, HasTraits, Instance, Int, Trait
from traitsui.api import Group, HGroup, Item, View, spring, Handler
from pyface.timer.api import Timer

import u3, u6, ue9, LabJackPython
from time import sleep
from datetime import datetime
import struct
import threading
import Queue
import ctypes, copy, sys
from scipy.signal import lfilter, firwin


# Chaco imports
from chaco.chaco_plot_editor import ChacoPlotItem

app=None
SomeNewEvent, EVT_SOME_NEW_EVENT = wx.lib.newevent.NewEvent()
f0 = open('ain0.out', 'a')
f1 = open('ain1.out', 'a')
#sdrThread=None
#sdr=None

#------------------------------------------------
# Create a FIR filter and apply it to signal.
#------------------------------------------------
# The Nyquist rate of the signal.
sample_rate = 1000.
nyq_rate = sample_rate / 2.
 
# The cutoff frequency of the filter: 6KHz = 6000.0
cutoff_hz = 1.0
#cutoff_hz = 0.003
 
# Length of the filter (number of coefficients, i.e. the filter order + 1)
numtaps = 29
 
# Use firwin to create a lowpass FIR filter
fir_coeff = firwin(numtaps, cutoff_hz/nyq_rate)

class Viewer(HasTraits):
    """ This class just contains the two data arrays that will be updated
    by the Controller.  The visualization/editor for this class is a 
    Chaco plot.
    """
    index0 = Array
    data0 = Array
    filtered_data0 = Array
    index1 = Array
    data1 = Array

    plot_type = Enum("line", "scatter")
    
    # This "view" attribute defines how an instance of this class will
    # be displayed when .edit_traits() is called on it.  (See MyApp.OnInit()
    # below.)
    view = View(ChacoPlotItem("index0", "data0",
                               type_trait="plot_type",
                               resizable=True,
                               x_label="Time",
                               y_label="AIN0",
                               color="blue",
                               bgcolor="white",
                               border_visible=True,
                               border_width=1,
                               padding_bg_color="lightgray",
                               width=800,
                               height=300,
                               show_label=False),
                               ChacoPlotItem("index0", "filtered_data0",
                               type_trait="plot_type",
                               resizable=True,
                               x_label="Time",
                               y_label="FilteredAIN0",
                               color="black",
                               bgcolor="white",
                               border_visible=True,
                               border_width=1,
                               padding_bg_color="lightgray",
                               width=800,
                               height=300,
                               show_label=False),
                               ChacoPlotItem("index1", "data1",
                               type_trait="plot_type",
                               resizable=True,
                               x_label="Time",
                               y_label="AIN1",
                               color="red",
                               bgcolor="white",
                               border_visible=True,
                               border_width=1,
                               padding_bg_color="lightgray",
                               width=800,
                               height=300,
                               show_label=False),
                #HGroup(spring, Item("plot_type", style='custom'), spring),
                resizable = True,
                buttons = ["OK"],
                width=900, height=650)

class Daq:

    def init_Daq(self):
        d = None
        ################################################################################
        ## U6
        ## Uncomment these lines to stream from a U6
        ################################################################################
        ## At high frequencies ( >5 kHz), the number of samples will be MAX_REQUESTS times 48 (packets per request) times 25 (samples per packet)
        d = u6.U6()
        ## For applying the proper calibration to readings.
        d.getCalibrationData()

        #print "configuring U6 stream"
        #d.streamConfig( NumChannels = 1, ChannelNumbers = [ 0 ], ChannelOptions = [ 0 ], SettlingFactor = 1, ResolutionIndex = 1, SampleFrequency = 50000 ) #SampleFrequency = 50000 
        #d.streamConfig( NumChannels = 1, ChannelNumbers = [ 0 ], ChannelOptions = [ 0 ], SettlingFactor = 1, ResolutionIndex = 7, SampleFrequency = 1000 ) #SampleFrequency = 50000 
        #d.streamConfig( NumChannels = 2, ChannelNumbers = [ 0, 1 ], ChannelOptions = [ 0, 0 ], SettlingFactor = 1, ResolutionIndex = 1, ScanFrequency = 5000 )
        d.streamConfig( NumChannels = 2, ChannelNumbers = [ 0, 1 ], ChannelOptions = [ 0, 0 ], SettlingFactor = 1, ResolutionIndex = 1, ScanFrequency = 1000 )
        return d

class StreamDataReader(object):
    def __init__(self, device):
        self.device = device
        self.data = Queue.Queue()
        self.dataCount = 0
        self.missed = 0
        self.running = False

    def readStreamData(self):
        self.running = True
        
        #start = datetime.now()
        self.device.streamStart()
        while self.running:
            # Calling with convert = False, because we are going to convert in
            # the main thread.
            returnDict = self.device.streamData(convert = False).next()
            
            self.data.put_nowait(copy.deepcopy(returnDict))
            
            #self.dataCount += 1
            wx.PostEvent(app, SomeNewEvent())
            #if self.dataCount > MAX_REQUESTS:
            #    self.running = False
        '''
        print "stream stopped."
        self.device.streamStop()
        stop = datetime.now()

        total = self.dataCount * self.device.packetsPerRequest * self.device.streamSamplesPerPacket
        print "%s requests with %s packets per request with %s samples per packet = %s samples total." % ( self.dataCount, d.packetsPerRequest, d.streamSamplesPerPacket, total )
        
        print "%s samples were lost due to errors." % self.missed
        total -= self.missed
        print "Adjusted number of samples = %s" % total
        
        runTime = (stop-start).seconds + float((stop-start).microseconds)/1000000
        print "The experiment took %s seconds." % runTime
        print "%s samples / %s seconds = %s Hz" % ( total, runTime, float(total)/runTime )
        '''

class Controller(HasTraits):
    
    # A reference to the plot viewer object
    viewer = Instance(Viewer)
    
    #d = fakeDaq()
    FIO0_DIR_REGISTER = 6100
    FIO0_STATE_REGISTER = 6000
    FIO1_DIR_REGISTER = 6101
    FIO1_STATE_REGISTER = 6001
    daq = Daq()
    d=daq.init_Daq()
    d.writeRegister(FIO0_DIR_REGISTER, 1)  # Set FIO0 to digital output
    d.writeRegister(FIO1_DIR_REGISTER, 1)  # Set FIO1 to digital output
    d.writeRegister(FIO0_STATE_REGISTER, 0) # Set FIO0 low
    d.writeRegister(FIO1_STATE_REGISTER, 0) # Set FIO1 low
    sdr = StreamDataReader(d)
    sdrThread = threading.Thread(target = sdr.readStreamData)
    #Start the stream and begin loading the result into a Queue
    sdrThread.start()
    
    # The max number of data points to accumulate and show in the plot
    max_num_points = Int(1000)
    
    # The number of data points we have received; we need to keep track of
    # this in order to generate the correct x axis data series.
    num_ticks = Int(0)
    
    gain = Enum("1", "10","100","1000")
    current_gain = 1
    
    # private reference to the random number generator.  this syntax
    # just means that self._generator should be initialized to
    # random.normal, which is a random number function, and in the future
    # it can be set to any callable object.
    #_generator = Trait(random.normal, Callable)
    
    view = View(Group('max_num_points',
                      'gain',  
                      orientation="vertical"),
                      buttons=["OK", "Cancel"])
                      
    
    def manage_data(self, *args):
        eq=False
        try:
            # Check if the thread is still running
            #if not sdr.running:
            #    break
        
            # Pull results out of the Queue in a blocking manner.
            result = self.sdr.data.get(True, 1)
        
            # If there were errors, print that.
            #if result['errors'] != 0:
            #    errors += result['errors']
            #    missed += result['missed']
            #    print "+++++ Total Errors: %s, Total Missed: %s" % (errors, missed)
            
            # Convert the raw bytes (result['result']) to voltage data.
            r = self.d.processStreamData(result['result'])
        
            # Do some processing on the data to show off.
            #print "Average of", len(r['AIN0']), "reading(s):", sum(r['AIN0'])/len(r['AIN0'])
            #print "here I should plot"
            ain0 = r['AIN0']
            for item in ain0:
                f0.write("%s\n" % item)
            chunk = len(r['AIN0'])
            ain1 = r['AIN1']
            for item in ain0:
                f1.write("%s\n" % item)
            
            if chunk != len(r['AIN1']):
                print "strange!"
            # strange behavour depends on the sampling rate
            '''
            print "chunk",chunk
            j2 = [i for i in a if (i >= 1 or i <= -1)]
            print j2
            '''
        except Queue.Empty:
            eq = True
            print "empty"
          
        # the queue is not empty, so the consumer, i.e. the controller, can take data
        if not eq:
            #new_val=ain0
            self.num_ticks += chunk
            #new_val = self._generator(self.mean, self.stddev)
            cur_data = self.viewer.data0
            new_data = hstack((cur_data[-self.max_num_points+chunk+1:], ain0))
            new_index = arange(self.num_ticks - len(new_data) + 1, self.num_ticks+0.1)
            self.viewer.index0 = new_index   
            self.viewer.data0 = new_data
            cur_data = self.viewer.data1
            new_data = hstack((cur_data[-self.max_num_points+chunk+1:], ain1))
            self.viewer.index1 = new_index   
            self.viewer.data1 = new_data
            self.viewer.filtered_data0 = lfilter(fir_coeff, 1.0, new_data)
        return
    # EI-1040 Instrumentatino Amplifier
    # GAIN  GS1 GS2
    # 1     0   0
    # 10    1   0
    # 100   0   1
    # 1000  1   1 
    def _gain_changed(self):
        # This listens for a change in the type of distribution to use.
        if self.gain == "10":
            self.d.writeRegister(self.FIO0_STATE_REGISTER, 1) # Set FIO0 high
            self.d.writeRegister(self.FIO1_STATE_REGISTER, 0) # Set FIO1 low
        elif self.gain == "100":
            self.d.writeRegister(self.FIO0_STATE_REGISTER, 0) # Set FIO0 low
            self.d.writeRegister(self.FIO1_STATE_REGISTER, 1) # Set FIO1 high
        elif self.gain == "1000":
            self.d.writeRegister(self.FIO0_STATE_REGISTER, 1) # Set FIO0 high
            self.d.writeRegister(self.FIO1_STATE_REGISTER, 1) # Set FIO1 high
        else:
            self.d.writeRegister(self.FIO0_STATE_REGISTER, 0) # Set FIO0 low
            self.d.writeRegister(self.FIO1_STATE_REGISTER, 0) # Set FIO1 low

        
   
class DemoHandler(Handler):
    
    def closed(self, info, is_ok):
        """ Handles a dialog-based user interface being closed by the user.
        Overridden here to stop the timer once the window is destroyed.
        """
        
        #info.object.timer.Stop()
        print "stop stop stop"
        return
    
class Demo(HasTraits):
    controller = Instance(Controller)
    viewer = Instance(Viewer, ())
    #timer = Instance(Timer)
    view = View(Item('controller', style='custom', show_label=False), 
                Item('viewer', style='custom', show_label=False), 
                handler = DemoHandler,
                resizable=True)
            
    def edit_traits(self, *args, **kws):        
        # Start up the timer! We should do this only when the demo actually
        # starts and not when the demo object is created.
        return super(Demo, self).edit_traits(*args, **kws)
            
    def configure_traits(self, *args, **kws):        
        # Start up the timer! We should do this only when the demo actually
        # starts and not when the demo object is created.
        return super(Demo, self).configure_traits(*args, **kws)
    
    def _controller_default(self):
        return Controller(viewer=self.viewer)
    
popup=Demo()

# wxApp used when this file is run from the command line.

class MyApp(wx.PySimpleApp):
    
    controller = None
    
    def OnInit(self, *args, **kw):
        viewer = Viewer()
        self.controller = Controller(viewer = viewer)
        
        # Pop up the windows for the two objects
        viewer.edit_traits()
        self.controller.edit_traits()
        self.setup_event(self.controller)
        self.Bind(wx.EVT_CLOSE, self.OnCloseWindow)
        return True
        
    def setup_event(self, controller):
        self.Bind(EVT_SOME_NEW_EVENT, controller.manage_data)
        return
        
    def OnCloseWindow(self, event):
        print "fine"
        self.Destroy()
        return
    def onFinishing(self):
        self.controller.d.streamStop()
        return
        



# This is called when this example is to be run in a standalone mode.
if __name__ == "__main__":
    try:
        app = MyApp()
        app.MainLoop()
    finally:
        f0.close()
        f1.close()
        app.onFinishing()
        #print "finito!"

# EOF
