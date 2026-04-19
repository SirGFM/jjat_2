.PHONY: linux linux-32 linux-64 linux-debug linux-debug-32 linux-debug-64 \
	win win-32 win-64 win-debug win-debug-32 win-debug-64 \
	web \
	all game clean


# =========================================================================
# General output configuration.

# Base name for the output binary.
OUTPUT_BIN := game
# =========================================================================


# =========================================================================
# Configure the OS/arch variables
# and ensure that only a single target was specified.
undefine TGT
ifneq ($(filter linux%,$(MAKECMDGOALS)),)
  OS := linux
  TGT := $(TGT)linux
endif
ifneq ($(filter win%,$(MAKECMDGOALS)),)
  OS := win
  EXT := .exe
  TGT := $(TGT)win
endif
ifneq ($(filter web,$(MAKECMDGOALS)),)
  OS := emscript
  EXT := .ignore
  TGT := $(TGT)web
endif

ifneq ($(filter-out linux win web,$(TGT)),)
  $(error multiple targets were specified!)
endif

undefine TGT
ifneq ($(filter %-32,$(MAKECMDGOALS)),)
  ARCH := i686
  TGT := $(TGT)32
endif
ifneq ($(filter %-64,$(MAKECMDGOALS)),)
  ARCH := x86_64
  TGT := $(TGT)64
endif

ifneq ($(filter-out 32 64,$(TGT)),)
  $(error multiple architectures were specified!)
endif
# =========================================================================


# =========================================================================
# Automatically configure the build environment.
-include /gframe-dev/Makefile.env
# =========================================================================


# =========================================================================
# Configure the flags for debug mode.
MODE := release
ifneq ($(findstring debug,$(MAKECMDGOALS)),)
  MODE := debug
endif

ifeq ($(filter-out debug,$(MODE)),)
  OLEVEL := -O0 -g
  CFLAGS := $(CFLAGS) $(OLEVEL) -DDEBUG
  LIB_SUFFIX := _dbg
endif
# =========================================================================

# =========================================================================
# Configure the required libs and include paths
CFLAGS := $(CFLAGS) -Iinclude

# XXX: Removed -lm
LDLIBS := -lGFraMe$(LIB_SUFFIX) -lCSynth

ifeq (win, $(OS))
  LDFLAGS := $(LDFLAGS) -mwindows
  LDLIBS := $(LDLIBS) -lSDL2 -lopengl32
endif
ifeq (emscript, $(OS))
  CFLAGS := $(CFLAGS)
  LDLIBS := -s USE_SDL=2 -s WASM=1 $(LIB_DIR)/libGFraMe.bc $(LIB_DIR)/libCSynth.bc
endif

ifeq ($(MODE)-$(OS), debug-linux)
  CFLAGS := $(CFLAGS) -fsanitize=address -fno-omit-frame-pointer
  LDFLAGS := $(LDFLAGS) -fsanitize=address -fno-omit-frame-pointer
endif

CFLAGS := $(CFLAGS) -I"./misc" -Wall

ifeq ($(ARCH), x86_64)
    CFLAGS := $(CFLAGS) -m64 -DALIGN=8
else
    CFLAGS := $(CFLAGS) -m32 -DALIGN=4
endif

# Ugly hack: I'll put everything specific to the JJAT engine within these #ifdefs
CFLAGS := $(CFLAGS) -DJJAT_ENABLE_BACKGROUND -DJJATENGINE
# =========================================================================


# =========================================================================
# Recursive wildcard from https://stackoverflow.com/a/18258352
#
# To find every .c file in src:
#   FILES := $(call rwildcard, , *.c)
# To find all the .c and .h files in src:
#   FILES := $(call rwildcard, src/, *.c *.h)
rwildcard=$(foreach d,$(wildcard $1*), \
	$(call rwildcard,$d/,$2) \
	$(filter $(subst *,%,$2),$d))
# =========================================================================


# =========================================================================
# Prepare a unique output directory based on the OS/arch
TGT_DIR := $(OS)-$(ARCH)-$(MODE)
# =========================================================================


# =========================================================================
# Set the default rule.
all: game
# =========================================================================


# =========================================================================
# Find every input file in ./src and convert it to a target object.
# Additionally, for Windows, also add the icon to the list of objects.
SOURCES := $(call rwildcard,src,*.c)

SOURCE_OBJECTS := $(SOURCES:%.c=obj/$(TGT_DIR)/%.o)

ifeq (win,$(OS))
  ICONS := $(call rwildcard,assets,*.rc)
  ICON_OBJECTS := $(ICONS:%.rc=obj/$(TGT_DIR)/%.o)
