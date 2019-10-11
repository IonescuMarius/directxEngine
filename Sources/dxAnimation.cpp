#include "..\Includes\dxAnimation.h"
#include "..\Includes\dxModel.h"
#include "..\Includes\dxInput.h"
#include "..\Includes\dxUtility.h"

dxAnimation::dxAnimation()
{

}
dxAnimation::~dxAnimation()
{

}

bool dxAnimation::init(LPDIRECT3DDEVICE9 device)
{
	model = new dxModel();
	model->loadModel(device,CUtility::GetTheCurrentDirectory()+"/Data/spaceship.x");
	if (!model)
	{
		return false;
	}
	position = D3DXVECTOR3(0.0f,0.0f,0.0f);
	rotation = D3DXVECTOR3(-5.0f,0.0f,0.0f);
	model->setRotation(rotation);
	model->setPosition(position);
	direction = 1;

	return true;
}

bool dxAnimation::initSpaceship(LPDIRECT3DDEVICE9 device)
{
	model = new dxModel();
	model->loadModel(device,CUtility::GetTheCurrentDirectory()+"/data/airplane.x");
	//model->loadModel(device,"airplane.x");
	if (!model)
	{
		return false;
	}
	position = D3DXVECTOR3(0.0f,0.0f,0.0f);
	rotation = D3DXVECTOR3(0.0f,0.0f,0.0f);
	model->setRotation(rotation);
	model->setPosition(position);
	direction = 1;

	return true;
}

void dxAnimation::updateFrames(int numberOfFrames)
{

	for (int i = 0; i<numberOfFrames; i++)
	{
		if (direction == 1)
		{
			position.x+=0.3f; //change the x position by 0.3 units per frame
		} 
		else 
		{
			position.x-=0.3f; //change the x position by 0.3 units per frame
		}
		if (position.x > 25.0f || position.x < -25.0f)
		{
			direction = direction * -1;
		}
	}
	model->setPosition(position);

}

void dxAnimation::render(LPDIRECT3DDEVICE9 device)
{
	model->render(device);
}

void dxAnimation::moveLeft(float index)
{
	position.x+=index;
	model->setPosition(position);
}

void dxAnimation::moveRight(float index)
{
	position.x-=index;
	model->setPosition(position);
}

void dxAnimation::moveUp(float index)
{
	position.y+=index;
	model->setPosition(position);
}

void dxAnimation::moveDown(float index)
{
	position.y-=index;
	model->setPosition(position);
}

void dxAnimation::scaling(D3DXVECTOR3 scal)
{
	model->setScale(scal);
}

void dxAnimation::rotationModel(D3DXVECTOR3 rot)
{
	model->setRotation(rot);
}

void dxAnimation::setPosition(D3DXVECTOR3 pos)
{
	model->setPosition(pos);
}

//-----------------------------------------------------------------------------
// Name : CAnimation () (Constructor)
//-----------------------------------------------------------------------------
CAnimation::CAnimation()
{
    m_pMatrix   = NULL;
    m_strName   = NULL;
}

//-----------------------------------------------------------------------------
// Name : ~CAnimation () (Destructor)
//-----------------------------------------------------------------------------
CAnimation::~CAnimation()
{
    if ( m_strName ) 
		free( m_strName ); 

    m_pMatrix   = NULL;
    m_strName   = NULL;
}

//-----------------------------------------------------------------------------
// Name : Attach ()
//-----------------------------------------------------------------------------
void CAnimation::Attach( LPD3DXMATRIX pMatrix, LPCTSTR strName /* = NULL */ )
{
    m_pMatrix = pMatrix;

	if ( m_strName ) 
		free( m_strName );
    m_strName = NULL;

    if ( strName ) 
		m_strName = _tcsdup( strName );
}

//-----------------------------------------------------------------------------
// Name : RotationX ()
// Desc : Rotate about the local X axis
//-----------------------------------------------------------------------------
void CAnimation::RotationX( float fRadAngle, bool bLocalAxis /* = true */ )
{
    D3DXMATRIX mtxRotate;

    if ( !m_pMatrix ) return;

    D3DXMatrixRotationX( &mtxRotate, fRadAngle );

    if ( bLocalAxis ) 
        D3DXMatrixMultiply( m_pMatrix, &mtxRotate, m_pMatrix );
    else
        D3DXMatrixMultiply( m_pMatrix, m_pMatrix, &mtxRotate );
}

//-----------------------------------------------------------------------------
// Name : RotationY ()
// Desc : Rotate about the local Y axis
//-----------------------------------------------------------------------------
void CAnimation::RotationY( float fRadAngle, bool bLocalAxis )
{
    D3DXMATRIX mtxRotate;

    if ( !m_pMatrix ) return;

    D3DXMatrixRotationY( &mtxRotate, fRadAngle );

    if ( bLocalAxis ) 
        D3DXMatrixMultiply( m_pMatrix, &mtxRotate, m_pMatrix );
    else
        D3DXMatrixMultiply( m_pMatrix, m_pMatrix, &mtxRotate );
}

//-----------------------------------------------------------------------------
// Name : RotationZ ()
// Desc : Rotate about the local Z axis
//-----------------------------------------------------------------------------
void CAnimation::RotationZ( float fRadAngle, bool bLocalAxis /* = true */ )
{
    D3DXMATRIX mtxRotate;

    if ( !m_pMatrix ) 
		return;

    D3DXMatrixRotationZ( &mtxRotate, fRadAngle );

    if ( bLocalAxis ) 
        D3DXMatrixMultiply( m_pMatrix, &mtxRotate, m_pMatrix );
    else
        D3DXMatrixMultiply( m_pMatrix, m_pMatrix, &mtxRotate );
}
