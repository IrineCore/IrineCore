#pragma once
#include "DnCustomDlg.h"
#include "DnBlindDlg.h"
#include "DnCustomControlCommon.h"
#include "DnGameTask.h"
#include "DnDungeonClearImp.h"

class CDnStageClearCounterMng;
class CDnStageClearRewardGoldDlg;
class CDnStageClearReportDlg : public CDnCustomDlg, public CDnBlindCallBack
{
public:
	CDnStageClearReportDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL  );
	virtual ~CDnStageClearReportDlg(void);

	struct SoundType 
	{
		enum 
		{
			RESULT_SHOW = 0,		//  When the settlement window appears 
			GRADE_RESULT,			//  Sound when evaluating rank

			MAX
		};
	};

	enum eBonusType
	{
		BONUS_COMBO,
		BONUS_KILL,
		BONUS_TIME,
		BONUS_PARTY,
//#ifdef PRE_ADD_ACTIVEMISSION
//		BONUS_ACTIVEMISSION,
//#endif // PRE_ADD_ACTIVEMISSION
		BONUS_MAX
	};

	enum ePhaseState
	{
		PHASE_NONE,
		PHASE_START,
		PHASE01_GAMERESULT,
		PHASE01_VALUE_START,
		PHASE01_VALUE_PROCESS,
		PHASE02_POINT_START,
		PHASE02_POINT_PROCESS,
		PHASE02_EXP_START,
		PHASE02_EXP_PROCESS,
		PHASE03_START,
		PHASE03_RANK_START,
		PHASE03_RANK_PROCESS,
		PHASE03_RANK_BONUS_COUNT_START,
		PHASE03_RANK_BONUS_COUNT_PROCESS,
		PHASE_FINISH,
	};

protected:
	CEtUIStatic *m_pStaticBase1;
	CEtUIStatic *m_pStaticBase2;

	CEtUIStatic *m_pStaticMaxCombo;
	CEtUIStatic *m_pMaxCombo;
	CEtUIStatic *m_pStaticComboBonus;
	CEtUIStatic *m_pComboBonus;

	CEtUIStatic *m_pStaticKillNumber;
	CEtUIStatic *m_pKillNumber;
	CEtUIStatic *m_pStaticKillBonus;
	CEtUIStatic *m_pKillBonus;

	CEtUIStatic *m_pStaticClearTime;
	CEtUIStatic *m_pClearTime;
	CEtUIStatic *m_pStaticTimeBonus;
	CEtUIStatic *m_pTimeBonus;

	CEtUIStatic *m_pStaticPartyNumber;
	CEtUIStatic *m_pPartyNumber;
	CEtUIStatic *m_pStaticPartyBonus;
	CEtUIStatic *m_pPartyBonus;

	CEtUIStatic *m_pStaticPoint;
	CEtUIStatic *m_pPoint;
	CEtUIStatic *m_pStaticEXP;
	CEtUIStatic *m_pEXP;
	CEtUIStatic *m_pStaticChannelBonus;
	CEtUIStatic *m_pStatic2ndPwdBonus;
	CEtUIStatic *m_pStaticEKeyBonus;
	CEtUIStatic *m_pStaticMaxLevelCharBonus;
	CEtUIStatic *m_pStaticMaxLevelCharCount;

	CEtUIStatic *m_pStaticRankCheck;
	CEtUIStatic *m_pStaticRank;
	CEtUIStatic *m_pStaticOpenDungeonName;
	CEtUIStatic *m_pStaticOpenText;
	CEtUIStatic *m_pStaticOpenLevel;
	CEtUIStatic *m_pStaticOpenBase;
	CEtUITextureControl *m_pTextureRank;
	CEtUITextureControl *m_pTextureRankAfterImage;
	CEtUITextureControl *m_pTextureOpenDungeon;

	EtTextureHandle m_hOpenDungeon;
	EtTextureHandle m_hRankTexture;

	CEtUIStatic *m_pStaticExpGauge;
	CEtUIStatic *m_pStaticExpGaugeValue;
	CEtUIProgressBar *m_pProgressExpGauge[5];

