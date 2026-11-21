#pragma once
#include "EtBillboardEffectData.h"

class CEtBillboardEffect : public CSmartPtrBase< CEtBillboardEffect >, public CBackgroundLoaderCallback
{
public:
	CEtBillboardEffect();
	~CEtBillboardEffect();

	// bool bReUseParticleInstance Enable in game to reduce load, disable in Effect Tool because changed values do not apply.
	void Initialize( EtBillboardEffectDataHandle hBillboardEffectData, EtMatrix *pInitMat, bool bReuseInstance = true );	
	void Show( bool bShow ){ m_bShow = bShow; }
	bool IsShow() { return m_bShow; }
	void EnableTracePos( bool bTracePos ) { m_bTracePos = bTracePos; }
	bool IsTracePos() { return m_bTracePos; }
	void EnableLoop( bool bLoop );
	bool IsLoop() { return m_bLoop; }
	void EnableSelfDelete( bool bDelete ) { m_bSelfDelete = bDelete; }
	void SetCullDist( float fDist ) { m_fCullDistance = fDist; }
	float GetCullDist() { return m_fCullDistance; }
	void EnableCull( bool bEnableCull ) { m_bEnableCull = bEnableCull; }
	void SetColor( EtColor *pColor ) { m_Color = *pColor; }
	bool IsCull();
	void StopPlay();
	bool IsPlay() { return !m_bStop; }
	void SetLink( EtMatrix WorldMat, bool bFixedY, float fY, char *szBoneName, EtVector3 vPos, EtVector3 vRotate, EtAniObjectHandle hParent );
	void UnLink() { m_bLink = false; }
	void SetParticleTick( int nTick ) { m_nBillboardEffectTick = nTick; }
	int GetParticleTick() { return m_nBillboardEffectTick;	}
	void EnableReduceFillRate( bool bReduce ) { m_bReduceFillRate = bReduce; }
	RenderType SetRenderType( RenderType Type );
	RenderType GetRenderType() { return m_RenderType; }
	EtBillboardEffectDataHandle GetBillboardEffectData() { return m_hBillboardEffectData; }

	void SetWorldMat( EtMatrix *pWorldMat ) { m_PrevWorldMat = m_WorldMat; m_WorldMat = *pWorldMat; }
	EtMatrix *GetWorldMat() { return &m_WorldMat; }
	void SetWorldPos( EtVector3 vPos ) { m_PrevWorldMat = m_WorldMat; *(EtVector3*)&m_WorldMat._41 = vPos; }
	void SetScale( float fScale ) { m_fScale = fScale; }
	float GetScale() { return m_fScale; }

	void SaveTracePosition();
	void InitRender();
	void Render();
	void RenderImmediate();
	bool CalcTick( float fElapsedTime );

	virtual void OnLoadComplete( CBackgroundLoader *pLoader );

	static void RenderBillboardEffectList( float fElapsedTime );
	static void ProcessDeleteBillboardEffectList( float fElapsedTime );

	static void SetEffectCountOption( int nOption ) { s_nEffectCountOption = nOption; }
	static int GetEffectCountOption() { return s_nEffectCountOption; }

protected:
	EtBillboardEffectDataHandle m_hBillboardEffectData;
	CEtBillboardEffectDataInstance *m_pInstance;
	CEtBillboardEffectDataInstance *m_pLoopInstance;

	int m_nBillboardEffectTick;
	float m_fFloatTick;
	float m_fScale;
	bool m_bShow;
	bool m_bTracePos;
	bool m_bLoop;
	bool m_bStop;
	bool m_bSelfDelete;
	bool m_bDelete;

	std::vector< EtVector4 > m_vecTracePos;
	int m_nPrevTracePos;

	RenderType m_RenderType;
	EtMatrix m_PrevWorldMat;
	EtMatrix m_WorldMat;
	EtColor m_Color;
	float m_fCullDistance;

	EtMatrix m_LinkWorldMat;
	bool m_bLinkFixedY;
	float m_fLinkY;
	int m_LinkBoneIndex;
	int m_nLinkType;
	EtVector3 m_LinkPos;
	EtVector3 m_LinkRotate;
	EtAniObjectHandle m_hLinkParent;
	bool m_bLink;
	bool m_bReuseInstance;
	bool m_bReduceFillRate;
	bool m_bEnableCull;

	static int s_nEffectCountOption;
};

typedef CSmartPtr< CEtBillboardEffect > EtBillboardEffectHandle;