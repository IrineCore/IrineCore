#pragma once
#include "DnCustomDlg.h"
#include "DnGaugeFace.h"
#include "DnSkill.h"
#include "DnPartyTask.h"
#include "DnBuffUIMng.h"

class CDnGaugeFace;
class CDnBuffUIMng;
#ifdef PRE_ADD_VIP
class CDnPopupVIPDlg;
#endif
#ifdef PRE_MOD_NESTREBIRTH
class CDnNestRebirthTooltipDlg;
#endif
class CDnJobIconStatic;

#ifdef PRE_WORLDCOMBINE_PARTY
#define GAUGE_PLAYERNAME_NO_COLOR -1
#endif

class CDnGaugeDlg : public CDnCustomDlg
{
public:
	CDnGaugeDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL  );
	virtual ~CDnGaugeDlg(void);

protected:
	CDnGaugeFace *m_pFace;
	CEtUIStatic *m_pStaticTextHP;
	CEtUIProgressBar *m_pHPBar;
	CEtUIProgressBar *m_pSPBar;		// MP or SuperArmor
	CEtUIStatic *m_pPlayerName;
	CEtUIStatic* m_pPlayerNameInParty;
	CEtUIStatic *m_pStaticHP;
	CEtUIStatic *m_pStaticSP;
	CEtUIStatic *m_pStaticGateReady;
	CEtUIStatic *m_pStaticLoading;
	CEtUIStatic *m_pStaticDamage;

	CEtUIStatic *m_pCoinFree;
	CEtUIStatic *m_pCoinCash;
	CEtUIStatic *m_pRebirthLimitBack;
#ifdef PRE_MOD_NESTREBIRTH
	CEtUIStatic *m_pRebirthLimitCoinCount;
	CEtUIStatic *m_pRebirthLimitItemCount;
	CEtUIStatic *m_pRebirthLimitCoinIcon;
	CEtUIStatic *m_pRebirthLimitItemIcon;

	CDnNestRebirthTooltipDlg* m_pNestRebirthTooltipDlg;
#endif
	CEtUIStatic *m_pRebirthLimit;

	CEtUIStatic *m_pPartyMaster;
#ifdef PRE_ADD_IMPROVE_MASTER_OBSERVER
	CEtUIStatic *m_pPartySelection;
#endif
#ifdef PRE_ADD_PVP_SHOW_PARTY_GAUGE
	CEtUIStatic *m_pStaticShowPvpGauge;
	CEtUICheckBox *m_pCheckShowPvpGauge;
#endif

	CEtUIStatic* m_pStaticPartyLootInfo;
	CEtUIButton* m_pButtonPartyQuit;
	CEtUIStatic* m_pStaticPartyItemLootRank;

	CEtUIStatic *m_pStaticItemRepair0;
	CEtUIStatic *m_pStaticItemRepair1;
	CEtUIStatic *m_pStaticItemRepair2;

	CEtUIStatic* m_pStaticVIPExpire;
	CEtUIStatic* m_pStaticVIPNormal;

	CEtUIStatic *m_pKeepPartyIcon;
	CEtUIStatic *m_pKeepMaster;
	CEtUIStatic *m_pKeepClassmate;
	enum eIconIndex
	{
		eParty_Icon = 0,
		eRepair_Icon,
		eMaster_Icon,
		eClassmate_Icon,
		eBuff_Icon1,
		eBuff_Icon2,
		eBuff_Icon3,
		eBuff_Icon4,
		eBuff_Icon5,
		eBuff_Icon6,
		eBuff_Icon7,

		eMax_Icon,
	};

	enum eVoiceState
	{
		VoiceNone = 0,
		VoiceReady = 1,
		VoiceActive = 2,
		VoiceMute = 3,

		NUM_VOICE_STATE = 4,
	};

	CEtUIButton *m_pButtonVoice[NUM_VOICE_STATE];
	CEtUIButton *m_pButtonComplaint;

	UINT m_nSessionID;
	std::wstring m_wszPlayerName;
	DWORD m_dwDefaultColor;
	GAUGE_FACE_TYPE m_FaceType;
	bool m_bDamage;
	float m_fFaceTime;

	int m_nPrevUsableCoinCount;
#ifdef PRE_MOD_NESTREBIRTH
	int m_nPrevUsableRebirthItemCount;
#endif

	CEtUIStatic*			m_pBuffCounter[MAX_BUFF_SLOT];
	CEtUITextureControl*	m_pBuffTextureCtrl[MAX_BUFF_SLOT];
	CEtUITextureControl*	m_pBubbleCoolTimeTextureCtrl[MAX_BUFF_SLOT];

	EtTextureHandle			m_hSkillIconList[BUFF_TEXTURE_COUNT];

	int m_nMonsterGrade;
	int m_nStageConstructionLevel;	// Stage difficulty
	bool m_bRefreshStaticTextHP;
	int m_nPrevLayerCount;

	bool m_bLifeConditionDlg;

	int					m_pIconPosition[eMax_Icon];
	SUICoord			m_pIconCoord[eMax_Icon];

	void RefreshRebirth();

	void RefreshIcon();
	int GetSeparateCount();

