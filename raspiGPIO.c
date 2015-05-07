/* GPIO Untility functions, mostly taken from 
 *   developer.ridgerun.com/wiki/index.php/Gpio-int-test.c
 * 
 * Used to demo gpio communication with RASPI
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
// alsa-project header
#include "sampler.h"
#include "config.h"
#include "raspiGPIO.h"
#include "device.h"

int raspiPinMap[] = {2, 3, 4, 7, 8, 9, 10, 11, 14, 15, 17, 18, 22, 23, 24, 25, 27};
int raspiStateMemory[NUM_RASPI_GPIO];
int numStateMemory;

/****************************************************************
 * Include gpio header interface 								*
 ****************************************************************/

#include "raspiGPIO.h"

/****************************************************************
 * gpio_export 													*
 ****************************************************************/

int gpio_export(unsigned int gpio)
{
	int fd, len;
	char buf[MAX_BUF];
 
	fd = open(SYSFS_GPIO_DIR "/export", O_WRONLY);
	if (fd < 0) {
		perror("gpio/export");
		return fd;
	}
 
	len = snprintf(buf, sizeof(buf), "%d", gpio);
	write(fd, buf, len);
	close(fd);
 
	return 0;
}

/****************************************************************
 * gpio_unexport												*
 ****************************************************************/

int gpio_unexport(unsigned int gpio)
{
	int fd, len;
	char buf[MAX_BUF];
 
	fd = open(SYSFS_GPIO_DIR "/unexport", O_WRONLY);
	if (fd < 0) {
		perror("gpio/export");
		return fd;
	}
 
	len = snprintf(buf, sizeof(buf), "%d", gpio);
	write(fd, buf, len);
	close(fd);
	return 0;
}

/****************************************************************
 * gpio_set_dir													*
 ****************************************************************/

int gpio_set_dir(unsigned int gpio, unsigned int out_flag)
{
	int fd, len;
	char buf[MAX_BUF];
 
	len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR  "/gpio%d/direction", gpio);
 
	fd = open(buf, O_WRONLY);
	if (fd < 0) {
		perror("gpio/direction");
		return fd;
	}
 
	if (out_flag)
		write(fd, "out", 4);
	else
		write(fd, "in", 3);
 
	close(fd);
	return 0;
}

/****************************************************************
 * gpio_set_value												*
 ****************************************************************/

int gpio_set_value(unsigned int gpio, unsigned int value)
{
	int fd, len;
	char buf[MAX_BUF];
 
	len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/value", gpio);
 
	fd = open(buf, O_WRONLY);
	if (fd < 0) {
		perror("gpio/set-value");
		return fd;
	}
 
	if (value)
		write(fd, "1", 2);
	else
		write(fd, "0", 2);
 
	close(fd);
	return 0;
}

/****************************************************************
 * gpio_get_value												*
 ****************************************************************/

int gpio_get_value(unsigned int gpio, unsigned int *value)
{
	int fd, len;
	char buf[MAX_BUF];
	char ch;

	len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/value", gpio);
 
	fd = open(buf, O_RDONLY);
	if (fd < 0) {
		perror("gpio/get-value");
		return fd;
	}
 
	read(fd, &ch, 1);

	if (ch != '0') {
		*value = 1;
	} else {
		*value = 0;
	}
 
	close(fd);
	return 0;
}


/****************************************************************
 * gpio_set_edge
 ****************************************************************/

int gpio_set_edge(unsigned int gpio, char *edge)
{
	int fd, len;
	char buf[MAX_BUF];

	len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/edge", gpio);
 
	fd = open(buf, O_WRONLY);
	if (fd < 0) {
		perror("gpio/set-edge");
		return fd;
	}
 
	write(fd, edge, strlen(edge) + 1); 
	close(fd);
	return 0;
}

/****************************************************************
 * gpio_fd_open
 ****************************************************************/

int gpio_fd_open(unsigned int gpio)
{
	int fd, len;
	char buf[MAX_BUF];

	len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/value", gpio);
 
	fd = open(buf, O_RDONLY | O_NONBLOCK);
	if (fd < 0) {
		perror("gpio/fd_open");
	}
	return fd;
}

/****************************************************************
 * gpio_fd_close
 ****************************************************************/

int gpio_fd_close(int fd)
{
	return close(fd);
}

/******************** wrapper functions! ***********************/

