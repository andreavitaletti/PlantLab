"""
This demo shows how Chaco and Traits can be used to easily build a data
acquisition and visualization system.

Two frames are opened: one has the plot and allows configuration of
various plot properties, and one which simulates controls for the hardware
device from which the data is being acquired; in this case, it is a mockup
random number generator whose mean and standard deviation can be controlled
by the user.
"""

# Major library imports
import random
import wx
import wx.lib.newevent
from numpy import arange, array, hstack, random

# Enthought imports
#from enthought.traits.api import Array, Bool, Callable, Enum, Float, HasTraits, \
#                                 Instance, Int, Trait
#from enthought.traits.ui.api import Group, HGroup, Item, View, spring, Handler
#from enthought.pyface.timer.api import Timer

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


# Chaco imports
#from enthought.chaco.chaco_plot_editor import ChacoPlotItem
# Chaco imports
from chaco.chaco_plot_editor import ChacoPlotItem

app=None
SomeNewEvent, EVT_SOME_NEW_EVENT = wx.lib.newevent.NewEvent()

class Viewer(HasTraits):
    """ This class just contains the two data arrays that will be updated
    by the Controller.  The visualization/editor for this class is a 
    Chaco plot.
    """
    
    index = Array
    
    data = Array

    plot_type = Enum("line", "scatter")
    
    # This "view" attribute defines how an instance of this class will
    # be displayed when .edit_traits() is called on it.  (See MyApp.OnInit()
    # below.)
    view = View(ChacoPlotItem("index", "data",
                               type_trait="plot_type",
                               resizable=True,
                               x_label="Time",
                               y_label="Signal",
                               color="blue",
                               bgcolor="white",
                               border_visible=True,
                               border_width=1,
                               padding_bg_color="lightgray",
                               width=800,
                               height=380,
                               show_label=False),
                HGroup(spring, Item("plot_type", style='custom'), spring),
                resizable = True,
                buttons = ["OK"],
                width=800, height=500)
class fakeDaq:

    def get_Data(self):
        a = [1,2,3,4,5]
        return a

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
        d.streamConfig( NumChannels = 2, ChannelNumbers = [ 0, 1 ], ChannelOptions = [ 0, 0 ], SettlingFactor = 1, ResolutionIndex = 1, ScanFrequency = 5000 )
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
        
        start = datetime.now()
        self.device.streamStart()
        while self.running:
            # Calling with convert = False, because we are going to convert in
            # the main thread.
            returnDict = self.device.streamData(convert = False).next()
            
            self.data.put_nowait(copy.deepcopy(returnDict))
            
            self.dataCount += 1
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
    print "ciao"
    daq = Daq()
    d=daq.init_Daq()
    sdr = StreamDataReader(d)
    sdrThread = threading.Thread(target = sdr.readStreamData)
    #Start the stream and begin loading the result into a Queue
    sdrThread.start()
    
    # Some parameters controller the random signal that will be generated
    distribution_type = Enum("normal", "lognormal")
    mean = Float(0.0)
    stddev = Float(1.0)
    
    # The max number of data points to accumulate and show in the plot
    max_num_points = Int(1000)
    
    # The number of data points we have received; we need to keep track of
    # this in order to generate the correct x axis data series.
    num_ticks = Int(0)
    
    # private reference to the random number generator.  this syntax
    # just means that self._generator should be initialized to
    # random.normal, which is a random number function, and in the future
    # it can be set to any callable object.
    _generator = Trait(random.normal, Callable)
    
    view = View(Group('distribution_type', 
                      'mean', 
                      'stddev',
                      'max_num_points',
                      orientation="vertical"),
                      buttons=["OK", "Cancel"])
                      
    def prova(self):
        a = [1,2,3,4,5]
        return a
        
    def timer_tick1(self, *args):
        # Generate a new number and increment the tick count
        a = self.d.get_Data()
        #a = self.prova()
        
        #[1,2,3,4,5]
		#a = self.test()
		#a=args[0] 
        chunk=len(a)
		#print "-->",chunk
        new_val=a
        self.num_ticks += chunk
		#new_val = self._generator(self.mean, self.stddev)
        cur_data = self.viewer.data
        new_data = hstack((cur_data[-self.max_num_points+chunk+1:], new_val))
        new_index = arange(self.num_ticks - len(new_data) +chunk + 1, self.num_ticks+0.01)
        self.viewer.index = new_index   
        self.viewer.data = new_data
        return
    
    def timer_tick2(self, *args):
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
            a = r['AIN0']
            #print a
            chunk = len(r['AIN0'])
        except Queue.Empty:
            eq = True
            #print "Queue is empty. Stopping..."
            #sdr.running = False
            #break
        #except KeyboardInterrupt:
            #sdr.running = False
        #except Exception, e:
            #print type(e), e
            #sdr.running = False
            #break

        # Generate a new number and increment the tick count
        #a = self.d.get_Data()
        #a = self.prova()
        
        #[1,2,3,4,5]
		#a = self.test()
		#a=args[0] 
        #chunk=len(a)
		#print "-->",chunk
        if not eq:
            new_val=a
            self.num_ticks += chunk
            #new_val = self._generator(self.mean, self.stddev)
            cur_data = self.viewer.data
            new_data = hstack((cur_data[-self.max_num_points+chunk+1:], new_val))
            new_index = arange(self.num_ticks - len(new_data) +chunk + 1, self.num_ticks+0.01)
            self.viewer.index = new_index   
            self.viewer.data = new_data
        return
        
    def timer_tick(self, *args):
        # Generate a new number and increment the tick count
        new_val = self._generator(self.mean, self.stddev)
        self.num_ticks += 1
        
        # grab the existing data, truncate it, and append the new point.
        # This isn't the most efficient thing in the world but it works.
        cur_data = self.viewer.data
        new_data = hstack((cur_data[-self.max_num_points+1:], [new_val]))
        new_index = arange(self.num_ticks - len(new_data) + 1, self.num_ticks+0.01)
        
        self.viewer.index = new_index
        self.viewer.data = new_data
        return
            

