#include "..\Includes\dxCustomVertex.h"
#include "..\Includes\dxMgr.h"
#include "..\Includes\dxWorldTransformation.h"
#include "..\Includes\dxInput.h"
#include "..\Common\dxTypes.h"

dxCustomVertex::dxCustomVertex(void)
{
	v_buffer = NULL;
	i_buffer = NULL;
	texture = NULL;
}

dxCustomVertex::~dxCustomVertex(void)
{
	v_buffer->Release();
	texture->Release();
	meshX->Release();
	meshSpaceship->Release();
}

void dxCustomVertex::initVertex(LPDIRECT3DDEVICE9 direct3dDevice)
{
     CUSTOMVERTEX1 vertices[] =
     {
		//{ 3.0f, -3.0f, 0.0f, D3DCOLOR_XRGB(0, 0, 255), },
		//{ 4.0f, 3.0f, 0.0f, D3DCOLOR_XRGB(0, 255, 0), },
		//{ -2.0f, -3.0f, 0.0f, D3DCOLOR_XRGB(255, 0, 0), },

        { -5.0f, 3.0f, 0.0f, D3DCOLOR_XRGB(0, 0, 255), },
        { -1.0f, 3.0f, 0.0f, D3DCOLOR_XRGB(0, 255, 0), },
		{ -3.0f, -3.0f, 0.0f, D3DCOLOR_XRGB(255, 0, 0), },
		{ 3.0f, 3.0f, 0.0f, D3DCOLOR_XRGB(0, 255, 255), },
		{ 5.0f, -3.0f, 0.0f, D3DCOLOR_XRGB(255, 255, 255), },
		{ 1.0f, -3.0f, 0.0f, D3DCOLOR_XRGB(255, 0, 255), },
     };

	 direct3dDevice->CreateVertexBuffer(6*sizeof(CUSTOMVERTEX1),
                               D3DUSAGE_DYNAMIC,
                               CUSTOMFVF,
                               D3DPOOL_DEFAULT,
                               &v_buffer,
                               NULL);

     VOID* pVoid;

     v_buffer->Lock(0, 0, (void**)&pVoid, 0);
     memcpy(pVoid, vertices, sizeof(vertices));
     v_buffer->Unlock();
}

void dxCustomVertex::drawGraph(LPDIRECT3DDEVICE9 direct3dDevice)
{
	direct3dDevice->SetFVF(CUSTOMFVF);

	/*dxWorldTransf = new dxWorldTransformation();
	dxWorldTransf->createTranslation(direct3dDevice, 0.05f, DIK_LEFT);*/

	/****TEST****/
	direct3dDevice->SetStreamSource(0, v_buffer, 0, sizeof(CUSTOMVERTEX1));

	D3DXMATRIX matTranslateA;    // a matrix to store the translation for triangle A
	D3DXMATRIX matTranslateB;    // a matrix to store the translation for triangle B
	D3DXMATRIX matRotateY;    // a matrix to store the rotation for each triangle
	static float index = 0.0f; index+=0.05f; // an ever-increasing float value

	// build MULTIPLE matrices to translate the model and one to rotate
	//D3DXMatrixTranslation(&matTranslateA, 0.0f, 0.0f, 2.0f);
	//D3DXMatrixTranslation(&matTranslateB, 0.0f, 0.0f, -2.0f);
	//D3DXMatrixRotationY(&matRotateY, index);    // the front side

	// tell Direct3D about each world transform, and then draw another triangle
	/*direct3dDevice->SetTransform(D3DTS_WORLD, &(matTranslateA));
	direct3dDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 2);*/

	/*direct3dDevice->SetTransform(D3DTS_WORLD, &(matTranslateB * matRotateY));
	direct3dDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 2);*/
}

