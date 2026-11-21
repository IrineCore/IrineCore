#pragma once

#if defined(_VILLAGESERVER) || defined(_GAMESERVER)
class CDNGameDataManager;
#else
#include "DNTableFile.h"
#endif

//------------------------------------------------------------------------------
/**
Note Hangi:

Load and hold data from skill tree related tables
Process data verification/query requests.

Create something new or modify data after reading it once data is configured
is not applied, so multithreading is not considered.
*/
//------------------------------------------------------------------------------


// Maximum number of skill slots, as it varies depending on how the tree is arranged 
// If max skill slots need to be increased, change this constant 
// Must also match the slot count in R:/GameRes/Resource/UI/Skill/SkillTreeContentDlg.ui.
const int MAX_SKILL_SLOT_COUNT = 28;

const int MAX_PARENT_SKILL_COUNT = 3;
const int MAX_SKILL_PER_LINE = 4;


class CDnSkillTreeSystem
{
private:
	struct S_UNLOCK_CONDITION
	{
		int iUnlockSkillBookItemID;

		S_UNLOCK_CONDITION( void ) : iUnlockSkillBookItemID( 0 )
		{

		};
	};

	struct S_ACQUIRE_CONDITION
	{
		// Required character level to unlock the skill. Defined at level 1 in the skill level table for each skill.
		int iNeedLevel;

		// Required parent skill level. Matches the order in NodeInfo and must do so.
		vector<int> vlParentSkillNeedLevel;

		S_ACQUIRE_CONDITION( void ) : iNeedLevel( 0 )
		{

		};
	};

public:
	// Struct for UI display.
	struct S_NODE_RENDER_INFO
	{
		int iSkillID;
		int iSlotIndex;
		int iJobID;
		const S_UNLOCK_CONDITION* pUnLockCondition;
		const S_ACQUIRE_CONDITION* pAcquireCondition;
		vector<int> vlChildSlotIndexInJob;		// Tree slot index going left to right. Child only within the same class tree.

		// A skill from another class can be the parent skill.
		vector<int> vlParentSkillJobIDs;
		vector<int> vlParentSkillIDs;

#if defined(PRE_ADD_SKILL_LEVELUP_LIMIT_BY_SP)
		vector<int> vlNeedJobSP;							//Required SP value per class group
#endif // PRE_ADD_SKILL_LEVELUP_LIMIT_BY_SP

		S_NODE_RENDER_INFO() : iSkillID( 0 ), iSlotIndex( 0 ), iJobID( 0 ), pUnLockCondition( NULL ) , pAcquireCondition( NULL ){};
		S_NODE_RENDER_INFO( int argiSkillID, int argiSlotIndex, int argiJobID, 
							const S_UNLOCK_CONDITION* argpUnLockCondition, const S_ACQUIRE_CONDITION* argpAcquireCondition ) : 
							iSkillID( argiSkillID ), iSlotIndex( argiSlotIndex ), iJobID( argiJobID ), 
							pUnLockCondition( argpUnLockCondition ), pAcquireCondition( argpAcquireCondition ) {};
	};

private:
	// Skill tree node info.
	struct S_NODE_INFO
	{
		int iJobID;
		int iSkillID;
		int iNeedSkillPointToAcquire;		// Skill points required to acquire

		//------------------------------------------------------------------------------
		/** 
Tree slot IDs are assigned sequentially starting from 0 from left to right.
Currently 4 slots per row.
		*/
		//------------------------------------------------------------------------------
		int iTreeSlotIndex;
		bool bDefaultLocked;

		// Conditions to unlock and acquire this node's skill
		S_UNLOCK_CONDITION UnLockCondition;
		S_ACQUIRE_CONDITION AcquireCondition;

		vector<const S_NODE_INFO*> vlpChildNodeInfo;
		vector<const S_NODE_INFO*> vlpParentNodeInfo;		// Up to 3 parent skill nodes can exist. All must be present to acquire the skill.

#if defined( PRE_ADD_PRESET_SKILLTREE )
		vector<int> vecAdviceSkillLevel;
#endif	// #if defined( PRE_ADD_PRESET_SKILLTREE )

#if defined( PRE_ADD_ONLY_SKILLBOOK )
		bool bNeedSkillBook;
#endif	// #if defined( PRE_ADD_ONLY_SKILLBOOK )

