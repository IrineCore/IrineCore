#pragma once

class CDnWorldOperationProp;
class CEtWorldEventArea;

class CDnOccupationZone
{
public :

	enum{
		ZONE_GUILD = 0,
		ZONE_A,
		ZONE_B,
		ZONE_C,
	};

	enum{
		COUNT_INDEX = 3,	// 3 types in dds (A,B,C)
	};

	CDnWorldOperationProp * m_pProp;
	int m_eState;

	UINT m_nOwnedUniqueID;			// User who succeeded at ownership time
	short m_nOwnedTemID;			// Current owning team

	UINT m_nTryUniqueID;			// When the attempter's PvPCommon::OccupationState is in the try state
	short m_nTryTeamID;				// Attempting team

	UINT m_nMoveWaitUniqueID;		// User who succeeded in move-wait state
	short m_nMoveWaitTeamID;		// Move-wait state team

	int m_nAreaID;
	SOBB m_OBB;

	int m_nRedClickStringID;
	int m_nBlueClickStringID;

	int m_nRedOwnStringID;
	int m_nBlueOwnStringID;

	int m_nRedDefendStringID;
	int m_nBlueDefendStringID;

	int m_nRedTeamStringID;
	int m_nBlueTeamStringID;

#ifdef PRE_MOD_PVPOBSERVER
	int m_nObserverRedTeamStringID;
	int m_nObserverBlueTeamStringID;
#endif // #ifdef PRE_MOD_PVPOBSERVER

	int m_nTrySound;
	int m_nOwnSound;

	int m_nTableModeID;

	std::wstring m_wszName;

	int m_nZoneIndex;	// Value for displaying Occupation War image index
	float m_fElapsedTime;

	CDnOccupationZone();
	virtual ~CDnOccupationZone();

	bool Initialize( CEtWorldEventArea * pArea, int nTableModeID );
	void Process( float fElapsedTime );
	
	void Clear();
	bool IsFlag( DnPropHandle hProp );
	bool IsFlag( int nAreaID );

	bool IsFlagOperation( DnPropHandle hProp );
	EtVector3 GetPropPosition();
	int GetImageIndex();
	DWORD GetImageColor();
};
