#pragma once

#ifdef PRE_MOD_ITEM_COMPOUND_DIALOG_RENEWAL

class CDnCompoundBase : public CEtUIDialog
{
public:
	CDnCompoundBase( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL, bool bAutoCursor = false );
	virtual ~CDnCompoundBase();

	enum eCompoundTabType
	{
		eSetItem = 0,
		eSuffix,
		eArtfice,
		eChandlery,
#if defined(PRE_ADD_TALISMAN_SYSTEM)
		eTalisman,
#endif

		eMaxCompoundTypeCount,

	};

	enum 
	{
		COMPOUND_SETITEM_DIALOG ,
		COMPOUND_SUFFIX_DIALOG,
		COMPOUND_ARTFICE_DIALOG,
		COMPOUND_CHANDLERY_DIALOG,
#if defined(PRE_ADD_TALISMAN_SYSTEM)
		COMPOUND_TALISMAN_DIALOG,
#endif
		MAX_COMPOUND_DIALOG_COUNT,
	};

	enum{
		COMPOUND_MESSAGE_DIALOG = 18,
		COMPOUND_ITEMLIST_DIALOG = 19,
	};

	enum
	{
		WARRIOR = 0,
		ARCHER,
		SOCERESS,
		CLERIC,
		ACADEMIC,
		Kali,

		PLAYER_MAX
	};

	enum 
	{
		eRARE = 2,
		eEPIC = 3,
		eUNIQUE = 4,
		eLEGEND = 5,

		eMAXRANK = 5, 
	};

	enum 
	{
		MAXLEVEL = 8,
		MAXSUFFIXSIZE = 14,
		MAXPVPSUFFIXSIZE = 6,
	};

	struct ItemGroupStruct
	{
		int nGroupTableID;				// Tab classification 
		int nCompoundGroupID;			
		int nCompoundMainCategory;		// Current main tab category (Set, Suffix, Technique, Misc) 
		int nCompoundGroupType;			// Classifies equipment parts with this (main weapon 1, main weapon 2, upper, lower...).
		int	nPartsID;					// Unique part ID (greatsword, axe, etc.) << UIString key value 
		int nItemID;
		int nLevel;
		int nRank;
		int nJob;
		int nActivateLevel;
		bool bIsPreview;
		int nPreviewDescUIStringID;

		int nCompoundSetItemID;			// Unique identifier ID of the set item << UIString key value 

		int	nCompoundSuffixID;			// Suffix ID 
		int nSuffixGrade;				// Suffix tier 

		int nCompooudSkillID;			// Skill ID 

		std::vector<int> nVecCompoundList;
		std::vector<int> nVecResultCompoundList;
	};

	struct SetItemStruct
	{
		int nSetIndex;
		std::vector< CDnCompoundBase::ItemGroupStruct* > m_SetItemList;	

		SetItemStruct( int nIndex ) : nSetIndex( nIndex ) {}
		~SetItemStruct(){}

		void Add( CDnCompoundBase::ItemGroupStruct* pGroupData )
		{	
			m_SetItemList.push_back( pGroupData );
		}

	};

	struct RankStruct
	{
		int nRare;
		int nEpic;
		int nUnique;
		int nLegend;
	};

	struct PartsItemStruct
	{
		int nJob;
		std::multimap<int, int> mutimapEquipList;

		PartsItemStruct( int Job ) : nJob( Job ){}

		void Reset()
		{
			nJob = -1;
			mutimapEquipList.clear();
		}
	};

	struct FilterTypeStruct
	{
		int nID;
		int nUniqueID;
		int nLevel;
		int nRank;
	};

public:

	virtual void SetCompoundTable( int nType ) = 0;
	virtual void UpdateGroupList( std::vector< CDnCompoundBase::ItemGroupStruct* >& pVecResultList ) = 0;
	virtual void SetRemoteCompoundDlg( CDnCompoundBase::ItemGroupStruct& CompoundGroupData ) = 0;
	virtual void Reset() = 0;
};

#endif 