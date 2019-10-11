/****************************************************************************
*                                                                           *
* dxTypes.h -- Types definition                                             *
*                                                                           *
* Copyright (c) Ionescu Marius. All rights reserved(2009).                  *
*                                                                           *
****************************************************************************/
#include <d3d9.h>
#include <d3dx9.h>

struct CUSTOMVERTEX1 {FLOAT X, Y, Z; DWORD COLOR;};
struct CUSTOMVERTEX2 {FLOAT X, Y, Z, RHW; DWORD COLOR;};
struct CUSTOMVERTEX3 {FLOAT X, Y, Z; D3DVECTOR NORMAL; FLOAT U, V;};

struct CUSTOMVERTEXP {FLOAT X, Y, Z; DWORD COLOR; FLOAT U, V;};
#define CUSTOMFVFP (D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1)

const float cameraEntityYawRollPitch=0.05f;
const float cameraWorldYawRollPitch=0.01f;
const float cameraWorldMovementSpeed=1.5f;
