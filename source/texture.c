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

// THIS IS DIRECTLY COPIED FROM LIBNDS, WITH JUST DC_FlushRange ADDED
int replacement_glTexImage2D(
    int target,
    int empty1,
    GL_TEXTURE_TYPE_ENUM type,
    int sizeX,
    int sizeY,
    int empty2,
    int param,
    const void* texture
) {
    //---------------------------------------------------------------------------------
    uint32 size = 0;
    uint32 typeSizes[9] = {
        0, 8, 2, 4, 8, 3, 8, 16, 16
    }; // Represents the number of bits per pixels for each format

    if (!glGlob->activeTexture)
        return 0;

    size = 1 << (sizeX + sizeY + 6);

    switch (type) {
        case GL_RGB:
        case GL_RGBA:
            size = size << 1;
            break;
        case GL_RGB4:
        case GL_COMPRESSED:
            size = size >> 2;
            break;
        case GL_RGB16:
            size = size >> 1;
            break;
        default:
            break;
    }
    if (!size)
        return 0;

    if (type == GL_NOTEXTURE)
        size = 0;

    gl_texture_data* tex =
        (gl_texture_data*)DynamicArrayGet(&glGlob->texturePtrs, glGlob->activeTexture);

    // Clear out the texture data if one already exists for the active texture
    if (tex) {
        uint32 texType = ((tex->texFormat >> 26) & 0x07);
        if ((tex->texSize != size) || (typeSizes[texType] != typeSizes[type])) {
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
        if (type != GL_NOTEXTURE) {
            if (type != GL_COMPRESSED) {
                tex->texIndex = vramBlock_allocateBlock(glGlob->vramBlocks[0], tex->texSize, 3);
            } else {
                uint8* vramBAddr = (uint8*)VRAM_B;
                uint8* vramACAddr = NULL;
                uint8 *vramBFound, *vramACFound;
                uint32 vramBAllocSize = size >> 1;
                if ((VRAM_B_CR & 0x83) != 0x83)
                    return 0;

                // Process of finding a valid spot for compressed textures is as follows...
                //		Examine first available spot in VRAM_B for the header data
                //		Extrapulate where the tile data would go in VRAM_A or VRAM_C if the spot
                //in VRAM_B were used 		Check the extrapulated area to see if it is an empty spot
                //			If not, then adjust the header spot in VRAM_B by a ratio amount found by
                //the tile spot
                while (1) {
                    // Check designated opening, and return available spot
                    vramBFound = vramBlock_examineSpecial(
                        glGlob->vramBlocks[0],
                        vramBAddr,
                        vramBAllocSize,
                        2
                    );
                    // Make sure that the space found in VRAM_B is completely in it, and not
                    // extending out of it
                    if (vramGetBank((uint16*)vramBFound) != VRAM_B ||
                        vramGetBank((uint16*)(vramBFound + vramBAllocSize) - 1) != VRAM_B) {
                        return 0;
                    }
                    // Make sure it is completely on either half of VRAM_B
                    if (((uint32)vramBFound - (uint32)VRAM_B < 0x10000) &&
                        ((uint32)vramBFound - (uint32)VRAM_B + vramBAllocSize > 0x10000)) {
                        vramBAddr = (uint8*)VRAM_B + 0x10000;
                        continue;
                    }
                    // Retrieve the tile location in VRAM_A or VRAM_C
                    uint32 offset = ((uint32)vramBFound - (uint32)VRAM_B);
                    vramACAddr = (uint8*)(offset >= 0x10000 ? VRAM_B : VRAM_A) + (offset << 1);
                    vramACFound =
                        vramBlock_examineSpecial(glGlob->vramBlocks[0], vramACAddr, size, 3);
                    if (vramACAddr != vramACFound) {
                        // Adjust the spot in VRAM_B by the difference found with VRAM_A/VRAM_C,
                        // divided by 2
                        vramBAddr += (((uint32)vramACFound - (uint32)vramACAddr) >> 1);
                        continue;
                    } else {
                        // Spot found, setting up spots
                        tex->texIndex =
                            vramBlock_allocateSpecial(glGlob->vramBlocks[0], vramACFound, size);
                        tex->texIndexExt = vramBlock_allocateSpecial(
                            glGlob->vramBlocks[0],
                            vramBlock_examineSpecial(
                                glGlob->vramBlocks[0],
                                vramBFound,
                                vramBAllocSize,
                                2
                            ),
                            vramBAllocSize
                        );
                        break;
                    }
                }
            }
        }
        if (tex->texIndex) {
            tex->vramAddr = vramBlock_getAddr(glGlob->vramBlocks[0], tex->texIndex);
            tex->texFormat = (sizeX << 20) | (sizeY << 23) |
                             ((type == GL_RGB ? GL_RGBA : type) << 26) |
                             (((uint32)tex->vramAddr >> 3) & 0xFFFF);
        } else {
            tex->vramAddr = NULL;
            tex->texFormat = 0;
            return 0;
        }
    } else
        tex->texFormat = (sizeX << 20) | (sizeY << 23) |
                         ((type == GL_RGB ? GL_RGBA : type) << 26) | (tex->texFormat & 0xFFFF);

    glTexParameter(target, param);

    // Copy the texture data into either VRAM or main memory
    if (type != GL_NOTEXTURE && texture) {
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

        if (type == GL_RGB) {
            uint16* src = (uint16*)texture;
            uint16* dest = (uint16*)tex->vramAddr;
            size >>= 1;
            while (size--) {
                *dest++ = *src | 0x8000;
                src++;
            }
        } else {
            DC_FlushRange(texture, size);
            dmaCopyWords(0, texture, tex->vramAddr, size);
            if (type == GL_COMPRESSED) {
                vramSetBankB(VRAM_B_LCD);
                DC_FlushRange((char*)texture + tex->texSize, size >> 1);
                dmaCopyWords(
                    0,
                    (char*)texture + tex->texSize,
                    vramBlock_getAddr(glGlob->vramBlocks[0], tex->texIndexExt),
                    size >> 1
                );
            }
        }
        vramRestorePrimaryBanks(vramTemp);
    }

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

        replacement_glTexImage2D(
            0,
            0,
            GL_RGBA,
            width_code,
            height_code,
            0,
            TEXGEN_TEXCOORD | GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T,
            (u8*)texture_bin
        );

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