void dxCustomVertex::initGraphicsCube(LPDIRECT3DDEVICE9 direct3dDevice)
{
    // create the vertices using the CUSTOMVERTEX struct
    CUSTOMVERTEX1 vertices[] =
    {
        { -3.0f, 3.0f, -3.0f, D3DCOLOR_XRGB(0, 0, 255), },
        { 3.0f, 3.0f, -3.0f, D3DCOLOR_XRGB(0, 255, 0), },
        { -3.0f, -3.0f, -3.0f, D3DCOLOR_XRGB(255, 0, 0), },
        { 3.0f, -3.0f, -3.0f, D3DCOLOR_XRGB(0, 255, 255), },
        { -3.0f, 3.0f, 3.0f, D3DCOLOR_XRGB(0, 0, 255), },
        { 3.0f, 3.0f, 3.0f, D3DCOLOR_XRGB(255, 0, 0), },
        { -3.0f, -3.0f, 3.0f, D3DCOLOR_XRGB(0, 255, 0), },
        { 3.0f, -3.0f, 3.0f, D3DCOLOR_XRGB(0, 255, 255), },
		{ 6.0f, -3.0f, -3.0f, D3DCOLOR_XRGB(0, 255, 255), },
    };

    // create a vertex buffer interface called v_buffer
    direct3dDevice->CreateVertexBuffer(8*sizeof(CUSTOMVERTEX1),
                               0,
                               CUSTOMFVF,
                               D3DPOOL_MANAGED,
                               &v_buffer,
                               NULL);

    VOID* pVoid;    // a void pointer

    // lock v_buffer and load the vertices into it
    v_buffer->Lock(0, 0, (void**)&pVoid, 0);
    memcpy(pVoid, vertices, sizeof(vertices));
    v_buffer->Unlock();

    // create the indices using an int array
    short indices[] =
    {
        0, 1, 2,    // side 1
        2, 1, 3,
        4, 0, 6,    // side 2
        6, 0, 2,
        7, 5, 6,    // side 3
        6, 5, 4,
        3, 1, 7,    // side 4
        7, 1, 5,
        4, 5, 0,    // side 5
        0, 5, 1,
        7, 3, 2,    // side 6
        2, 7, 6,
    };

    // create an index buffer interface called i_buffer
    direct3dDevice->CreateIndexBuffer(36*sizeof(short), //3*12
                              0,
                              D3DFMT_INDEX16,
                              D3DPOOL_MANAGED,
                              &i_buffer,
                              NULL);

    // lock i_buffer and load the indices into it
    i_buffer->Lock(0, 0, (void**)&pVoid, 0);
    memcpy(pVoid, indices, sizeof(indices));
    i_buffer->Unlock();
}

void dxCustomVertex::initGraphicsPyramid(LPDIRECT3DDEVICE9 direct3dDevice)
{
    // create the vertices using the CUSTOMVERTEX
	struct CUSTOMVERTEX1 vertices[] =
	{
		// base
		{ -3.0f, 0.0f, 3.0f, D3DCOLOR_XRGB(0, 255, 0), },
		{ 3.0f, 0.0f, 3.0f, D3DCOLOR_XRGB(0, 0, 255), },
		{ -3.0f, 0.0f, -3.0f, D3DCOLOR_XRGB(255, 0, 0), },
		{ 3.0f, 0.0f, -3.0f, D3DCOLOR_XRGB(0, 255, 255), },

		// top
		{ 0.0f, 7.0f, 0.0f, D3DCOLOR_XRGB(0, 255, 0), },
	};

	// create a vertex buffer interface called v_buffer
	direct3dDevice->CreateVertexBuffer(5*sizeof(CUSTOMVERTEX1),
							   0,
							   CUSTOMFVF,
							   D3DPOOL_MANAGED,
							   &v_buffer,
							   NULL);

    VOID* pVoid;    // a void pointer

    // lock v_buffer and load the vertices into it
    v_buffer->Lock(0, 0, (void**)&pVoid, 0);
    memcpy(pVoid, vertices, sizeof(vertices));
    v_buffer->Unlock();

    // create the indices using an int array
	short indices[] =
	{
		0, 2, 1,    // base
		1, 2, 3,
		0, 1, 4,    // sides
		1, 3, 4,
		3, 2, 4,
		2, 0, 4,
	};

	// create a index buffer interface called i_buffer
	direct3dDevice->CreateIndexBuffer(18*sizeof(short),
							  0,
							  D3DFMT_INDEX16,
							  D3DPOOL_MANAGED,
							  &i_buffer,
							  NULL); 

    // lock i_buffer and load the indices into it
    i_buffer->Lock(0, 0, (void**)&pVoid, 0);
    memcpy(pVoid, indices, sizeof(indices));
    i_buffer->Unlock();
}

