
#pragma once


class CPlantSeed:public TBoostMemoryPool<CPlantSeed>
{
public:

	CPlantSeed( int iItemID, DWORD dwElapsedTick, INT64 biCharacterDBID, WCHAR* pwszCharName );

	void	Process( float fDelta );

	int				GetSeedItemID() const { return m_iSeedItemID; }
	INT64			GetOwnerCharacterDBID() const { return m_biOwnerCharacterDBID; }
	const WCHAR*	GetOwnerCharacterName() const { return m_wstrOwnerCharacterName.c_str(); }
	void			MakeAttachItemInfo( TFarmAreaInfo& Info );
	DWORD			GetElapsedTimeSec() const { return m_dwElapsedTick/1000; }
	int				GetFirstAttachItemID() const { return m_mAttachItems.empty() ? 0 : (*m_mAttachItems.begin()).first; }
	int				GetConsumeWater(){ return GetElapsedTimeSec()*m_iConsumeWaterPerSec; }
	DWORD			GetRequiredTimeSec(){ return m_dwRequiredTick/1000; }
	const std::vector<int>&	GetResultItems(){ return m_vResultItems; }

	bool	bIsValid();
	bool	bIsComplete();
	bool	bIsEnoughWater();
	bool	bIsFullWater();
	bool	bIsDBCreate() const { return m_bIsDBCreate; }
	bool	bIsForceComplete() const { return m_bForceComplete; }
	
	//
	void	SetDBCreateFlag( bool bFlag ){ m_bIsDBCreate = bFlag; }
	//
	bool	AttachItem( int iItemID );
	bool	ForceAttachItem( int iItemID, int iCount );
	bool	PushResultItem( int iItemID );
	void	ResetResultItem(){ m_vResultItems.clear(); }
	void	CalcAttachItem();
	void	ResetUpdateTick(){ m_dwUpdateTick=0; }
	void	ForceComplete(){ m_dwElapsedTick = m_dwRequiredTick; m_bForceComplete = true; }

	// Check if DBUpdate is needed
	bool	CanDBUpdate();
	bool	CanAttach( const TItem* pItem, bool bFirst );

	// Cheat
	void	SkipSeed( float fMinusSec );
	bool	UpdateOwnerCharacterName (MAChangeCharacterName* pPacket);

private:

	void	_LoadData();

	bool				m_bIsDBCreate;
	int					m_iSeedItemID;
	DWORD				m_dwElapsedTick;
	DWORD				m_dwRequiredTick;
	DWORD				m_dwUpdateTick;
	int					m_iDefaultWater;		// Base water amount that this plant has
	int					m_iAttachWater;			// Added water amount
	int					m_iAttachWaterBottle;	// Added water bucket
	int					m_iConsumeWaterPerSec;	// Amount of water consumed per second
	std::map<int,int>	m_mAttachItems;
	std::vector<int>	m_vResultItems;
	bool				m_bForceComplete;

	// OwnerInfo
	INT64			m_biOwnerCharacterDBID;
	std::wstring	m_wstrOwnerCharacterName;
};