		// Node rendering info
		S_NODE_RENDER_INFO* pNodeRenderInfo;

		S_NODE_INFO() : iTreeSlotIndex( 0 ), iJobID( 0 ), iSkillID( 0 ), iNeedSkillPointToAcquire( 0 ), 
						bDefaultLocked( false ), pNodeRenderInfo( NULL )
#if defined( PRE_ADD_ONLY_SKILLBOOK )
						, bNeedSkillBook( false )
#endif	// #if defined( PRE_ADD_ONLY_SKILLBOOK )
		{
			
		};
	};

	struct S_SKILLTREE_INFO
	{
		int iJobID;
		
		vector<S_NODE_INFO*> vlpNodeList;				// Access skill node via list.
		map<int, S_NODE_INFO*> mapNodeBySkillID;		// Access skill node using Skill ID as key.

		vector<S_NODE_RENDER_INFO> vlNodeRenderInfo;	// Info for node rendering.

		~S_SKILLTREE_INFO()
		{
			SAFE_DELETE_PVEC( vlpNodeList );
		}
	};
	
	vector<S_SKILLTREE_INFO*> m_vlpJobSkillTreeList;
	map<int, S_SKILLTREE_INFO*> m_mapSkillTreeByJobID;	// Access skill tree by job (class) ID.
	map<int, S_NODE_INFO*> m_mapNodeListBySkillBookID;	// Node info can be found by skill book item ID.

protected:
	S_NODE_INFO* _FindNode( int iSkillID );
	void _MakeupParentSkillLinks( DNTableFileFormat* pSkillTreeTable, DNTableFileFormat* pSkillTable );

public:
	CDnSkillTreeSystem(void);
	~CDnSkillTreeSystem(void);

	// Load tables and initialize required info.
	// Since the server has data organized in DataManager, initialize using that.
#if defined(_VILLAGESERVER) || defined(_GAMESERVER)
	bool InitializeTableUsingDataManager( CDNGameDataManager* pDataManager );
#endif
	bool InitializeTable( void );

	struct S_POSSESSED_SKILL_INFO
	{
		int iSkillID;
		int iSkillLevel;		// Not level table ID. Raw current skill level
		bool bCurrentLock;		// Whether currently locked or not.

		S_POSSESSED_SKILL_INFO() : iSkillID( 0 ), iSkillLevel( 0 ), bCurrentLock( true )
		{

		};

		S_POSSESSED_SKILL_INFO( int argiSkillID, int argiSkillLevel, bool argbCurrentLock ) : iSkillID(argiSkillID), 
																					iSkillLevel(argiSkillLevel), bCurrentLock(argbCurrentLock)
		{

		};
	};

	// Case that should never be called in the first place.
	enum ERROR_CODE
	{
		E_NONE = 0,					// Not an error
		E_CANT_FIND_SKILLNODE,		// Cannot find skill tree node (including job mismatch cases).
		E_MISMATCH_JOB,
		E_INVALID_CALL,				// Invalid call
	};

	// 
	enum RESULT_CODE
	{
		R_SUCCESS,
		R_NOT_ENOUGH_CHAR_LEVEL,			// Character level requirement not met.
		R_INVALID_SKILLBOOK_ITEMID,			// Skill book item ID does not match.
		R_DONT_HAVE_PARENT_SKILL,			// No prerequisite (parent) skill.
		R_LOCKED_PARENTSKILL,				// Parent skill is locked.
		R_NOT_ENOUGH_PARENT_SKILL_LEVEL,	// Parent skill level requirement not met.
		R_NOT_ENOUGH_SKILLPOINT_TO_ACQUIRE,	// Cannot acquire skill due to insufficient skill points.
		R_EXCLUSIVE_SKILL,
		R_ONLY_SKILL_BOOK,					// Skill that can only be learned via a skill book.
		R_FAIL,
		R_ERROR,
	};

#if defined( PRE_ADD_PRESET_SKILLTREE )
	enum
	{
		E_ADVICE_SKILL_LEVEL_COUNT = 3
	};
#endif	// #if defined( PRE_ADD_PRESET_SKILLTREE )