void dxCustomVertex::initGraphicsStarCraft(LPDIRECT3DDEVICE9 direct3dDevice)
{
	D3DXCreateTextureFromFile(direct3dDevice,    // the Direct3D device
                              "Carved.jpg",    // the filename of the texture
                              &texture);    // the address of the texture storage

    struct CUSTOMVERTEX3 vertices[] =
    {
        // fuselage
        { 3.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,  0.0f, 0.0f, },
        { 0.0f, 3.0f, -3.0f, 0.0f, 0.0f, 1.0f,   1.0f, 0.0f, },
        { 0.0f, 0.0f, 10.0f, 0.0f, 0.0f, 1.0f,   0.0f, 1.0f, },
        { -3.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,   1.0f, 1.0f, },

        // left gun
        { 3.2f, -1.0f, -3.0f, 0.0f, 0.0f, -1.0f,  0.0f, 0.0f, },
        { 3.2f, -1.0f, 11.0f, 0.0f, 0.0f, -1.0f,  0.0f, 1.0f, },
        { 2.0f, 1.0f, 2.0f, 0.0f, 0.0f, -1.0f,  1.0f, 0.0f, },

        // right gun
        { -3.2f, -1.0f, -3.0f, 0.0f, 1.0f, 0.0f,  0.0f, 0.0f, },
        { -3.2f, -1.0f, 11.0f, 0.0f, 1.0f, 0.0f,   0.0f, 1.0f, },
        { -2.0f, 1.0f, 2.0f, 0.0f, 1.0f, 0.0f,   1.0f, 0.0f, },

		// front gun
        { -3.2f, -1.0f, 6.0f, 0.0f, -1.0f, 0.0f,  0.0f, 0.0f, },
        { 3.2f, -1.0f, 6.0f, 0.0f, -1.0f, 0.0f,  1.0f, 0.0f, },
        { 0.0f, -1.0f, -6.0f, 0.0f, -1.0f, 0.0f,  0.0f, 1.0f, },

		// back wing
        { -3.2f, -1.0f, 11.0f, 1.0f, 0.0f, 0.0f,  4.0f, 5.0f, },
        { 3.2f, -1.0f, 11.0f, 1.0f, 0.0f, 0.0f,   4.0f, 6.0f, },
        { 0.0f, 1.0f, 6.0f, 1.0f, 0.0f, 0.0f,   4.0f, 5.0f, },
    };

    direct3dDevice->CreateVertexBuffer(16*sizeof(CUSTOMVERTEX3),
                               0,
                               CUSTOMFVF3,
                               D3DPOOL_MANAGED,
                               &v_buffer,
                               NULL);

    VOID* pVoid;

    v_buffer->Lock(0, 0, (void**)&pVoid, 0);
    memcpy(pVoid, vertices, sizeof(vertices));
    v_buffer->Unlock();

    short indices[] =
    {
        0, 1, 2,    // fuselage
        2, 1, 3,
        3, 1, 0,
        0, 2, 3,
        4, 5, 6,    // wings
        7, 8, 9,
		10, 11, 12,
		13, 14, 15,
    };

    direct3dDevice->CreateIndexBuffer(24*sizeof(short),
                              0,
                              D3DFMT_INDEX16,
                              D3DPOOL_MANAGED,
                              &i_buffer,
                              NULL);

    i_buffer->Lock(0, 0, (void**)&pVoid, 0);
    memcpy(pVoid, indices, sizeof(indices));
    i_buffer->Unlock(); 
}

