#include <linux/spi/spidev.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

unsigned char spi_bpw = 8;
int spi_fd = 0;

int transfer_spi(int spi_fd, unsigned char *tbuf, unsigned char *rbuf, int len) {
	struct spi_ioc_transfer spi_msg;
	memset(&spi_msg, 0, sizeof(spi_msg));

	spi_msg.tx_buf = (unsigned long) tbuf;
	spi_msg.rx_buf = (unsigned long) rbuf;
	spi_msg.len = len;

	return ioctl(spi_fd, SPI_IOC_MESSAGE(1), &spi_msg);
}

void setup_spi(int spi_fd, int speed, int mode) {
	unsigned char tbuf[2]; // [0]addr; [1]data;
	int i;

	if (ioctl(spi_fd, SPI_IOC_WR_MODE, &mode) < 0) {
		printf("setup_spi: MODE setup error\n");
		exit(1);
	}
	if (ioctl(spi_fd, SPI_IOC_WR_BITS_PER_WORD, &spi_bpw) < 0) {
		printf("setup_spi: BPW setup error\n");
		exit(1);
	}
	if (ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) < 0) {
		printf("setup_spi: SPEED setup error\n");
		exit(1);
	}

	for (i = 0; i < 8; i++) {
		tbuf[0] = 0x1 + i;
		tbuf[1] = 0;
		if (transfer_spi(spi_fd, tbuf, NULL, 2) < 0) {
			printf("setup_spi: initialize digit 0 error\n");
			exit(1);
		} // digit i := 0 0 0 0 0 0 0 0
	}

	tbuf[0] = 0xC;
	tbuf[1] = 0x1;
	if (transfer_spi(spi_fd, tbuf, NULL, 2) < 0) {
		printf("setup_spi: set Shutdown as Normal Operation error\n");
		exit(1);
	} // Shutdown := Normal Operation

	tbuf[0] = 0x9;
	tbuf[1] = 0x0;
	if (transfer_spi(spi_fd, tbuf, NULL, 2) < 0) {
		printf("setup_spi: set DecodeMode as None error\n");
		exit(1);
	} // Decode-Mode := No decode for digits 7-0

	tbuf[0] = 0xA;
	tbuf[1] = 0x1;
	if (transfer_spi(spi_fd, tbuf, NULL, 2) < 0) {
		printf("setup_spi: set Intensity 3/32 error\n");
		exit(1);
	} // Intensity := 3/32 (minimum)

	tbuf[0] = 0xB;
	tbuf[1] = 0x7;
	if (transfer_spi(spi_fd, tbuf, NULL, 2) < 0) {
		printf("setup_spi: set ScanLimit ALL error\n");
		exit(1);
	} // Scan-Limit := Display digits 0 1 2 3 4 5 6 7

	tbuf[0] = 0xF;
	tbuf[1] = 0x0;
	if (transfer_spi(spi_fd, tbuf, NULL, 2) < 0) {
		printf("setup_spi: set DisplayTest as Normal Operation error\n");
		exit(1);
	} // DisplayTest := Normal Operation
}

int is_leap(int year) {
	if (year % 400 == 0)
		return 1;
	if (year % 100 == 0)
		return 0;
	if (year % 4 == 0)
		return 1;
	return 0;
}

int get_weekday(struct tm *time) {
	int i, result = 0;
	int year = time->tm_year + 1900;
	int month = time->tm_mon + 1;

	int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	if (is_leap(year))
		days[1]++;

	for (i = 1900; i < year; i++) {
		if (is_leap(i))
			result += 366;
		else
			result += 365;
	}
	for (i = 1; i < month; i++) {
		result += days[i-1];
	}
	result += time->tm_mday;

	return result % 7; // [SUN, MON, TUE, WED, THU, FRI, SAT]
}

