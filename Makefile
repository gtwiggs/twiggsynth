# Project Name
TARGET = Twiggsynth

USE_DAISYSP_LGPL = 1

# Sources
CPP_SOURCES = Twiggsynth.cpp

# Library Locations
LIBDAISY_DIR = ../electro-smith/libDaisy
DAISYSP_DIR = ../electro-smith/DaisySP

# Core location, and generic makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile
