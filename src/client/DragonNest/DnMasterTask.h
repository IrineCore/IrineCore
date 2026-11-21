#include "Task.h"
#include "MessageListener.h"

using namespace MasterSystem;

class CDnMasterTask : public CTask, public CTaskListener, public CEtUICallback, public CSingleton<CDnMasterTask> {
public:
	CDnMasterTask();
	virtual ~CDnMasterTask();

	struct CharacterInfo
	{
		MasterSystem::MasterClassmateKind::eCode	Code;
		__time64_t	tLastConnectDate;

		INT64	biCharacterDBID;
		wstring	wszCharName;
		wstring	wszSelfIntroduction;
		BYTE	cLevel;
		BYTE	cJobCode;
		BYTE	cGenderCode;
		int		iRespectPoint;
		int		iGraduateCount;
		
		BYTE cServerLocation;
		int nChannelID;
		int nMapIdx;

		CharacterInfo()
		{
			Code = MasterSystem::MasterClassmateKind::eCode::Max;
			tLastConnectDate = 0;
			biCharacterDBID = 0;
			cLevel = 0;
			cJobCode = 0;
			cGenderCode = 0;
			iRespectPoint = 0;
			iGraduateCount = 0;

			cServerLocation = 0;
			nChannelID = 0;
			nMapIdx = 0;
		}
	};

	struct MasterInfo
	{
		INT64	biCharacterDBID;
		wstring	wszCharName;
		BYTE	cLevel;
		BYTE	cJobCode;
		BYTE	cGenderCode;
		int		iRespectPoint;
		wstring	wszSelfIntroduction;
		int		iGraduateCount;
	};

	struct SMasterList
	{
		BYTE cCount;
		MasterInfo MasterInfoList[MasterSystem::Max::MasterPageRowCount];

		SMasterList() : cCount(0)	{}
	};

	enum{
		PUPIL_CONFIRM,
		MASTER_CONFIRM,
		RECALL_CONFIRM,
		JOIN_CONFIRM,
	};

protected :
	TMasterSystemSimpleInfo	m_SimpleInfo;
	SMasterList				m_MasterList;
	TMasterCharacterInfo	m_MasterCharacterInfo;
	bool					m_bIntroductionRegist;
	SCPupilList				m_PupilList;
	SCMasterClassmate		m_pMasterClassmate;
	TMyMasterInfoWithLocation	m_MyMasterInfo;
	TClassmateInfoWithLocation  m_ClassmateInfo;
	SCCountInfo				m_PartyCountInfo;
	int						m_iPenalyRespectPoint;
	int						m_iRespectPoint;

	std::vector<CharacterInfo>	m_vPupilInfo;
	std::vector<CharacterInfo>	m_vMasterClassmateInfo;

	int						m_nAcceptType;
	bool					m_bAcceptRequest;
	float					m_fAcceptTime;
	std::wstring			m_wszCharName;
	BYTE					m_cLevel;				// Apprentice level
	BYTE					m_cJob;					// Apprentice class

public:
	bool Initialize();
	void Finalize();

	const SMasterList & GetMasterList()	{ return m_MasterList; }
	const TMasterSystemSimpleInfo & GetSimpleInfo()	{ return m_SimpleInfo; }
	const TMasterCharacterInfo & GetMasterCharacterInfo()	{ return m_MasterCharacterInfo; }
	const bool GetIntroductionRegister()	{ return m_bIntroductionRegist; }
	const SCPupilList * GetPupilList()	{ return &m_PupilList; }
	const SCMasterClassmate * GetMasterClassmate()	{ return &m_pMasterClassmate; }
	const TMyMasterInfoWithLocation & GetMyMasterInfo()	{ return m_MyMasterInfo; }
	const TClassmateInfoWithLocation & GetClassmateInfo()	{ return m_ClassmateInfo; }
	const int GetPenaltyRespectPoint()	{ return m_iPenalyRespectPoint; }
	const int GetRespectPoint()	{ return m_iRespectPoint; }

	float GetShortestAcceptTime();
	void  OpenAcceptRequestDialog();

	void RequestMasterList( const UINT uiPage, const BYTE cJob, const BYTE cGender );
	void RequestMasterCharacterInfo();
	void RequestMasterIntroduction( const bool bRegister, const WCHAR * wszSelfIntroduction );
	void RequestMasterApplication( INT64 biCharacterDBID );
	void RequestMasterJoin( INT64 biMasterCharacterDBID, const WCHAR * wszMasterCharName );
	void RequestMasterAndClassmate();
	void RequestMyMasterInfo( INT64 biMasterCharacterDBID );
	void RequestClassmateInfo( INT64 biClassmateCharacterDBID );
	void RequestLeave( INT64 biDestCharacterDBID, bool bIsMaster );
	void RequestInvitePupil( const WCHAR * wszCharName );			//Master request (when self is the master)
	void RequestJoinDirect( const WCHAR * wszMasterCharName );		//Apprentice request (when self is apprentice)
	void RequestJoinDirectConfirm( const bool bIsAccept, const WCHAR * wszPupilCharName );
	void RequestInvitePupilConfirm( const bool bIsAccept, const WCHAR * wszMasterCharName );
	void RequestRecallMaster( const WCHAR * wszMasterCharName );

	void OnRecvMasterSystemMessage( int nSubCmd, char *pData, int nSize );

	void OnRecvMasterSimpleInfo( SCSimpleInfo * pPacket );
	void OnRecvMasterList( SCMasterList * pPacket );
	void OnRecvMasterCharacterInfo( SCMasterCharacterInfo * pPacket );
	void OnRecvMasterPupilList( SCPupilList * pPacket );
	void OnRecvMasterIntroduction( SCIntroduction * pPacket );
	void OnRecvMasterApplication( SCMasterApplication * pPacket );
	void OnRecvMasterJoin( SCJoin * pPacket );
	void OnRecvMasterAndClassmate( SCMasterClassmate * pPacket );
	void OnRecvMyMasterInfo( SCMyMasterInfo * pPacket );
	void OnRecvClassmateInfo( SCClassmateInfo * pPacket );
	void OnRecvLeave( SCLeave * pPacket );
	void OnRecvInvitePupil( SCInvitePupil * pPacket );
	void OnRecvInvitePupilConfirm( SCInvitePupilConfirm * pPacket );
	void OnRecvJoinDirect( SCJoinDirect * pPacket );
	void OnRecvJoinDirectConfirm( SCJoinDirectConfirm * pPacket );
	void OnRecvGraduate( SCGraduate * pPacket );
	void OnRecvCountInfo( SCCountInfo * pPacket );
	void OnRecvRespectPoint( SCRespectPoint * pPacket );
	void OnRecvFavorPoint( SCFavorPoint * pPacket );
	void OnRecvRecallMaster( SCRecallMaster * pPacket );
	void OnRecvConnect( SCConnect * pPacket );
	void OnRecvJoinConfirm( SCJoinComfirm * pPacket );

	INT64 IsMasterPlayer( const WCHAR * wszName );
	bool  IsPlayWithPupil( DnActorHandle hActor );
	void  CloseAcceptDlg();
	void RejectRecall();

	virtual void OnDispatchMessage( int nMainCmd, int nSubCmd, char *pData, int nSize );

public :

	virtual void OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
};

#define GetMasterTask()	CDnMasterTask::GetInstance()
