#  Configurable environment variables:
#    - OS: Inferred from `uname` (supported: Linux, Win)
#    - ARCH: Inferred from `uname -m` (supported: x86_64, i386)
#  OS specific configurable environment variables:
#    - MINGW_INCLUDES
#    - MINGW_LIBS
#    - GFRAME_INCLUDES
#    - GFRAME_LIBS
#    - CC

#=======================================================================
# CONFIGURABLE VARIABLES
#=======================================================================
# Define the list of subdirectories (which exist within src/)
  SUBDIRLIST := base

# Define every object required by compilation
  OBJS := \
         main.o \
         base/cmdParse.o \
         base/static.o \
         base/setup.o

# Define the target name
  TARGET := game

# Define the generated icon
#      Required files:
#        - assets/icon.ico
#        - assets/icon.rc
# TODO Uncomment this to add an icon to the game
#  WINICON := assets/icon.o

# List every header file
  HEADERS := $(shell find include-new/ -name *.h)
#=======================================================================


#=======================================================================
# COMPILATION FLAGS/SETTINGS
#=======================================================================
  # Set OS
  OS ?= $(shell uname)
  # TODO Check proper/valid Window's flags
  ifneq (, $(findstring $(OS), MINGW))
    # Since Window's build might differ from the system's arch, set it
    # manually here
    ifneq (, $(findstring $(OS), MINGW64))
      ARCH ?= x86_64
    else
      ARCH ?= i386
    endif
    OS := Win
  endif

  # Set the architecture
  ARCH ?= $(shell uname -m)

  # Set OS specific setting
  DIRLIST :=
  include conf/Makefile.*

  # Define CFLAGS
  ifneq (, $(GFRAME_INCLUDES))
    CFLAGS := $(CFLAGS) -I$(GFRAME_INCLUDES)
  endif
  CFLAGS := $(CFLAGS) -I"./include-new/" -Wall

  ifeq ($(ARCH), x86_64)
    CFLAGS := $(CFLAGS) -m64 -DALIGN=8
  else
    CFLAGS := $(CFLAGS) -m32 -DALIGN=4
  endif

  # Define LDFLAGS
  LDFLAGS := $(LDFLAGS) -L$(GFRAME_LIBS)
  ifeq ($(RELEASE), yes)
    LDFLAGS := $(LDFLAGS) -lGFraMe
  else
    LDFLAGS := $(LDFLAGS) -lGFraMe_dbg
  endif
#=======================================================================


#=======================================================================
# (OTHER) VARIABLE EXPASION AND DEFITION
#=======================================================================
  ifeq ($(RELEASE), yes)
    MODE := release
  else
    MODE := debug
  endif
  OBJLIST := $(OBJS:%=obj/$(OS)_$(MODE)/%)

  VPATH := src-new
#=======================================================================


#=======================================================================
# CONFIGURATION RULES
#=======================================================================
# Clear the suffixes' default rule, since there's an explicit one
.SUFFIXES:

# Define all targets that doesn't match its generated file
.PHONY: all clean mkdirs __clean
#=======================================================================


#=======================================================================
# RULES
#=======================================================================
all: bin/$(OS)_$(MODE)/$(TARGET)

# Rule for building/linking the game
bin/$(OS)_release/$(TARGET): $(OBJLIST) $(ICON)
	$(CC) $(CFLAGS)    -O3 -o $@ $(OBJLIST) $(ICON) $(LDFLAGS)

bin/$(OS)_debug/$(TARGET): $(OBJLIST) $(ICON)
	$(CC) $(CFLAGS) -g -O0 -o $@ $(OBJLIST) $(ICON) $(LDFLAGS)

# Rule for compiling any .c into its object
obj/$(OS)_release/%.o: %.c
	$(CC) $(CFLAGS)    -O3 -o $@ -c $<

obj/$(OS)_debug/%.o: %.c
	$(CC) $(CFLAGS) -g -O0 -o $@ -c $<

# Specific rule to re-compile static.o whenever any header is updated
obj/$(OS)_release/base/static.o: base/static.c $(HEADERS)
	$(CC) $(CFLAGS)    -O3 -o $@ -c $<
obj/$(OS)_debug/base/static.o: base/static.c $(HEADERS)
	$(CC) $(CFLAGS) -g -O0 -o $@ -c $<

# Rule for generating the icon
$(WINICON):
	windres assets/icon.rc $(WINICON)

clean: __clean mkdirs

mkdirs:
	mkdir -p $(DIRLIST)

__clean:
	rm -rf $(DIRLIST) bin/ obj/
#=======================================================================

