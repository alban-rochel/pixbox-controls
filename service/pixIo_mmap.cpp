#include <list>
#include <iostream>
#include <linux/input.h>
#include <linux/uinput.h>
#include <cstdio>
#include <cstring>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

//#define DEBUG_PIX 1

#define DEBOUNCE_MICROSECS 3000

#include "gpio_mmap.h"

using namespace std;

int uinputFd;

class Button
{
	public:
		PIN gpio;
		__u16 key;
		struct timespec lastChange;
		bool isPressed;
		bool toBeTested;
#ifdef DEBUG_BOUTONS
		std::string description;
#endif

		Button(const PIN& gpio, __u16 key, const std::string& description):
			gpio(gpio),
//			children(),
			key(key),
			isPressed(false),
			toBeTested(true)
#ifdef DEBUG_BOUTONS
			, description(description)
#endif
		{
			clock_gettime(CLOCK_MONOTONIC, &lastChange);
		}

		bool hasChanged(bool& pressed);
};

list<Button*> buttons;

list<PIN> gpios;

void createButtons()
{
	buttons.clear();

#define ADD_BUTTON(_key, _gpio, _description) \
	{ \
		gpios.push_back(_gpio); \
		Button* button = new Button(_gpio, _key, _description); \
		buttons.push_back(button); \
	}

	// QUIT
	ADD_BUTTON(KEY_ESC, P8_8, "Quit");
	// COIN
	ADD_BUTTON(KEY_5, P9_15, "Coin");

	// 1P Up
	ADD_BUTTON(KEY_UP, P8_10, "P1 Up");
	// 1P Left
        ADD_BUTTON(KEY_LEFT, P8_14, "P1 Left");
	// 1P Right
        ADD_BUTTON(KEY_RIGHT, P8_16, "P1 Right");
	// 1P Down
        ADD_BUTTON(KEY_DOWN, P8_12, "P1 Down");
	// 1P start
	ADD_BUTTON(KEY_1, P8_7, "P1 Start");
	// 1P fire 1
	ADD_BUTTON(KEY_A, P8_9, "P1 1");
	// 1P fire 2
	ADD_BUTTON(KEY_S, P8_13, "P1 2");
	// 1P fire 3
	ADD_BUTTON(KEY_D, P8_17, "P1 3");
	// 1P fire 4
	ADD_BUTTON(KEY_Q, P8_26, "P1 4");
	// 1P fire 5
	ADD_BUTTON(KEY_W, P8_11, "P1 5");
	// 1P fire 6
	ADD_BUTTON(KEY_E, P8_15, "P1 6");
	// 1P mode
	ADD_BUTTON(KEY_Z, P8_19, "P1 7");
	// 1P extra button
	ADD_BUTTON(KEY_X, P9_26, "P1 8");


	// 2P Up
        ADD_BUTTON(KEY_I, P9_17, "P2 Up");
        // 2P Left
        ADD_BUTTON(KEY_J, P9_23, "P2 Left");
        // 2P Right
        ADD_BUTTON(KEY_L, P9_27, "P2 Right");
        // 2P Down
        ADD_BUTTON(KEY_K, P9_21, "P2 Down");
	// 2P start
	ADD_BUTTON(KEY_2, P9_11, "P2 Start");
	// 2P fire 1
	ADD_BUTTON(KEY_F, P9_13, "P2 1");
	// 2P fire 2
	ADD_BUTTON(KEY_G, P9_14, "P2 2");
	// 2P fire 3
	ADD_BUTTON(KEY_H, P9_18, "P2 3");
	// 2P fire 4
	ADD_BUTTON(KEY_B, P9_24, "P2 4");
	// 2P fire 5
	ADD_BUTTON(KEY_T, P9_12, "P2 5");
	// 2P fire 6
	ADD_BUTTON(KEY_Y, P9_16, "P2 6");
	// 2P mode
	ADD_BUTTON(KEY_V, P9_22, "P2 7");
	// 2P extra button
	ADD_BUTTON(KEY_R, P9_30, "P2 8");

#undef ADD_BUTTON

}

#define SYSFS_GPIO_DIR "/sys/class/gpio"
#define MAX_BUF 64
int gpio_export(unsigned int gpio)
{
#ifdef DEBUG_PIX
	printf("Exporting gpio %d \n", gpio);
#endif
	int fd, len;
	char buf[MAX_BUF];

	fd = open(SYSFS_GPIO_DIR "/export", O_WRONLY);
	if (fd < 0) {
#ifdef DEBUG_PIX
	printf("**** FAILED Exporting gpio %d \n", gpio);
#endif
		return fd;
	}

	len = snprintf(buf, sizeof(buf), "%d", gpio);
	write(fd, buf, len);
	close(fd);

#ifdef DEBUG_PIX
	printf("DONE Exporting gpio %d \n", gpio);
#endif

	return 0;
}

