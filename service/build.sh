#!/bin/bash
g++ -lrt -O3 pixIo_mmap.cpp gpio_mmap.cpp -o pixIo_mmap
g++ -lrt -O3 -DDEBUG_BOUTONS=1 pixIo_mmap.cpp gpio_mmap.cpp -o pixIo_mmap_debug


