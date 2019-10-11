#include "..\Includes\dxLight.h"

dxLight::dxLight(LPDIRECT3DDEVICE9 device)
{
	//
	d3dDevice = device;
	d3dDevice->SetRenderState(D3DRS_LIGHTING,   TRUE);
}

dxLight::~dxLight()
{

}

int dxLight::createLight(){

	D3DLIGHT9 *newLight = new D3DLIGHT9;
	newLight->Type = D3DLIGHT_POINT;
	
    newLight->Diffuse.r    = newLight->Diffuse.g  = newLight->Diffuse.b  = 1.0f;
    newLight->Specular.r   = newLight->Specular.g = newLight->Specular.b = 1.0f;
    newLight->Ambient.r    = newLight->Ambient.g  = newLight->Ambient.b  = 0.0f;
    newLight->Position     = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
	newLight->Range        = 60.0f;

	newLight->Attenuation0 = 1.0f;
	newLight->Attenuation1 = 0.0f;
	newLight->Attenuation2 = 0.0f;

	newLight->Phi = D3DXToRadian(40.0f);    // set the outer cone to 30 degrees
    newLight->Theta = D3DXToRadian(20.0f);    // set the inner cone to 10 degrees
    newLight->Falloff = 1.0f;    // use the typical falloff

	lights.push_back((D3DLIGHT9*) newLight);

	d3dDevice->SetLight((int)lights.size()-1, (D3DLIGHT9 *)lights[((int)lights.size()-1)]);
	d3dDevice->LightEnable((int)lights.size()-1, true);

	return ((int)lights.size() - 1);

}

int dxLight::createLightCustom()
{
	D3DLIGHT9 newLight;

    ZeroMemory(&newLight, sizeof(newLight));    // clear out the struct for use
    newLight.Type = D3DLIGHT_DIRECTIONAL;    // make the light type 'directional light'
    newLight.Diffuse.r = 0.5f;    // .5 red
    newLight.Diffuse.g = 0.5f;    // .5 green
    newLight.Diffuse.b = 0.5f;    // .5 blue
    newLight.Diffuse.a = 1.0f;    // full alpha

    D3DVECTOR vecDirection = {-1.0f, -0.3f, -1.0f};    // the direction of the light
    newLight.Direction = vecDirection;    // set the direction

    d3dDevice->SetLight(0, &newLight);    // send the light struct properties to light #0
    d3dDevice->LightEnable(0, TRUE);    // turn on light #0

	return ((int)lights.size() - 1);
}

int dxLight::createLightCustom2()
{
	// Create a directional light
	D3DLIGHT9 light;
	ZeroMemory( &light, sizeof(D3DLIGHT9) );
	light.Type       = D3DLIGHT_DIRECTIONAL;
	light.Diffuse.r  = 1.0f;
	light.Diffuse.g  = 1.0f;
	light.Diffuse.b  = 1.0f;
	light.Diffuse.a  = 1.0f;
	light.Range      = 1000.0f;

	// Direction for our light - it must be normalized - pointing down and along z
	D3DXVECTOR3 vecDir;
	vecDir = D3DXVECTOR3(0.0f,-0.3f,0.5f);
	D3DXVec3Normalize( (D3DXVECTOR3*)&light.Direction, &vecDir );
	return ((int)lights.size() - 1);
}

void dxLight::enableLight(int lightNumber)
{
    d3dDevice->LightEnable(lightNumber , TRUE);
}

void dxLight::disableLight(int lightNumber)
{
	d3dDevice->LightEnable(lightNumber , FALSE);
}

void dxLight::setDiffuse(int lightNumber, float r, float g, float b)
{
	lights[lightNumber]->Diffuse.r = r;
	lights[lightNumber]->Diffuse.g = g;
	lights[lightNumber]->Diffuse.b = b;
	d3dDevice->SetLight(lightNumber, (D3DLIGHT9 *)lights[lightNumber]);
}

void dxLight::setSpecular(int lightNumber, float r, float g, float b){
	lights[lightNumber]->Specular.r = r;
	lights[lightNumber]->Specular.g = g;
	lights[lightNumber]->Specular.b = b;
	d3dDevice->SetLight(lightNumber, (D3DLIGHT9 *)lights[lightNumber]);
}

void dxLight::setAmbient(int lightNumber, float r, float g, float b){
	lights[lightNumber]->Ambient.r = r;
	lights[lightNumber]->Ambient.g = g;
	lights[lightNumber]->Ambient.b = b;
	d3dDevice->SetLight(lightNumber, (D3DLIGHT9 *)lights[lightNumber]);
}

void dxLight::setPosition(int lightNumber, D3DXVECTOR3 newPosition)
{
	lights[lightNumber]->Position = newPosition;
	d3dDevice->SetLight(lightNumber, (D3DLIGHT9 *)lights[lightNumber]);
}

void dxLight::setRange(int lightNumber, float newRange)
{
	lights[lightNumber]->Range = newRange;
	d3dDevice->SetLight(lightNumber, (D3DLIGHT9 *)lights[lightNumber]);
}

bool dxLight::initLightCustom(LPDIRECT3DDEVICE9 d3dd3v)
{
	D3DLIGHT9 light;						 
	ZeroMemory( &light, sizeof(D3DLIGHT9) );
	light.Type       = D3DLIGHT_DIRECTIONAL;
	light.Diffuse.r  = 1.0f;
	light.Diffuse.g  = 1.0f;
	light.Diffuse.b  = 1.0f;
	light.Diffuse.a  = 1.0f;
	light.Range      = 1000.0f;
	D3DXVECTOR3 vecDir;
	vecDir = D3DXVECTOR3(0.0f,-0.3f,0.5f);
	D3DXVec3Normalize( (D3DXVECTOR3*)&light.Direction, &vecDir );
	d3dd3v->SetLight( 0, &light );
	d3dd3v->LightEnable( 0, TRUE );
	
	return true;
}
