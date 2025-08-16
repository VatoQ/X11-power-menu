#!/bin/bash

gcc main.c -o powermenu $(pkg-config --cflags --libs xft cairo) -lX11

path_to_i3="/home/$USER/.config/i3blocks/scripts"



rm -rf $path_to_i3/powermenu

cp powermenu $path_to_i3
