# LP4MRR_MSP430_Speedtrap_LCD_1_2
Code Composer Written Speed Trap with LCD for the Launchpads for Model Railroading Project


Please view the Layout Schematics & Instructions PDF

------------------------------------------------------
Revision History::

6/1/2015

I discovered recently while reviewing this repository that the schematic design had two serious "but not dangerous" issues that needed to
be corrected.  The pinout for the LCD was backwards, and a ground trace was missing on the schematic.  These issues have been resolved.
I have also included larger versions of the schematic and breadboard layouts in both PDF & PNG formats.  There is currently no Circuit
Board design associated with this project.  If anyone wishes to develop one that provides for the MSP430G2553 IC and a regulated 3.3
Volts & 5 Volt Supply as required for the circuit please by all means do so and share.  I'm all about fair attribution and open to
improvement ideas.
----------

6/5/2015

I discovered that the speed & counter variables were not clearing properly.  This was corrected by adding counter = 0 and speed = 0 to the
primary WHILE loop.  This error resulted in a miscalculation of the speed. The LCD code was also relocated to speed the processing of the
counter and reduce program lag time.
----------


