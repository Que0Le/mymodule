#!/bin/bash
#for i in `seq 1 4`;
#  do
#  gnome-terminal -- /bin/sh -c 'cd /home/que/Desktop/mymodule; exec bash'
#done 
gnome-terminal --working-directory=/home/que/Desktop/mymodule
gnome-terminal --working-directory=/home/que/Desktop/mymodule
gnome-terminal --working-directory=/home/que/Desktop/mymodule --geometry=170x25 -- /bin/sh -c 'dmesg -wH; exec bash'