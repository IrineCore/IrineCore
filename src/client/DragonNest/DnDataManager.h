#pragma once
#include <tuple>
#include "DNPacket.h"



/** Struct needed for dialogue questions; holds only the question message.
*/
struct TALK_QUESTION
{
	std::wstring szQuestion;
};

/** Struct needed for dialogue responses; includes info on which dialogue follows when this response is selected.
*/
struct TALK_ANSWER 
{
	/// Target to link upon response (another dialogue file or another quest file)
	std::wstring szLinkTarget;
	/// Index to link upon response
	std::wstring szLinkIndex;
	/// Response message
	std::wstring szAnswer;
	/// true if linking to another talk file
	bool		 bOtherTargetLink;	

};

/** Unit displayed in the dialogue window per interaction with an NPC 
*/
struct TALK_PARAGRAPH
{
	/// Index of this dialogue paragraph
	std::wstring				szIndex;
	/// Background image name when this dialogue paragraph is set
	std::string					szImageName;
	/// Item index when this dialogue paragraph is set
	int								nItemIndex;
	/// Question struct
	TALK_QUESTION				Question;
	/// List of answer structs for the question
	std::vector<TALK_ANSWER>	Answers;

};

/// key - paragraph string index, second - paragraph
typedef std::map<std::wstring, TALK_PARAGRAPH>	TALK_PARAGRAPH_MAP;
typedef TALK_PARAGRAPH_MAP::iterator			TALK_PARAGRAPH_MAP_IT;

/// key - dialog xml file name, second - full dialog
typedef std::map<std::wstring, TALK_PARAGRAPH_MAP> TALK_MAP;
typedef TALK_MAP::iterator						   TALK_MAP_IT;

/// Journal item
struct  JournalItem
{
	enum Type
	{
		Item = 1,
		Monster = 2,
		Gold = 3
	};

	/// Type is the JournalItem::Type value above.
	char cType;
	/// Index for item/monster, amount for Gold (Glod)
	UINT nValue;
	/// Count
	UINT nCount;
};

/// Type of quest notifier direction indicator
struct ArrowType
{
	enum 
	{
		Hide = 0,
		NormalArrow,
		ArrivalMark,
		BattleMark,
		FindMark,
		MaxCount
	};
};

/** Structure setting the values needed for the quest journal page
@remarks
If a quest has 10 journal steps while completing it, there will be 10 journal pages.
*/
struct JournalPage 
{
	/*
none - default
hide - turn off all
arrival - all arrived
stage_hide - Hide when entering any zone map (dungeon)
stage_arrival - Show arrival icon when entering any zone map (dungeon)
select_hide:10,11,12 - Hide when in selected maps (10,11,12)
select_arrival:10,11,12 - Show arrival icon when in selected maps (10,11,12)
stage_hide_except:10,11,12 - Hide in stage maps except maps 10, 11, 12.
custom:10@1,10@2 - By default show HIDE on dungeon map and NORMAL on world map, and show the corresponding icon for entries with values
	*/


	enum DestinationMode
	{
		none = 0,
		hide,
		arrival,
		stage_hide,
		stage_arrival,
		select_hide,
		select_arrival,
		stage_hide_except,
		custom,
	};

	struct DestinationMark
	{
		int nMapIdx;
		int nMarkIdx;
	};
		
	bool bOnlyMinimap;
	int nStateMarkNpcID;
	int nStateMarkType;
	int	nJournalPageIndex;
	int nChapterIndex;
	int	nDestnationMapIndex;
	int nDestinationNpc;
	int	nDestnationMode; 

	std::wstring szTitle; 
	std::wstring szTitleImage;
	std::wstring szContents; 
	std::wstring szContentsImage; 
	std::wstring szTodoMsg;
	std::wstring szDestination; 

	std::vector<int> DestnationModeValue;
	std::vector<JournalItem> NeedItemList; 
	std::vector<DestinationMark>	CustomDestnationMark; 
		
	D3DXVECTOR2  vDestinationPos; 