void dxCustomVertex::drawGraphIndices(LPDIRECT3DDEVICE9 direct3dDevice)
{
	direct3dDevice->SetFVF(CUSTOMFVF3);
	static float indexRot = 0.0f; 
	static float indexTrans = 0.0f;

	D3DXMATRIX matRotateY;
	D3DXMATRIX matTranslate;
	D3DXMATRIX meshMat;

	
	//if(KEY_DOWN(VK_F1))
	//{
	//	indexRot+=0.03f;
	//	D3DXMatrixRotationY(&matRotateY, indexRot);
	//	//D3DXMatrixMultiply(&meshMat, &matRotateY, &matTranslate);
	//	direct3dDevice->SetTransform(D3DTS_WORLD, &(matRotateY));
	//}

	//if(KEY_DOWN(VK_F2))
	//{
	//	indexTrans+=0.03f;
	//	D3DXMatrixTranslation(&matTranslate, indexTrans, 0.0f, 0.0f);
	//	//D3DXMatrixMultiply(&meshMat, &matRotateY, &matTranslate);
	//	direct3dDevice->SetTransform(D3DTS_WORLD, &(matTranslate));
	//}

    direct3dDevice->SetStreamSource(0, v_buffer, 0, sizeof(CUSTOMVERTEX3));
    direct3dDevice->SetIndices(i_buffer);

    // draw the starcraft
	direct3dDevice->SetTexture(0, texture);
	direct3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 16, 0, 8);

	// draw the cube
	//direct3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 24, 0, 12); 
	// draw the pyramid
	//direct3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 5, 0, 6);
}

void dxCustomVertex::initLight(LPDIRECT3DDEVICE9 d3ddev)
{
   D3DLIGHT9 light;    // create the light struct

    ZeroMemory(&light, sizeof(light));    // clear out the struct for use
    light.Type = D3DLIGHT_DIRECTIONAL;    // make the light type 'directional light'
    light.Diffuse.r = 0.5f;    // .5 red
    light.Diffuse.g = 0.5f;    // .5 green
    light.Diffuse.b = 0.5f;    // .5 blue
    light.Diffuse.a = 1.0f;    // full alpha (we'll get to that soon)

    D3DVECTOR vecDirection = {-1.0f, -0.3f, -1.0f};    // the direction of the light
    light.Direction = vecDirection;    // set the direction

    d3ddev->SetLight(0, &light);    // send the light struct properties to light #0
    d3ddev->LightEnable(0, TRUE);    // turn on light #0
}

void dxCustomVertex::initVertexBlending(LPDIRECT3DDEVICE9 direct3dDevice)
{
	// create the vertices using the CUSTOMVERTEX struct
    CUSTOMVERTEX1 t_vert[] =
    {
        // square 1
        { -3.0f, 3.0f, 3.0f, D3DCOLOR_ARGB(255, 0, 0, 255), },
        { -3.0f, -3.0f, 3.0f, D3DCOLOR_ARGB(255, 0, 255, 0), },
        { 3.0f, 3.0f, 3.0f, D3DCOLOR_ARGB(255, 255, 0, 0), },
        { 3.0f, -3.0f, 3.0f, D3DCOLOR_ARGB(255, 0, 255, 255), },

        // square 2
        { -3.0f, 3.0f, 3.0f, D3DCOLOR_ARGB(192, 0, 0, 255), },
        { -3.0f, -3.0f, 3.0f, D3DCOLOR_ARGB(192, 0, 255, 0), },
        { 3.0f, 3.0f, 3.0f, D3DCOLOR_ARGB(192, 255, 0, 0), },
        { 3.0f, -3.0f, 3.0f, D3DCOLOR_ARGB(192, 0, 255, 255), },
   };

    // create a vertex buffer interface called t_buffer
    direct3dDevice->CreateVertexBuffer(8*sizeof(CUSTOMVERTEX1),
                               0,
                               CUSTOMFVF,
                               D3DPOOL_MANAGED,
                               &v_buffer,
                               NULL);

    VOID* pVoid;    // a void pointer

    // lock t_buffer and load the vertices into it
    v_buffer->Lock(0, 0, (void**)&pVoid, 0);
    memcpy(pVoid, t_vert, sizeof(t_vert));
    v_buffer->Unlock();

    return;
}

