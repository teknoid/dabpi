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

#include "si46xx.h"

void si46xx_init_fm(void) {
	si46xx_reset();
	si46xx_powerup();
	si46xx_hostload("firmware/rom00_patch.016.bin");
	// si46xx_hostload("firmware/fmhd_radio_3_0_19.bif");
	si46xx_hostload("firmware/fmhd_radio_5_0_4.bin");
	si46xx_boot();

	si46xx_get_sys_state();
	si46xx_get_partinfo();

	si46xx_set_property(SI46XX_PIN_CONFIG_ENABLE, 0x0002); // enable I2S output (BUG!!! either DAC or I2S seems to work)
	// si46xx_set_property(SI46XX_FM_VALID_RSSI_THRESHOLD,0x0000);
	// si46xx_set_property(SI46XX_FM_VALID_SNR_THRESHOLD,0x0000);
	// si46xx_set_property(SI46XX_FM_SOFTMUTE_SNR_LIMITS, 0x0000); // set the SNR limits for soft mute attenuation
	// si46xx_set_property(SI46XX_FM_TUNE_FE_CFG, 0x0000); // front end switch open
	// si46xx_set_property(SI46XX_DIGITAL_IO_OUTPUT_FORMAT, 0x1000); // SAMPL_SIZE = 16, I2S mode
	si46xx_set_property(SI46XX_FM_RDS_CONFIG, 0x0001); // enable RDS
	si46xx_set_property(SI46XX_FM_AUDIO_DE_EMPHASIS, SI46XX_AUDIO_DE_EMPHASIS_EU); // set de-emphasis for Europe
	// si46xx_fm_tune_freq(105500,0);
}

uint8_t si46xx_rds_parse(uint16_t *block) {
	uint8_t addr;
	fm_rds_data.pi = block[0];
	if ((block[1] & 0xF800) == 0x00) { // group 0A
		addr = block[1] & 0x03;
		fm_rds_data.ps_name[addr * 2] = (block[3] & 0xFF00) >> 8;
		fm_rds_data.ps_name[addr * 2 + 1] = block[3] & 0xFF;
		fm_rds_data.group_0a_flags |= (1 << addr);
	} else if ((block[1] & 0xF800) >> 11 == 0x04) { // group 2A
		addr = block[1] & 0x0F;
		if ((block[1] & 0x10) == 0x00) { // parse only string A
			fm_rds_data.radiotext[addr * 4] = (block[2] & 0xFF00) >> 8;
			fm_rds_data.radiotext[addr * 4 + 1] = (block[2] & 0xFF);
			fm_rds_data.radiotext[addr * 4 + 2] = (block[3] & 0xFF00) >> 8;
			fm_rds_data.radiotext[addr * 4 + 3] = (block[3] & 0xFF);

			if (fm_rds_data.radiotext[addr * 4] == '\r') {
				fm_rds_data.radiotext[addr * 4] = 0;
				fm_rds_data.group_2a_flags = 0xFFFF;
			}
			if (fm_rds_data.radiotext[addr * 4 + 1] == '\r') {
				fm_rds_data.radiotext[addr * 4 + 1] = 0;
				fm_rds_data.group_2a_flags = 0xFFFF;
			}
			if (fm_rds_data.radiotext[addr * 4 + 2] == '\r') {
				fm_rds_data.radiotext[addr * 4 + 2] = 0;
				fm_rds_data.group_2a_flags = 0xFFFF;
			}
			if (fm_rds_data.radiotext[addr * 4 + 3] == '\r') {
				fm_rds_data.radiotext[addr * 4 + 3] = 0;
				fm_rds_data.group_2a_flags = 0xFFFF;
			}
			fm_rds_data.group_2a_flags |= (1 << addr);
		}
	}
	if (fm_rds_data.group_0a_flags == 0x0F && fm_rds_data.group_2a_flags == 0xFFFF) {
		fm_rds_data.ps_name[8] = 0;
		fm_rds_data.radiotext[128] = 0;
		return 1;
	}
	return 0;
}

