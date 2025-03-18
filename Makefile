# Project Name
TARGET = Twiggsynth

USE_DAISYSP_LGPL = 1

# DaisyExample repo root directory
DAISYEXAMPLES_DIR = ../electro-smith/DaisyExamples

# Sources
CPP_SOURCES = Twiggsynth.cpp

# Library Locations
LIBDAISY_DIR = $(DAISYEXAMPLES_DIR)/libDaisy
DAISYSP_DIR = $(DAISYEXAMPLES_DIR)/DaisySP

# Core location, and generic makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile
