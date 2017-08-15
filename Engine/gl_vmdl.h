/*
Copyright (C) 1996-2001 Id Software, Inc.
Copyright (C) 2002-2009 John Fitzgibbons and others
Copyright (C) 2007-2008 Kristian Duske
Copyright (C) 2010-2014 QuakeSpasm developers
Copyright (C) 2017 Gie Vanommeslaeghe

This code is based on code by James 'Ender' Brown [ender@quakesrc.org] (Copyright 2001),
 who released in under the GNU General Public License v2. (same as this engine)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#define VMDLPOLYHEADER  (('T' << 24) + ('S' << 16) + ('D' << 8) + 'I')  //"IDST" header in little-endian.
#define VMDLHEADER      "IDST"

//Main model header:
typedef struct {
    int     fileTypeID;     //IDSP
    int     version;        //10
    char    name[64];
    int     filesize;
    vec3_t  unknown3[5];
    int     unknown4;
    int     numBones;
    int     boneIndex;
    int     numControllers;
    int     controllerIndex;
    int     unknown5[2];
    int     numSeq;
    int     seqIndex;
    int     unknown6;
    int     seqGroups;
    int     numTextures;
    int     textures;
    int     unknown7[3];
    int     skins;
    int     numBodyParts;
    int     bodyPartIndex;
    int     unkown9[8];
} vmdl_header_t;

//Skin info:
typedef struct {
    char    name[64];
    int     flags;
    int     width;
    int     height;
    int     offset; //(index)
} vmdl_tex_t;

//Body part index:
typedef struct {
    char    name[64];
    int     numModels;
    int     base;
    int     modelIndex;
} vmdl_bodypart_t;

//Meshes:
typedef struct {
    int     numTris;
    int     index;
    int     skinIndex;
    int     unknown2;
    int     unknown3;
} vmdl_mesh_t;

//Bones:
typedef struct {
    char    name[32];
    int     parent;
    int     unknown1;
    int     boneController[6];
    int     value[6];
    int     scale[6];
} vmdl_bone_t;

//Bone controllers:
typedef struct {
    int     name;
    int     type;
    float   start;
    float   end;
    int     unknown1;
    int     index;
} vmdl_bonecontroller_t;

//Model descriptor:
typedef struct {
    char    name[64];
    int     unknown1;
    float   unknown2;
    int     numMesh;
    int     meshIndex;
    int     numVerts;
    int     vertInfoIndex;
    int     vertIndex;
    int     unknown3[5];
} vmdl_model_t;

//Animation:
typedef struct {
    unsigned short  offset[6];
} vmdl_anim_t;

//Animation frames:
typedef union {
    struct {
        byte   valid;
        byte   total;
    } num;

    short       value;
} vmdl_animvalue_t;

//Sequence descriptions:
typedef struct {
    char    name[32];
    float   timing;
    int     loop;
    int     unknown1[4];
    int     numFrames;
    int     unknown2[2];
    int     motionType;
    int     motionBone;
    vec3_t  unknown3;
    int     unknown4[2];
    vec3_t  bbox[2];
    int     hasBlendSeq;
    int     index;
    int     unknown7[2];
    float   unknown[4];
    int     unknown8;
    int     seqIndex;
    int     unknown9[4];
} vmdl_sequencelist_t;

//Sequence groups
typedef struct {
    char    name[96];       //Should be split label[32] and name[64]
    void*   cache;
    int     data;
} vmdl_sequencedata_t;

//Model internal structure
typedef struct {
    float   controller[5];  //Position of the bone controllers
    float   adjust[5];

    //Static pointers
    vmdl_header_t           *header;
    vmdl_header_t           *texHeader;
    vmdl_tex_t              *textures;
    vmdl_bone_t             *bones;
    vmdl_bonecontroller_t   *boneCtls;
    texinfo_t               *texNums;
} vmdl_t;

typedef struct {    //The cache. A vmdl_t is generated when drawing.
    int     header;
    int     texHeader;
    int     textures;
    int     bones;
    int     boneCtls;
    int     texNums;
} vmdlcache_t;

//Mathlib prototypes:
void    QuaternionGLAngle(const vec3_t angles, vec4_t quaternion);
void    QuaternionGLMatrix(float x, float y, float z, float w, vec4_t *GLM);

//Drawing:
qboolean    Mod_LoadVMDL(qmodel_t *mod, void *buffer);
void        R_DrawVMDL(entity_t *current);

//Physics:
void        *Mod_GetVMDLData(qmodel_t *mod);
