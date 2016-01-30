src = $(wildcard src/*.c)
imgfiles = $(wildcard *.png)
obj = $(src:.c=.o) $(imgfiles:.png=.img.o)
dep = $(obj:.o=.d)
name = gbamtg
elf = $(name).elf
bin = $(name).gba
elf_mb = $(name)_mb.elf
bin_mb = $(name)_mb.gba

ARCH = arm-none-eabi

CPP = $(ARCH)-cpp
CC = $(ARCH)-gcc
AS = $(ARCH)-as
OBJCOPY = $(ARCH)-objcopy
EMU = vbam

opt = -O3 -fomit-frame-pointer -mcpu=arm7tdmi -mtune=arm7tdmi
#dbg = -g

CFLAGS = $(opt) $(dbg) -pedantic -Wall -I. -I../gbasys/src
LDFLAGS = ../gbasys/libgbasys.a -lm
EMUFLAGS = -T 100 -f 1 --agb-print

.PHONY: all
all: $(bin) $(bin_mb)

$(bin): $(elf)
	$(OBJCOPY) -O binary $(elf) $(bin)
	gbafix $(bin)

$(bin_mb): $(elf_mb)
	$(OBJCOPY) -O binary $(elf_mb) $(bin_mb)
	gbafix $(bin_mb)

$(elf): $(obj) ../gbasys/libgbasys.a
	$(CC) -o $(elf) $(obj) -specs=gba.specs $(LDFLAGS)

$(elf_mb): $(obj) ../gbasys/libgbasys.a
	$(CC) -o $(elf_mb) $(obj) -specs=gba_mb.specs $(LDFLAGS)

-include $(dep)

%.d: %.c data.h
	$(CPP) $(CFLAGS) $< -MM -MT $(@:.d=.o) >$@

%.img.c: %.png
	img2gba $< -h /dev/null

data.h: $(imgfiles)
	img2gba $(imgfiles) -o /dev/null

.PHONY: clean
clean:
	rm -f $(obj) $(dep) $(bin) $(bin_mb) $(elf) $(elf_mb) data.h

.PHONY: install
install: $(bin)
	if2a -n -f -W $<

.PHONY: run
run: $(bin_mb)
	if2a -m $<

.PHONY: simrun
simrun: $(bin)
	$(EMU) $(EMUFLAGS) $(bin)
