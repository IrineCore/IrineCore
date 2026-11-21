#pragma once

#define MINIMAP_SIZE 256 

#define MIN_MINIMAP_ZOOM 2.0f
#define MAX_MINIMAP_ZOOM 5.0f

class CDnMinimap : public CSingleton< CDnMinimap >, public CEtCustomRender
{
public:
	enum emICON_INDEX
	{
		indexNone = -1,
		indexPlayer = 0,
		indexBoss = 1,
		indexEnemy = 2,
		indexNPC = 3,					// Nothing in particular
		indexNPC_Weapon = 4,			// Weapon Merchant
		indexNPC_Parts = 5,				// Armor merchant
		indexNPC_Normal = 6,			// General goods merchant
		indexNPC_Storage = 7,			// Warehouse
		indexNPC_Market = 8,			// Unmanned Trade Shop
		indexNPC_Mail = 9,				// Mailbox
		indexNPC_Guild = 10,			// Guild management		
		indexNPC_WarriorSkill = 11,		// Warrior skill
		indexNPC_ArcherSkill = 12,		// Archer skill
		indexNPC_SoceressSkill = 13,	// Sorceress skill
		indexNPC_ClericSkill = 14,		//Cleric skill

		indexParty = 15,

		indexQuestRecompense = 16,	// Quest reward
		indexQuestPalying = 17,		// Quest progress
		indexQuestAvailable = 18,	// Accept quest

		indexGateVillage = 19,
		indexGateDungeon = 20,
		indexGateWorldMap = 21,
		indexGateX = 22,
		indexGateUnknown = 23,

		indexSubQuestRecompense = 24,	// Sub quest reward
		indexSubQuestPalying = 25,		// Sub quest in progress
		indexSubQuestAvailable = 26,	// Accept sub quest

		indexGuild = 27,	// Guild member
		indexNPC_Plate = 28,			// Crest Jewel
		indexMail_New = 29,				// Received new mail

		indexRadioMsgHelp = 33,
		indexRadioMsgHeal = 34,
		indexRadioMsgFollow = 35,
		indexRadioMsgBack = 36,
		indexRadioMsgAttack = 37,
		indexRadioMsgStop = 38,

		indexEnemyCaptain = 39, // Captain Mode enemy captain icon
		indexAllyCaptain = 40,  // Captain Mode ally captain icon

		indexReputationFavorGrant = 41,   // Accept affinity quest
		indexReputationFavorNow = 42,     // Affinity quest in progress
		indexReputationFavorComplete = 43,// Affinity quest reward

		indexPartyMemberArcher = 44,
		indexPartyMemberSoceress = 45,
		indexPartyMemberWarrior = 46,

		indexRoot = 47,
		indexCereals = 48,
		indexLeaf = 49,
		indexFungus = 50,
		indexFruit = 51,

		indexNPC_AdventurerBoard = 52,          // Board-type NPC minimap icon
		indexAdventurerBoard_Available = 53,	// Adventurer's Board accept
		indexAdventurerBoard_Playing = 54,		// Adventurer's Board in progress
		indexAdventurerBoard_Recompense = 55,	// Adventurer's Board reward

#ifdef PRE_ADD_LOWLEVEL_QUEST_HIDE
		// Calculate the index based on the horizontal and vertical icon counts in MinimapIcon.dds.  
		// Order must have Reward as the smallest value and Accept as the largest value. 
		// Previously this was reversed, causing daily quest icons at the bottom of the minimap to display swapped. 
		indexQuestGlobal_Available = 58,	// Global quest
		indexQuestGlobal_Playing = 57,		//
		indexQuestGlobal_Recompense = 56,	//
#else
		indexQuestGlobal_Available = 56,	// Global quest
		indexQuestGlobal_Playing = 57,		//
		indexQuestGlobal_Recompense = 58,	//
#endif

		indexEmptyArea = 59,	// Farm empty space
		indexTraceQuest = 60,	// Quest tracking icon
		indexRevengeUser = 60,
		indexPartyMemberAcademic = 61, // Academic
		indexNPC_Vehicle = 62,   // Tamer
		indexNPC_Ticket = 63,          // Ticket NPC
		indexNPC_AcademicSkill = 64,   // Academic skill NPC
		indexNPC_Warp = 65,
		indexPartyMemberKali = 66, // Kali
		indexNPC_KaliSkill = 67 ,  // Kali skill NPC
		indexdNPC_PcBang = 68 ,    // PCBang Npc
		indexdNPC_Notice_PcBang = 69, // PcBang icon
		indexdNPC_Repair = 70, // Repair NPC
		indexdNPC_Notice_Repair = 71, // Repair speech bubble
		indexNPCPresent = 72,
		indexdNPC_Notice_Present = 73,
		indexAmount,
		indexGateAmount = 5,

	};

	struct SNPCQuestInfo
	{
		emICON_INDEX emQuestIndex;
		emICON_INDEX nIndexIcon;
		EtVector2 vNpcPos;
		int		nNpcID;
		bool bShow;
	};

	struct SPartyMemberPosInfo
	{
		EtVector3 pos;
		int classId;
		int team;
		emICON_INDEX eIconIndex;

		SPartyMemberPosInfo() : classId(-1), eIconIndex( emICON_INDEX::indexNone ), team(0) {}
		SPartyMemberPosInfo(const EtVector3& rhsPos, int rhsClassId, int rhsTeam )  { pos = rhsPos; classId = rhsClassId; team = rhsTeam; }
	};

	struct SOutInfo
	{
		emICON_INDEX eIconType;
		float fDegree;

		SOutInfo( emICON_INDEX eIcon , float fRange )
		{
			eIconType = eIcon;
			fDegree = fRange;
		}
	};

