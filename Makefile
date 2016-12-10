LDFLAGS=-lwiringPi -lpthread -lm
#CFLAGS=-Wall -Werror -Wextra
dabpi_ctl: dabpi_ctl.o si46xx_dab.o si46xx_fm.o si46xx_core.o utils.o

.PHONY: clean

clean:
	rm -f dabpi_ctl *.o
