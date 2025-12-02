# arduino-mill
This could be a replacement for the control PCB of a Tunturi(R) Platinum(R) Treadmill 11PTTR1000 1.8kW, maybe for other models too. the inverter PCB and the base PCB are still working, but the microcontroller fujitsu MB95F118NS seams dead. also the video-controller weltrend WT8881A doesn't drive the display. we did a reverse engineering of the connection between the top and base PCB to control the treadmill with a arduino. 


control-pcb: YJ-3002-V1.2 20110114  
base-pcb: 2009 V1.0 SJ-1246  
inverter: rhymebus RM5LD-2002 (input 1PH AC200-240V, output 3PH AC200-240V 7.5A)


connector controlPCB - basePCB
pin  description
1. enable inverter, active 16.7V
2. inclination down, active GND 
3. inclination up, active GND
4. V 16.7V from basePCB, power supply
5. increase speed, frequency (0.7-20Hz?) gives the acceleration rate
6. decrease speed, frequency (0.7-20Hz?) gives the deceleration rate
7. GND
8. GND sensor
9. pulse of wheel, idle 5V 
10. 5V, to sensor
11. analog sensor height
12. GND

connector basePCB - inverter (connection between 3 and 6, parallel swithing U1 and U4, optocoupler)
pin  description
1. U2 increase speed, frequency (0.7-20Hz?) gives the acceleration rate
2. U3 decrease speed, frequency (0.7-20Hz?) gives the deceleration rate
3. U1 enable
4. U4 enable 
5. common
6. U1 enable




