/*
 * dabpi_ctl - raspberry pi fm/fmhd/dab receiver board control interface
 * Copyright (C) 2014  Bjoern Biesenbach <bjoern@bjoern-b.de>
 * Copyright (C) 2016  Heiko Jehmlich <hje@jecons.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <fcntl.h>				// Needed for SPI port
#include <sys/ioctl.h>			// Needed for SPI port
#include <linux/spi/spidev.h>	// Needed for SPI port
#include <wiringPi.h>			// Needed for GPIO reset

#include "si46xx.h"

#define SI46XX_RESET 	4

#define SPI_DEVICE		"/dev/spidev0.0"
#define SPI_SPEED		10000000
#define SPI_BITS		8
#define SPI_DELAY		0

int spi_fd;

void spi_init() {
	uint8_t mode = SPI_MODE_0;
	uint8_t bits = SPI_BITS;
	uint32_t speed = SPI_SPEED;

	spi_fd = open(SPI_DEVICE, O_RDWR);
	if (spi_fd < 0) {
		printf("error open: %s\n", strerror(errno));
	}

	if (ioctl(spi_fd, SPI_IOC_RD_MODE, &mode) < 0) {
		printf("SPI_IOC_RD_MODE Change failure: %s\n", strerror(errno));
	}
	if (ioctl(spi_fd, SPI_IOC_WR_MODE, &mode) < 0) {
		printf("SPI_IOC_WR_MODE Change failure: %s\n", strerror(errno));
	}

	if (ioctl(spi_fd, SPI_IOC_RD_BITS_PER_WORD, &bits) < 0) {
		printf("SPI_IOC_RD_BITS_PER_WORD Change failure: %s\n", strerror(errno));
	}
	if (ioctl(spi_fd, SPI_IOC_WR_BITS_PER_WORD, &bits) < 0) {
		printf("SPI_IOC_WR_BITS_PER_WORD Change failure: %s\n", strerror(errno));
	}

	if (ioctl(spi_fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed) < 0) {
		printf("SPI_IOC_RD_MAX_SPEED_HZ Change failure: %s\n", strerror(errno));
	}
	if (ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) < 0) {
		printf("SPI_IOC_WR_MAX_SPEED_HZ Change failure: %s\n", strerror(errno));
	}
}

int spi(uint8_t *data, int len) {
	struct spi_ioc_transfer spi;
	memset(&spi, 0, sizeof(spi));
	spi.tx_buf = (unsigned long) data;
	spi.rx_buf = (unsigned long) data;
	spi.len = len;
	spi.speed_hz = SPI_SPEED;
	spi.bits_per_word = SPI_BITS;
	spi.delay_usecs = SPI_DELAY;
	int ret = ioctl(spi_fd, SPI_IOC_MESSAGE(1), &spi);
	if (ret < 0) {
		printf("SPI_IOC_MESSAGE ioctl error: %s\n", strerror(errno));
	}
	msleep(1);
	return ret;
}

int si46xx_reply(const char *log) {
	uint8_t data[5];
	uint8_t timeout = 10;

	while (timeout--) {
		data[0] = 0;
		spi(data, 5);
		if (data[1] & 0x80) { // CTS ?
			hexDump(log, data, 5);
			return 1;
		}
		msleep(10);
	}
	printf("timeout on %s\r\n", log);
	return 0;
}

void si46xx_boot() {
	uint8_t data[2];
	data[0] = SI46XX_BOOT;
	data[1] = 0;
	spi(data, 2);
	msleep(300); // 63ms at analog fm, 198ms at DAB
	si46xx_reply("BOOT");
}

void si46xx_hostload(const char *filename) {
	FILE *fp;
	size_t result;

	uint8_t init[2];
	init[0] = SI46XX_LOAD_INIT;
	init[1] = 0;
	spi(init, 2);
	si46xx_reply("LOAD_INIT");

	fp = fopen(filename, "rb");
	if (fp == NULL) {
		printf("file error %s\r\n", filename);
		return;
	}

	int loop = 0;
	uint8_t data[4092 + 4];
	while (1) {
		data[0] = SI46XX_HOST_LOAD;
		data[1] = 0;
		data[2] = 0;
		data[3] = 0;
		result = fread(data + 4, 1, 4092, fp);
		if (result == 0) {
			break;
		}
		spi(data, result + 4);
		msleep(5);
		// printf("loop %d, count %d\r\n", loop++, result);
		si46xx_reply("HOST_LOAD");
	}
	fclose(fp);
}

void si46xx_powerup() {
	uint8_t data[16];

	data[0] = SI46XX_POWER_UP; // COMMAND
	data[1] = 0x00; // ARG1 disable CTSIEN
	data[2] = (1 << 4) | (7 << 0); // ARG2 CLK_MODE=0x1 TR_SIZE=0x7
	data[3] = 0x48; // ARG3 IBIAS=0x48
	data[4] = 0x00; // ARG4 XTAL
	data[5] = 0xF9; // ARG5 XTAL // F8
	data[6] = 0x24; // ARG6 XTAL
	data[7] = 0x01; // ARG7 XTAL 19.2MHz
	data[8] = 0x1F; // ARG8 CTUN
	data[9] = 0x00 | (1 << 4); // ARG9
	data[10] = 0x00; // ARG10
	data[11] = 0x00; // ARG11
	data[12] = 0x00; // ARG12
	data[13] = 0x00; // ARG13 IBIAS_RUN
	data[14] = 0x00; // ARG14
	data[15] = 0x00; // ARG15

	spi(data, 16);
	si46xx_reply("POWER_UP");
}

void si46xx_get_partinfo() {
	uint8_t data[23];
	data[0] = SI46XX_GET_PART_INFO;
	data[1] = 0;
	spi(data, 2);
	msleep(5);

	data[0] = 0;
	spi(data, 23);
	hexDump("GET_PART_INFO", data, 23);
}

void si46xx_get_sys_state() {
	uint8_t data[7];
	data[0] = SI46XX_GET_SYS_STATE;
	data[1] = 0;
	spi(data, 2);
	msleep(5);

	data[0] = 0;
	spi(data, 7);
	hexDump("GET_SYS_STATE", data, 7);

	printf("Current mode: \n");
	switch (data[5]) {
	case 0:
		printf("Bootloader is active\n");
		break;
	case 1:
		printf("FMHD is active\n");
		break;
	case 2:
		printf("DAB is active\n");
		break;
	case 3:
		printf("TDMB or data only DAB image is active\n");
		break;
	case 4:
		printf("FMHD is active\n");
		break;
	case 5:
		printf("AMHD is active\n");
		break;
	case 6:
		printf("AMHD Demod is active\n");
		break;
	default:
		break;
	}
}

void si46xx_set_property(uint16_t name, uint16_t value) {
	uint8_t data[6];
	data[0] = SI46XX_SET_PROPERTY;
	data[1] = 0;
	data[2] = name & 0xFF;
	data[3] = (name >> 8) & 0xFF;
	data[4] = value & 0xFF;
	data[5] = (value >> 8) & 0xFF;
	spi(data, 6);
	si46xx_reply("SET_PROPERTY");
}

void si46xx_reset() {
	wiringPiSetup();
	pinMode(SI46XX_RESET, OUTPUT);

	/* reset si46xx  */
	digitalWrite(SI46XX_RESET, 0);
	msleep(10);
	digitalWrite(SI46XX_RESET, 1);
	msleep(10);
}
