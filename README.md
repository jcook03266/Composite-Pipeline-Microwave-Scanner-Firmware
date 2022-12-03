# Composite-Pipeline-Microwave-Scanner-Firmware

<div align="left">
<img src="https://github.com/jcook03266/Composite-Pipeline-Microwave-Scanner-Firmware/blob/main/Resources/cluster-shot.png" width="500">
</div>
<div align="right">
<img src="https://github.com/jcook03266/Composite-Pipeline-Microwave-Scanner-Firmware/blob/main/Resources/frontal-shot.png" width="500">
</div>

## Overview
This robot uses microwave reflectometry to scan non-composite pipelines (plastic or any material in that catergory) for defects. The system works on the basis of a data aggregation algorithm used to cluster up reference scans into a referential data frame that will be used to classify future scans as skewed, normal, or defective. 

The reason for the three categories of classification is because of the feedback loop this system is inherently built upon. The scanner moves along a pipeline in a +/- Z direction at a variable speed controlled by a detection manager that adjusts the speed to get a higher fidelity image of the current section of the pipe. This behavior can be best explained through the following flow chart diagram:

<div align="center">
  
## Flow Chart
  
<img src="https://github.com/jcook03266/Composite-Pipeline-Microwave-Scanner-Firmware/blob/main/Resources/flowchart.png" width="800">
</div>
 
<div align="center">
By having a dynamic reactionary movement protocol the robot can control the quality of its data ingestion, and as result improve the system's response to future input. Given the feedback loop it would be pretty evident what kind of control system controller this firmware implements, and that is a PID controller. A PID, or proportional integral derivative controller, relies on multiple components to accomplish its primary goal which is to induce a steady-state by adding a control effect based on the accumulated historical error present in the input data stream, and by adding an anticipatory control effect by predicting the input's next trend based on the rate of change exhibited by the cumulative value of the error. The controller integrates the difference between a desired reference point, so the average of a couple scans on a clean pipe with no defects, and a measured process variable, the error-corrected input going into the system. And, the controller derives the next input value by looking at the error's rate of change; making the system respond ahead of time to limit residual error e(t) of the proportional component of the system whihc is SP-PV, setpoint - processs variable.
  
</div>

<div align="center">
  
## Block Diagram
  
<img src="https://github.com/jcook03266/Composite-Pipeline-Microwave-Scanner-Firmware/blob/main/Resources/block-diagram.png" width="800">
</div>

### So to break things down, here's how the system functions:
## Startup Process:
1.) Robot starts scanning and ingesting data <br>
2.) Movement protocol activates <br>

## States:
3.) Moving and scanning normally, no abnormal data yet <br>
4.) Possible defect detected ? -> Movement speed decreases by -∆Z <br>

## Classification Branching:
5. Defect | Scan fidelity increases, Defect found -> Movement Stops -> Alert broadcasted -> Back to step 1.)
5. No-defect | Scan fidelity increases, Defect not found -> Movement speed increases by +∆Z, to return back to the base ∆Z -> Back to step 3.)

## How is the data interpretted? 

The microwave antennas are powered by a function generator operation at a frequency of 2.1[GHz]; the function generator's signal enters an RF switch board with a addressable pin cluster comprised of 3-pins which allows for 2^3 (8) possible binary combinations, thus 8 individual ports are present on the board, with one being used as RF common for the function generator to enter through and propagate to the other ports. After the signal propagates to the dual-band antennas, the signal is reflected back from the environment and received by the same antenna, communicating back any intelligence present in the signal from the surrounding environmental objects. The reflected signal is then sent through an RF circulator to isolate it from the incoming function generator signal, and received by an RF power meter which then converts the intelligence from the frequency domain to the time domain in the form a voltage through inverse fourier transform. That voltage is the input of this system, it's the process variable and the basis for the desired reference point, it's the only input. 

## What's the abstract role of the signal generator in this system?

The signal generator is a tuning element that produces optimal function control by altering the system response. Microwave antennas operate at specific frequencies, and so do RF power meters, thus a bandwidth limit is placed on the entire circuit dependent on these two factors, however, given a selection of possible frequencies, these frequencies all exhibit different behaviors, and introduce varying degrees of error into the system's input vector. Through some testing I've found out that 2.1[GHz] lies within the system's bandwidth, and produces very favorable results. At lower frequencies the antennas all have drastically different outputs, which is bad because the accuracy of the system is dependent on the antennas working together to produce a uniform dataset unperturbed by error. Increasing the frequency resolves this issue and bundles up the antennas' output values closer to one another.
