# dabpi
Raspberry Si4688 FM / DAB+ Receiver

Original Project from Bjoern Biesenbach, see https://github.com/elmo2k3/dabpi_ctl

## News

10.12.2016 - initial github project, only DAB implemented

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

say <M> for "Support for DABPi featuring a Si4688 FM/FMHD/DAB receiver"

compile, install the kernel and modules, add following lines to /boot/config.txt

```bash
dtparam=i2s=on
dtparam=spi=on

dtoverlay=rpi-dabpi
```
then reboot

Step 2: compile Si4688 controller software

```bash
git clone https://github.com/teknoid/dabpi
cd dabpi
make clean && make
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
Set Frequency Region (0 - 15)

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
Starting service RAS Swiss Pop+   42f1 b
DAB_START_DIGITAL_SERVICE:
  0000  00 81 80 00 c0                                   .....
```

Enjoy ;-)
 
 