#def _distribution_type_changed(self):
        # This listens for a change in the type of distribution to use.
#        if self.distribution_type == "normal":
#            self._generator = random.normal
#        else:
#            self._generator = random.lognormal
            
#===============================================================================
# # Demo class that is used by the demo.py application.
#===============================================================================
# NOTE: The Demo class is being created for the purpose of running this
# example using a TraitsDemo-like app (see examples/demo/demo.py in Traits3). 
# The demo.py file looks for a 'demo' or 'popup' or 'modal popup' keyword
# when it executes this file, and displays a view for it.
   
class DemoHandler(Handler):
    
    def closed(self, info, is_ok):
        """ Handles a dialog-based user interface being closed by the user.
        Overridden here to stop the timer once the window is destroyed.
        """
        
        info.object.timer.Stop()
        return
    
class Demo(HasTraits):
    controller = Instance(Controller)
    viewer = Instance(Viewer, ())
    timer = Instance(Timer)
    view = View(Item('controller', style='custom', show_label=False), 
                Item('viewer', style='custom', show_label=False), 
                handler = DemoHandler,
                resizable=True)
            
    def edit_traits(self, *args, **kws):        
        # Start up the timer! We should do this only when the demo actually
        # starts and not when the demo object is created.
        self.timer=Timer(100, self.controller.timer_tick_new)
        return super(Demo, self).edit_traits(*args, **kws)
            
    def configure_traits(self, *args, **kws):        
        # Start up the timer! We should do this only when the demo actually
        # starts and not when the demo object is created.
        self.timer=Timer(100, self.controller.timer_tick_new)
        return super(Demo, self).configure_traits(*args, **kws)
    
    def _controller_default(self):
        return Controller(viewer=self.viewer)
    
popup=Demo()

# wxApp used when this file is run from the command line.

class MyApp(wx.PySimpleApp):
    
    def OnInit(self, *args, **kw):
        viewer = Viewer()
        controller = Controller(viewer = viewer)
        
        # Pop up the windows for the two objects
        viewer.edit_traits()
        controller.edit_traits()
        #EVT_QUANTITY_CHANGED = wx.PyEventBinder(wx.NewEventType(), 17)
        
        # Set up the timer and start it up
        # self.setup_timer(controller)
        self.setup_event(controller)
        return True
        
    def setup_event(self, controller):
		
		#self.Bind(EVT_SOME_NEW_EVENT, self.handler)
        self.Bind(EVT_SOME_NEW_EVENT, controller.timer_tick2)
        return

    def handler (self, evt):
        print "handler"
        controller.timer_tick2
        return

    def setup_timer(self, controller):
        # Create a new WX timer
        timerId = wx.NewId()
        self.timer = wx.Timer(self, timerId)
        self.Bind(wx.EVT_TIMER, controller.timer_tick2, id=timerId)
        self.timer.Start(50.0, wx.TIMER_CONTINUOUS)
        return

# This is called when this example is to be run in a standalone mode.
if __name__ == "__main__":
    app = MyApp()
    app.MainLoop()    

# EOF
