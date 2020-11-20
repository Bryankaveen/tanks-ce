# todo: seems to be calculating the list of C files prior to running convimg

NAME ?= TANKS
ICON ?= iconc.png
DESCRIPTION ?= "Tanks! CE"
COMPRESSED ?= YES
ARCHIVED ?= YES

OUTPUT_MAP ?= YES

CFLAGS ?= -Wall -Wextra -O3

# ----------------------------

ifndef CEDEV
$(error CEDEV environment path variable is not set)
endif

include $(CEDEV)/meta/makefile.mk

# ----------------------------

tiles:
	@$(MAKE) -C src/tiles --no-print-directory

SHELL = bash
IMAGE_NAMES = {{pl,en}_{base,turret},shell}_{0..8}
CONVIMG_INPUT = $(shell echo src/gfx/trimmed/$(IMAGE_NAMES).png) src/gfx/tileset.png
CONVIMG_OUTPUT = $(shell echo src/gfx/$(IMAGE_NAMES).{c,h}) src/gfx/tileset.c src/gfx/tileset.h src/gfx/palette.c src/gfx/palette.h src/gfx/gfx.h src/gfx/enemy_palette.bin
BLENDER_OUTPUT = $(shell echo src/gfx/rendered/$(IMAGE_NAMES).png)
OFFSET_FILES = $(shell echo src/gfx/offsets/$(IMAGE_NAMES).h)

clean_gfx:
	rm -f src/gfx/*.{c,h,txt,bin}
	rm -rf src/gfx/{rendered,trimmed,offsets}

$(shell echo src/gfx/rendered/%_{0..8}.png): tank.blend
	mkdir -p src/gfx/rendered
	blender --background tank.blend --scene $* --engine BLENDER_EEVEE --render-output src/gfx/rendered/$*_#.png --render-anim

src/gfx/trimmed/%.png src/gfx/offsets/%.h: src/gfx/rendered/%.png
	mkdir -p src/gfx/{trimmed,offsets}
	convert $< -background '#00ff00' -alpha remove -alpha off -fuzz 25% -trim -print "#define $*_offset_x %X\n#define $*_offset_y %Y\n" $@ | sed 's/\+//g' > src/gfx/offsets/$*.h

src/gfx/offsets/offsets.h: $(OFFSET_FILES)
	echo \#ifndef H_OFFSETS > src/gfx/offsets/offsets.h
	echo \#define H_OFFSETS >> src/gfx/offsets/offsets.h
	echo \#define H_OFFSETS >> src/gfx/offsets/offsets.h
	echo \#ifdef __cplusplus >> src/gfx/offsets/offsets.h
	echo extern \"C\" \{ >> src/gfx/offsets/offsets.h
	echo \#endif >> src/gfx/offsets/offsets.h
	for CURRENT_TYPE in $(IMAGE_NAMES); do \
		echo \#include \"$$CURRENT_TYPE.h\" ; \
	done >> src/gfx/offsets/offsets.h
	echo \#ifdef __cplusplus >> src/gfx/offsets/offsets.h
	echo \} >> src/gfx/offsets/offsets.h
	echo \#endif >> src/gfx/offsets/offsets.h
	echo \#endif >> src/gfx/offsets/offsets.h

src/gfx/enemy_pal.c src/gfx/enemy_pal.h: src/gfx/enemy_palette.bin src/gfx/generate_enemy_palette.py
	cd src/gfx/ && python3 generate_enemy_palette.py


$(CONVIMG_OUTPUT): src/gfx/convimg.yaml $(CONVIMG_INPUT)
	cd src/gfx && convimg

all_gfx: $(CONVIMG_OUTPUT) src/gfx/offsets/offsets.h src/gfx/enemy_pal.c src/gfx/enemy_pal.h

.PHONY: tiles all_gfx clean_gfx