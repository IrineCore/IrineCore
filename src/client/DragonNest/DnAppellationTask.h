#pragma once

#include "Task.h"
#include "MessageListener.h"

class CDnAppellationTask : public CTask, public CTaskListener, public CSingleton<CDnAppellationTask> {
public:
	CDnAppellationTask();
	virtual ~CDnAppellationTask();

	enum DESC_INDEX
	{
		EFFECT_DESC = 0,
		CONTANT_DESC,
		CONDITION_DESC,
		ANOTHER_DESC	//Show whichever exists between effect description and title description.
	};

	enum PARAM_INDEX
	{
		NONE = -1,
		STRENGTH,
		AGILITY,
		INTELIGENCE,
		STAMINA,
		ATTACK_MIN_P,
		ATTACK_MAX_P,
		ATTACK_MIN_M,
		ATTACK_MAX_M,
		DEFENCE_P,
		DEFENCE_M,
		STIFF,
		STIFFRES,
		CRITICAL,
		CRITICALRES,
		STUN,
		STUNRES,
		FIRE_ATTACK,
		ICE_ATTACK,
		LIGHT_ATTACK,
		DARK_ATTACK,
		FIRE_DEFENSE,
		ICE_DEFENNSE,
		LIGHT_DEFENSE,
		DARK_DEFENSE,
		MOVE_SPEED,
		MAX_HP,
		MAX_SP,
		RECOVER_SP,
		SUPER_AMMOR,
		FINAL_DAMAGE,
		SPIRIT,
		MAX_STATE,


	};



	struct AppellationStruct {
		int		nArrayIndex;
		tstring szName;
		tstring szEffectDescription;
		tstring szContantDescription;
		tstring szTakeCondition;
		tstring szPeriodAppellation;
		__time64_t tExpireDate;
		bool	bNew;
		char	cColorType;
		std::vector< std::pair<int,int> > vParamIndex;

		DWORD ColorValue;

		AppellationStruct() : nArrayIndex( 0 ), tExpireDate( 0 ), bNew( false ), cColorType( 0 ), ColorValue( 0 ) {}
	};

	struct stAppellationCollection
	{
		int		m_nTitleID;						// Collection ID
		bool	m_bOpen;						// Public visibility flag
		int		m_nDifficulty;					// Collection difficulty (0 ~ 4)
		int		m_nRewardAppellationIndex;		// Collection reward title ID
		float	m_fPercent;						// Collection completion rate

		tstring m_strCollectionTitle;			// Collection title
		tstring m_strCollectionDescription;		// Collection description
		
		std::map<int, int>	m_mapAppellationIndex;	// Collection honor title

		~stAppellationCollection() { m_mapAppellationIndex.clear(); }
	};

protected:
	std::vector<AppellationStruct *>	m_pVecAppellationList;
	std::vector< std::pair<int, int> >	m_VecLocalData[4];
	int m_nSelectAppellation;
	int m_nSelectCoverAppellation;
	std::vector<stAppellationCollection>	m_vecAppellationCollection;

#ifdef PRE_ADD_BESTFRIEND
	static int m_BFappellationIndex; // Best friend title index.
	bool m_bAddChatBestFriend;
#endif

protected:
	virtual void OnRecvAppellationMessage( int nSubCmd, char *pData, int nSize );
	AppellationStruct *AddAppellation( int nArrayIndex, bool nNew = false );
	void GetParamIndex( int nArrayIndex, std::vector< std::pair<int,int> > & nParamIndex );

public:
	bool Initialize();
	void Finalize();

	virtual void Process( LOCAL_TIME LocalTime, float fDelta );

	virtual void OnDisconnectTcp( bool bValidDisconnect );
	virtual void OnDisconnectUdp();

	virtual void OnDispatchMessage( int nMainCmd, int nSubCmd, char *pData, int nSize );
	virtual void OnRecvAppellationList( SCAppellationList *pPacket );
	virtual void OnRecvGainAppellation( SCAppellationGain *pPacket );
	virtual void OnRecvSelectAppellation( SCSelectAppellation *pPacket );
	virtual void OnRecvChangeAppellation( SCChangeAppellation *pPacket );
	virtual void OnRecvPeriodAppellation( SCPeriodAppellationTIme *pPacket );
	virtual void OnRecvPCBangAppellation();

	void OnGainAppellation( AppellationStruct *pStruct );
	void OnSelectAppellation( int nSelectIndex );

	void RequestSelectAppellation( int nArrayIndex, int nCoverArrayIndex );
	void SelectAppellation( int nCoverArrayIndex, int nArrayIndex, bool bSetName=true );

	DWORD GetAppellationCount() { return (DWORD)m_pVecAppellationList.size(); }
	AppellationStruct *GetAppellationInfo( DWORD dwIndex );
	int GetSelectAppellation() { return m_nSelectAppellation; }
	void SelectCoverAppellation( int nArrayIndex ) { m_nSelectCoverAppellation = nArrayIndex; }
	int GetSelectCoverAppellation() { return m_nSelectCoverAppellation; }
	void RequestSelectCoverAppellation( int nArrayIndex );
	void LoadAppellationCollectionList();
	void UpdateMyAppellationCollectionList();
	std::vector<stAppellationCollection>& GetAppellationCollectionList() { return m_vecAppellationCollection; }
	bool IsExistAppellation( int nArrayIndex );
	tstring MakePeriodAppellation(__time64_t tExpireDate);

	tstring MakeDescription( int nArrayIndex, const DESC_INDEX nIndex = ANOTHER_DESC);

	static tstring GetAppellationName( int nArrayIndex, CDnPlayerActor * pActor=NULL );

#ifdef PRE_MOD_APPELLATIONBOOK_RENEWAL
	AppellationStruct *GetAppellationInfoByArrayIndex( DWORD dwArrayID );
#endif

#ifdef PRE_ADD_BESTFRIEND
	void SetBFappellation();	
	bool IsAddChatBestFriend(){ return m_bAddChatBestFriend; }
	void ResetAddChatBestFriend(){ m_bAddChatBestFriend = false; }
	static int GetBFAppellationIndex(){ return m_BFappellationIndex; } // Best friend title index.
#endif
};