void dxCustomVertex::drawGraphBlending(LPDIRECT3DDEVICE9 direct3dDevice)
{
	    // set an ever-increasing float value
    static float index = 0.0f; index+=0.03f;

    // set the view transform
    D3DXMATRIX matView;    // the view transform matrix
    D3DXMatrixLookAtLH(&matView,
    &D3DXVECTOR3 ((float)sin(index) * 20.0f, 2.0f, 25.0f),    // the camera position
    &D3DXVECTOR3 (0.0f, 0.0f, 0.0f),    // the look-at position
    &D3DXVECTOR3 (0.0f, 1.0f, 0.0f));    // the up direction
    direct3dDevice->SetTransform(D3DTS_VIEW, &matView);    // set the view transform to matView

    // set the projection transform
    D3DXMATRIX matProjection;    // the projection transform matrix
    D3DXMatrixPerspectiveFovLH(&matProjection,
                               D3DXToRadian(45),    // the horizontal field of view
                               (FLOAT)SCREEN_WIDTH / (FLOAT)SCREEN_HEIGHT, // aspect ratio
                               1.0f,    // the near view-plane
                               100.0f);    // the far view-plane
    direct3dDevice->SetTransform(D3DTS_PROJECTION, &matProjection);    // set the projection

    // set the stream source
    direct3dDevice->SetStreamSource(0, v_buffer, 0, sizeof(CUSTOMVERTEX1));

    // set the first world transform
    D3DXMATRIX matTranslate;
    D3DXMatrixTranslation(&matTranslate, 0.0f, 0.0f, -10.0f);
    direct3dDevice->SetTransform(D3DTS_WORLD, &(matTranslate));    // set the world transform

    // draw the first square
    direct3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);

    // set the second world transform
    D3DXMatrixTranslation(&matTranslate, 0.0f, 0.0f, 0.0f);
    direct3dDevice->SetTransform(D3DTS_WORLD, &(matTranslate));    // set the world transform

    // draw the second square
    direct3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 4, 2); 
}

