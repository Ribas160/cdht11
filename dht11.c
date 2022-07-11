#include <stdio.h>
#include <bcm2835.h>
#include <unistd.h>
#include <sys/time.h>

#define DHT11 RPI_GPIO_P1_11

void startSignal();
int responseSignal();
long getMicrotime();
void readData(int data[5]);

int main() {

	if (!bcm2835_init()) {
		return 1;
	}

	while (1) {
		int data[5] = {0};

		bcm2835_gpio_fsel(DHT11, BCM2835_GPIO_FSEL_OUTP);
	
		startSignal();

		bcm2835_gpio_fsel(DHT11, BCM2835_GPIO_FSEL_INPT);

		if (!responseSignal()) {
			puts("ERROR: Response singnal has not been received");
			goto next;
		}
	
		readData(data);

		if (data[4] != (data[0] + data[1] + data[2] + data[3])) {
			puts("Some data has been losted");
			goto next;
		}

		printf("Temperature: %d.%d *C, Humidity: %d.%d %\n", data[2], data[3], data[0], data[1]);

		next:
		sleep(3);
	}

	return 0;
}


long getMicrotime(){
	struct timeval currentTime;
	gettimeofday(&currentTime, NULL);
	return currentTime.tv_sec * (int)1e6 + currentTime.tv_usec;
}


void startSignal() {
	bcm2835_gpio_write(DHT11, HIGH);

	usleep(500000); // 500 ms

	bcm2835_gpio_write(DHT11, LOW);

	usleep(18000); // 18 ms	

	bcm2835_gpio_write(DHT11, HIGH);
}


int responseSignal() {
	long startTime, time[2];
	int laststate = LOW;

	while (bcm2835_gpio_lev(DHT11) == HIGH) {
	}

	for (int i = 0; i < 2; i++) {
		startTime = getMicrotime();

		while (bcm2835_gpio_lev(DHT11) == laststate) {
		}
		
		time[i] = getMicrotime() - startTime;
		
		laststate = bcm2835_gpio_lev(DHT11);
	}

	if (70 < time[1] && time[1] < 90) {
		return 1;
	}

	return 0;
}


void readData(int data[5]) {
	long startTime;
	
	for (int i = 0; i < 40; i++) {
		
		while (bcm2835_gpio_lev(DHT11) == LOW) {
		}
		
		startTime = getMicrotime();

		while (bcm2835_gpio_lev(DHT11) == HIGH) {
		}

		if (i != 0 && i % 8 == 0) {
			*data++;
		}

		*data <<= 1;

		if ((getMicrotime() - startTime) > 50) {
			*data |= 1;

		}

	}

}
