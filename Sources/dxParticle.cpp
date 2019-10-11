#include "..\Includes\dxParticle.h"
#include "..\Common\dxTypes.h"
#include "..\Includes\dxUtility.h"

static const int PARTICLECOUNT = 300;
PARTICLE *particle[PARTICLECOUNT];
float random_number(float low, float high);
LPDIRECT3DVERTEXBUFFER9 t_buffer;
LPDIRECT3DVERTEXBUFFER9 g_buffer;
LPDIRECT3DTEXTURE9 texture;

dxParticle::dxParticle(LPDIRECT3DDEVICE9 d3dd3v)
{
	x_d3dDevice = d3dd3v;
	
	t_buffer = NULL;
	g_buffer = NULL;
	particle[PARTICLECOUNT] = new PARTICLE(d3dd3v);	
}

dxParticle::~dxParticle(void)
{
	t_buffer->Release();    // close and release the particle vertex buffer
	g_buffer->Release();    // close and release the grid vertex buffer
	texture->Release();    // close and release the texture
}

void dxParticle::init_particle(void)
{
    // load the texture we will use
    std::string texturePath = CUtility::GetTheCurrentDirectory()+"/data/fire.png"; 
    D3DXCreateTextureFromFile(x_d3dDevice,
                              texturePath.c_str(),
                              &texture);

    // create the vertices using the CUSTOMVERTEX
    struct CUSTOMVERTEXP t_vert[] =
    {
        {-1.0f, 1.0f, 0.0f, D3DCOLOR_XRGB(118, 89, 55), 1, 0,},
        {-1.0f, -1.0f, 0.0f, D3DCOLOR_XRGB(118, 89, 55), 0, 0,},
        {1.0f, 1.0f, 0.0f, D3DCOLOR_XRGB(118, 89, 55), 1, 1,},
        {1.0f, -1.0f, 0.0f, D3DCOLOR_XRGB(118, 89, 55), 0, 1,},
    };

    // create a vertex buffer interface called t_buffer
    x_d3dDevice->CreateVertexBuffer(4*sizeof(CUSTOMVERTEXP),
                               0,
                               CUSTOMFVFP,
                               D3DPOOL_MANAGED,
                               &t_buffer,
                               NULL);

    VOID* pVoid;    // a void pointer

    // lock t_buffer and load the vertices into it
    t_buffer->Lock(0, 0, (void**)&pVoid, 0);
    memcpy(pVoid, t_vert, sizeof(t_vert));
    t_buffer->Unlock();

    return;
}

// this is the function that positions, rotates, scales and renders the particle
void PARTICLE::set_particle(float camx, float camy, float camz)
{
    // Before setting the world transform, do the intense mathematics!
    // a. Calculate the Differences
    static float difx, dify, difz;
    difx = camx - position.x;
    dify = camy - position.y;
    difz = camz - position.z;

    // b. Calculate the Distances
    static float FlatDist, TotalDist;
    FlatDist = sqrt(difx * difx + difz * difz);
    TotalDist = sqrt(FlatDist * FlatDist + dify * dify);

    // c. Y Rotation
    D3DXMatrixIdentity(&matRotateY);
    matRotateY._11 = matRotateY._33 = difz / FlatDist;    // cosY
    matRotateY._31 = difx / FlatDist;    // sinY
    matRotateY._13 = -matRotateY._31;    // -sinY

    // d. X Rotation
    D3DXMatrixIdentity(&matRotateX);
    matRotateX._22 = matRotateX._33 = FlatDist / TotalDist;    // cosX
    matRotateX._32 = dify / TotalDist;    // sinX
    matRotateX._23 = -matRotateX._32;    // -sinX

    // e. Tranlation
    static D3DXMATRIX matTranslate;
    D3DXMatrixTranslation(&matTranslate, position.x, position.y, position.z);

    // f. Scaling
    static D3DXMATRIX matScale;
    D3DXMatrixIdentity(&matScale);
    matScale._11 = matScale._22 = matScale._33 = radius;


    // Now build the world matrix and set it
    p_d3dDevice->SetTransform(D3DTS_WORLD, &(matScale * matRotateX* matRotateY* matTranslate));

    return;
}

// this function renders the particle
void PARTICLE::render_particle()
{
    p_d3dDevice->SetFVF(CUSTOMFVFP);
    p_d3dDevice->SetStreamSource(0, t_buffer, 0, sizeof(CUSTOMVERTEXP));
    p_d3dDevice->SetTexture(0, texture);
    p_d3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);

    return;
}

// this function updates the particle
void PARTICLE::run_particle(float seconds)
{
    // handle lifespan
    life += seconds;
    if(life > lifespan)
    {
        reset_particle();
        return;
    }

    velocity += acceleration * seconds;
    position += velocity * seconds;

    return;
}

// this function sets the particle back to its original state
void PARTICLE::reset_particle()
{
    active = false;
    position.x = 0.0f;
    position.y = 0.0f;
    position.z = 0.0f;
    velocity.x = random_number(-2.0f, 2.0f);
    velocity.y = 3.0f;
    velocity.z = random_number(-2.0f, 2.0f);
    acceleration.x = 0.0f;
    acceleration.y = random_number(15.0f, 25.0f);
    acceleration.z = 0.0f;
    radius = 1.0f;
    lifespan = 0.5f;
    life = 0.0f;

    return;
}


float random_number(float low, float high)
{
    return low + ((float)((float)rand() / (float)RAND_MAX) * (float)((high) - (low)));
}