	JournalPage()
	{
		bOnlyMinimap = false;
		nStateMarkNpcID = 0;
		nStateMarkType = 0;
		nJournalPageIndex = 0;
		nChapterIndex = 0;
		nDestnationMapIndex = 0;
		nDestnationMode = 0;
		nDestinationNpc = -1;

		vDestinationPos = D3DXVECTOR2(0.f,0.f);
	}

};

/** Journal struct
@remarks
Journal information required for quest progression. Journal info includes basic data needed to display quest info and
Holds all journal pages. The client shows up to that page based on the current journal step.
*/ 
struct Journal 
{
	enum {
		emQuestType_Main = 1,
		emQuestType_Sub,
		emQuestType_Day,
		emQuestType_Rep,
	};

	/// Quest index
	UINT						nQuestIndex;
	/// Quest title
	std::wstring				wszQuestTitle;
	/// Quest execution level title
	std::wstring				wszQuestLevelTitle;
	/// Quest execution level
	int							nQuestLevel;
	/// Quest mark type (1 for one exclamation mark, 3 for three)
	int							nQuestMarkType;
	
	typedef std::vector<JournalPage>	JOURNALPAGE_VEC;
	/// Journal pages..
	JOURNALPAGE_VEC				JounalPages;

	/// Chapter index
	int							nChapterIndex;

	/// Reward table ID 
	int							nRecompenseTableID;
	int							nRecompenseTableIDForClass[ CLASSKINDMAX ];

	// Quest type
	int							nQuestType;

	/// Constructor
	Journal()
		: nQuestIndex(0), nQuestLevel(0), nChapterIndex(0), nQuestMarkType(0), nRecompenseTableID(0), nQuestType(0)
	{
		for( int i = 0; i < CLASSKINDMAX; i++) {
			nRecompenseTableIDForClass[ i ] = 0;
		}
	}

	/** Get the desired journal page from the current Quest Journal.
	@param
nJounalPageIndex journal page index, starts from 1.
	*/
	JournalPage* FindJournalPage(int nJournalPageIndex)
	{
		for ( size_t i = 0 ; i < JounalPages.size() ; i++ )
		{
			if ( JounalPages[i].nJournalPageIndex == nJournalPageIndex )
				return &JounalPages[i];
		}
		return NULL;
	}

	bool IsMainQuest() { return (nChapterIndex > 0 );}

	int GetMyRecompenseID( int nClass )
	{
		int nClassIndex = nClass - 1;

		if( 0 > nClassIndex || CLASSKINDMAX <= nClassIndex )
			return nRecompenseTableID;

		int nClassRecompenseID = nRecompenseTableIDForClass[ nClassIndex ];
		
		if( nClassRecompenseID != 0 )
			return nClassRecompenseID;

		return nRecompenseTableID;
	}
};

/**	Quest condition check interface structure
*/
struct ConditionBase 
{
	bool bUse;
	virtual void Init(std::wstring& data) { if ( data.empty() ) bUse = false; }
	virtual bool Check() {	return false; }
	
	virtual bool ShowCheck() { return true;} 
	// ShowCheck() :: Used to hide the quest StateMark < acceptable > // Please add virtual settings inside the required condition 
	// Used to hide the mark if level differs by 10 or more from current __UserLevel. Set if such constraints are added per condition

	ConditionBase()
	{
		bUse = false;
	}
	virtual ~ConditionBase(){ }
};

/** Object that checks quest conditions. If conditions like required level higher than current level are not met when accepting a quest,
Cannot accept quest. This object checks such conditions.
*/ 
struct QuestCondition 
{
	UINT nQuestIndex;
	std::vector<ConditionBase*>	ConditionList;
	bool Check();
	bool ShowCheck();
	
	QuestCondition();
	virtual ~QuestCondition();
};

/** Structure holding quest info loaded from the quest table
*/
struct QuestInfo 
{
	UINT				nQuestIndex;
	std::wstring		szQuestName;
	std::wstring		szQuestTalk;
	std::wstring		szQuestScript;
	int					nSortIndex;
	char				cQuestType;
	int					nMapIndex;
	bool				bLevelCapReward;
};

struct PeriodQuestInfo
{
	int nType;
	std::vector<int> vecQuestID;
};

struct TActorData
{
	int nActorID;
	BYTE cClass;						// Unit type kinds
};