//#ifdef PRE_ADD_ACTIVEMISSION
//	CEtUIStatic * m_pStaticActiveMissionNumber;
//	CEtUIStatic * m_pStaticActiveMissionBonus;	
//	CEtUIStatic * m_pStaticActiveMissionResult;
//#endif // PRE_ADD_ACTIVEMISSION

	CDnDungeonClearImp::SDungeonClearInfo *m_pPlayerDungeonClearInfo;
	CDnDungeonClearImp::SDungeonClearValue *m_pDungeonClearValue;

	float m_fPhaseTime;
	ePhaseState		m_CurPhase;
	bool			m_bRollCounter;
	float			m_PhaseTimer;
	float			m_RankCheckTimer;
	float			m_RankAfterImangeTimer;
	float			m_Phase01PreTimer;
	float			m_ChannelBonusTimer;
	float			m_MaxLevelCharCountTimer;
	SUICoord		m_BonusStaticPos[BONUS_MAX];
	SUICoord		m_BonusPos[BONUS_MAX];
	bool			m_bStartBonusSliding[BONUS_MAX];
	float			m_BonusSlidingSpeed;
	const float		m_BonusSlidingTime;
	int				m_BonusSlidingIdx;
	float			m_RankAccel;
	float			m_CheckAccel;
	SUICoord		m_RankCheckOrgCoord;
	bool			m_bRankNextStart;

	const float		m_RankShowTime;
	const float		m_RankInitScale;
	const float		m_CheckShowTime;
	const float		m_CheckInitScale;

	const float		m_RankAfterShowTime;
	const float		m_RankAfterLastScale;
	const float		m_RankAfterInitAlpha;

	CDnStageClearCounterMng*	m_PointMng;
	CDnStageClearCounterMng*	m_ExpMng;
	CDnStageClearRewardGoldDlg*	m_pRewardGoldDlg;

	int m_nMapIndex;
	int m_nMapLevel;

	// Sound / BGM when showing this window
	int	m_nSoundIndexArray[SoundType::MAX];
	EtSoundChannelHandle m_hBgmSound;

	DWORD m_dwStartExp;
	DWORD m_dwGainExp;
	float m_fExpGaugeDelta;
	
	bool m_bIgnoreRewardItem;

protected:
	void SetStageClearInfo();
	void SetOpenDungeon( int nMapIndex, int nMapLevel );

	void HideAllControl();

	void InitPhase_Point();
	void InitPhase_Exp(bool bWithSign, bool bCounting, int initExp, int targetExp);
	void InitPhase_RankChecker();
	void InitPhase_Rank();
	void InitPhase_ChannelBonus();
#if defined(PRE_ADD_WORLD_EVENT)
	void InitPhase_EventBouns(WorldEvent::eDetailEventType type);
#else
	void InitPhase_EventBouns(eEventType3 type);
#endif //#if defined(PRE_ADD_WORLD_EVENT)
	void InitPhase_PromotionBonus();

	void OnClose();
	void ShowPhase_GameResultStatic();
	void ShowPhase_GameResult();
	void ShowPhase_ValueStatic();
	void ShowPhase_Value();
	bool ShowPhase_Point(float fElapsedTime);
	bool ShowPhase_Exp(float fElapsedTime);
	bool ShowPhase_Rank(float fElapsedTime);
	bool ShowPhase_RankChecker(float fElapsedTime);
	void ShowPhase_ChannelBonus(float fElapsedTime);
	void ShowPhase_PromotionBonus(float fElapsedTime);

	void RefreshBonusAlarmCoord();
	bool ProcessPhase01(float fElapsedTime);
	bool ProcessBonus(float fElapsedTime, CEtUIStatic* bonusStatic, float targetX);
	void SetCoords(CEtUIStatic* uiStatic, float fX, float fY, bool bAdd);
	void GetUICtrlCenter(EtVector2& result, CEtUIControl* ctrl, float scale);
	void Render( float fElapsedTime );

	void UpdateExpGauge( float fElapsedTime );
	void CalcUpdateLevelRange( int nExp, int &nCurExp, int &nMaxExp );

public:
	void SetOpenDungeonInfo( int nIndex, int nDifficult );

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void Show( bool bShow );
	virtual void Process( float fElapsedTime );

public:
	virtual void OnBlindOpen();
	virtual void OnBlindOpened();
	virtual void OnBlindClosed();
};
