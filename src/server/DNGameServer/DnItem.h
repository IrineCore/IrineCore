#pragma once

#include "DnActorState.h"
#include "DnState.h"
#include "MultiElement.h"
#include "DNItemTypeDef.h"

const int NUM_DISJOINT_DROP = 2;

class CDnItem : public CDnState, virtual public CMultiElement {
public:
	CDnItem( CMultiRoom *pRoom );
	virtual ~CDnItem();

	enum ItemReversionEnum {
		NoReversion,
		Equip,
		Gain,

		ItemReversionEnum_Amount,
	};
	enum ItemSkillApplyType {
		None,
		ApplyStateBlow,
		ApplySkill,
		Eternity,					// Permanent effect item
		SkillLevelUp,				// Skill level-up item
		PrefixSkill,				// Item for prefix skill
	};
	enum GlyphType {
		TemperedStatus = 1,
		TemperedSkill,
		AddSKill
	};

	struct DropItemStruct {
		int nItemID;
		int nSeed;
		char cOption;
		int nCount;
		DWORD dwUniqueID;
#if defined(PRE_ADD_STAGE_CLEAR_ENCHANT_REWARD)
		int nEnchantID;
#endif	// #if defined(PRE_ADD_STAGE_CLEAR_ENCHANT_REWARD)
	};

	struct RealDropItemStruct : public DropItemStruct
	{
		RealDropItemStruct( const DropItemStruct& DropItem, int _nRotate, int _nOwnerSessionID=-1 )
		{
			this->nItemID			= DropItem.nItemID;
			this->nSeed				= DropItem.nSeed;
			this->cOption			= DropItem.cOption;
			this->nCount			= DropItem.nCount;
			this->dwUniqueID		= DropItem.dwUniqueID;
			this->nRotate			= _nRotate;
			this->nOwnerSessionID	= _nOwnerSessionID;
#if defined(PRE_ADD_STAGE_CLEAR_ENCHANT_REWARD)
			this->nEnchantID = DropItem.nEnchantID;
#endif	// #if defined(PRE_ADD_STAGE_CLEAR_ENCHANT_REWARD)
		}
		int nRotate;
		int nOwnerSessionID;
	};

protected:
	int m_nSeed;
	int m_nClassID;

	int m_nNameStringID;
	int m_nDescriptionStringID;
	eItemTypeEnum m_ItemType;
	eItemRank m_Rank;
	ItemReversionEnum m_Reversion;
	ItemSkillApplyType m_SkillApplyType;
	int m_nItemTypeParam[2];
	int m_nLevelLimit;
	bool m_bIsCashItem;
	int m_nAmount;
	int m_nMaxOverlapCount;
	int m_nOverlapCount;
	bool m_bCanDestruction; // Whether the item is destructible
	bool m_bNeedAuthentication; // Whether the item requires auth

	bool m_bAuthentication;
	DWORD m_dwColor;

	int m_nSkillID;
	int m_nSkillLevel;
	DnSkillHandle m_hSkill;

	INT64 m_nSerialID;

	int m_nEnchantTableID;
	int m_nEnchantID;
	char m_cEnchantLevel;
	char m_cPotentialIndex;
	char m_cOptionIndex;
	bool m_bSoulBound;

	std::vector<int> m_nVecPermitJobList;

	CDnState *m_pEnchant;
	CDnState *m_pPotential;

	int m_nDisjointType;
	bool m_bCanDisjoint;
	int m_nDisjointCost;
	int m_nDisjointDropTableID[ NUM_DISJOINT_DROP ];

	DNVector(DropItemStruct) m_VecDropItemList;

	char m_cSealCount;
#if defined( PRE_FIX_69709 )
	int m_iEnchantDisjointDrop;
#endif // #if defined( PRE_FIX_69709 )

protected:
	static int GetRandomStateNumber( CMultiRoom *pRoom, int nMin, int nMax );
	static float GetRandomStateRatio( CMultiRoom *pRoom, float fMin, float fMax );
	bool IsPossibleProb( DNTableFileFormat *pSox, const char *szLabel );
	void DummyRandom( int nCount = 1 );

	void InitializeEnchant();
	void InitializeOption();
	void InitializePotential();

	// Pre-generate items to drop on disassemble
	void _GenerationDropItem( void ); 

public:
	virtual bool Initialize( int nTableID, int nSeed, char cOptionIndex = 0, char cEnchantLevel = 0, char cPotentialIndex = 0, char cSealCount = 0, bool bSoulBound = false );