const int QUEST_MAX_CNT = 50;
struct TNpcData
{
	int nNpcID;
	int nParam[2];
	WCHAR wszName[EXTDATANAMELENMAX];
	char szTalkFile[256];					// Dialogue file name
	char szScriptFile[256];					// script filename
	int QuestIndexArr[QUEST_MAX_CNT];		// Quest index
	int ReputeQuestIndexArr[QUEST_MAX_CNT];	// Affinity quest index
	TActorData ActorData;
};

typedef map<int, TActorData*> TMapActorData;
typedef map<int, TNpcData*> TMapNpcData;

typedef std::map<UINT, QuestCondition*>		QUEST_CONDITION_MAP;
typedef QUEST_CONDITION_MAP::iterator		QUEST_CONDITION_MAP_IT;

typedef std::map<UINT, Journal*>			JOURNAL_DATA_MAP;
typedef JOURNAL_DATA_MAP::iterator			JOURNAL_DATA_MAP_IT;

typedef std::map<std::wstring, QuestInfo>	QUEST_INFO_MAP;
typedef QUEST_INFO_MAP::iterator			QUEST_INFO_MAP_IT;

typedef std::vector< PeriodQuestInfo >			PERIOD_QUEST_INFO_VEC;
typedef PERIOD_QUEST_INFO_VEC::const_iterator	PERIOD_QUEST_INFO_VEC_IT;

typedef std::map<int, std::wstring>			TALK_PARAM_MAP;
typedef TALK_PARAM_MAP::iterator			TALK_PARAM_MAP_IT;

typedef map<DWORD, std::wstring> TXMLFileMap;
typedef map<DWORD, std::wstring> TXMLIndexMap;

/// Quest reward item table info. Can set money and up to 6 item types. nItemID and nItemCount form a pair.
const int RECOMPENSE_ITEM_MAX = 6;

/**	Quest reward table structure
*/
struct TQuestRecompense 
{
	struct _ITEMSET
	{
		int		nItemID;
		int		nItemCount;
	};

	/// Quest index
	int			nQuestIndex;
	/// Reward table index
	UINT		nIndex;			
	/// Reward table string
	UINT		nStringIndex;	
	/// Reward type: 1 = all rewards / 2 = selective reward
	char		cType;			
	/// Max number of items to choose for selectable reward.
	char		cSelectMax;		
	/// Reward amount
	UINT		nCoin;			
	/// Reward EXP
	int			nExp;			
	_ITEMSET	ItemArray[RECOMPENSE_ITEM_MAX];
	/// SN in the CashCommodity table for the reward cash item
	int			CashItemSNArray[RECOMPENSE_ITEM_MAX];
	int			nLevelCapStringIndex;
	int			nRewardFatigue;
	int			nOperator;
	int			nDifficult;
};

struct TQuestLevelCapRecompense
{
	struct _ITEMSET
	{
		int nItemID;
		int nItemCount;
	};

	char cType;
	char cClassID;
	_ITEMSET RecompenseItem[MAX_QUEST_LEVEL_CAP_REWARD];
};

typedef map<UINT, TQuestRecompense>		 QUEST_RECOMPENSE_MAP;
typedef QUEST_RECOMPENSE_MAP::iterator	 QUEST_RECOMPENSE_MAP_IT;

// Quest rewards are based on the reward table and set by the reward table ID specified in the script, so 
// Cannot reverse-lookup rewards by quest ID, so create a struct pairing reward table ID and quest index; the server has this too
typedef map<UINT,UINT>                        QUEST_INDEX_RECOMPENSE_MAP; 
typedef QUEST_INDEX_RECOMPENSE_MAP::iterator  QUEST_INDEX_RECOMPENSE_IT;

typedef map<std::tuple<char, char>, TQuestLevelCapRecompense> QUEST_LEVEL_CAP_RECOMPENSE_MAP;
typedef QUEST_LEVEL_CAP_RECOMPENSE_MAP::iterator QUEST_LEVEL_CAP_RECOMPENSE_IT;

// Because an event callback is needed during dialogue text substitution (action play) 
// If {action:cry_npc} is included in the dialogue, the NPC plays the action cry_npc.
struct StaticParamCallback
{
	virtual void OnAction(std::string& szActionName) { }
	virtual void OnSound(std::string& szActionName) { }
};

