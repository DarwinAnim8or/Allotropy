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

#include "quakedef.h"

#ifdef VMODELS_ENABLED

#include "glquake.h"
#include "gl_vmdl.h"

#define MAX_BONES   128

void QuaternionGLAngle(const vec3_t angles, vec4_t quaternion) {
    float       yaw = angles[2] * 0.5;
    float       pitch = angles[1] * 0.5;
    float       roll = angles[0] * 0.5;
    float       siny = sin(yaw);
    float       cosy = cos(yaw);
    float       sinp = sin(pitch);
    float       cosp = cos(pitch);
    float       sinr = sin(roll);
    float       cosr = cos(roll);

    quaternion[0] = sinr * cosp * cosy - cosr * sinp * siny;
    quaternion[1] = cosr * sinp * cosy + sinr * cosp * siny;
    quaternion[2] = cosr * cosp * siny - sinr * sinp * cosy;
    quaternion[3] = cosr * cosp * cosy + sinr * sinp * siny;
} //QuaternionGLAngle

void QuaternionGLMatrix(float x, float y, float z, float w, vec4_t *GLM) {
    GLM[0][0] = 1 - 2 * y * y - 2 * z * z;
    GLM[1][0] = 2 * x * y + 2 * w * z;
    GLM[2][0] = 2 * x * z - 2 * w * y;
    GLM[0][1] = 2 * x * y - 2 * w * z;
    GLM[1][1] = 1 - 2 * x * x - 2 * z * z;
    GLM[2][1] = 2 * y * z + 2 * w * x;
    GLM[0][2] = 2 * x * z + 2 * w * y;
    GLM[1][2] = 2 * y * z - 2 * w * x;
    GLM[2][2] = 1 - 2 * x * x - 2 * y * y;
} //QuaternionGLMatrix

matrix3x4   transform_matrix[MAX_BONES];    //Vertex transformation matrix

void GL_Draw_VMDL_AliasFrame(short *order, vec3_t *transformed, float tex_w, float tex_h);

//Mod_LoadVMDL  -   Read in the model's constituent parts
extern char loadname[];
qboolean Mod_LoadVMDL(qmodel_t *mod, void *buffer) {
    int i;
    vmdlcache_t             *model;
    vmdl_header_t           *header;
    vmdl_header_t           *texHeader;
    vmdl_tex_t              *tex;
    vmdl_bone_t             *bones;
    vmdl_bonecontroller_t   *boneCtls;
    texture_t               *texNums;
    int                     start, end, total;

    //Checksum the model:
    if (mod->flags & MDLF_DOCRC) {
        unsigned short crc;
        byte *p;
        int len;
        char st[40];

        CRC_Init(&crc);
        for (len = com_filesize, p = buffer; len; len--, p++)
            CRC_ProcessByte(&crc, *p);

        sprintf(st, "%d", (int)crc);
        //Info_SetValueForKey (cls.userinfo[0], (mod->flags & MDLF_PLAYER) ? pmodel_name : emodel_name, st, sizeof(cls.userinfo[0]));

        /*if (cls.state >= ca_connected) {

        }*/
    }

    start = Hunk_LowMark();

    //load the model
    model = Hunk_Alloc(sizeof(vmdlcache_t));
    header = Hunk_Alloc(com_filesize);
    memcpy(header, buffer, com_filesize);

    if (header->version != 10) {
        Con_Printf("Cannot load model %s - unknown version %i\n", mod->name, header->version);
        Hunk_FreeToLowMark(start);
        return false;
    }

    if (header->numControllers > MAX_BONES) {
        Con_Printf("Cannot load model %s - too many controllers %i\n", mod->name, header->numControllers);
        Hunk_FreeToLowMark(start);
        return false;
    }

    if (header->numBones > MAX_BONES) {
        Con_Printf("Cannot load model %s - too many bones %i\n", mod->name, header->numBones);
        Hunk_FreeToLowMark(start);
        return false;
    }

    texHeader = NULL;
    if (!header->numTextures) {
        char texmodelname[MAX_QPATH];
        COM_StripExtension(mod->name, texmodelname, sizeof(texmodelname));
        //no textures? eesh. They must be stored externally.
        unsigned int pathid;
        texHeader = (vmdl_header_t*)COM_LoadHunkFile(va("%st.mdl", texmodelname), pathid);
        if (texHeader) {
            if (texHeader->version != 10)
                texHeader = NULL;
        }
    }

    if (!texHeader)
        texHeader = header;
    else
        header->numTextures = texHeader->numTextures;

    tex = (vmdl_tex_t *) ((byte *) texHeader + texHeader->textures);
    bones = (vmdl_bone_t *) ((byte *) header + header->boneIndex);
    boneCtls = (vmdl_bonecontroller_t *) ((byte *) header + header->controllerIndex);

    model->header = (char *)header - (char *)model;
	model->texHeader = (char *)texHeader - (char *)model;
	model->textures = (char *)tex - (char *)model;
	model->bones = (char *)bones - (char *)model;
	model->boneCtls = (char *)boneCtls - (char *)model;

    texNums = Hunk_Alloc(texHeader->numTextures*sizeof(model->texNums));
    model->texNums = (char *)texNums - (char *)model;
    for (i = 0; i < texHeader->numTextures; i++) {
        texNums[i].gltexture->texnum = GL_LoadTexture32("", tex[i].width, tex[i].height, (byte *) texHeader + tex[i].offset, (byte *) texHeader + tex[i].width * tex[i].height + tex[i].offset, TEXPREF_NONE);
    }

    end = Hunk_LowMark();
    total = end - start;
    mod->type = mod_vmdl;

    Cache_Alloc(&mod->cache, total, mod->name);
    if (mod->cache.data) {
        return false;
    }

    memcpy(mod->cache.data, model, total);
    Hunk_FreeToLowMark(start);
    return true;
} //Mod_LoadVMDL

#ifdef HLSERVER
void *Mod_GetVMDLData()(model_t *mod) {
    hlmodelcache_t *mc;
    if (!mod || mod->type != mod_halflife)
        return NULL;	//halflife models only, please

    mc = Mod_Extradata(mod);
    return (void*)((char*)mc + mc->header);
}
#endif

/*int VMod_FrameForName(qmodel_t *mod, char *name) {
    int i;
    vmdl_header_t *h;
    vmdl_sequencelist_t *seqs;
    vmodelcache_t *mc;
    if (!mod || mod->type != mod_vmdl)
        return -1;	//halflife models only, please

    mc = Mod_Extradata(mod);

    h = (vmdl_header_t *)((char *)mc + mc->header);
    seqs = (vmdl_sequencelist_t*)((char*)h+h->seqIndex);

    for (i = 0; i < h->numseq; i++) {
        if (!strcmp(seqs[i].name, name))
            return i;
    }
    return -1;
}

int VMod_BoneForName(qmodel_t *mod, char *name) {
    int i;
    hlmdl_header_t *h;
    hlmdl_bone_t *bones;
    hlmodelcache_t *mc;
    if (!mod || mod->type != mod_halflife)
        return -1;	//halflife models only, please

    mc = Mod_Extradata(mod);

    h = (hlmdl_header_t *)((char *)mc + mc->header);
    bones = (hlmdl_bone_t*)((char*)h+h->boneindex);

    for (i = 0; i < h->numbones; i++) {
        if (!strcmp(bones[i].name, name))
            return i+1;
    }
    return 0;
}*/

#endif
