FormsOfResistance

This folder cotains companion data for a non-yet accepted publication

1/ Data

Data from three recording are stored on 16-bit integers at 256Hz.
There is no header. Each file -with extension ".sig" contains a single EEG channel.
Each data folder contains six ".sig" files. The first channels -from E1 to E3- are in the focal area. The last one have been collected outside the area where the seizure started.
The file "crise.txt" contains the onset -in secondes- of the seizures for this recording.

2/ Scripts

Scripts for the first steps of the processing are provided. They can help you to reprocess the data used for the publication or to work with your own dataset. Scripts are in Matlab (tested of 2006b) or in C (Lerner density estimation need a lot of simple operations, C in more efficent for that).  
With the current scripts you can filter the data (remove 50Hz noise), prepare batch to launch several "Lerner density estimation" in parallel (depending on your microprocessor and OS), estimate Lerner density and convert data format for the next step (reparametrisations, extraction of parameters and figures creation). 
More scripts will be available once the paper accepted. 