void sigalrm_handler(int sig) {
	unsigned char tbuf[2]; // [0]addr; [1]data;
	time_t now;
	struct tm *t;
	int i, year, month, weekday;
	
	time(&now);
	t = (struct tm*) localtime(&now);
	weekday = get_weekday(t);
//	printf("%04d/%02d/%02d/%d %02d:%02d:%02d\n",
//		t->tm_year+1900, t->tm_mon+1, t->tm_mday, weekday, t->tm_hour, t->tm_min, t->tm_sec);

	year = t->tm_year+1900;
	month = t->tm_mon+1;

	tbuf[0] = 8;
	tbuf[1] = ((year / 1000) << 4) | (t->tm_hour / 10);
	if (transfer_spi(spi_fd, tbuf, NULL, 2) < 0) {
		printf("setup_spi: time setting error\n");
		exit(1);
	} // Year[3] Hour[1]

	tbuf[0] = 7;
	tbuf[1] = (((year / 100) % 10) << 4) | (t->tm_hour % 10);
	if (transfer_spi(spi_fd, tbuf, NULL, 2) < 0) {
		printf("setup_spi: time setting error\n");
		exit(1);
	} // year[2] hour[0]

	tbuf[0] = 6;
	tbuf[1] = (((year / 10) % 10) << 4) | (t->tm_min / 10);
	if (transfer_spi(spi_fd, tbuf, NULL, 2) < 0) {
		printf("setup_spi: time setting error\n");
		exit(1);
	} // year[1] min[1]

	tbuf[0] = 5;
	tbuf[1] = ((year % 10) << 4) | (t->tm_min % 10);
	if (transfer_spi(spi_fd, tbuf, NULL, 2) < 0) {
		printf("setup_spi: time setting error\n");
		exit(1);
	} // year[0] min[0]

	tbuf[0] = 4;
	tbuf[1] = ((month / 10) << 4) | (t->tm_sec / 10);
	if (transfer_spi(spi_fd, tbuf, NULL, 2) < 0) {
		printf("setup_spi: time setting error\n");
		exit(1);
	} // mon[1] sec[1]

	tbuf[0] = 3;
	tbuf[1] = ((month % 10) << 4) | (t->tm_sec % 10);
	if (transfer_spi(spi_fd, tbuf, NULL, 2) < 0) {
		printf("setup_spi: time setting error\n");
		exit(1);
	} // mon[0] sec[0]

	tbuf[0] = 2;
	tbuf[1] = ((t->tm_mday / 10) << 4);
	if (transfer_spi(spi_fd, tbuf, NULL, 2) < 0) {
		printf("setup_spi: time setting error\n");
		exit(1);
	} // day[1] None

	tbuf[0] = 1;
	tbuf[1] = ((t->tm_mday % 10) << 4) | weekday;
	if (transfer_spi(spi_fd, tbuf, NULL, 2) < 0) {
		printf("setup_spi: time setting error\n");
		exit(1);
	} // day[0] weekday
}

void sigint_handler(int sig) {
	unsigned char tbuf[2]; // [0]addr; [1]data;

	tbuf[0] = 0xC;
	tbuf[1] = 0x0;
	if (transfer_spi(spi_fd, tbuf, NULL, 2) < 0) {
		printf("setup_spi: set Shutdown as Normal Operation error\n");
		exit(1);
	} // Shutdown := Shutdown mode

	exit(0);
}

int main(int argc, char *argv[]) {
	struct sigaction sigalrm;
	struct sigaction sigint;
	struct itimerval itimer;

	if ((spi_fd = open("/dev/spidev0.0", O_RDWR)) < 0) {
		printf("open /dev/spidev error\n");
		exit(1);
	}

	setup_spi(spi_fd, 1000000, 0);

	sigalrm.sa_handler = sigalrm_handler;
	sigalrm.sa_flags = SA_NOMASK;
	sigaction(SIGALRM, &sigalrm, NULL);

	sigint.sa_handler = sigint_handler;
	sigint.sa_flags = SA_NOMASK;
	sigaction(SIGINT, &sigint, NULL);

	itimer.it_value.tv_sec = 1;
	itimer.it_value.tv_usec = 0;
	itimer.it_interval.tv_sec = 1;
	itimer.it_interval.tv_usec = 0;
	setitimer(ITIMER_REAL, &itimer, NULL);

	while(1);
	
	return 0;
}
