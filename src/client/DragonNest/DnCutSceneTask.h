#pragma once
#include "Task.h"
//#include "IDnCutSceneDataReader.h"
#include "DnCutScenePlayer.h"
#include "InputReceiver.h"
#include "DnWorldActProp.h"

class IDnCutSceneDataReader;
class ICustomActorProcessor;


#include "DnLoadingTask.h"

class CDnFadeInOutDlg;
// Real-time video related Task - written by Hangi
class CDnCutSceneTask : public CTask, public CInputReceiver, public CDnLoadingStateMachine
{
protected:
	struct S_ACT_PROP_DEFAULT
	{
		DnPropHandle hProp;
		CDnWorldActProp* pActProp;
		string strDefaultAction;
		bool bShow;

		S_ACT_PROP_DEFAULT( void ) : pActProp( NULL ), bShow( true ) {}
	};


	IDnCutSceneDataReader*	m_pCutSceneData;
	
	ICustomActorProcessor*	m_pActorProcessor;
	CDnCutScenePlayer		m_CutScenePlayer;
	
#if defined(TEST_CUTSCENE)
	bool					m_bFirstLoop;
#endif

	deque<DnActorHandle>	m_dqhLiveActors;
	vector<MatrixEx>	m_vlOriMatExWorlds;
	vector<std::string>		m_szOriAction;
	vector<bool>			m_bVecPlayerBattleMode;
	vector<DnActorHandle>	m_vlhAdditionalLoadedActors;
	deque<DnActorHandle>	m_dqhExceptActors;
	vector<DnActorHandle>	m_vlhDeadMonster;		// If a dead mob was used in a cutscene, HP is temporarily filled; keep track of it separately to hide after cutscene ends.
	
	// Store whether player actors were originally in battle mode, and draw their weapons again or not after the cutscene ends.
	// The vector list is set with the same indices as m_dqhLiveActor regardless of whether it is a player actor.
	// 1 means battle mode; otherwise 0.
	vector<int>				m_vlIsPlayerBattleMode;		
												

	CDnWorld*				m_pWorld;
	vector<S_ACT_PROP_DEFAULT> m_vlActPropDefault;

	SCameraInfo				m_OriginalCameraInfo;

	CEtDOFFilter*			m_pDOFFilter;
	float					m_fNearStart;
	float					m_fNearEnd;
	float					m_fFarStart;
	float					m_fFarEnd ;
	float					m_fFocusDist;
	float					m_fNearBlurSize;
	float					m_fFarBlurSize;

	bool m_bAutoFadeIn;
	bool m_bRequestSkip;
	CDnFadeInOutDlg *m_pSkipFade;
	bool m_bSetFadeModeCalledForSkip;			// When playing a cutscene via quest script in village, accept ESC input once and ignore further input while fading. (#11892)
	bool m_bLoadingComplete;
	bool m_bCheatCutScene;						// Is the current cutscene being shown via cheat?

	DnActorHandle m_hActor;

	// Keep separate info on equipped parts for exception handling when equipping a one-piece outfit.
	typedef struct _SPartsInfo {
		int nType;
		int nItemID;
	} SPartsInfo;
	std::vector<SPartsInfo> m_vecPartsInfo;

	vector< std::string > m_arrCharacterName; // #60295 Kali is not visible.

public:
	CDnCutSceneTask(void);
	virtual ~CDnCutSceneTask(void);

	bool Initialize( const char* pFileName, int nQuestIndex = -1, int nQuestStep = -1, bool bAutoFadeIn = true, DWORD dwQuestPlayerUniqueID = UINT_MAX );

	void PrepareNormalCutScene( DNTableFileFormat*  pActorTable );
	void PrepareQuestCutScene( DNTableFileFormat*  pActorTable, int nQuestIndex, int nQuestStep, DWORD dwQuestPlayerUniqueID );
	void StartPlay( void );
	void Finalize( void );			// When exiting client during cutscene playback, this function is called externally first.

	// from CTask
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	
	// from CInputReceiver
	virtual void OnInputReceive( int nReceiverState, LOCAL_TIME LocalTime = -1 );

	bool IsPlayEnd( void ) { return m_CutScenePlayer.IsEndScene(); };

	void FadeOutSkip();
	bool RenderScreen( LOCAL_TIME LocalTime, float fDelta );

	void FromCheat( void ) { m_bCheatCutScene = true; };

	// #60295 - Enchant level. Weapons have enhancement effects so enchant level can be passed as an argument. Ignored for armor.
	bool AttachItem( int nItemID, int nEnchantLevel = 0, int nSubArg1 = 0, int nLookItemID = ITEMCLSID_NONE );

private:
	void AddClonePlayer( DnActorHandle & hFindedActor ); // #60295 - Clone the player actor when 2 or more Player actors are created.
};
