This file contains a bit of basic info, how to run all the different nodes in the project.


Firstly, you have to start the robot up, by providing it with a battery and a computer. Then you have to click the button on the side of the little plastic box on top of it(just beside the kinect), so the green light turns on, identifying that the kinect sensor has power (the sensor itself needs more electricity than is provided with the USB, that's why you have to click the button).

then you have to connect the computer via the usb cable.
!!!!!! USE THE 2.0 USB port on the computer, otherwise, there may be issues. !!!!!!!

Then go to the console, change PWD(via the 'cd' command) to exercise6, open up a few tabs( ctrl+shift+t ), and begin by running:

roslaunch turtlebot_bringup minimal.launch

in a seperate tab run:

roslaunch exercise6 amcl_demo.launch

also run:

roslaunch exercise6 launchPointCLoudManipulations.launch(this sets up all the nodes with the ring and cylinder location and color data).


Then, on a stationairy machine run:

roslaunch turtlebot_rviz bringup_navigation.launch

and for the goal node, launch:
roslaunch exercise6 mapGoals.launch (you can run this one anywhere..)


That's about it actually...

The todoList.txt contains the TO-DOs for our project. Feel free to edit anything.
