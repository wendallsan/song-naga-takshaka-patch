# Project Name
TARGET = TakshakaPatch

# Sources
CPP_SOURCES = TakshakaPatch.cpp BlockSuperSawOsc.cpp SmartKnob.cpp BlockComb.cpp BlockOscillator.cpp BlockAtone.cpp BlockMoogLadder.cpp BlockSawOsc.cpp

# Library Locations
LIBDAISY_DIR = ../../libDaisy/
DAISYSP_DIR = ../../DaisySP/

# Core location, and generic Makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile
