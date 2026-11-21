#pragma once

#ifdef PRE_ADD_NPC_REPUTATION_SYSTEM

const int NPC_REACTION_COUNT_MAX = 10;

class CReputationSystemRepository;

// Handle reaction based on affinity between player character and NPC in village.
class CDnNpcReaction
{
private:
	struct S_NPC_REACTION_INFO
	{
		int iNpcID;
		int aiFavorThresholdPercent[ NPC_REACTION_COUNT_MAX ];
		int aiSpeechUIStringID[ NPC_REACTION_COUNT_MAX ];
		string astrEffectActionName[ NPC_REACTION_COUNT_MAX ];


		S_NPC_REACTION_INFO( void ) : iNpcID( 0 )
		{
			SecureZeroMemory( aiFavorThresholdPercent, sizeof(aiFavorThresholdPercent) );
			SecureZeroMemory( aiSpeechUIStringID, sizeof(aiSpeechUIStringID) );
		};
	};

	// Whether initialized
	bool m_bInitialized;

	// Pointer to NPC affinity storage object passed down from VillageTask
	CReputationSystemRepository* m_pReputationRepos;
	map<int, S_NPC_REACTION_INFO> m_mapNpcReaction;
	set<DWORD> m_setNpcsInRange;

	// Store the action name corresponding to the index when an NPC effect request arrives from a server script.
	vector<string> m_vlSCNpcEffectActionName;

public:
	CDnNpcReaction( void );
	virtual ~CDnNpcReaction( void );

	void Initialize( CReputationSystemRepository* pReputationRepos );
	bool IsInitialized( void ) { return m_bInitialized; };
	void Clear( void );
	void AttachNpcEffect( UINT uiNpcID, int iEffectdIndex );
	
	void Process( LOCAL_TIME LocalTime, float fDelta );

};

#endif // #ifdef PRE_ADD_NPC_REPUTATION_SYSTEM