	static void CalcStateValue( CMultiRoom *pRoom, CDnState *pState, int nStateType, char *szMin, char *szMax, bool bRandomValue = true, bool bResetValue = true , float fRatio = 1.0f );
#ifdef PRE_FIX_MEMOPT_ENCHANT
	static void CalcStateValue( CMultiRoom *pRoom, CDnState *pState, int nStateType, float fMin, float fMax, int nMin, int nMax, bool bRandomValue = true, bool bResetValue = true );
#endif

	// Handle cooldown and etc.
	void Process( LOCAL_TIME LocalTime, float fDelta );

	int GetClassID() { return m_nClassID; }
	int GetSeed() { return m_nSeed; }

	int GetMaxOverlapCount() { return m_nMaxOverlapCount; }
	int GetItemAmount() { return m_nAmount; }
	int GetLevelLimit() { return m_nLevelLimit; }

	void SetOverlapCount( int nValue ) { m_nOverlapCount = nValue; }
	int GetOverlapCount() { return m_nOverlapCount; }

	INT64 GetSerialID() { return m_nSerialID; }
	void SetSerialID( INT64 nSerial ) { m_nSerialID = nSerial; }

	eItemTypeEnum GetItemType() { return m_ItemType; }
	eItemRank GetItemRank() { return m_Rank; }
	ItemReversionEnum GetReversion() { return m_Reversion; }
	bool IsNeedAuthentication() { return m_bNeedAuthentication; }
	bool IsCanDestruction() { return m_bCanDestruction; }

	bool IsCashItem() { return m_bIsCashItem; }

	bool IsPermitPlayer( const std::vector<int> &nVecJobList );

	bool ActivateSkillEffect( DnActorHandle hActor );
	bool DeactivateSkillEffect();
	
	static CDnItem *CreateItem( CMultiRoom *pRoom, int nTableID, int nSeed );
	static eItemTypeEnum GetItemType( int nItemTableID );
	static eItemRank GetItemRank( int nItemTableID );

	int GetSkillID( void ) { return m_nSkillID; };
	int GetSkillLevel( void ) { return m_nSkillLevel; };

	DWORD GetColor() { return m_dwColor; }
	void SetColor( DWORD dwColor ) { m_dwColor = dwColor; }
	bool IsAuthentication() { return m_bAuthentication; }
	void SetAuthentication( bool bValue ) { m_bAuthentication = bValue; }
	// bool IsIdentity() { return m_bIdentity; }
	// void SetIdentity( bool bValue ) { m_bIdentity = bValue; }

	bool IsActiveEnchant() { return ( m_cEnchantLevel == 0 ) ? false : true; }
	bool IsActivePotential() { return ( m_cPotentialIndex == 0 ) ? false : true; }
	char GetEnchantLevel() { return m_cEnchantLevel; }
	char GetPotentialIndex() { return m_cPotentialIndex; }
	char GetOptionIndex() { return m_cOptionIndex; }
	CDnState *GetEnchantState() { return m_pEnchant; }
	CDnState *GetPotentialState() { return m_pPotential; }
	bool IsSoulbBound() const;
	char GetSealCount() const { return m_cSealCount; }
	void SetSoulBound(bool bSet) { m_bSoulBound = bSet; }

	bool CanDisjoint();
	bool CanDisjointType( int nDisjointType );
	bool CanDisjointLevel( int iUserLevel );

	int GetDisjointCost( void ) { return m_nDisjointCost; };
	void GetDisjointItems( /*IN OUT*/ DNVector(DropItemStruct)& VecDisjointItems ) { VecDisjointItems = m_VecDropItemList; };	
	int* GetTypeParam() {return m_nItemTypeParam;}

	virtual CDnItem &operator = ( TItem &e );

	bool HasPrefixSkill(int &nSkillID, int &nSkillLevel);

	int GetSkillApplyType( void ) const { return m_SkillApplyType; };
	bool HasLevelUpInfo(int &nSkillID, int &nSkillLevel, int &nSkillUsingType);
};

class IBoostPoolDnItem:public CDnItem, public TBoostMemoryPool< IBoostPoolDnItem >
{
public:
	IBoostPoolDnItem( CMultiRoom *pRoom ):CDnItem( pRoom )
	{
		// Call Initialize due to virtual inheritance constructor call order. T.T
		CMultiElement::Initialize( pRoom );
	}
	virtual ~IBoostPoolDnItem(){}
};
