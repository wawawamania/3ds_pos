#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------

ifeq ($(strip $(DEVKITPRO)),)
$(error "Please set DEVKITPRO in your environment. export DEVKITPRO=<path to>/devkitpro")
endif

ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>/devkitARM")
endif

TOPDIR ?= $(CURDIR)

# ここ重要
include $(DEVKITARM)/3ds_rules

# 🔥 これが今回の修正ポイント
LD := arm-none-eabi-gcc

#---------------------------------------------------------------------------------
TARGET      := $(notdir $(CURDIR))
BUILD       := build
SOURCES     := source
DATA        :=
INCLUDES    :=
ROMFS       :=

#---------------------------------------------------------------------------------
ARCH        := -march=armv6k -mtune=mpcore -mfloat-abi=hard -mtp=soft

CFLAGS      := -g -Wall -O2 -mword-relocations -ffunction-sections $(ARCH)
CFLAGS      += $(INCLUDE) -D__3DS__
CXXFLAGS    := $(CFLAGS) -fno-rtti -fno-exceptions
ASFLAGS     := -g $(ARCH)
LDFLAGS     := -specs=3dsx.specs -g $(ARCH) -Wl,-Map,$(notdir $*.map)

#---------------------------------------------------------------------------------
LIBS        := -lctru -lm
LIBDIRS     := $(CTRULIB) $(PORTLIBS)

#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))

export OUTPUT    := $(CURDIR)/$(TARGET)
export TOPDIR    := $(CURDIR)
export VPATH     := $(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
                    $(foreach dir,$(DATA),$(CURDIR)/$(dir))
export DEPSDIR   := $(CURDIR)/$(BUILD)

CFILES      := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES    := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
SFILES      := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))
BINFILES    := $(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.*)))

export OFILES    := $(addsuffix .o,$(BINFILES)) \
                    $(CFILES:.c=.o) $(CPPFILES:.cpp=.o) $(SFILES:.s=.o)

export INCLUDE   := $(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
                    -I$(CURDIR)/$(BUILD) \
                    $(foreach dir,$(LIBDIRS),-I$(dir)/include) \
                    -I$(CURDIR)/include

export LIBPATHS  := $(foreach dir,$(LIBDIRS),-L$(dir)/lib)

.PHONY: all clean

all: $(BUILD)

$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

clean:
	@echo clean ...
	@rm -fr $(BUILD) $(TARGET).3dsx $(TARGET).elf $(TARGET).smdh $(TARGET).cia

else

DEPENDS := $(OFILES:.o=.d)

$(OUTPUT).3dsx: $(OFILES)

-include $(DEPENDS)

endif