endif

OBJECTS := $(SOURCE_OBJECTS) $(ICON_OBJECTS)

WEB_ASSETS_BMP := $(call rwildcard,assets,*bmp)
WEB_ASSETS := $(WEB_ASSETS_BMP)
# =========================================================================


# =========================================================================
# Track dependencies in source files.
# This is only executed if any rule other than clean is specified.
ifneq ($(filter-out clean,$(MAKECMDGOALS)),)
-include $(SOURCE_OBJECTS:%.o=%.d)
endif
# =========================================================================


# =========================================================================
# Create directories as needed.
%.mkdir:
	@ echo -e '\tmkdir $(@D)'
	@ mkdir -p $(@D)
	@ touch $@
# =========================================================================


# =========================================================================
# Build the final game.
# Instead of manually defining paths, use $(TGT_DIR) to simplify the rules.
linux linux-32 linux-64 linux-debug linux-debug-32 linux-debug-64 \
	win win-32 win-64 win-debug win-debug-32 win-debug-64: game

web: bin/$(TGT_DIR)/$(OUTPUT_BIN).html
game: bin/$(TGT_DIR)/$(OUTPUT_BIN)$(EXT)

bin/$(TGT_DIR)/$(OUTPUT_BIN)$(EXT): $(OBJECTS) | bin/$(TGT_DIR)/$(OUTPUT_BIN).mkdir
	@ echo -e '\t[ CC] Release target: $@'
	@ $(CC) $(OLEVEL) $(LDFLAGS) -o $@ $^ $(LDLIBS)

bin/$(TGT_DIR)/$(OUTPUT_BIN).html: bin/$(TGT_DIR)/$(OUTPUT_BIN).bc $(WEB_ASSETS)
	@ echo -e '\t[ CC] Release target: $@'
	@ $(CC) -o $@ $< $(LDLIBS) $(foreach FILE,$(WEB_ASSETS),--preload-file $(FILE)@/$(FILE:assets/%=%))

bin/$(TGT_DIR)/$(OUTPUT_BIN).bc: $(OBJECTS) | bin/$(TGT_DIR)/$(OUTPUT_BIN).mkdir
	@ echo -e '\t[ CC] Release target: $@'
	@ $(CC) $(OLEVEL) -shared -o $@ $^
# =========================================================================


# =========================================================================
# Build object files from source files.
obj/$(TGT_DIR)/%.o: %.rc | obj/$(TGT_DIR)/%.mkdir
	@ echo -e '\t[WRC] $< -> $@'
	@ $(WINDRES) $< $@

obj/$(TGT_DIR)/%.o: %.c | obj/$(TGT_DIR)/%.mkdir
	@ echo -e '\t[ CC] $< -> $@'
	@ $(CC) $(CFLAGS) -o $@ -c $<

obj/$(TGT_DIR)/%.d: %.c | obj/$(TGT_DIR)/%.mkdir
	@ echo -e '\t[DEP] $< -> $@'
	@ gcc $(CFLAGS) -MM -MG -MT "$@ $(@:%.d=%.o)" $< > $@

misc/auto/collisioncases.c: misc/collision.json
	@ echo '[OFF] Generating collision switch-case'
	@ mkdir -p misc/auto/
	@ python misc/collision.py $< $@
# =========================================================================


# =========================================================================
# Clean up.
clean:
	rm -rf ./obj/ ./bin/
# =========================================================================


# =========================================================================
# Deploy rule
# TODO: Review this
#deploy:
#	@ echo 'Generating version file...'
#	@ git tag -l --points-at `git log -n 1 --oneline | awk ' {print $$1} '` > VERSION.new
#	@ if [ -z "`cat VERSION.new`" ]; then echo "Failed to deploy: missing version/tag"; false; fi
#	@ if [ -f VERSION ] && [ "`cat VERSION`" == "`cat VERSION.new`" ]; then \
#        echo "Failed to deploy: version/tag has already been deployed"; false; fi
#	@ echo 'Deploying Linux 64 version...'
#	@ ./rush/rush protoman linux64 VERSION.new
#	@ echo 'Deploying Linux 32 version...'
#	@ ./rush/rush megaman linux32 VERSION.new
#	@ echo 'Deploying Windows 64 version...'
#	@ ./rush/rush crashman-64 win64 VERSION.new
#	@ echo 'Deploying Windows 32 version...'
#	@ ./rush/rush crashman-32 win32 VERSION.new
#	@ rm -f VERSION
#	@ mv VERSION.new VERSION
# =========================================================================
