#include <stdio.h>
#include <zvb_gfx.h>
#include "assets.h"

gfx_error load_palette(gfx_context* ctx)
{
    // Load the palette
    const size_t palette_size = &_palette_end - &_palette_start;
    return gfx_palette_load(ctx, &_palette_start, palette_size, 0);
}

gfx_error load_tiles(gfx_context* ctx, gfx_tileset_options* options)
{
    const size_t size = &_tiles_end - &_tiles_start;
    return gfx_tileset_load(ctx, &_tiles_start, size, options);
}

uint8_t* get_tilemap_start(void) {
    return &_tilemap_start;
}

uint8_t* get_tilemap_end(void) {
    return &_tilemap_end;
}

uint8_t* get_tilemap_l1_start(void) {
    return &_tilemap_l1_start;
}

uint8_t* get_tilemap_l1_end(void) {
    return &_tilemap_l1_end;
}

void __assets__(void) __naked
{
    __asm__(
        // shared palette
        "__palette_start:\n"
        "    .incbin \"assets/piano.ztp\"\n"
        "__palette_end:\n"

        // tiles
        "__tiles_start:\n"
        "    .incbin \"assets/piano.zts\"\n"
        "__tiles_end:\n"

        // tilemap
        "__tilemap_start:\n"
        "    .incbin \"assets/piano-0.ztm\"\n"
        "__tilemap_end:\n"

        // tilemap_l1
        "__tilemap_l1_start:\n"
        "    .incbin \"assets/piano-1.ztm\"\n"
        "__tilemap_l1_end:\n"
    );
}