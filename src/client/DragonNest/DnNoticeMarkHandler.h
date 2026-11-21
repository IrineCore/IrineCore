#pragma once

#if defined(PRE_ADD_WEEKLYEVENT)
#define _MAX_MARK_COUNT	6
#elif defined(PRE_ADD_ACTIVEMISSION)
#define _MAX_MARK_COUNT	7
#else
#define _MAX_MARK_COUNT	5
#endif

enum eMarkGroupType
{
	MARK_NONE,
	MARK_REPUTE,
	MARK_VIP_FARM,
	MARK_GUILDWAR_ALERT,
	MARK_GUILDREWARD_ALERT,
	MARK_WEEKLYEVENT_ALERT,
	MARK_ACTIVEMISSION_ALERT,
};

class CDnMarkInformer
{
public:
	CDnMarkInformer(eMarkGroupType type, int iconIdx) : m_Type(type), m_bBlink(false), m_IconIndex(iconIdx), 
		m_MarkSlotIndex(-1), m_fBlinkTime(0.f), m_nBlinkPreventTimeBack(0) {}
	virtual ~CDnMarkInformer() {}

	virtual void	OnMouseOver(float fX, float fY)	{}

	void			SetBlink(bool bBlink, float fBlinkTime = 10.f);	// set default blink time for 10 sec.
	void			SetBlinkPreventTimeBack(int nTimeBack)		{ m_nBlinkPreventTimeBack = nTimeBack; }
	void			SetMarkSlotIndex(int slotIdx)		{ m_MarkSlotIndex = slotIdx; }
	void			ProcessBlinkElapsedTime(float time)	{ m_fBlinkTime -= time; }

#ifdef PRE_ADD_ACTIVEMISSION
	virtual void Process( float time ){}
#endif // PRE_ADD_ACTIVEMISSION

	eMarkGroupType	GetGroupType() const			{ return m_Type; }
	int				GetIconIdex() const				{ return m_IconIndex; }
	float			GetBlinkTime() const			{ return m_fBlinkTime; }
	int				GetBlinkPreventTimeBack() const	{ return m_nBlinkPreventTimeBack; }

	void			ResetBlink();

	bool			IsNeedBlink() const				{ return m_bBlink; }

private:
	eMarkGroupType	m_Type;
	int				m_IconIndex;
	int				m_MarkSlotIndex;

	bool			m_bBlink;
	float			m_fBlinkTime;
	int				m_nBlinkPreventTimeBack;
};

class CDnUnionMarkInformer : public CDnMarkInformer, public TBoostMemoryPool<CDnUnionMarkInformer>
{
public:
	CDnUnionMarkInformer(eMarkGroupType type, int iconIdx, int tableId) : CDnMarkInformer(type, iconIdx), m_TableId(tableId) {}
	virtual ~CDnUnionMarkInformer() {}

	void	OnMouseOver(float fX, float fY);
	int		GetTableId() const { return m_TableId; }

private:
	int m_TableId;
};

class CDnVIPFarmMarkInformer : public CDnMarkInformer, public TBoostMemoryPool<CDnVIPFarmMarkInformer>
{
#define VIPFARM_ICON_INDEX 4
public:
	CDnVIPFarmMarkInformer(eMarkGroupType type, int iconIdx = VIPFARM_ICON_INDEX) : CDnMarkInformer(type, iconIdx) {}
	virtual ~CDnVIPFarmMarkInformer() {}

	void	OnMouseOver(float fX, float fY);
};

class CDnGuildWarMarkInformer : public CDnMarkInformer, public TBoostMemoryPool<CDnGuildWarMarkInformer>
{
#define GUILDWAR_ICON_INDEX 5
public:
	CDnGuildWarMarkInformer(eMarkGroupType type, int iconIdx = GUILDWAR_ICON_INDEX) : CDnMarkInformer(type, iconIdx) {}
	virtual ~CDnGuildWarMarkInformer() {}

	void OnMouseOver(float fX, float fY);
};

