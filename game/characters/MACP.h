#pragma once

class MACP {
public:
	MACP();
	virtual ~MACP();

	enum CPTypeEnum {
		MaxComboCount,
		KillBossCount,
		KillMonsterScore,
		CriticalHitScore,
		StunHitScore,
		SuperAmmorBreakScore,
		BrokenShieldScore,
		GenocideScore,
		AirComboScore,
		PartyComboScore,
		RebirthPlayerScore,
		UseSkillScore,
		ComboScore,
		AttackedCount,
		AttackedHitScore,
		AttackedCriticalHitScore,
		AttackedStunHitScore,
		DieCount,
		DeadScore,
		PropBreakScore,
		GetItemScore,
		AssistMonsterScore,
		CPType_Max
	};

	typedef struct tagMACP
	{
		int nAccumulationCP;

		// Values preloaded from tables are shared with the server, so calling Get anytime is problematic. Could hold them in GameTask, though retrieving them is awkward.
		int pScore[CPTypeEnum::CPType_Max];

		// Items added for mission use, though not directly used in current cp
		int nPartyMemberDieCount;
		int nStartPartyCount;

		// When at max level, there was nowhere else to put EXP gain handling, so put it here.
		int nMaxLevelGainExperience;

		// Stage clear EXP
		int nCompleteExperience;
	}SMACP;

protected:
	
	CDnActor *m_pActor;
	SMACP m_Data;

	int m_nCPTableID;

protected:
	virtual void OnEventCP( CPTypeEnum Type, int nResult );
	bool SetCPTableID( const int nMapIndex, const int nClassID );

public:
	virtual void ResetCP();
	virtual bool InitializeCPScore( const int nMapIndex, const int nClassID, const int nDungeonClearID );
	bool Initialize( CDnActor *pActor );

	void UpdateMaxCombo( int nValue );
	void UpdateKillBoss();

	virtual bool UpdateKillMonster( int *pResult = NULL );
	bool UpdateCriticalHit( int *pResult = NULL );
	bool UpdateStunHit( int *pResult = NULL );
	bool UpdateSuperAmmorBreak( int *pResult = NULL );
	bool UpdateBrokenShield( int *pResult = NULL );
	virtual bool UpdateGenocide( int *pResult = NULL );
	bool UpdateAirCombo( int *pResult = NULL );
	virtual bool UpdatePartyCombo( int nCombo, int *pResult = NULL );
	virtual bool UpdateRebirthPlayer( int *pResult = NULL );
	virtual bool UpdateUseSkill( DnSkillHandle hSkill, int *pResult = NULL );
	virtual bool UpdateCombo( int nCombo, int *pResult = NULL );

	bool UpdateAttackedHit( int *pResult = NULL );
	bool UpdateAttackedCriticalHit( int *pResult = NULL );
	bool UpdateAttackedStunHit( int *pResult = NULL );
	bool UpdateDead( int *pResult = NULL );

	// For adding missions
	bool UpdatePartyMemberDead( int *pResult = NULL );
	bool UpdateStartPartyCount( int nPartyCount );

	int GetMaxComboCount() { return m_Data.pScore[CPTypeEnum::MaxComboCount]; }
	int GetKillBossCount() { return m_Data.pScore[CPTypeEnum::KillBossCount]; }
	int GetAccumulationCP() { return m_Data.nAccumulationCP; }
	int GetDieCount() { return m_Data.pScore[CPTypeEnum::DieCount]; }
	int GetAttackedCount() { return m_Data.pScore[CPTypeEnum::AttackedCount]; }

	// For adding missions
	int GetPartyMemberDieCount() { return m_Data.nPartyMemberDieCount; }
	int GetStartPartyCount() { return m_Data.nStartPartyCount; }

	virtual bool UpdatePropBreak( int* pResult = NULL );
	virtual bool UpdateGetItem( int* pResult = NULL );

	// Made for cheats.. should never be used to set values normally.
	void SetAccumulationCP( int nValue ) { m_Data.nAccumulationCP = nValue; }

	bool UpdateAssistScore( int myAggro, int wholeAggro );

	bool UpdateMaxLevelGainExperience( int nExp );
	int GetMaxLevelGainExperience() { return m_Data.nMaxLevelGainExperience; }

	inline void AddCompleteExperience( int nValue ) { m_Data.nCompleteExperience += nValue; };
	inline int GetCompleteExperience() { return m_Data.nCompleteExperience; };

	const SMACP & GetCP_Data() { return m_Data; }
};