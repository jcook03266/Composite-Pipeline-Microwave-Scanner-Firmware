# Composite-Pipeline-Microwave-Scanner-Firmware
This robot uses microwave reflectometry to scan non-composite pipelines (plastic or any material in that catergory) for defects. The system works on the basis of a data aggregation algorithm used to cluster up reference scans into a referential data frame that will be used to classify future scans as skewed, normal, or defective. 

The reason for the three categories of classification is because of the feedback loop this system is inherently built upon. The scanner moves along a pipeline in a +/- Z direction at a variable speed controlled by a detection manager that adjusts the speed to get a higher fidelity image of the current section of the pipe. This behavior can be best explained through the following flow chart diagram.

<div align="center">
<img src="https://github.com/jcook03266/Composite-Pipeline-Microwave-Scanner-Firmware/blob/main/Resources/flowchart.png" width="800">
</div>