	protected:

		std::vector< DnActorHandle > m_vechEnemy;
		std::vector< DnActorHandle > m_vechFriend;
		std::vector< DnActorHandle > m_vechBoss;
		std::vector< DnActorHandle > m_vechNpc;
		std::vector< SNPCQuestInfo > m_vecNPCQuestInfo;
		std::map<int, std::pair<EtVector3,int>> m_mHarvestInfo;
		std::map<int, std::pair<EtVector3,int>> m_mEmptyInfo;
#if defined( PRE_ADD_REVENGE )
		std::vector< DnActorHandle > m_vecRevengeUser;
#endif
		std::vector< boost::tuple< DnActorHandle, int, LOCAL_TIME> > m_RadioActors;

		std::vector< SOutInfo > m_vecOutInfo;

		LOCAL_TIME m_LocalTime;
	
		EtTextureHandle m_hRenderTarget;
		EtMaterialHandle m_hMaterial;
		std::vector< SCustomParam > m_vecCustomParam;

		EtTextureHandle m_hMinimap;
		EtTextureHandle m_hEnemy;
		EtTextureHandle m_hOpacity;

		float m_fZoom;
		std::vector< STextureDiffuseVertex > m_vecVertex;
		std::vector< WORD > m_vecIndex;
		float m_fElapsedTime;
		float m_fTotalTime;
		bool m_bShow;
		std::string m_strMinimapFileName;

		bool m_IsAllPlayerShow;
		bool m_bShowFunctionalNPC;
		bool m_bShowOtherNPC;

public:
	CDnMinimap(void);
	virtual ~CDnMinimap(void);

protected:
	emICON_INDEX ConvertNpcTypeToIconIndex( int nJobType );
	emICON_INDEX ConvertStateMarkToQuestIconIndex( int nStateMark );
	emICON_INDEX ConvertPartyMarkToPartyIconIndex( int classId );
	emICON_INDEX ConvertPvPMark( emICON_INDEX original, DnActorHandle hActor );

	void ScanActor( const EtVector3 *pPlayerPos );
	void RenderGate( const EtVector3 *pPlayerPos );
	void RenderPlayer( bool bAnimateAlpha );
	void RenderNPC( std::vector<DnActorHandle> &vechActor, const EtVector3 *pPlayerPos );	
	void RenderActor( std::vector<DnActorHandle> &vechActor, const EtVector3 *pPlayerPos, emICON_INDEX iconIndex );
	void RenderPartyMember( const EtVector3 *pPlayerPos );
	void RenderHarvest( const EtVector3 *pPlayerPos );
	void RenderEmptyArea( const EtVector3 *pPlayerPos );
	void RenderTraceQuest( const EtVector3 *pPlayerPos );
#if defined( PRE_ADD_REVENGE )
	void RenderRevengeUser( const EtVector3 *pPlayerPos );
#endif	// #if defined( PRE_ADD_REVENGE )

	void GetAvailablePartyMember( std::vector<DnActorHandle> &ResultActorList , std::vector<SPartyMemberPosInfo> &ResultPosList );
	bool CheckPvPCondition( DnActorHandle hActor );
	void GetLinePoint( const EtVector3 *pPlayerPos, EtVector2 *pLinePoint, float fDegree, float fRatio );
	bool PointInLine( const EtVector2 *pTargetPosition, const EtVector2 *pLineBegin, const EtVector2 *pLineEnd );

	void InitVertexIndex( int nCount );
	void SetVertexIndex( int nIndex, const EtVector3 *pScreenPos, emICON_INDEX indexIcon, bool bRotate, BYTE btAlpha = 255, float fScale = 1.f );

	void RotateIcon( EtVector3 *pPos, const EtVector3 *pScreenPos );

	bool IsPVPGame();

public:
	void SetTextures( const char *pMinimap );
	void Zoom( float fZoomValue );
	std::string& GetMinimapName() { return m_strMinimapFileName; }

public:
	void Initialize();
	void InitializeNPC( int nMapIndex );
	void ReInitializeNPC( int nMapIndex );
	void Process( LOCAL_TIME LocalTime, float fDelta );
	void Show( bool bShow ) { m_bShow = bShow; }

	void InsertNpc( DnActorHandle hActor );
	void RemoveNpc( DnActorHandle hActor );

	void ShowNpcQuestInfo( int nNpcID, bool bShow );

	EtTextureHandle GetTexture() { return m_hRenderTarget; }

	bool IsMinimapOut(EtVector3 vAxisPos,EtVector3 vTartGetPos,float fMaxSizeHalf,float fZoom);
	float GetDistanceDegree(EtVector3 vAxis,EtVector3 vTarget);

	void InsertHarvest( int nAreaIndex, EtVector3 etVector3, char * szHarvestIconName );
	void DeleteHarvest( int nAreaIndex );
	void InsertEmptyArea( int nAreaIndex, EtVector3 etVector3 );
	void DeleteEmptyArea( int nAreaIndex );
	void AllDeleteEmptyArea();

	void InsertOutInfo( const EtVector3 *pPlayerPos , SPartyMemberPosInfo *pMemberInfo );
	std::vector<SOutInfo> &GetMiniMapOutInfo() { return m_vecOutInfo; }

	virtual void RenderCustom( float fElapsedTime );
	void SetAllPlayerShow(bool bSet ){m_IsAllPlayerShow =  bSet;};

	void ShowFunctionalNPC( bool bShow ) {m_bShowFunctionalNPC = bShow;}
	void ShowOtherNPC( bool bShow ) { m_bShowOtherNPC = bShow;}

	void SetRadioMark( DnActorHandle hActor, int nIconIndex );
};

#define GetMiniMap()		CDnMinimap::GetInstance()