public:

	enum eGagueMode
	{
		Normal = 0,
		Percentage = 1,
		Protected = 2,
	};

	enum eProgressBarType
	{
		HPBar = 0,
		SPBar = 1,
	};

	void SetFaceID( int nID );
	int GetFaceID();
	void SetFaceType( GAUGE_FACE_TYPE Type );
	void UpdatePortrait( EtObjectHandle hHandle, bool bDelete, bool bBoss = false, int nAniIndex = 0, float fFrame = 0.0f );
	bool IsUpdatePortrait();	
	void BackupPortrait();
	void SetHP( float fHP );
	void SetSP( float fSP );
	void SetHP( INT64 nCurHP, INT64 nMaxHP , eGagueMode eType = eGagueMode::Normal );
	void SetSP( int nCurSP, int nMaxSP , eGagueMode eType = eGagueMode::Normal );
#ifdef PRE_WORLDCOMBINE_PARTY
	void SetPlayerName(int nLevel, LPCWSTR pwszName, DWORD dwColor, int nServerID);
	bool IsPlayerNameWithServerName(int nServerID) const;
#else
	void SetPlayerName( int nLevel, LPCWSTR pwszName, DWORD dwColor = -1 );
#endif
	void SetSessionID( UINT nSessionID ) { m_nSessionID = nSessionID; }
	UINT GetSessionID(){ return m_nSessionID; }
	void UpdateRebirthCoin();
	void ProcessBuffs(DnActorHandle hActor);
	void ProcessBuffList( CDnBuffUIMng::BUFFLIST& buffSlotList , int &slotIdx , bool bUseTooltip = false );
	void ProcessBubbleList( CDnBuffUIMng::BUBBLELIST& bubbleSlotList , int &slotIdx );

#ifdef PRE_ADD_NAMEDITEM_SYSTEM
	void ProcessVillagePartyBuff();
	void AddVillagePartyBuffTexture( int nControIndex, int nItemID, int nSkillID, float fRemainTime, bool bEternity );
#endif

	void RefreshDurabilityIcon();
	void ShowPartyLootInfo(bool bShow, const wchar_t* pInfoString);
	void ShowPartyQuitButton(bool bShow);
	void ShowPartyItemLootRank(bool bShow, const wchar_t* pInfoString);
	void ShowGateReady( bool bShow );
	void ShowLoading( bool bShow );
	void ApplyBuffToolTip( int nSkilID, int nSkillLevelID , float fDuration  , CEtUITextureControl* pBuffTextureCtrl );

#ifdef _USE_VOICECHAT
	void UpdateVoiceButtonMode(bool bVoiceAvailable, bool bMute = false);
	void UpdateVoiceSpeaking(bool bSpeak);
	void HideVoiceButtonMode();
	void SetToggleMicHotKey( BYTE cVK );
#endif

	EtTextureHandle GetPortraitTexture();

	bool Empty();
	void SetEmpty();
	void OnDamage();
	void SetImmediateChange( bool bHP );
	
	void Initialize( const char *pFileName, GAUGE_FACE_TYPE FaceType, bool bShow );

	virtual void InitialUpdate();
	virtual void Show( bool bShow );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );
	virtual void Process( float fElapsedTime );
	virtual void Render( float fElapsedTime );

	void SetPartyMaster( bool bMaster );
	void SetProgressIndex( int nType = eProgressBarType::HPBar , int nIndex = 0, bool bUse = false );
	void SetMonsterGrade( int nMonsterGrade );
#ifdef PRE_ADD_IMPROVE_MASTER_OBSERVER
	void SetPartySelection( bool bSelect );
#endif
#ifdef PRE_ADD_PVP_SHOW_PARTY_GAUGE
	void ShowPvpPartyGauge(bool bShow);
	bool CheckShowPvpPartyGauge();
#endif

#ifdef PRE_ADD_VIP
	CDnPopupVIPDlg*	m_pPopupVIPDlg;
	void			RefreshVIP();
	void			ShowPopupVIPDlg(bool bShow, float fX, float fY);
#endif // PRE_ADD_VIP

	virtual bool	MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
#ifdef PRE_ADD_BEGINNERGUILD
	void UpdateKeepPartyInfo( int nKeepPartyRate, int nBestFriendRate, int nBeginnerGuildBonusRate );
#else
	void UpdateKeepPartyInfo( int nKeepPartyRate, int nBestFriendRate );
#endif
	void ShowPartyContextMenu(bool bShow, float fX, float fY, const CDnPartyTask::PartyStruct& memberInfo);

	void UpdateKeepMasterInfo( int nMasterCount, int nPupilCount, int nClassmateCount );
	void ResetKeepMasterInfo();
	int SetKeepMasterTooltip( std::wstring& strText, int nExpBonusRate, int nMemberCount, int nStrIndex );
	int GetMasterFavorBonusExp( int nFavorPoint );

	void InitializeFarm();
	void FinalizeFarm();
	void ToggleLifeCondition();
	bool IsLifeCondition()	{ return m_bLifeConditionDlg; }
	const std::wstring& GetName() const { return m_wszPlayerName; }

	void RenderBubbleRemainTime(float fElapsedTime);
	bool IsShowGateReady();

private:
	CDnBuffUIMng*	m_pBuffUIMng;
	std::wstring			m_strMasterBonusText;
#ifdef PRE_MOD_NESTREBIRTH
	void ShowNestRebirthTooltipDlg(float fMouseX, float fMouseY);
	void CloseNestRebirthTooltipDlg();
#endif
	
};
