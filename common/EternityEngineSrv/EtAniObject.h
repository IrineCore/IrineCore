#pragma once
#include "EtObject.h"
#include "EtAni.h"
#include "EtBone.h"

#include "MultiSmartPtr.h"
#include "MultiCommon.h"

class CEtAniObject;
typedef CMultiSmartPtr< CEtAniObject > EtAniObjectHandle;

class CEtAniObject : public CEtObject, public TBoostMemoryPool< CEtAniObject >
{
public:
	CEtAniObject( CMultiRoom *pRoom );
	virtual ~CEtAniObject();

protected:
	int m_nCalcPositionFlag;
	EtAniHandle m_hAni;
	SBoneAniInfo m_AniInfo;
	std::vector< EtMatrix > m_vecTransMat;
	bool m_bCalcAni;

public:
	int Initialize( EtSkinHandle hSkin, EtAniHandle hAni );

	EtAniHandle &GetAniHandle() { return m_hAni; }
	int GetAniCount() { if( !m_hAni ) return 0; return m_hAni->GetAniCount(); }
	int GetAniLength( int nAni ) { if( !m_hAni ) return 0; return m_hAni->GetAniLength( nAni ); }
	float GetLastFrame( int nAni ) { if( !m_hAni ) return 0.f; return m_hAni->GetLastFrame( nAni ); }
	const char *GetAniName( int nAni ) { if( !m_hAni ) return ""; return m_hAni->GetAniName( nAni ); }
	void CalcAniDistance( int nAni, float fCurFrame, float fPrevFrame, EtVector3 &DistVec )
	{
		m_hAni->CalcAniDistance( nAni, fCurFrame, fPrevFrame, DistVec );
	}
	void SetCalcPositionFlag( int nFlag ) { m_nCalcPositionFlag = nFlag; }
	int GetCalcPositionFlag() { return m_nCalcPositionFlag; }
	virtual int GetAniFileIndex() { return m_hAni->GetMyIndex(); }

	virtual EtMatrix *GetBoneTransMat( int nBoneIndex );
	virtual EtMatrix *GetBoneMat( int nBoneIndex );
	void SetAniFrame( int nAni, float fFrame );

	void CalcAni();
	bool IsCalcAni() { return m_bCalcAni; }

	// On game server, we may need to get the bone matrix while this player flag is off, so temporarily turn it on and off. #28532
	void SetCalcAni( bool bCalcAni ) { m_bCalcAni = true; };
};
