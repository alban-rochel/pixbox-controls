pixbox-controls
=========

This is the code for the joystick mapping in the Pixbox project: http://pixbox-project.blogspot.fr/

There are several pieces of code here, I suggest you take a look at the project page to get what they mean.
* overlay folder: the Linux device-tree configuration to setup the GPIO properly
* service: the "service" that polls the GPIO and maps the status changes as keyboard events
  * file am335x.h is provided by Ethan Hayon under an MIT License
    * See https://github.com/ehayon/BeagleBone-GPIO for original code and license
  * file gpio_mmap.cpp is adapted from gpio.c, provided by Ethan Hayton under an MIT License
    * See https://github.com/ehayon/BeagleBone-GPIO for original code and license

License
-------

Some of the code is copied or adapted from the works of Ethan Hayton, and is distributed under the MIT License.

As for the other pieces of code, you can do whatever pleases with them. I would just be happy if you could post a comment on the Pixbox project if you end up using it or deriving it!
	
Acknowledgements
-------

The whole GPIO control scheme is inspired by the tutorials of Derek Molloy. Go have a look at his works! http://derekmolloy.ie/tag/beaglebone-black/
