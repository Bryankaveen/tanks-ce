
NAME        ?= TANKS
COMPRESSED  ?= YES
ICONSRC     ?= iconc.png
OUTPUT_MAP   ?= YES
DESCRIPTION ?= "Tanks! CE"

# ----------------------------

default: all_gfx all

include $(CEDEV)/include/.makefile

# ----------------------------

tiles:
	@$(MAKE) -C src/tiles --no-print-directory

SHELL = bash
IMAGE_NAMES = {{pl,en}_{base,turret},shell}_{0..8}
CONVIMG_INPUT = $(shell echo src/gfx/trimmed/$(IMAGE_NAMES).png) src/gfx/tileset.png
CONVIMG_OUTPUT = $(shell echo src/gfx/$(IMAGE_NAMES).{c,h}) src/gfx/tileset.c src/gfx/tileset.h src/gfx/palette.c src/gfx/palette.h src/gfx/gfx.h
BLENDER_OUTPUT = $(shell echo src/gfx/rendered/$(IMAGE_NAMES).png)
OFFSET_FILES = $(shell echo src/gfx/offsets/$(IMAGE_NAMES).h)

clean_gfx:
	rm -f src/gfx/*.{c,h}
	rm -rf src/gfx/{rendered,trimmed,offsets}

$(shell echo src/gfx/rendered/%_{0..8}.png): tank.blend
	mkdir -p src/gfx/rendered
	blender --background tank.blend --scene $* --render-output src/gfx/rendered/$*_#.png --render-anim

src/gfx/trimmed/%.png src/gfx/offsets/%.h: src/gfx/rendered/%.png
	mkdir -p src/gfx/{trimmed,offsets}
	convert $< -fuzz 25% -trim -print "#define $*_offset_x %X\n#define $*_offset_y %Y\n" $@ | sed 's/\+//g' > src/gfx/offsets/$*.h

src/gfx/offsets/offsets.h: $(OFFSET_FILES)
	echo \#ifndef H_OFFSETS > src/gfx/offsets/offsets.h
	echo \#define H_OFFSETS >> src/gfx/offsets/offsets.h
	for CURRENT_TYPE in $(IMAGE_NAMES); do \
		echo \#include \"$$CURRENT_TYPE.h\" ; \
	done >> src/gfx/offsets/offsets.h
	echo \#endif >> src/gfx/offsets/offsets.h


$(CONVIMG_OUTPUT): src/gfx/convimg.yaml $(CONVIMG_INPUT)
	cd src/gfx && convimg

all_gfx: $(CONVIMG_OUTPUT) src/gfx/offsets/offsets.h

.PHONY: tiles all_gfx clean_gfx