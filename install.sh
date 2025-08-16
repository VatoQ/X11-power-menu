#!/bin/bash

gcc main.c -o powermenu $(pkg-config --cflags --libs xft cairo) -lX11