class CDnGuildRewardMarkInformer : public CDnMarkInformer, public TBoostMemoryPool<CDnGuildRewardMarkInformer>
{
#define GUILDREWARD_ICON_INDEX 6
public:
	CDnGuildRewardMarkInformer(eMarkGroupType type, int iconIdx = GUILDREWARD_ICON_INDEX) : CDnMarkInformer(type, iconIdx) {}
	virtual ~CDnGuildRewardMarkInformer() {}

	void OnMouseOver(float fX, float fY);
};

#if defined(PRE_ADD_WEEKLYEVENT)
class CDnWeeklyEventMarkInformer : public CDnMarkInformer, public TBoostMemoryPool<CDnWeeklyEventMarkInformer>
{
#define WEEKLYEENT_ICON_INDEX 7
public:
	CDnWeeklyEventMarkInformer(eMarkGroupType type, int iconIdx = WEEKLYEENT_ICON_INDEX) : CDnMarkInformer(type, iconIdx) {}
	virtual ~CDnWeeklyEventMarkInformer() {}

	void OnMouseOver(float fX, float fY);
};
#endif

#if defined(PRE_ADD_ACTIVEMISSION)
class CDnActiveMissionMarkInformer : public CDnMarkInformer
{
#define ACTIVEMESSION_ICON_INDEX 8

private:
	
	std::wstring m_strTooltip;
	int m_activemissionID;
	bool m_bOpen;
	float m_OpenTime; // Opens every 120 seconds.
	float m_CloseTime;// Display for 30 sec then close.

	float m_fX;
	float m_fY;

public:
	CDnActiveMissionMarkInformer(eMarkGroupType type, int iconIdx = ACTIVEMESSION_ICON_INDEX) : CDnMarkInformer(type, iconIdx), m_activemissionID(0), 
								m_bOpen(true), m_OpenTime(0.0f), m_CloseTime(0.0f), m_fX(0.0f), m_fY(0.0f) {}
	virtual ~CDnActiveMissionMarkInformer() {}

	void SetActiveMissionID( int id ){
		m_activemissionID = id;
	}
	int GetActivemissionID(){
		return m_activemissionID;
	}
	void SetActiveMissionTooltip( std::wstring & str ){
		m_strTooltip.assign( str );
	}
	void OnMouseOver(float fX, float fY);	

	virtual void Process( float time );

};
#endif // PRE_ADD_ACTIVEMISSION






class CDnNoticeMarkHandler
{
public:
	CDnNoticeMarkHandler();
	virtual ~CDnNoticeMarkHandler();

	void SetUpdateMarksFlag(bool bSet)	{ m_bIsUpdateMarks = bSet; }
	const std::list<CDnMarkInformer*>& GetMarkInformerList() const { return m_pMarkInfoList; }
	void OnMouseOver(int slotIndex, float fX, float fY);

	static bool CompareInformers(const CDnMarkInformer* pMark1, const CDnMarkInformer* pMark2);
	bool IsUpdateMarks() const			{ return m_bIsUpdateMarks; }
	bool IsMarkGroupTypeInInfoList(eMarkGroupType type) const;
	void RemoveInfoByGroupType(eMarkGroupType type);
	CDnMarkInformer * CDnNoticeMarkHandler::GetMarkInfo(eMarkGroupType type);

	void UpdateMarks(float fElapsedTime, CEtUITextureControl** ppControls);
	void SetUnionMarks(std::vector<int>& unionMarkTableIds);

#if defined(PRE_ADD_VIP_FARM)
	void ProcessVIPFarmMark();
#endif
	void SetGuildWarMark(bool bShow, bool bNew);


	void SetGuildRewardMark(bool bShow, bool bNew);

#if defined(PRE_ADD_WEEKLYEVENT)
	void SetWeeklyEventMark( bool bShow, bool bNew );
#endif

#ifdef PRE_ADD_ACTIVEMISSION
	void SetActiveMissionEventMark( int acMissionID, std::wstring & str, bool bShow, bool bNew );
#endif // PRE_ADD_ACTIVEMISSION

protected:
	void UpdateMarkBlink(float fElapsedTime, CEtUITextureControl** ppControls);

	std::list<CDnMarkInformer*> m_pMarkInfoList;
	bool						m_bIsUpdateMarks;
};