void si46xx_fm_tune_freq(uint32_t khz, uint16_t antcap) {
	uint8_t data[7];
	uint8_t timeout = 255;

	data[0] = SI46XX_FM_TUNE_FREQ;
	data[1] = 0;
	data[2] = khz & 0xFF;
	data[3] = (khz >> 8) & 0xFF;
	data[4] = antcap & 0xFF;
	data[5] = (antcap >> 8) & 0xFF;
	data[6] = 0;
	spi(data, 7);

	while (timeout--) {
		data[0] = 0;
		spi(data, 5);
		if ((data[1] & 0x80) && (data[1] & 0x01)) { // CTS + tuned ?
			hexDump("FM_TUNE_FREQ", data, 5);
			return;
		}
		msleep(10);
	}
	printf("timeout on FM_TUNE_FREQ\r\n");
}

void si46xx_fm_rsq_status(void) {
	uint8_t data[23];

	data[0] = SI46XX_FM_RSQ_STATUS;
	data[1] = 0;
	spi(data, 2);

	data[0] = 0;
	spi(data, 23);
	hexDump("FM_RSQ_STATUS", data, 23);

	printf("SNR        : %d dB\r\n", (int8_t) data[11]);
	printf("RSSI       : %d dBuV\r\n", (int8_t) data[10]);
	printf("Frequency  : %d kHz\r\n", (data[8] << 8 | data[7]) * 10);
	printf("FREQOFF    : %d\r\n", (int8_t) data[9] * 2);
	printf("READANTCAP : %d\r\n", (int8_t) (data[13] + (data[14] << 8)));
}

void si46xx_fm_rds_status(void) {
	uint8_t data[21];
	uint16_t blocks[4];
	uint16_t timeout = 5000; // work on 1000 rds blocks max

	while (timeout--) {
		data[0] = SI46XX_FM_RDS_STATUS;
		data[1] = 1;
		spi(data, 2);

		data[0] = 0;
		spi(data, 21);
		// hexDump("FM_RDS_STATUS", data, 21);

		blocks[0] = data[13] + (data[14] << 8);
		blocks[1] = data[15] + (data[16] << 8);
		blocks[2] = data[17] + (data[18] << 8);
		blocks[3] = data[19] + (data[20] << 8);

		fm_rds_data.sync = (data[6] & 0x02) ? 1 : 0;
		if (!fm_rds_data.sync)
			break;
		if (si46xx_rds_parse(blocks))
			break;
		if (fm_rds_data.group_0a_flags == 0x0F) // stop at ps_name complete
			break;
	}
	if (!timeout)
		printf("Timeout\r\n");

	printf("RDSSYNC       : %u\r\n", (data[6] & 0x02) ? 1 : 0);
	printf("PI            : %d\r\n", fm_rds_data.pi);
	printf("Name          : %s\r\n", fm_rds_data.ps_name);
	printf("Radiotext     : %s\r\n", fm_rds_data.radiotext);
}

void si46xx_fm_rds_blockcount(void) {
	uint8_t data[11];

	data[0] = SI46XX_FM_RDS_BLOCKCOUNT;
	data[1] = 0;
	// data[1] = 1; // clears block counts if set
	spi(data, 2);

	data[0] = 0;
	spi(data, 11);
	hexDump("FM_RDS_BLOCKCOUNT", data, 11);

	printf("Expected      : %d\r\n", data[5] | (data[6] << 8));
	printf("Received      : %d\r\n", data[7] | (data[8] << 8));
	printf("Uncorrectable : %d\r\n", data[9] | (data[10] << 8));
}

void si46xx_fm_seek_start(uint8_t up, uint8_t wrap) {
	uint8_t data[6];

	data[0] = SI46XX_FM_SEEK_START;
	data[1] = 0;
	data[2] = (up & 0x01) << 1 | (wrap & 0x01);
	data[3] = 0;
	data[4] = 0;
	data[5] = 0;
	spi(data, 6);

	si46xx_reply("FM_SEEK_START");
}