	// Currently acquired skills and skill book usage info.
	struct S_TRY_UNLOCK
	{
		int iCurrentCharLevel;
		int iTryUnlockSkillID;					// Skill ID to obtain
		int iSkillBookItemID;					// ItemID of the skill book in use
		int iJobID;								// Current class. Think about how to handle unlocking previous skills upon class advancement.
		//const vector<S_POSSESSED_SKILL_INFO>& vlPossessedSkillInfo; // Possessed skill info. (Avoid unnecessary copies)
	
		//S_CURRENT_USERSKILL_INFO( const vector<S_POSSESSED_SKILL_INFO>& _vlSkillInfo ) : 
		//							 vlPossessedSkillInfo( _vlSkillInfo ),
		//							 iCurrentCharLevel( 0 ), iSkillBookItemID( 0 ), iTryUnlockSkillID( 0 ), iJobID( 0 )
		//{

		//};

		S_TRY_UNLOCK( void ) : iCurrentCharLevel( 0 ), iTryUnlockSkillID( 0 ), 
										   iSkillBookItemID( 0 ), iJobID( 0 )
		{

		}
	};

	struct S_TRY_ACQUIRE
	{
		int iCurrentCharLevel;
		int iTryAcquireSkillID;
		int iHasSkillPoint;
		int iJobID;

		const vector<S_POSSESSED_SKILL_INFO>& vlPossessedSkillInfo;

		S_TRY_ACQUIRE( const vector<S_POSSESSED_SKILL_INFO>& _vlSkillInfo ) : 
							vlPossessedSkillInfo( _vlSkillInfo ), iCurrentCharLevel( 0 ),
																  iTryAcquireSkillID( 0 ), 
																  iHasSkillPoint( 0 ), iJobID( 0 )
		{

		};
	};

	// Request result.
	struct S_OUTPUT
	{
		RESULT_CODE eResult;
		ERROR_CODE eErrorCode;

		S_OUTPUT() : eResult( R_FAIL ), eErrorCode( E_NONE )
		{
			
		};
	};

#ifdef PRE_ADD_ONLY_SKILLBOOK
	bool IsNeedSkillBook( int iSkillID, bool& bExistSkill );
#endif // PRE_ADD_ONLY_SKILLBOOK

	// Attempt to unlock skill. Before calling this function, make sure not to call it for already unlocked skills!
	void TryUnLockSkill( /*IN*/ const S_TRY_UNLOCK& PresentInfo, /*OUT*/ S_OUTPUT* pResult );
	void TryAcquireSkill( /**/ const S_TRY_ACQUIRE& TryAcquire, /*OUT*/ S_OUTPUT* pResult );
	int FindSkillBySkillBook( int iSkillBookItemID );
#ifdef PRE_ADD_LEVELUP_GUIDE
	void GetLevelUpSkillInfo(std::vector<int>& newSkillIdList, int newLevel, int jobId);
	void GetLevelUpSkillInfo(std::vector<int>& newSkillIdList, int newLevel, const std::vector<int>& jobHistoryVec);
#endif

//#ifndef (_GAMESERVER)&(_VILLAGESERVER)
	// Tree traversal used only on client.
	int GetNodeCount( int iJobID ) const;		// Number of valid skill nodes.
	int GetSlotCount( int iJobID ) const;		// Number of slots in use. (Empty slots may exist in between)

	void GetNodeRenderInfo( int iSlotIndex, int iJobID, /*OUT*/S_NODE_RENDER_INFO* pNodeRenderInfo ) const;
//#endif

#if defined(PRE_ADD_SKILL_LEVELUP_LIMIT_BY_SP)
	void GetNeedSPValuesByJob(int nSkillID, std::vector<int>& nNeedSPValues);
#endif // PRE_ADD_SKILL_LEVELUP_LIMIT_BY_SP

	int IsParentSkill( const int nParentSkillID, const int nChildSkillID );

	void GetParentSkillData( const int nChildSkillID, std::vector< std::pair<int, int> > & vecParentSkillData );

#if defined( PRE_ADD_PRESET_SKILLTREE )
	void GetAdviceSkillTree( const int nAdviceSkill_Index, const std::vector<int> & vecJobList, std::vector< std::pair<int, BYTE> > & vecData );
#endif	// #if defined( PRE_ADD_PRESET_SKILLTREE )
};
