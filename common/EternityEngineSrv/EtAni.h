#pragma once
#include "EtResource.h"

#define ANI_FILE_STRING	"Eternity Engine Ani File 0.1"
#define ANI_FILE_VERSION	11
#define ANI_HEADER_RESERVED	( 1024 - sizeof( SAniFileHeader ) )
struct SAniFileHeader
{
	char szHeaderString[ 256 ];
	int nVersion;
	int nBoneCount;
	int nAniCount;
};

class CEtBone;
class CEtAni;
struct SBoneAniInfo;
typedef CSmartPtr< CEtAni >	EtAniHandle;

class CEtAni : public CEtResource, public TBoostMemoryPool< CEtAni >
{
public:
	CEtAni( CMultiRoom *pRoom );
	virtual ~CEtAni();
	void Clear();

protected:
	SAniFileHeader m_AniHeader;
	std::map<std::string, CEtBone *> m_mapBone;
	std::vector< CEtBone * >	m_vecBone;
	std::vector< std::string >	m_vecAniName;
	std::vector< int >			m_vecAniLength;

public:
	int GetAniCount() { return m_AniHeader.nAniCount; }
	int GetAniLength( int nAni ) { return m_vecAniLength[ nAni ]; }
	float GetLastFrame( int nAni ) { return m_vecAniLength[ nAni ] - 1.0f; }
	const char *GetAniName( int nAni ) { return m_vecAniName[ nAni ].c_str(); }

	int GetBoneCount() { return m_AniHeader.nBoneCount; }
	int GetBoneIndex( const char *pBoneName );
	CEtBone *GetBone( int nIndex ) { return m_vecBone[ nIndex ]; }
	CEtBone *FindBone( const char *pBoneName );

	int LoadAni( CStream *pStream );
	int LoadMultiAni( CStream *pStream );
	int LoadResource( CStream *pStream );
	void BuildHierarchy();

	void Reset();
	float CalcCorrectFrame( int nAni, float fFrame );
	void SetAni( SBoneAniInfo *pAniInfo );
	void CalcAni( std::vector< EtMatrix > &vecInvWorldMat, std::vector< EtMatrix > &vecTransMat );
	void CalcAniDistance( int nAni, float fCurFrame, float fPrevFrame, EtVector3 &DistVec );
	void SetCalcPositionFlag( int nFlag );

	int MergeAni( CEtAni *pAni );
};
