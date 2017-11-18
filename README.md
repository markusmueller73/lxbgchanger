lxbgchanger (in short lxbgc)
(c) 2017 by markus dot mueller dot 73 (at) hotmail dot de


INTRO
-----

Program to change the desktop wallpaper for a LXDE desktop with the 
PCManFM filemanager.

The PCManFM is a quite nice filemanager and desktop environment. 
But the functionality to switch the wallpaper after a user defined
time is still missing (maybe in the QT release?).

So I decide to write this small program as an exercise to myself in plain C. I
am still learning plain C so, there maybe many tweaks an experienced programmer
can found in this piece of software.

Have fun to optimize or simple use.


USAGE
-----

Type 'lxdbc -h' for help. Simple as is:

- add 'lxbgc' to your autostart of LXDE/Lubuntu
- use the command line parameter '-d' for your image directory
- optional use the command line parameter '-t' for the time in minutes between changes

E.g.:

lxbgc -d /home/USERNAME/Pictures/Wallpapers -t 15


DEPENDENCIES
------------

You must use the PCManFM for your desktop.

