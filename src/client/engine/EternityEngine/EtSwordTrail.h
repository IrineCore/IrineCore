#pragma once
#include "etbaseeffect.h"

class CEtSwordTrail;
typedef CSmartPtr< CEtSwordTrail >	EtSwordTrailHandle;

class CEtRNS
{
protected:
	struct splineData
	{
		D3DXVECTOR3 position;
		D3DXVECTOR3 velocity;
		float distance;
	};
	std::vector<splineData> m_nodes;
	float m_maxDistance;
	int m_nodeCount;
	int	m_maxCount;
public:
	D3DXVECTOR3 GetPositionOnCubic(const D3DXVECTOR3 &startPos, const D3DXVECTOR3 &startVel, const D3DXVECTOR3 &endPos, const D3DXVECTOR3 &endVel, float time);
	void Init( int maxCount_ );
	void AddNode(const D3DXVECTOR3 &pos);
	float GetDistance(int index) { return m_nodes[index].distance;}
	D3DXVECTOR3 GetVelocityDir( int index ) { return m_nodes[index].velocity;}
	virtual void BuildSpline();
	D3DXVECTOR3 GetPosition(float time);
	int Count() { return m_maxCount;}
	D3DXVECTOR3 GetStartVelocity(int index);
	D3DXVECTOR3 GetEndVelocity(int index);
};

#define MAX_SWORD_TRAIL_VERTEX	300
class CEtSwordTrail : public CEtBaseEffect
{
public:
	CEtSwordTrail(void);
	virtual ~CEtSwordTrail(void);

protected:
	float m_fLifeTime;
	EtColor m_SwordColor;

	struct TrailSet
	{
		std::vector< EtVector3 > vecPosition;
		std::vector< float > vecTime;
		std::vector< EtVector3 > vecPositionResult;	
		std::vector< EtVector2 > vecTimeResult;
		CEtRNS		spline[ 2 ];
		int				nDummyIndex[ 2 ];
		CEtMeshStream *pMeshStream;
	};

	std::vector< TrailSet > m_Trails;

	static EtMaterialHandle s_hMaterial;
	static EtIndexBuffer *s_pCommonIndexBuffer;

	EtAniObjectHandle m_hParent;	
	bool m_bAdd;
	bool m_bPrevAdd;
	int		m_nAddIndex;

	int		m_nTechniqueIndex;

	float	m_fElapsedTime;
	float	m_fTotalTime;
	float	m_fFinishTime;
	int		m_nLengthTime;

public:
	void Initialize( EtAniObjectHandle hAniObject, EtTextureHandle hTexture, EtTextureHandle hNormalTexture, float fLifeTime, float fMinSegment = 50.0f );
	void SetSwordColor( EtColor &SwordColor ) { m_SwordColor = SwordColor; }	
	void CreateVertexBuffer( CEtMeshStream *pMeshStream );
	void CopyVertexBuffer();
	void AddPoint( int nIndex, int nLengtTime );	 // Add all if -1, otherwise add only the specified index if >= 0.
	virtual void Render( float fElapsedTime );
	void SetTechniqueIndex( int nTechIndex ) { m_nTechniqueIndex = nTechIndex; }

	void ProcessAdd();
	void ChangeTexture( EtTextureHandle hTexture, EtTextureHandle hNormalTexture );

	static void InitializeEffect();
	static void FinializeEffect();
};
