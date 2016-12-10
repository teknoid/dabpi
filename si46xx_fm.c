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

void si46xx_fm_tune_freq(uint32_t khz, uint16_t antcap) {
}

void si46xx_fm_rsq_status(void) {
}

void si46xx_fm_rds_status(void) {
}

void si46xx_fm_rds_blockcount(void) {
}

void si46xx_fm_seek_start(uint8_t up, uint8_t wrap) {
}

void si46xx_init_fm(void) {
	si46xx_reset();
	si46xx_powerup();
	si46xx_hostload("firmware/rom00_patch.016.bin");
	si46xx_hostload("firmware/fmhd_radio_3_0_19.bif");
	// si46xx_hostload("firmware/fmhd_radio_5_0_4.bin");
	si46xx_boot();

	si46xx_get_sys_state();
	si46xx_get_partinfo();
}
