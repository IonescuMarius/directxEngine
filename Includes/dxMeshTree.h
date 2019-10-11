/****************************************************************************
*                                                                           *
* dxMeshTree.h -- Mesh Tree                                                 *
*                                                                           *
* Copyright (c) Ionescu Marius. All rights reserved(2009).                  *
*                                                                           *
****************************************************************************/

#ifndef _DXMESHTREE_H_
#define _DXMESHTREE_H_

#include "..\Includes\dxMeshAnimation.h"

//-----------------------------------------------------------------------------
// Typedef, Structures & Enumerators
//-----------------------------------------------------------------------------
enum BranchNodeType { BRANCH_BEGIN = 1, BRANCH_SEGMENT = 2, BRANCH_END = 3, BRANCH_FROND = 4 };

typedef struct _BranchNode
{
    D3DXVECTOR3     Position;
    D3DXVECTOR3     Direction;
    D3DXVECTOR3     Right;
    D3DXVECTOR3     Dimensions;
    BranchNodeType  Type;
    _BranchNode    *Parent;
    _BranchNode    *Child;
    _BranchNode    *Sibling;
    USHORT          Iteration;          // The iteration at which this was generated
    USHORT          BranchSegment;
    USHORT          VertexStart;        
    ULONG           UID;
    bool            BoneNode;
    LPD3DXFRAME     pBone;

    // Auto Heirarchy Destructor
    ~_BranchNode() { if ( Child ) delete Child; if ( Sibling ) delete Sibling; }

    // Auto clearing constructor
    _BranchNode()  { ZeroMemory( this, sizeof(_BranchNode) ); }

} BranchNode;

typedef struct _TreeGrowthProperties
{
    USHORT      Max_Iteration_Count;        // The maximum number of iterations that a branch chain can grow
    USHORT      Initial_Branch_Count;       // Initial number of branches growing from the root
    USHORT      Min_Split_Iteration;        // This number of iterations must have passed before splitting can begin
    USHORT      Max_Split_Iteration;        // Once this number of iterations have passed, splitting will cease.
    float       Min_Split_Size;             // Splitting can only occur if the size of the branch is at least this
    float       Max_Split_Size;             // Splitting will not occur if the size of the branch is larger than this

    bool        Include_Fronds;             // Include fronds in the build
    USHORT      Min_Frond_Create_Iteration; // Minimum iteration that fronds can be created.
    float       Frond_Create_Chance;        // Chance that a split branch will be a frond.
    D3DXVECTOR3 Frond_Min_Size;             // The smallest size a frond can be (fronds get smaller closer to branch tips)
    D3DXVECTOR3 Frond_Max_Size;             // The largest size a frond can be (fronds get larger closer to branch roots)

    float       Two_Split_Chance;           // Chance that a branch will split into two.
    float       Three_Split_Chance;         // Chance that a branch will split into three.
    float       Four_Split_Chance;          // Chance that a branch will split into four.
    float       Split_End_Chance;           // Chance that a branch will end when a split occurs.

    float       Segment_Deviation_Chance;   // Chance that a new segment's angle will deviate from that of it's parent.
    float       Segment_Deviation_Min_Cone; // The minimum cone angle that a branches growth angle can deviate from it's parent
    float       Segment_Deviation_Max_Cone; // The maximum cone angle that a branches growth angle can deviate from it's parent
    float       Segment_Deviation_Rotate;   // Max Polar rotation for segments
    
    float       Length_Falloff_Scale;       // The length of each segment falls off linearly from tree root to tip. This allows us to scale that falloff.

    float       Split_Deviation_Min_Cone;   // Minimum cone angle in which the angle of split branches can deviate their parent
    float       Split_Deviation_Max_Cone;   // Maximum cone angle in which the angle of split branches can deviate their parent
    float       Split_Deviation_Rotate;     // Max Polar rotation for splits

    float       SegDev_Parent_Weight;       // Weight with which the original direction of the segments parent is averaged with the deviation
    float       SegDev_GrowthDir_Weight;    // Weight with which the growth direction vector is averaged with the deviated segment direction

    USHORT      Branch_Resolution;          // Number of vertices used to for the branch mesh segment ring
    USHORT      Bone_Resolution;            // Specify the number of nodes that are referenced by each bone (i.e. a new Bone is created every N nodes)

    float       Texture_Scale_U;            // U Scale to use for applying branch textures
    float       Texture_Scale_V;            // V Scale to use for applying branch textures.

    D3DXVECTOR3 Growth_Dir;

} TreeGrowthProperties;

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : CTreeActor (Class)
// Desc : Derived actor, providing the ability to generate and render trees.
//-----------------------------------------------------------------------------
class CTreeActor : public dxMeshAnimation
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors for This Class.
    //-------------------------------------------------------------------------
             CTreeActor( );
    virtual ~CTreeActor( );

    //-------------------------------------------------------------------------
    // Public Functions for This Class
    //-------------------------------------------------------------------------
    void                    SetGrowthProperties ( const TreeGrowthProperties & Prop );
    TreeGrowthProperties    GetGrowthProperties ( ) const;
    void                    SetBranchMaterial   ( LPCTSTR strTexture, D3DMATERIAL9 * pMaterial = NULL );
    void                    SetFrondMaterial    ( LPCTSTR strTexture, D3DMATERIAL9 * pMaterial = NULL );

    HRESULT                 GenerateTree        ( ULONG Options, LPDIRECT3DDEVICE9 pD3DDevice, const D3DXVECTOR3 & vecDimensions, const D3DXVECTOR3 & vecInitialDir = D3DXVECTOR3( 0.0f, 1.0f, 0.0f ), ULONG BranchSeed = 0 );
    HRESULT                 GenerateAnimation   ( D3DXVECTOR3 vecWindDir, float fWindStrength, bool bApplyCustomSets = true );
    virtual void            Release             ( );
    
    //-------------------------------------------------------------------------
    // Public Overloaded Functions (CActor::) for This Class
    //-------------------------------------------------------------------------
    void                    DrawActor           ( );
    void                    DrawActorSubset     ( ULONG AttributeID );