/**
@brief
Container class that loads and holds quest-related information
@remarks
Quest/NPC dialogue info, quest journal info, quest reward info, NPC info, dialogue substitution info
Class that can Load and Get etc.
Declared as global external object g_DataManager; access it wherever info is needed.

@note
Private functions starting with _ were mostly added to split up long functions when parsing large amounts of data.
Never called directly from outside.

*/
class CDnDataManager
{
public:

	enum eQuestTalkPriorityType
	{
		Main,
		Complete,
		Playing,
		Available,
		LowPriority,
		Text,
		Max,
	};
	/// Default constructor. Calls LoadData on creation to load all info.
	CDnDataManager();
	/// Destructor. Releases loaded data.
	virtual ~CDnDataManager();

	/// Cleanup function called from the destructor.
	void ClearAll();
	/// Load all data. Load any additional data here.
	bool LoadData();

	/**	Get dialogue paragraph.
@param wszIndex Index of the dialogue paragraph; "start" for the very beginning.
@param wszTarget Dialogue filename e.g. npc_001_halod.xml
@param talkParam List for text substitution; if present, matching strings are substituted.
@param talk Passed by reference, so the retrieved dialogue paragraph is set here.
@param pCallback Callback class used when non-text actions like {action} {sound} are needed during base dialogue substitution. See StaticParamCallback struct.
	*/
	bool GetTalkParagraph(IN std::wstring& wszIndex, IN std::wstring& wszTarget, IN std::vector<TalkParam>& talkParam, OUT TALK_PARAGRAPH& talk, IN StaticParamCallback* pCallback = NULL );

	/**	Communicate with the server by converting the dialogue file index or paragraph filename into a hashcode.
	@remarks
For example, when sending an index like start.xml, exchange the hashcode of "start.xml" instead of the string
Distinguish which paragraph or index was selected.
	*/ 
	bool GetTalkFileName(DWORD nHashCode, OUT std::wstring& wszIndex);
	bool GetTalkIndexName(DWORD nHashCode, OUT std::wstring& wszIndex);

	/// Generate a hashcode from the string.
	DWORD GetStringHashCode(const WCHAR* pString);
	
	/// Load the quest reward table.
	bool LoadQuestRecompense();
	/// Get the quest reward table. 
	bool GetQuestRecompense(UINT nRecompenseIndex, OUT TQuestRecompense& recom);
	bool GetQuestRecompenseByQuestIndex(UINT nQuestID,OUT TQuestRecompense& recom);

	bool LoadQuestLevelCapRecompense();
	bool GetQuestLevelCapRecompense( const char cQuestType, const char cClassID, OUT TQuestLevelCapRecompense & questLevelCapRecompense );

	/// Get the journal object for the quest.
	Journal* GetJournalData(UINT nQuestIndex);
	/// Get the condition object for the quest.
	QuestCondition* GetQuestCondition(UINT nQuestIndex);

	/// Load Actor info 
	bool LoadActorData();
	TActorData* GetActorData(int nActorIndex);
	
	/// Load NPC data
	bool LoadNpcData();
	bool IsExistStartQuestFromNpc( int nNpcIndex, int nQuestIndex );
	TNpcData* GetNpcData( int nNpcIndex );

	/** Function used for default predefined dialogue substitution 
	@remarks
Callback class used when non-text actions like {action} or {sound} must be performed during default dialogue substitution.
Refer to the StaticParamCallback structure.
	*/ 
	void ReplaceStaticTalkParam(std::wstring& str, IN StaticParamCallback* pCallback = NULL );

	/// utility func
	std::vector<std::wstring> GetTalkFileList();

	/// Utility functions
	std::wstring GetChapterString(int nChapterIdx);
	std::string GetChapterImageFileName(int nChapterIdx);
	std::string GetQuestImageFileName(int nQuestIndex);
	JOURNAL_DATA_MAP &GetJournalList() { return m_JournalMap; }
	char GetQuestType(int nQuestIndex) const;
	bool GetQuestHideType(int nQuestIndex);

#ifdef PRE_ADD_LOWLEVEL_QUEST_HIDE
	bool IsEventQuest( int nQuestindex );
	// Low-level quest filtering
	bool IsLowLevelQuest( Journal* pJournal);
#endif 
	/// Get the quest info object from the quest dialogue file (.xml).
	QuestInfo* FindQuestInfo(std::wstring& szTalkFilename);
	/// Get the quest info object by quest index.
	QuestInfo* FindQuestInfo(UINT nQuestID);
	/// Check if it is a time-limited quest
	bool IsPeriodQuest( const int nQuestID ) const;