/****************************************************************
 * gpio_OPEN(unsigned int gpio) 								*
 * args: 	gpio == gpio id to open								*
 * ret: 	the fd of the opened gpio							*
 ****************************************************************/

int gpio_OPEN(unsigned int gpio) {
  	fprintf(stderr, "SETTING UP GPIO %d!\n", gpio);

  	// export gpio
  	gpio_export(gpio);
  	// set direction
  	gpio_set_dir(gpio, 0);
  	// set edge
  	gpio_set_edge(gpio, "rising");

  	return gpio_fd_open(gpio);
}

/****************************************************************
 * gpio_CLOSE(unsigned int gpio_fd) 							*
 * args: 	gpio_fd == gpio file descriptor to close			*
 * ret: 	1 if successful, -1 if failure						*
 ****************************************************************/

 int gpio_CLOSE(unsigned int gpio_fd) {
   	fprintf(stderr, "CLOSING GPIO (FD = %d)!\n", gpio_fd);
   	return gpio_fd_close(gpio_fd);
 }

 /***************************************************************
  * gpio_GETVAL(unsigned int gpio_fd)							*
  * args: 	gpio_fd == gpio file descriptor to read from		*
  * ret: 	value of the gpio 									*
  ***************************************************************/

int gpio_GETVAL(unsigned int gpio_fd) {
   int retval = -1;
   gpio_get_value(gpio_fd, &retval);
   return retval;
}

 /***************************************************************
  * gpio_SETVAL(unsigned int gpio_fd)							*
  * args: 	gpio_fd == gpio file descriptor to write to			*
  * ret: 	0 if success, fd if not 							*
  ***************************************************************/

int gpio_SETVAL(unsigned int gpio_fd, int val) {
   return gpio_set_value(gpio_fd, val);
}

/****************************************************************
 * readRaspiGPIO() 												*
 ****************************************************************/

void readRaspiGPIO(int fd, char* val) {
	char ch;
	// fprintf(stderr, " - READ RASPI GPIO CALLED FOR FD %d\n", fd);

	read(fd, &ch, 1);

	if (ch != '0') {
		*val = 1;
	} else {
		*val = 0;
	}
	// *val = (char)gpio_GETVAL(fd);
	return;
}

/****************************************************************
 * initRaspiGPIO() 												*
 ****************************************************************/

int initRaspiGPIO(CONFIG* c) {
	int i = 0, j = 0, idx = 0;

	// populate raspiGPIOpin array with default values
	fprintf(stderr, "*** INIT RASPI GPIO CALLED! ***\n");

	// set reading ports
	for (i = 0, idx = 0; i < c->numReadPorts; i++) {
		// verify legal pin
		for (j = 0; j < NUM_RASPI_GPIO; j++)
			if (c->readMap[i] == raspiPinMap[j])
				break;
		
		if (j == NUM_RASPI_GPIO) {
			fprintf(stderr, "*** ERROR: invalid raspi gpio pin! ***\n");
			break;
		}
		else {
			fprintf(stderr, " - setting to READ idx %d to %d\n", idx, c->readMap[i]);
			controlDev.readPorts[idx] = gpio_OPEN(c->readMap[i]);
			controlDev.numReadPorts++;
			fprintf(stderr, " * fd = %d\n", controlDev.readPorts[idx]);
			idx++;
		}
	}

	// set writing ports
	for (i = 0, idx = 0; i < c->numWritePorts; i++) {
		// verify legal pin
		for (j = 0; j < NUM_RASPI_GPIO; j++)
			if (c->writeMap[i] == raspiPinMap[j])
				break;
		
		if (j == NUM_RASPI_GPIO) {
			fprintf(stderr, "*** ERROR: invalid raspi gpio pin! ***\n");
			break;
		}
		else {
			fprintf(stderr, " - setting to WRITE device idx %d to %d\n", idx, c->writeMap[i]);
			controlDev.writePorts[idx] = gpio_OPEN(c->writeMap[i]);
			controlDev.numWritePorts++;
			fprintf(stderr, " * fd = %d\n", controlDev.writePorts[idx]);
			idx++;
		}
	}

	// set function pointers
	controlDev.digitalRead = &readRaspiGPIO;

	return 1;
}

/****************************************************************
 * closeRaspiGPIO() 											*
 ****************************************************************/

int closeRaspiGPIO() {
	fprintf(stderr, "*** CLOSING RASPI GPIO - NOT IMPLEMENTED! ***");
	return 1;
}