private:
    //-------------------------------------------------------------------------
    // Private Functions for This Class
    //-------------------------------------------------------------------------
    HRESULT     GenerateBranches    ( const D3DXVECTOR3 & vecDimensions, const D3DXVECTOR3 & vecInitialDir = D3DXVECTOR3( 0.0f, 1.0f, 0.0f ), ULONG Seed = 0 );
    void        BuildBranchNodes    ( BranchNode * pNode, ULONG & BranchUID, ULONG Iteration = 0 );

    bool        ChanceResult        ( float fValue ) const;
    void        DeviateNode         ( BranchNode * pNode, float fAzimuthThetaMin, float fAzimuthThetaMax, float fPolarTheta = 360.0f ) const;

    HRESULT     BuildFrameHierarchy ( ID3DXAllocateHierarchy * pAllocate );
    HRESULT     BuildNode           ( BranchNode * pNode, D3DXFRAME * pParent, CTriMesh * pMesh, const D3DXMATRIX & mtxCombined, ID3DXAllocateHierarchy * pAllocate );
    HRESULT     BuildSkinInfo       ( BranchNode * pNode, CTriMesh * pMeshData, LPD3DXSKININFO * ppSkinInfo );
    HRESULT     BuildNodeAnimation  ( BranchNode * pNode, const D3DXVECTOR3 & vecWindAxis, float fWindStrength, LPD3DXKEYFRAMEDANIMATIONSET pAnimSet );
    HRESULT     AddBranchSegment    ( BranchNode * pNode, CTriMesh * pMesh );
    HRESULT     AddBranchFrond      ( BranchNode * pNode, CTriMesh * pMesh );

    //-------------------------------------------------------------------------
    // Private Variables for This Class
    //-------------------------------------------------------------------------
    ULONG                   m_nBranchSeed;      // The random seed used to generate tree branches
    TreeGrowthProperties    m_Properties;       // Growth properties.
    BranchNode             *m_pHeadNode;        // Head Tree Branch Node.

    D3DMATERIAL9            m_Material;         // The material to apply to the tree
    LPTSTR                  m_strTexture;       // The texture to apply to the tree.

    D3DMATERIAL9            m_FrondMaterial;    // The material to apply to the fronds.
    LPTSTR                  m_strFrondTexture;  // The texture to apply to the fronds.

    ULONG                   m_nFrondAttribute;  // Stores the final attribute ID of the frond data
};

#endif