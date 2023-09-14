#include "texture.h"
#include <stdio.h>

#include "tinf.h"

enum GL_TEXTURE_SIZE_ENUM texture_size_decode(u16 size) {
    switch (size) {
        case 1024:
            return TEXTURE_SIZE_1024;
        case 512:
            return TEXTURE_SIZE_512;
        case 256:
            return TEXTURE_SIZE_256;
        case 128:
            return TEXTURE_SIZE_128;
        case 64:
            return TEXTURE_SIZE_64;
        case 32:
            return TEXTURE_SIZE_32;
        case 16:
            return TEXTURE_SIZE_16;
        case 8:
            return TEXTURE_SIZE_8;
        default:
            return TEXTURE_SIZE_256;
    }
}

extern void vramBlock_init(s_vramBlock* mb);
extern s_vramBlock* vramBlock_Construct(uint8* start, uint8* end);
extern void vramBlock_terminate(s_vramBlock* mb);
extern void vramBlock_Deconstruct(s_vramBlock* mb);
extern uint8* vramBlock__allocateBlock(
    s_vramBlock* mb,
    struct s_SingleBlock* block,
    uint8* addr,
    uint32 size
);
extern uint32 vramBlock__deallocateBlock(s_vramBlock* mb, struct s_SingleBlock* block);
extern uint8* vramBlock_examineSpecial(s_vramBlock* mb, uint8* addr, uint32 size, uint8 align);
extern uint32 vramBlock_allocateSpecial(s_vramBlock* mb, uint8* addr, uint32 size);
extern uint32 vramBlock_allocateBlock(s_vramBlock* mb, uint32 size, uint8 align);
extern uint32 vramBlock_deallocateBlock(s_vramBlock* mb, uint32 index);
extern void vramBlock_deallocateAll(s_vramBlock* mb);
extern uint8* vramBlock_getAddr(s_vramBlock* mb, uint32 index);
extern uint32 vramBlock_getSize(s_vramBlock* mb, uint32 index);
extern uint16* vramGetBank(uint16* addr);

int custom_glSetImage(int width, int height, const void* texture) {
    //---------------------------------------------------------------------------------
    uint32 size = 0;
    int sizeX = 0, sizeY = 0;

    sizeX = texture_size_decode(width);
    sizeY = texture_size_decode(height);

    if (!glGlob->activeTexture)
        return 0;

    size = 1 << (sizeX + sizeY + 6);

    // bc GL_RGBA
    size = size << 1;
    if (!size)
        return 0;

    gl_texture_data* tex =
        (gl_texture_data*)DynamicArrayGet(&glGlob->texturePtrs, glGlob->activeTexture);

    // Clear out the texture data if one already exists for the active texture
    if (tex) {
        if ((tex->texSize != size)) {
            if (tex->texIndexExt)
                vramBlock_deallocateBlock(glGlob->vramBlocks[0], tex->texIndexExt);
            if (tex->texIndex)
                vramBlock_deallocateBlock(glGlob->vramBlocks[0], tex->texIndex);
            tex->texIndex = tex->texIndexExt = 0;
            tex->vramAddr = NULL;
        }
    }

    tex->texSize = size;

    // Allocate a new space for the texture in VRAM
    if (!tex->texIndex) {
        tex->texIndex = vramBlock_allocateBlock(glGlob->vramBlocks[0], tex->texSize, 3);

        if (tex->texIndex) {
            tex->vramAddr = vramBlock_getAddr(glGlob->vramBlocks[0], tex->texIndex);
            tex->texFormat = (sizeX << 20) | (sizeY << 23) | ((GL_RGBA) << 26) |
                             (((uint32)tex->vramAddr >> 3) & 0xFFFF);
        } else {
            tex->vramAddr = NULL;
            tex->texFormat = 0;
            return 0;
        }
    } else
        tex->texFormat =
            (sizeX << 20) | (sizeY << 23) | ((GL_RGBA) << 26) | (tex->texFormat & 0xFFFF);

    glTexParameter(0, GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T | TEXGEN_TEXCOORD);

    // Copy the texture data into either VRAM or main memory
    uint32 vramTemp = VRAM_CR;
    uint16* startBank = vramGetBank((uint16*)tex->vramAddr);
    uint16* endBank = vramGetBank((uint16*)((char*)tex->vramAddr + size - 1));

    do {
        if (startBank == VRAM_A)
            vramSetBankA(VRAM_A_LCD);
        else if (startBank == VRAM_B)
            vramSetBankB(VRAM_B_LCD);
        else if (startBank == VRAM_C)
            vramSetBankC(VRAM_C_LCD);
        else if (startBank == VRAM_D)
            vramSetBankD(VRAM_D_LCD);
        startBank += 0x10000;
    } while (startBank <= endBank);

    DC_FlushRange(texture, size);
    dmaCopyWords(0, texture, tex->vramAddr, size);
    vramRestorePrimaryBanks(vramTemp);

    return 1;
}
texture_t texture_load(const u8* data, size_t datalen) {
    // validate magic (NRGB)
    const u8 magic[4] = { 'N', 'R', 'G', 'B' };
    if (memcmp(data, magic, 4) != 0) {
        iprintf("Error: invalid magic\n");

        texture_t texture = {
            .handle = 0,
            .dyn = NULL,
        };
        return texture;
    }

    u16 width = *(u16*)(data + 4);
    u16 height = *(u16*)(data + 6);

    enum GL_TEXTURE_SIZE_ENUM width_code, height_code;

    width_code = texture_size_decode(width);
    height_code = texture_size_decode(height);

    bool compressed = data[8];

    texture_handle tex;
    glGenTextures(1, &tex);
    glBindTexture(0, tex);

    if (compressed) {
        texture_t texture = {
            .handle = tex,
            .dyn = malloc(sizeof(dyn_texture_t)),
        };

        texture.dyn->width = width;
        texture.dyn->height = height;
        texture.dyn->type = GL_RGBA;
        size_t expected_len = (width * height * 2);

        texture.dyn->data = malloc(expected_len);
        memset(texture.dyn->data, 0, expected_len);

        // compressed source
        u8* source = (u8*)data + 16;
        size_t outlen = expected_len;

        iprintf("compressed: %u\n", datalen);

        // decompress using tinf
        int res = tinf_gzip_uncompress(texture.dyn->data, &outlen, source, datalen - 16);

        if ((res != TINF_OK) || (outlen != expected_len)) {
            iprintf("Error: failed to decompress texture\n");
            iprintf("res: %d\n", res);

            texture_t texture = {
                .handle = 0,
                .dyn = NULL,
            };
            return texture;
        }

        iprintf("decompressed: %u\n", outlen);

        rgb* texture_bin = (rgb*)texture.dyn->data;

        custom_glSetImage(width, height, texture_bin);

        return texture;
    } else {
        rgb* texture_bin = (rgb*)(data + 16);

        glTexImage2D(
            0,
            0,
            GL_RGBA,
            width_code,
            height_code,
            0,
            TEXGEN_TEXCOORD | GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T,
            (u8*)texture_bin
        );

        texture_t texture = {
            .handle = tex,
            .dyn = NULL,
        };

        return texture;
    }
}

void texture_free(texture_t texture) {
    if (texture.dyn) {
        free(texture.dyn->data);
        free(texture.dyn);
    }
    glDeleteTextures(1, &texture.handle);
}
