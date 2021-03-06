# Makefile for libbokeh testing
# by Raphael Kim

CPP = gcc
CXX = g++
AR  = ar

# FLTK configs 
FCG = fltk-config --use-images
FLTKCFG_CXX := $(shell ${FCG} --cxxflags)
FLTKCFG_LFG := $(shell ${FCG} --ldflags)

SRC_PATH = src
OBJ_PATH = obj
BIN_PATH = bin
RES_PATH = res
FLI_PATH = ../fl_imgtk/lib
TARGET   = bokehtest

SRCS  = $(wildcard $(SRC_PATH)/*.cpp)
OBJS  = $(SRCS:$(SRC_PATH)/%.cpp=$(OBJ_PATH)/%.o)

CFLAGS  = -mtune=native -fopenmp -ffast-math -fomit-frame-pointer
CLFAGS += -O3 -s
CFLAGS += -I$(SRC_PATH)
CFLAGS += -I$(FLI_PATH)
CFLAGS += -I$(RES_PATH)
#CFLAGS += -g
#CFLAGS += -DDEBUG
CFLAGS += $(FLTKCFG_CXX)

LSTATICFLAG =
ifeq (static,$(firstword $(MAKECMDGOALS)))
	LSTATICFLAG = -static
endif

ifeq (noomp,$(firstword $(MAKECMDGOALS)))
	CFLAGS := $(filter-out -fopenmp,$(CFLAGS))
	CFLAGS += -DNOOPENMP
endif

LFLAGS  = -L$(FLI_PATH)
LFLAGS += $(LSTATICFLAG)
LFLAGS += -lfl_imgtk
LFLAGS += $(FLTKCFG_LFG)

static: all
noomp: all

all: prepare $(BIN_PATH)/$(TARGET)

prepare:
	@mkdir -p $(OBJ_PATH)
	@mkdir -p $(BIN_PATH)

clean:
	@rm -rf $(OBJ_PATH)/*.o
	@rm -rf $(BIN_PATH)/$(TARGET)

$(OBJS): $(OBJ_PATH)/%.o: $(SRC_PATH)/%.cpp
	@echo "Compiling $< ..."
	@$(CXX) $(CFLAGS) -c $< -o $@

$(BIN_PATH)/$(TARGET): $(OBJS)
	@echo "Linking $@ ..."
	@$(CXX) $(OBJ_PATH)/*.o $(CFLAGS) $(LFLAGS) -o $@
