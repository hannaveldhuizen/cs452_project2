obj-m += card_driver.o

KDIR := /lib/modules/$(shell uname -r)/build

PWD := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

blackjack:
	gcc -o blackjack blackjack.c
