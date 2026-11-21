#pragma once

#include "DnActor.h"
#include "MAWalkMovement.h"
#include "MASingleBody.h"

class CEtWorldEventArea;

class CDnNPCActor : public CDnActor, public MASingleBody
{
public:
	enum _StateMark
	{
		MarkTypeNone = -1,
		/*
Available (acceptable) - exclamation mark
Playing - In progress - question mark (gray)
Recompense reward - question mark

The number at the end is the count of exclamation (question) marks
Main -> Orange
Sub -> Blue
		*/

		QuestAvailable_Main = 1,
		QuestPlaying_Main = 2,
		QuestRecompense_Main = 3,

		QuestAvailable_Sub = 10,
		QuestPlaying_Sub = 11,
		QuestRecompense_Sub = 12,

		CrossFavorGrant    =  15,
		CrossFavorNow      =  16,
		CrossFavorComplete =  17,

		CrossReQuestGrant = 19,
		CrossReQuestNow = 20,
		CrossReQuestComplete = 21,

		MailReceiveNew = 22,

		CrossQuestGlobalGrant = 23,
		CrossQuestGlobalNow = 24, 
		CrossQuestGlobalComplete = 25,

		CrossNoticeWarpNpc = 26, // Wants to force a speech balloon to pop up on the warp NPC.
		CrossNoticePcBang = 27,  // PcBang notification
		CrossNoticeRepair = 28,  // Repair notification
		//added new!
		CrossNoticePresent1 = 29,  // Repair notification
		CrossNoticePresent2 = 30,  // Repair notification



		MaxCount,
	}; // These types are related to quest priority and progress. If this part changes, please also update GetQuestStateFromMark / CalcStateMarkRank

	enum emJobType
	{
		typeNone,			// Nothing in particular
		typeWeapon,			// Weapon Merchant
		typeParts,			// Armor merchant
		typeNormal,			// General goods merchant
		typeStorage,		// Warehouse
		typeMarket,			// Unmanned Trade Shop
		typeMail,			// Mailbox
		typeGuild,			// Guild management
		typeWarriorSkill,	// Warrior skill
		typeArcherSkill,	// Archer skill
		typeSoceressSkill,	// Sorceress skill
		typeClericSkill,	// Cleric skill
		typePlate,	        // Crest Jewel
		typeCompleteQuest,  // NPC dedicated to Adventurer's Board quest rewards
		typeAdventureBoard, // Normal adventurer board type
		typeVehicle,        // Mount trainer
		typeTicket,         // Airship ticket NPC
		typeAcademicSkill,  // Academic skill
		typeKaliSkill,      // Kali skill
		typePcBang,         // PcBang 
		typeRepair,         // Repair NPC
		typePresent,
		typeEvent,
		typeAmount,	        // Total count
	}; // If this is added and icons are added to minimap/zone map, find and configure the related code.

	enum eSoundType
	{
		BeginTalk = 0, 
		FinishTalk,
		OpenShop,
		CloseShop
	};

	struct NpcData 
	{
		UINT nNpcID;
		int nParam[2];
		std::wstring wszNpcName;
		std::wstring wszTalkFileName;

		std::vector<int>	BeginTalkSound;
		std::vector<int>	FinishTalkSound;
		std::vector<int>	OpenShopSound;
		std::vector<int>	CloseShopSound;
		EtSoundChannelHandle	m_hCurPlaySound;
		NpcData():nNpcID(0xffffffff) { memset(nParam, 0, sizeof(nParam)); }
	};

	CDnNPCActor( int nClassID, bool bProcess = true );
	virtual ~CDnNPCActor();

#ifdef PRE_FIX_GAMESERVER_OPTIMIZE
	virtual bool Initialize();
#endif
	virtual bool IsNpcActor(){ return true; }

	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void Initialize(NpcData& data);
	virtual void RenderCustom( float fElapsedTime );	
	virtual bool LoadSkin( const char *szSkinName, const char *szAniName, bool bTwoPassAlpha = true );

	virtual MAMovementBase* CreateMovement()
	{
		MAMovementBase* pMovement = new MAWalkMovement();
		return pMovement;
	}

	// ActionBase
	virtual void OnSignal( SignalTypeEnum Type, void *pPtr, LOCAL_TIME LocalTime, LOCAL_TIME SignalStartTime, LOCAL_TIME SignalEndTime, int nSignalIndex );
	virtual void OnChangeAction( const char *szPrevAction );
	virtual void OnChangeActionQueue( const char *szPrevAction );

	// TransAction Message
	virtual void OnDispatchMessage( DWORD dwActorProtocol, BYTE *pPacket );

	
protected:

private:
	EtTextureHandle m_hStateMarkTex[ MaxCount ];
	NpcData m_NpcData;
	SOBB m_SafeZone;
	bool m_bExistSafeZone;
	
	int m_nStateMark;
	LOCAL_TIME m_nLastUpdateTime;

	emJobType m_emNPCJobType;
	bool m_bEnableOperator;
	DnPropHandle m_hPropHandle;
	std::string m_ForceNextActionName;

	EtSoundChannelHandle m_hScriptSoundChannel;

	bool m_bNpcMoveActive;
	EtVector3 m_vNpcMoveTarget;
	float m_fNpcMoveSpeed;

public:

	static CDnNPCActor* FindNpcActorFromID( int nNpcID );
	static void GetAvailableQuestList(std::vector<int> &vecAvailableQuestList , int nNpcID );
	static void GetAvailableQuestName(std::vector<std::wstring> &vecAvailableQuestName , int nNpcID );

	void RefreshSafeZone();
	void UpdateStateMark();
	void OnSoundPlay(eSoundType type);

	emJobType GetNPCJobType() { return m_emNPCJobType; }
	void SetNPCJobType( emJobType jobType ) { m_emNPCJobType = jobType; }
	void SetPropHandle( DnPropHandle hProp ) { m_hPropHandle = hProp; }
	DnPropHandle GetPropHandle() { return m_hPropHandle; }

	void EnableOperator( bool bEnable ) { m_bEnableOperator = bEnable; }
	bool IsEnableOperator() { return m_bEnableOperator; }

	bool CanTalk();
	bool IsNpcProp() { return ( m_hPropHandle ) ? true : false; }

	NpcData& GetNpcData() { return m_NpcData; }
	
	int GetNPCStateMark() { return m_nStateMark; }
	static int CalcStateMark( int nNpcID );
	static int GetQuestStateFromMark( int nStateMark ); // Use state marks to indicate whether a quest is in progress, available, or completable
	// 0 = Acceptable, 1 = In progress, 2 = Completed: The only way to know if the current state is completed is via the state mark.

	static int CalcStateMarkRank ( int nStateMark );    // Score priority based on the quest mark.

	virtual void OnFinishAction(const char* szPrevAction, const char *szNextAction, LOCAL_TIME time);
	void SetForceActionOnFinishCurrentAction(const std::string& actionName);

	void SetScriptSoundChannel( EtSoundChannelHandle hScriptSoundChannel ) { m_hScriptSoundChannel = hScriptSoundChannel; }
	void RemoveScriptSoundChannel() { CEtSoundEngine::GetInstance().RemoveChannel( m_hScriptSoundChannel ); }
};

DnActorHandle CreateNpcActor(UINT uUniqueID, UINT nNpcID, EtVector3 Position, float fRotate);