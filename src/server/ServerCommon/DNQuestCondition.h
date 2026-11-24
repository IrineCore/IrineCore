#pragma once

class CDNUserBase;
/**	Quest condition check interface structure
*/
class ConditionBase 
{
public:
	bool bUse;
	virtual void Init(const std::wstring& data) { if( data.empty() ) bUse = false; }
	virtual bool Check(CDNUserBase * pUserBase) {	return false; }
	ConditionBase()
	{
		bUse = false;
	}
	virtual ~ConditionBase(){ }
};

/** Object that checks quest conditions. If conditions like required level higher than current level are not met when accepting a quest,
Cannot accept quest. This object checks such conditions.
*/ 
class QuestCondition 
{
public:
	UINT nQuestIndex;
	std::vector<ConditionBase*>	ConditionList;
	bool Check( CDNUserBase * pUserBase );

	QuestCondition();
	virtual ~QuestCondition();
};

//////////////////////////////////////////////////////////////////////////
// __HaveNormalItem
//////////////////////////////////////////////////////////////////////////
class __HaveNormalItem : public ConditionBase
{
public:
	~__HaveNormalItem()	{}

	int nItemTableNumber;
	int nItemNumber;
	virtual void Init(const std::wstring& data);
	virtual bool Check(CDNUserBase * pUserBase);
};

//////////////////////////////////////////////////////////////////////////
// __UserLevel
//////////////////////////////////////////////////////////////////////////
class __UserLevel : public ConditionBase 
{
public:
	~__UserLevel()	{}

	int nMinUserLevel;
	int nMaxUserLevel;
	virtual void Init(const std::wstring& data);
	virtual bool Check(CDNUserBase * pUserBase);

};

//////////////////////////////////////////////////////////////////////////
// __UserClass
//////////////////////////////////////////////////////////////////////////
class __UserClass : public ConditionBase 
{
public:
	~__UserClass()	{}

	std::vector<int> ClassIDList;
	virtual void Init(const std::wstring& data);
	virtual bool Check(CDNUserBase * pUserBase);
};

//////////////////////////////////////////////////////////////////////////
// __PrevQuest
//////////////////////////////////////////////////////////////////////////
class __PrevQuest : public ConditionBase 
{
public:
	~__PrevQuest()	{}

	std::vector<UINT> PrevQuestList;
	virtual void Init(const std::wstring& data);
	virtual bool Check(CDNUserBase * pUserBase);
};

//////////////////////////////////////////////////////////////////////////
// __CompleteMission
//////////////////////////////////////////////////////////////////////////
class __CompleteMission : public ConditionBase 
{
public:
	~__CompleteMission()	{}

	std::vector<int> MissionIDList;
	virtual void Init(const std::wstring& data);
	virtual bool Check(CDNUserBase * pUserBase);
};

//////////////////////////////////////////////////////////////////////////
// __Reputation_Favor
//////////////////////////////////////////////////////////////////////////
class __Reputation_Favor : public ConditionBase // Likeability value
{
public:
	~__Reputation_Favor()	{}

	int nNpcID;
	int nReputationValue_Min;
	int nReputationValue_Max;

	virtual void Init(const std::wstring& data);
	virtual bool Check(CDNUserBase * pUserBase);
};

//////////////////////////////////////////////////////////////////////////
// __Reputation_Hatred
//////////////////////////////////////////////////////////////////////////
struct __Reputation_Hatred : public ConditionBase // Dislike value
{
public:
	~__Reputation_Hatred()	{}

	int nNpcID;
	int nReputationValue_Min;
	int nReputationValue_Max;

	virtual void Init(const std::wstring& data);
	virtual bool Check(CDNUserBase * pUserBase);
};

#if defined(PRE_ADD_QUEST_CHECKCAHEITEM)
//////////////////////////////////////////////////////////////////////////
// __HaveCashlItem
//////////////////////////////////////////////////////////////////////////
class __HaveCashItem : public ConditionBase
{
public:
	~__HaveCashItem()	{}

	int nItemTableNumber;
	int nItemNumber;
	virtual void Init(const std::wstring& data);
	virtual bool Check(CDNUserBase * pUserBase);
};
#endif