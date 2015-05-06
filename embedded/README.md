PlantLab
========

The main purpose of this page is to present some of the tools (HW and SW) employed in the [FET OPEN Project PLEASED](http://www.pleased-fp7.eu) to investigate the use of plants as bio sensors.

To investigate the propagation of signals, similarly to what has been already done by [Stankovic et al.](http://www.ncbi.nlm.nih.gov/pmc/articles/PMC158572/), we used a [labjack U6](http://labjack.com/u6) with a custom made instrumentation amplifier

![My image](https://raw.github.com/andreavitaletti/PlantLab/master/configprop.png)

the schematics of a single channel in Eagle

![My image](https://raw.github.com/andreavitaletti/PlantLab/master/Singlechannel.png)

A diagram of the whole process to acquire data from plants is shown in the following

![My image](https://raw.github.com/andreavitaletti/PlantLab/master/diag.png)

A picture of an experiment with 4 plants

![My image](https://raw.github.com/andreavitaletti/PlantLab/master/groupsetup1.png)

The corrensponding output that is stored in [plot.dat](https://raw.github.com/andreavitaletti/PlantLab/master/plot.dat) and the following picture is obianed plotting plot.dat [KST](https://kst-plot.kde.org/).
Notice that KST allow us to plot the data in real-time.

![My image](https://raw.github.com/andreavitaletti/PlantLab/master/output.png)
