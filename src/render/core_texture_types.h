#pragma once
typedef enum {
    // defines the different texture types e.g. TextureGrass
#   define _TextureType(type) Texture##type,
#   include "textures.def"
#   undef _TextureType
    TextureTypeCount,
} TextureType;

const char* TextureTypeToString(TextureType type);