void dxCustomVertex::init_Texture(LPDIRECT3DDEVICE9 d3ddev)
{
    D3DXCreateTextureFromFile(d3ddev,    // the Direct3D device
                              "wood.png",    // the filename of the texture
                              &texture);    // the address of the texture storage

    // create the vertices using the CUSTOMVERTEX struct
    CUSTOMVERTEX3 vertices[] =
    {
        { -3.0f, -3.0f, 3.0f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f, },    // side 1
        { 3.0f, -3.0f, 3.0f,  0.0f, 0.0f, 1.0f,  1.0f, 0.0f, },
        { -3.0f, 3.0f, 3.0f,  0.0f, 0.0f, 1.0f,  0.0f, 1.0f, },
        { 3.0f, 3.0f, 3.0f,  0.0f, 0.0f, 1.0f,  1.0f, 1.0f, },

        { -3.0f, -3.0f, -3.0f,  0.0f, 0.0f, -1.0f,  0.0f, 0.0f, },    // side 2
        { -3.0f, 3.0f, -3.0f,  0.0f, 0.0f, -1.0f,  0.0f, 1.0f, },
        { 3.0f, -3.0f, -3.0f,  0.0f, 0.0f, -1.0f,  1.0f, 0.0f, },
        { 3.0f, 3.0f, -3.0f,  0.0f, 0.0f, -1.0f,  1.0f, 1.0f, },

        { -3.0f, 3.0f, -3.0f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f, },    // side 3
        { -3.0f, 3.0f, 3.0f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f, },
        { 3.0f, 3.0f, -3.0f,  0.0f, 1.0f, 0.0f,  1.0f, 0.0f, },
        { 3.0f, 3.0f, 3.0f,  0.0f, 1.0f, 0.0f,  1.0f, 1.0f, },

        { -3.0f, -3.0f, -3.0f,  0.0f, -1.0f, 0.0f,  0.0f, 0.0f, },    // side 4
        { 3.0f, -3.0f, -3.0f,  0.0f, -1.0f, 0.0f,  1.0f, 0.0f, },
        { -3.0f, -3.0f, 3.0f,  0.0f, -1.0f, 0.0f,  0.0f, 1.0f, },
        { 3.0f, -3.0f, 3.0f,  0.0f, -1.0f, 0.0f,  1.0f, 1.0f, },

        { 3.0f, -3.0f, -3.0f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f, },    // side 5
        { 3.0f, 3.0f, -3.0f,  1.0f, 0.0f, 0.0f,  0.0f, 1.0f, },
        { 3.0f, -3.0f, 3.0f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f, },
        { 3.0f, 3.0f, 3.0f,  1.0f, 0.0f, 0.0f,  1.0f, 1.0f, },

        { -3.0f, -3.0f, -3.0f,  -1.0f, 0.0f, 0.0f,  0.0f, 0.0f, },    // side 6
        { -3.0f, -3.0f, 3.0f,  -1.0f, 0.0f, 0.0f,  1.0f, 0.0f, },
        { -3.0f, 3.0f, -3.0f,  -1.0f, 0.0f, 0.0f,  0.0f, 1.0f, },
        { -3.0f, 3.0f, 3.0f,  -1.0f, 0.0f, 0.0f,  1.0f, 1.0f, },
    };

    // create a vertex buffer interface called v_buffer
    d3ddev->CreateVertexBuffer(24*sizeof(CUSTOMVERTEX3),
                               0,
                               CUSTOMFVF3,
                               D3DPOOL_MANAGED,
                               &v_buffer,
                               NULL);

    VOID* pVoid;    // a void pointer

    // lock v_buffer and load the vertices into it
    v_buffer->Lock(0, 0, (void**)&pVoid, 0);
    memcpy(pVoid, vertices, sizeof(vertices));
    v_buffer->Unlock();

    // create the indices using an int array
    short indices[] =
    {
        0, 1, 2,    // side 1
        2, 1, 3,
        4, 5, 6,    // side 2
        6, 5, 7,
        8, 9, 10,    // side 3
        10, 9, 11,
        12, 13, 14,    // side 4
        14, 13, 15,
        16, 17, 18,    // side 5
        18, 17, 19,
        20, 21, 22,    // side 6
        22, 21, 23,
    };

    // create an index buffer interface called i_buffer
    d3ddev->CreateIndexBuffer(36*sizeof(short),
                              0,
                              D3DFMT_INDEX16,
                              D3DPOOL_MANAGED,
                              &i_buffer,
                              NULL);

    // lock i_buffer and load the indices into it
    i_buffer->Lock(0, 0, (void**)&pVoid, 0);
    memcpy(pVoid, indices, sizeof(indices));
    i_buffer->Unlock();
}

void dxCustomVertex::initMesh(LPDIRECT3DDEVICE9 d3ddev)
{
	//D3DXCreateTeapot(d3ddev, &meshX, NULL);

	LPD3DXBUFFER bufShipMaterial;

    D3DXLoadMeshFromX("spaceship 2.x",    // load this file
                      D3DXMESH_DYNAMIC,    // load the mesh into system memory
                      d3ddev,    // the Direct3D Device
                      NULL,    // we aren't using adjacency
                      &bufShipMaterial,    // put the materials here
                      NULL,    // we aren't using effect instances
                      &numMaterials,    // the number of materials in this model
                      &meshSpaceship);    // put the mesh here

    // retrieve the pointer to the buffer containing the material information
    D3DXMATERIAL* tempMaterials = (D3DXMATERIAL*)bufShipMaterial->GetBufferPointer();

    // create a new material buffer for each material in the mesh
    material = new D3DMATERIAL9[numMaterials];

    for(DWORD i = 0; i < numMaterials; i++)    // for each material...
    {
        material[i] = tempMaterials[i].MatD3D;    // get the material info
        material[i].Ambient = material[i].Diffuse;    // make ambient the same as diffuse
    }
}

void dxCustomVertex::drawMesh(LPDIRECT3DDEVICE9 d3ddev)
{

    // draw the spaceship
    for(DWORD i = 0; i < numMaterials; i++)    // loop through each subset
    {
        d3ddev->SetMaterial(&material[i]);    // set the material for the subset
        meshSpaceship->DrawSubset(i);    // draw the subset
    } 
}
