DLLNAME := xinput9_1_0.dll

SRCFILES := \
	main.c \
	util.c \
	tas.c

CC := x86_64-w64-mingw32-gcc
CFLAGS := -Wall -Wextra -Wpedantic -Wconversion -g
CFLAGS += -fPIC
LFLAGS := -shared -static -Wl,-rpath,.

OBJFOLDER := ./obj

# No edit below please

OBJNAMES := $(SRCFILES:.c=.o)
OBJFILES := $(addprefix $(OBJFOLDER)/,$(OBJNAMES))

BEAREXISTS := $(shell bear --version > /dev/null)
ifneq (BEAREXISTS,)
MAKEPFX := bear --config ~/.local/bin/bearcfg.json --
else
MAKEPFX :=
endif

all: bear

bear:
	$(MAKEPFX) $(MAKE) prebuild

prebuild: clean $(OBJFILES) build

clean:
	rm -rf $(OBJFOLDER)/*
	rm -f $(DLLNAME)

$(OBJFOLDER)/%.o: ./src/%.c
	mkdir -p $(dir $@)
	-$(CC) -c -o $@ $< $(CFLAGS)

$(OBJFILES): | $(OBJFOLDER)

$(OBJFOLDER):
	mkdir -p $(OBJFOLDER)

build:
	$(CC) -o $(DLLNAME) $(OBJFILES) $(LFLAGS)