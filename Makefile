#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------

ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

GAME_TITLE := Molebo
GAME_SUBTITLE1 := Kill those fish!
GAME_SUBTITLE2 := by duckonaut
GAME_ICON := $(PWD)/icon.bmp

include $(DEVKITARM)/ds_rules

#---------------------------------------------------------------------------------
# TARGET is the name of the output
# BUILD is the directory where object files & intermediate files will be placed
# SOURCES is a list of directories containing source code
# INCLUDES is a list of directories containing extra header files
#---------------------------------------------------------------------------------
TARGET		:=	$(shell basename $(CURDIR))
BUILD		:=	build
SOURCES		:=	source lib/tinf/src lib/dsma
DATA		:=	data
ASSETS		:=	assets
INCLUDES	:=	include lib/tinf/include lib/dsma

#---------------------------------------------------------------------------------
# options for code generation
#---------------------------------------------------------------------------------
ARCH	:=	-mthumb-interwork

CFLAGS	:=	-g -Wall -O2 \
 			-march=armv5te -mtune=arm946e-s -fno-gcse\
			-ffast-math \
			$(ARCH)

CFLAGS	+=	$(INCLUDE) -DARM9
CXXFLAGS	:= $(CFLAGS) -fno-rtti -fno-exceptions

ASFLAGS	:=	-g $(ARCH)
LDFLAGS	=	-specs=ds_arm9.specs -g $(ARCH) -Wl,-Map,$(notdir $*.map)

#---------------------------------------------------------------------------------
# any extra libraries we wish to link with the project
#---------------------------------------------------------------------------------
LIBS	:= -lnds9 -lm
 
 
#---------------------------------------------------------------------------------
# list of directories containing libraries, this must be the top level containing
# include and lib
#---------------------------------------------------------------------------------
LIBDIRS	:=	$(LIBNDS)
 
#---------------------------------------------------------------------------------
# no real need to edit anything past this point unless you need to add additional
# rules for different file extensions
#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))
#---------------------------------------------------------------------------------
 
export OUTPUT	:=	$(CURDIR)/$(TARGET)
 
export VPATH	:=	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
					$(foreach dir,$(DATA),$(CURDIR)/$(dir))

export DEPSDIR	:=	$(CURDIR)/$(BUILD)

CFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
SFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))
BINFILES	:=	$(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.*)))
PNGFILES	:=	$(foreach dir,$(ASSETS),$(notdir $(wildcard $(dir)/*.png)))
OBJFILES	:=	$(foreach dir,$(ASSETS),$(notdir $(wildcard $(dir)/*.obj)))
NRGBFILES	:= 	$(PNGFILES:.png=.nrgb)
NMSHFILES	:= 	$(OBJFILES:.obj=.nmsh)
MD5MESHFILES	:= 	$(foreach dir,$(ASSETS),$(notdir $(wildcard $(dir)/*.md5mesh)))
MD5ANIMFILES	:= 	$(foreach dir,$(ASSETS),$(notdir $(wildcard $(dir)/*.md5anim)))
DSMFILES	:= 	$(MD5MESHFILES:.md5mesh=.dsm)
DSAFILES	:= 	$(MD5ANIMFILES:.md5anim=.dsa)
 
#---------------------------------------------------------------------------------
# use CXX for linking C++ projects, CC for standard C
#---------------------------------------------------------------------------------
ifeq ($(strip $(CPPFILES)),)
#---------------------------------------------------------------------------------
	export LD	:=	$(CC)
#---------------------------------------------------------------------------------
else
#---------------------------------------------------------------------------------
	export LD	:=	$(CXX)
#---------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------

export OFILES	:=	$(addsuffix .o,$(BINFILES)) \
					$(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(SFILES:.s=.o)
 
export INCLUDE	:=	$(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
					$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
					$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
					-I$(CURDIR)/$(BUILD)
 
export LIBPATHS	:=	$(foreach dir,$(LIBDIRS),-L$(dir)/lib)
 
.PHONY: $(BUILD) tools clean
 
#---------------------------------------------------------------------------------
$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile
 
#---------------------------------------------------------------------------------
clean:
	@echo clean ...
	@rm -fr $(BUILD) $(TARGET).elf $(TARGET).nds $(TARGET).ds.gba 
	@rm -fr $(DATA)/*.nrgb $(DATA)/*.nmsh $(DATA)/*.dsa $(DATA)/*.dsm
	@$(MAKE) --no-print-directory -C $(CURDIR)/tools -f $(CURDIR)/tools/Makefile clean
 
 
#---------------------------------------------------------------------------------
TOOLS	:=	$(CURDIR)/tools/png2nds-rgb

tools:
	@echo "Building tools"
	@$(MAKE) --no-print-directory -C $(CURDIR)/tools -f $(CURDIR)/tools/Makefile

assets: tools $(NRGBFILES) $(NMSHFILES) $(DSMFILES) $(DSAFILES)
	@echo "Assets converted"

$(NRGBFILES):
	@echo "Converting PNG to NRGB"
	@$(foreach png,$(PNGFILES),$(CURDIR)/tools/png2nds-rgb $(CURDIR)/$(ASSETS)/$(png) $(CURDIR)/$(DATA)/$(png:.png=.nrgb) -g;)

$(NMSHFILES):
	@echo "Converting OBJ to NMSH"
	@$(foreach obj,$(OBJFILES),$(CURDIR)/tools/obj2nds-mesh $(CURDIR)/$(ASSETS)/$(obj) $(CURDIR)/$(DATA)/$(obj:.obj=.nmsh) -g;)

$(DSAFILES): $(DSMFILES)
	@echo "Converting MD5ANIM to DSA"
	@$(foreach md5anim,$(MD5ANIMFILES),$(CURDIR)/tools/dsma/md5_to_dsma.py --anims $(CURDIR)/$(ASSETS)/$(md5anim) --name anim --output $(CURDIR)/$(DATA);)

$(DSMFILES):
	@echo "Converting MD5MESH to DSM"
	@$(foreach md5mesh,$(MD5MESHFILES),$(CURDIR)/tools/dsma/md5_to_dsma.py --model $(CURDIR)/$(ASSETS)/$(md5mesh) --name $(md5mesh:.md5mesh=) --output $(CURDIR)/$(DATA) --texture $(shell cat $(CURDIR)/$(ASSETS)/$(md5mesh:.md5mesh=.meta)) --blender-fix;)
 
else
 
DEPENDS	:=	$(OFILES:.o=.d)
 
#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------
$(OUTPUT).nds	: 	$(OUTPUT).elf
$(OUTPUT).elf	:	$(OFILES)

#---------------------------------------------------------------------------------
%.bin.o	:	%.bin
	@echo $(notdir $<)
	@$(bin2o)

%.nrgb.o	:	%.nrgb
	@echo $(notdir $<)
	@$(bin2o)
 
%.nmsh.o	:	%.nmsh
	@echo $(notdir $<)
	@$(bin2o)

%.dsa.o	:	%.dsa
	@echo $(notdir $<)
	@$(bin2o)

%.dsm.o	:	%.dsm
	@echo $(notdir $<)
	@$(bin2o)
 
-include $(DEPENDS)
 
#---------------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------------
