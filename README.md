# dabpi
Raspberry Si4688 FM / DAB+ Receiver

Original Project from Bjoern Biesenbach, see https://github.com/elmo2k3/dabpi_ctl

## News

13.12.2016 - added FM functionality

10.12.2016 - initial github project, only DAB implemented, contributors welcome

### Installation Instructions

Step 1: patch kernel to get I2S Audio Record Device for Si468x

```bash
patch -p 1 < 0001-Added-support-for-Si468x-Digital-Radio-Receiver-DABP.patch
```
Follow kernel build instructions at https://www.raspberrypi.org/documentation/linux/kernel/building.md

To enable the Si4688 driver execute (for cross-compiling)

```bash
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- bcm2709_defconfig
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- menuconfig
```
go to "Device Drivers" --> "Sound Card Support" --> "Advanced Linux Sound Architecture" --> "Alsa for SoC Audio Support"

say "module" for "Support for DABPi featuring a Si4688 FM/FMHD/DAB receiver"

Compile and install the kernel + modules, add following lines to /boot/config.txt

```bash
dtparam=i2s=on
dtparam=spi=on

dtoverlay=rpi-dabpi
```
Then reboot. On success you should see correct I2S initialization and a new alsa recording source:

```bash
root@pidev:~# dmesg |grep i2s
[    6.411170] snd-rpi-dabpi soc:sound: si468x-hifi <-> 3f203000.i2s mapping ok

root@pidev:~# arecord -l
**** List of CAPTURE Hardware Devices ****
card 0: sndrpirpidabpi [snd_rpi_rpi_dabpi], device 0: DABPi Hifi si468x-hifi-0 []
  Subdevices: 0/1
  Subdevice #0: subdevice #0
```

Step 2: compile Si4688 controller software

```bash
git clone https://github.com/teknoid/dabpi
cd dabpi
make clean && make
```
Step 3: place firmware in ../si46xx_firmware/

```bash
root@pidev:/anus/si46xx_firmware# ls -la
total 2040
drwxr-xr-x 2 hje  hje     4096 Dec  9  2016 .
drwxrwxr-x 5 root users   4096 Dec 10  2016 ..
-rw-r--r-- 1 hje  hje   517708 Aug 15  2014 dab_radio_3_2_7.bif
-rw-r--r-- 1 hje  hje   521448 Dec  9  2015 dab_radio_5_0_5.bin
-rw-r--r-- 1 hje  hje   493128 Aug 15  2014 fmhd_radio_3_0_19.bif
-rw-r--r-- 1 hje  hje   530180 Dec  2  2015 fmhd_radio_5_0_4.bin
-rw-r--r-- 1 hje  hje     5796 Nov  9  2012 rom00_patch.016.bin
```

### Usage

Enter DAB+ mode

```bash
root@pidev:/anus/dabpi# ./dabpi_ctl -a
dabpi_ctl version v0.01-43-g369d8de
POWER_UP:
  0000  00 80 00 00 80                                   .....
LOAD_INIT:
  0000  00 80 00 00 80                                   .....
HOST_LOAD:
  0000  00 80 00 00 80                                   .....
  
...

HOST_LOAD:
  0000  00 80 00 00 80                                   .....
HOST_LOAD:
  0000  00 80 00 00 80                                   .....
BOOT:
  0000  00 80 80 00 c0                                   .....
GET_SYS_STATE:
  0000  00 80 80 00 c0 02 ff                             .......
Current mode: 
DAB is active
GET_PART_INFO:
  0000  00 80 80 00 c0 00 00 00 00 50 12 00 00 00 00 50  .........P.....P
  0010  12 00 00 00 00 01 00                             .......
SET_PROPERTY:
  0000  00 80 80 00 c0                                   .....
SET_PROPERTY:
  0000  00 80 80 00 c0                                   .....
SET_PROPERTY:
  0000  00 80 80 00 c0                                   .....
SET_PROPERTY:
  0000  00 80 80 00 c0                                   .....

```
Set Frequency depending on your Region (0 - 16) - example for South Tyrol

```bash
root@pidev:/anus/dabpi# ./dabpi_ctl -j 15
dabpi_ctl version v0.01-43-g369d8de
DAB_SET_FREQ_LIST:
  0000  00 80 80 00 c0                                   .....

```
Tune to DAB channel

```bash
root@pidev:/anus/dabpi# ./dabpi_ctl -i 0
dabpi_ctl version v0.01-43-g369d8de
DAB_TUNE_FREQ:
  0000  00 81 80 00 c0                                   .....

```
Get ensemble information

```bash
root@pidev:/anus/dabpi# ./dabpi_ctl -g
dabpi_ctl version v0.01-43-g369d8de
DAB_GET_DIGITAL_SERVICE_LIST:
  0000  00 81 80 00 c0 56 01                             .....V.
List size:     342
List version:  36
Services:      12

 Nr | Service ID | Service Name     | Component IDs
--------------------------------------------------
 00 |       42f1 | RAS Swiss Pop+   | 11 
 01 |       42f2 | RAS SwissClassic | 10 
 02 |       43e2 | RAS RSI Rete 2+  | 12 
 03 |       5203 | Rai Radio3+      | 3 
 04 |       5301 | Rai Radio1+ TAA  | 1 
 05 |       5302 | Rai Radio2+ TAA  | 2 
 06 |       5304 | RAI SUEDTIROL+   | 4 
 07 |       d220 | RAS DKULTUR+     | 9 
 08 |       d313 | RAS BAYERN 3     | 5 
 09 |       d314 | RAS BR-KLASSIK   | 6 
 10 |       d315 | RAS B5 aktuell   | 7 
 11 |       df95 | RAS KIRAKA+      | 8 
 ```
 Start one of the services in ensemble 
 
 ```bash
 root@pidev:/anus/dabpi# ./dabpi_ctl -f 0

...
Starting service RAS Swiss Pop+   42f1 b
DAB_START_DIGITAL_SERVICE:
  0000  00 81 80 00 c0                                   .....
```

Start alsa playback

```bash
ssh hje@pidev 'arecord -D hw:0,0 -f dat -' | aplay -f dat -
```

Enjoy the music ;-)
 
