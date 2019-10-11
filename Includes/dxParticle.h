/****************************************************************************
*                                                                           *
* dxParticle.h -- PARTICLE ENGINE                                           *
*                                                                           *
* Copyright (c) Ionescu Marius. All rights reserved(2009).                  *
*                                                                           *
****************************************************************************/

#include <windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <string>

class PARTICLE
{
public:
    PARTICLE(LPDIRECT3DDEVICE9 d3dd3v)
    {
        reset_particle();
        d3dd3v = p_d3dDevice;
    }

    void render_particle();
    void set_particle(float camx, float camy, float camz);
    void run_particle(float seconds);
    void reset_particle();

    bool active;

protected:
    D3DXVECTOR3 position;
    D3DXVECTOR3 velocity;
    D3DXVECTOR3 acceleration;
    LPDIRECT3DDEVICE9 p_d3dDevice;
    float radius;
    float lifespan;
    float life;
    D3DXMATRIX matRotateX;
    D3DXMATRIX matRotateY;
};

class dxParticle
{
	public:
		dxParticle(LPDIRECT3DDEVICE9 d3dd3v);
		~dxParticle(void);

		void init_particle();
		float random_number(float low, float high);

	private:
		LPDIRECT3DDEVICE9 x_d3dDevice;
		
		
};		