	/// Initialize quest and dialog info when the stage starts
	void InitNpcTalkData( int nMapTableID );
	/// Clear quest and dialog info when the stage ends
	void ClearTalkData();
	/// Load dialogue info individually
	bool LoadNpcTalkData( int nNpcIndex );
	/// Load quest info individually
	bool LoadNpcQuestData( int nNpcIndex );
	/// Load quest info individually
	bool LoadQuestData( int nQuestID );

private:
	/// Load the quest dialogue file (.xml). 
	bool _AddTalk(const WCHAR* _wszTalkFileName);
	/// Get the corresponding dialogue file.
	bool _GetTalk(const WCHAR* wszTalkFileName, OUT TALK_PARAGRAPH_MAP& TalkParagraphMap );
	/// Remove the quest dialogue file. 
	bool _RemoveTalk(const WCHAR* wszTalkFileName);
	/// Remove all journal data
	void _RemoveAllJournalData();
	/// Remove all quest condition objects
	void _RemoveAllQuestCondition();

	/// Journal data parsing function
	void _LoadJournalData(TiXmlElement* pRootElement, UINT nQuestID);
	/// Journal data parsing function
	void _ParseJournalItemlist(std::wstring& wszString, std::vector<JournalItem>& j_item_list);
	/// Journal data parsing function
	void _ParseDestMode(std::wstring& wszString, JournalPage&  page);
	
	/// Condition data parsing function
	void _LoadQuestCondition(TiXmlElement* pRootElement, UINT nQuestID);
	/// Load quest list
	bool _LoadQuestList();
	/// Load time-limited quest info
	bool _LoadPeriodQuestList();
	/// Load dialogue substitution info	
	bool _LoadTalkParamTable();
	/// Internal function used for dialogue substitution
	std::wstring _GetString(int nType, std::vector<std::wstring>& tokens, IN StaticParamCallback* pCallback = NULL );
	/// Internal function used for dialogue substitution; searches for a placeholder matching the given type.
	std::wstring _GetTalkParam(int nIndex);
	/// Load dialogue info
	bool _LoadTalkData();

	/// Journal data map 
	JOURNAL_DATA_MAP		m_JournalMap;
	/// Quest condition
	QUEST_CONDITION_MAP		m_QuestConditionMap;
	QUEST_INFO_MAP			m_QuestInfoMap;
	TALK_MAP				m_TalkMap;
	TALK_PARAM_MAP			m_TalkParamMap;
	TXMLFileMap				m_XMLFileMap; 
	TXMLIndexMap			m_XMLIndexMap; 
	QUEST_RECOMPENSE_MAP	m_QuestRecompenseMap;
	TMapActorData			m_pActorData;
	TMapNpcData				m_pNpcData;
	QUEST_INDEX_RECOMPENSE_MAP m_QuestIndexRecompenseMap;
	QUEST_LEVEL_CAP_RECOMPENSE_MAP m_QuestLevelCapRecompenseMap;
	PERIOD_QUEST_INFO_VEC	m_vPeriodQuestInfo;

	struct  __StaticTalkParamInfo
	{
		enum 
		{
			user_nick = 1,
			user_class,
			monster,
			npc,
			item,
			skill,
			map,
			action,
			sound,
			uistring,
			repair_cost,
			repair_cost_all,
			money,
			quest_level,
		};
		int nType;
		std::wstring	szKey;
		__StaticTalkParamInfo(int type, const WCHAR* wszKey)
		{
			nType = type;
			szKey = wszKey;
		}
	};

	std::vector<__StaticTalkParamInfo>	m_StaticTalkParamList;


private:

#ifdef _DEBUG
	FILE * m_fp;
#endif


};

extern CDnDataManager g_DataManager;
