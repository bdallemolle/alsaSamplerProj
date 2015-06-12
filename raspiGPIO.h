#define SYSFS_GPIO_DIR 		"/sys/class/gpio"		// directory of gpios	
#define GPIO_TIMEOUT 		(1 * 1000) 				// gpio listening timeout
#define NUM_RASPI_GPIO		17						// number of raspi GPIO pins
#define MAX_BUF 			64						// some random buffer
#define GPIO_DEMO1			17						// gpio (1) pin id for demo
#define GPIO_DEMO2			4						// gpio (2) pin id for demo
#define GPIO_DEBUG			1 						// gpio debugging flag

int initRaspiGPIO(CONFIG* c);
int closeRaspiGPIO();