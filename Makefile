# Cross toolchain variables
# If these are not in your path, you can make them absolute.
XT_PRG_PREFIX = mipsel-linux-gnu-
CC = $(XT_PRG_PREFIX)gcc
LD = $(XT_PRG_PREFIX)ld

# uMPS3-related paths

# Simplistic search for the umps3 installation prefix.
# If you have umps3 installed on some weird location, set UMPS3_DIR_PREFIX by hand.
ifneq ($(wildcard /usr/bin/umps3),)
	UMPS3_DIR_PREFIX = /usr
else
	UMPS3_DIR_PREFIX = /usr/local
endif

UMPS3_DATA_DIR = $(UMPS3_DIR_PREFIX)/share/umps3
UMPS3_INCLUDE_DIR = $(UMPS3_DIR_PREFIX)/include/umps3

# Compiler options
CFLAGS_LANG = -ffreestanding # -ansi
CFLAGS_MIPS = -mips1 -mabi=32 -mno-gpopt -G 0 -mno-abicalls -fno-pic -mfp32
CFLAGS = $(CFLAGS_LANG) $(CFLAGS_MIPS) -I$(UMPS3_INCLUDE_DIR) -Wall -O0

# Linker options
LDFLAGS = -G 0 -nostdlib -T $(UMPS3_DATA_DIR)/umpscore.ldscript

# Add the location of crt*.S to the search path
VPATH = $(UMPS3_DATA_DIR)

.PHONY : all clean

all : kernel.core.umps

kernel.core.umps : kernel
	umps3-elf2umps -k $<

kernel : ./phase1/p1test.o ./phase1/msg.o ./phase1/pcb.o crtso.o libumps.o
	$(LD) -o $@ $^ $(LDFLAGS)

clean :
	-rm -f *.o ./phase1/*.o ./phase2/*.o kernel kernel.*.umps

# Pattern rule for assembly modules
%.o : %.S
	$(CC) $(CFLAGS) -c -o $@ $<
