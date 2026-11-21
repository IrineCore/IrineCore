#pragma once

#include "Task.h"
#include "MessageListener.h"


class CDnItem;
class CDnQuickSlotButton;


struct stPetOption
{
	bool	m_bGetItem;
	bool	m_bGetNormalItem;
	bool	m_bGetMagicItem;
	bool	m_bGetRareItem;
	bool	m_bGetEpicItem;
	bool	m_bGetUniqueItem;
	bool	m_bGetLegendItem;
//	bool	m_bAutoUseHPPotion;
//	bool	m_bAutoUseMPPotion;
//	int		m_nAutoUseHPPotionPercent;
//	int		m_nAutoUseMPPotionPercent;
	bool	m_bAutoFeed;
	int		m_nAutoFeedPercent;
	int		m_nFoodItemID;

	stPetOption()
	: m_bGetItem( false )
	, m_bGetNormalItem( false )
	, m_bGetMagicItem( false )
	, m_bGetRareItem( false )
	, m_bGetEpicItem( false )
	, m_bGetUniqueItem( false )
	, m_bGetLegendItem( false )
//	, m_bAutoUseHPPotion( false )
//	, m_bAutoUseMPPotion( false )
//	, m_nAutoUseHPPotionPercent( 0 )
//	, m_nAutoUseMPPotionPercent( 0 )
	, m_bAutoFeed( false )
	, m_nAutoFeedPercent( 0 )
	, m_nFoodItemID( 0 )
	{} 
};

enum ePetChatType
{
	PET_CHAT_NORMAL = 0,
	PET_CHAT_EAT,
	PET_CHAT_EATFAIL,
	PET_CHAT_STARVE,
	PET_CHAT_SATIETY_ZERO,
	PET_CHAT_LEVELUP,
	PET_CHAT_USER_LEVEL_UP,
	PET_CHAT_USER_DIE,
	PET_CHAT_USER_CREATE_GUILD,
	PET_CHAT_USER_JOIN_GUILD,
	PET_CHAT_USER_ENCHANT_SUCCESS,
	PET_CHAT_USER_ENCHANT_FAIL,
};

class CDnPetTask : public CTask, public CTaskListener, public CEtUICallback, public CSingleton<CDnPetTask>
{
public:
	CDnPetTask();
	virtual ~CDnPetTask();

	bool Initialize();
	virtual void OnDispatchMessage( int nMainCmd, int nSubCmd, char* pData, int nSize );
	virtual void OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg = 0 );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );

public:
	void OnRecvPetSummon( int nSessionID, int nPetItemID, LPCWSTR strPetName, bool bSummonAni = false );
	void OnRecvPetSummonedOff( int nSessionID, bool bSummonAni = false );
	void OnRecvPetNaming( SCChangePetName* pPacket );
	void OnRecvPetEquipList( CDnPlayerActor* pPlayer, TVehicleCompact& stPetEquipInfo );

	void UsePetItem( CDnItem* pItem, bool bCallGestureQuickSlot = false );
	stPetOption GetPetOption() { return m_stPetOption; }
	void LoadPetConfig();
	void SetPetOption( stPetOption stPetOptionData );
	bool CheckCanMovePet( bool bSummonedOff );
	void RequestPetNaming(INT64 itemSerial, INT64 petSerial, const wchar_t* pName);
	void SendPetExtendPeriod( INT64 iExtendPeriodItemSerial, INT64 iItemSerial, int nExtendPeriod );
	int GetPetLevelTableIndex( int nSessionID );
	int GetPetMaxLevelExp( int nPetItemID );
	int GetPetLevel( INT64 nSerialID );
	bool IsPetSkill( int nSkillTableID );
	void OnRecvPetSkillItem( SCPetSkillItem* pPacket );
	void OnRecvPetSkillExpand( SCPetSkillExpand* pPacket );
	void RequestRemovePetSkill( int nSlotIndex, CDnSkill* pSkillItem );
	void OnRecvRemovePetSkillItem( SCPetSkillDelete* pPacket );
	bool CheckRemovePetSkill( int nSkillID );
	void OnRecvPetExtendPeriod( SCModItemExpireDate* pPacket );
	bool IsLoadPetConfig() { return m_bLoadPetConfig; }
	void OnRecvCurrentSatiety( SCPetCurrentSatiety* pPacket );
	void OnRecvPetFoodEat( SCPetFoodEat* pPacket );
	void SetPetSatietyInfo( int nPetItemID, int nCurrentSatiety );
	float GetSatietyApplyStateRatio();
	bool EnableSatietyPickup();
	int GetSatietySelection();
	void CheckSatiety();
	void DoPetChat( ePetChatType chatType );
	void DoNotPetChat( ePetChatType chatType );
	void CheckPetChat( float fDelta );
	void CheckAutoFeeding();
	void ResetPetSatietyInfo();
	int GetPetFoodItemCount( int nPetFoodItemID );
	int GetCurrentSatiety() { return m_nCurrentSatiety; }
	int GetMaxSatiety() { return m_nMaxSatiety; }
	float GetSatietyPercent() { return m_fSatietyPercent; }

protected:
	void RemovePetSkill( int nSkillID, bool bForceRemove = false );

public:
	enum 
	{
		PET_SUMMON_PROGRESS = 0,
		MSGBOX_REQUEST_REMOVE_PETSKILL,
	};

protected:
	CDnItem*			m_pItem;

	stPetOption			m_stPetOption;
	bool				m_bIsNowSummonPet;
	int					m_nRemoveSlotIndex;
	struct stRemoveSkillInfo
	{
		DnSkillHandle	hSkillHandle;
		int				nSessionID;
		bool			bForceRemove;
	};
	std::map<int, stRemoveSkillInfo>	m_mapRemoveSkillQueue;

	struct stPetChatProb
	{
		int nTableID;
		float fProb;
	};

	struct stPetChat
	{
		float		fChatCoolTime;
		__time64_t	tStartTime;

		std::vector<stPetChatProb>	vecPetChatProb;
		~stPetChat() { vecPetChatProb.clear(); }
	};

	std::map<int, stPetChat>	m_mapPetChat;

	bool				m_bLoadPetConfig;		// Must read even before pet summon due to pet food
	bool				m_bNotifySatietyHalf;	// Whether to output system message below 50% satiety
	bool				m_bNotifySatietyEnd;	// Whether to output system message at 0% satiety
	bool				m_bRequestUsePetFood;	// Send pet food use request (re-check pet food after receiving response)
	float				m_fCheckCoolTime;
	int					m_nChatID;
	int					m_nCurrentSatiety;
	int					m_nMaxSatiety;
	float				m_fSatietyPercent;
	int					m_nSatietySelectionPickup[Pet::MAX_SATIETY_SELECTION];
	int					m_nSatietySelectionApplyStatePercent[Pet::MAX_SATIETY_SELECTION];
	int					m_nSatietySelectionUseSkillNum[Pet::MAX_SATIETY_SELECTION];
};

#define GetPetTask()	CDnPetTask::GetInstance()