int gpio_set_input(unsigned int gpio)
{
#ifdef DEBUG_PIX
	printf("Setting input gpio %d \n", gpio);
#endif
	int fd;
	char buf[MAX_BUF];

	snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR  "/gpio%d/direction", gpio);

	fd = open(buf, O_WRONLY);
	if (fd < 0) {
#ifdef DEBUG_PIX
	printf("**** FAILED Setting input gpio %d \n", gpio);
#endif
		return fd;
	}

	write(fd, "in", 3);

	close(fd);

#ifdef DEBUG_PIX
	printf("DONE Setting input gpio %d \n", gpio);
#endif
	return 0;
}

void initGpios()
{
  for(list<PIN>::const_iterator gpioIt = gpios.begin();
      gpioIt != gpios.end();
      ++gpioIt)
  {
    gpio_export((*gpioIt).gpio);
    gpio_set_input((*gpioIt).gpio);
  }
}

void initUinput()
{
	uinputFd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);

	if(uinputFd<0)
		printf("error opening uinput\n");

	int val(-1);
	val = ioctl(uinputFd, UI_SET_EVBIT, EV_KEY);
	if(val == -1)
		printf("err setting EV_KEY\n");

	val = ioctl(uinputFd, UI_SET_EVBIT, EV_SYN);
	if(val == -1)
		printf("err setting EV_SYN\n");

#define ENABLE(key) \
	val = ioctl(uinputFd, UI_SET_KEYBIT, key); if(val==-1) printf("failed setting key\n");

	for(list<Button*>::const_iterator buttonIt = buttons.begin();
			buttonIt != buttons.end();
			++buttonIt)
	{
		ENABLE((*buttonIt)->key);

	}

#undef ENABLE

  uinput_user_dev uidev;
  memset(&uidev, 0, sizeof(uidev));
  snprintf(uidev.name, UINPUT_MAX_NAME_SIZE, "pixbox-controller");
  uidev.id.bustype = BUS_USB;
  uidev.id.vendor  = 0x1234;
  uidev.id.product = 0x4242;
  uidev.id.version = 1;

  val=write(uinputFd,&uidev,sizeof(uidev));
  if(val == -1)
    printf("failed conf\n");

  val=ioctl(uinputFd, UI_DEV_CREATE);
  if(val == -1)
    printf("failed create\n");
}

void sendKey(__u16 key, unsigned int val)
{
  struct input_event ev;
  memset(&ev, 0, sizeof(ev));

  ev.type = EV_KEY;
  ev.code = key;
  ev.value = val;
  write(uinputFd, &ev, sizeof(ev));
}

void syncKeys()
{
  struct input_event ev;
  memset(&ev, 0, sizeof(ev));

  ev.type = EV_SYN;
  ev.code = SYN_REPORT;
  write(uinputFd, &ev, sizeof(ev));
}



void mainLoop()
{
  struct timespec now;

  while(1)
  {
    // reset
    for(list<Button*>::iterator buttonIt = buttons.begin();
        buttonIt != buttons.end();
        ++buttonIt)
    {
      Button* button = *buttonIt;
      button->toBeTested = true;
    }

    clock_gettime(CLOCK_MONOTONIC, &now);

    bool hasChanged(false);
    // check what has changed
    for(list<Button*>::iterator buttonIt = buttons.begin();
        buttonIt != buttons.end();
        ++buttonIt)
    {
      Button* button = *buttonIt;

      unsigned long long deltaUs = (now.tv_sec * 1000000l + now.tv_nsec/1000l) - (button->lastChange.tv_sec * 1000000l + button->lastChange.tv_nsec/1000l);

      if(button->toBeTested && deltaUs > DEBOUNCE_MICROSECS)
      {
        bool pressed(true);

	pressed = !(digitalRead(button->gpio));


        if(pressed != button->isPressed)
        {
          button->lastChange = now;
          button->isPressed = pressed;

          hasChanged = true;

          // emit key
          if(button->isPressed)
          {
#ifdef DEBUG_BOUTONS
printf("Pressed %s\n", button->description.c_str());
#endif
            sendKey(button->key, 1);
          }
          else
          {
#ifdef DEBUG_BOUTONS
printf("Released %s\n", button->description.c_str());
#endif
            sendKey(button->key, 0);
          }

        } // end if(pressed != button->isPressed)

      } // end if(button->toBeTested)
    } // for(list<Button*>::iterator buttonIt = buttons.begin();

    if(hasChanged)
      syncKeys();
    usleep(3000);
  } // end while(1)
}

int main(int argc, char**argv)
{
	createButtons();

	initGpios();

	initUinput();

	mainLoop();

	// TODO cleanup

	return 0;
}
