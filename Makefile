ifeq ($(OS), Windows_NT)
PRG_SUFFIX_FLAG := 1
else
PRG_SUFFIX_FLAG := 0
endif

LDFLAGS :=
CFLAGS := -I. -Itests/deps -g -Wall

TOOLS := $(wildcard tests/*.c)
PRGS := $(patsubst %.c,%,$(TOOLS))
PRG_SUFFIX=.exe
BINS := $(patsubst tests/%,build/%$(PRG_SUFFIX),$(PRGS))
OBJS := $(patsubst %,%.o,$(PRGS))
ifeq ($(PRG_SUFFIX_FLAG),0)
	OUTS = $(PRGS)
else
	OUTS = $(BINS)
endif

.SECONDEXPANSION:
OBJ = $(patsubst build/%$(PRG_SUFFIX),tests/%.o,$@)
ifeq ($(PRG_SUFFIX_FLAG),0)
	BIN = $(patsubst %$(PRG_SUFFIX),%,$@)
else
	BIN = $@
endif
%$(PRG_SUFFIX): $(OBJS)
	$(CC) $(OBJ) -o $(BIN)

tests: $(BINS) clean

clean:
	rm $(OBJS)

default: tests

.PHONY: tests clean default
