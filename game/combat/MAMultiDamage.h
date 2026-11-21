#pragma once

#include "MASingleDamage.h"


class MonsterParts
{
public:

#if defined( PRE_ADD_LOTUSGOLEM )
	enum ePartsState
	{
		eNormal		= 0,		// Normal state
		eIgnore		= 1,		// Ignore parts(?) state
		eNoDamage	= (1<<1),	// In this state, treat as a whiff(?) on the client
	};
#endif // #if defined( PRE_ADD_LOTUSGOLEM )

	struct _PartsInfo 
	{
		_PartsInfo( const char* pszMeshName, const char* pszBoneName, bool bApplyEffect ):szMeshName(pszMeshName),szBoneName(pszBoneName),bApplyDeadEffect(bApplyEffect){}

		std::string szBoneName;
		std::string szMeshName;
		bool bApplyDeadEffect;
	};

	struct _HitCondition
	{
		int iRequiredPartsID;
		int iRequiredPartsUnderHP;
	};

	struct  _Info
	{
		UINT	uiMonsterPartsTableID;
#if defined( PRE_ADD_LOTUSGOLEM )
		int	PartsState;
#endif // #if defined( PRE_ADD_LOTUSGOLEM )
		int		nHP;				// Parts HP
		int		nHPRefreshTimems;	// HP recovery interval (ms)
		float	fMainDamageRate;	// Main body damage ratio
		float	fDefenseRate;		// Defense ratio
		int		nPassiveSkillIndex;	// Parts skill effect
		int		nPassiveSkillLevel;	// Parts skill level
		bool	bIsRefreshUseSkill;	// Flag for whether to auto-create nPassiveSkillIndex when parts regenerate
		int		nDestroySkillIndex;	// Skill index to use when destroyed
		std::string szDeadSkinName;
		std::string szDeadAniName;
		std::string szDeadActName;
		std::string szAction;
		std::string szDeadActorActName;
		std::string szDeadActorActBoneName;
		std::string szDeadActorFixBoneName;

#ifndef _GAMESERVER
		std::wstring wszPartsName;
		bool m_bEnalbeUI;
#endif

#if defined( _GAMESERVER )
		std::vector<UINT>			vRebirthPartsTableID;
#endif
		DNVector(std::string)		vLimitAction;	// Action restriction
		DNVector(_PartsInfo)		vParts;
		DNVector(_HitCondition)		vHitCondition;

		_Info()
		{
			uiMonsterPartsTableID = 0;
			nHP = 0;
			nHPRefreshTimems = 0;
			fMainDamageRate = 0.f;
			fDefenseRate = 0.f;
			nPassiveSkillIndex = 0;	
			nPassiveSkillLevel = 0;
			bIsRefreshUseSkill = false;
			nDestroySkillIndex = 0;
#ifndef _GAMESERVER
			m_bEnalbeUI = false;
#endif


#if defined( PRE_ADD_LOTUSGOLEM )
			PartsState	= eNormal;
#endif // #if defined( PRE_ADD_LOTUSGOLEM )
		}
	};

	MonsterParts();
	~MonsterParts();

	bool			Process( const float fDelta );
	bool			Create( const _Info& info );
	bool			CreateSkill( DnActorHandle hActor );
	bool			HasBone( const char* pszBoneName );
	bool			IsLimitAction( const char* pszActionName );
	int             GetMaxHP(){ return m_OriginalInfo.nHP; }
	int				GetHP(){ return m_Info.nHP; }
	void			SetHP( const int nHP ){ m_Info.nHP = nHP; }
	void			SetRefresh( DnActorHandle hActor );
	void			SetDestroy( DnActorHandle hActor );
	void			ResetSetting() { m_Info = m_OriginalInfo; }
	void			ResetDestoryDelta(){ m_fDestroyDelta = 0.f; }
	//
	float			GetDefenseRate(){ return m_Info.fDefenseRate; }
	float			GetMainDamageRate(){ return m_Info.fMainDamageRate; }
	int				GetDestroySkillIndex(){ return m_Info.nDestroySkillIndex; }	// Skill index to use when parts are destroyed
	const _Info&	GetPartsInfo() const { return m_Info; }
	const _Info&	GetPartsOriginalInfo() const { return m_OriginalInfo; }
	UINT			GetPartsTableID() const { return m_Info.uiMonsterPartsTableID; }
	DnSkillHandle	GetSkillHandle(){ return m_hSkill; }

#ifndef _GAMESERVER
	std::wstring GetPartsName(){return m_Info.wszPartsName;}
	bool IsEnalbeUIParts(){return m_Info.m_bEnalbeUI;}
#endif

#if defined( PRE_ADD_LOTUSGOLEM )
	bool CheckPartsState( int State ){ return m_Info.PartsState&State ? true : false; }
	void SetIgnore( bool bFlag )
	{
		if( bFlag )
			m_Info.PartsState |= ePartsState::eIgnore;
		else
			m_Info.PartsState &= ~ePartsState::eIgnore;
	}
	void SetNoDamage( bool bFlag )
	{ 
		if( bFlag )
			m_Info.PartsState |= ePartsState::eNoDamage;
		else
			m_Info.PartsState &= ~ePartsState::eNoDamage;
	}
#endif // #if defined( PRE_ADD_LOTUSGOLEM )

private:

	_Info			m_OriginalInfo;
	_Info			m_Info;
	float			m_fDestroyDelta;
	DnSkillHandle	m_hSkill;
};


class MAMultiDamage : virtual public MASingleDamage
{
public:
	MAMultiDamage() { }
	virtual ~MAMultiDamage() {}


	void LoadMultiDamageInfo();
	inline int GetPartsSize() { return (int)m_Parts.size(); }

	inline void ResetAllParts() 
	{
		for ( size_t i = 0 ; i < m_Parts.size() ; i++ )
		{
			m_Parts[i].ResetSetting();
		}
	}

	virtual void		OnSetPartsHP( DnActorHandle hActor, const int iPartsTableID, const int iCurHP , const int iHitterID);

	MonsterParts*		GetParts( const UINT uiTableID );
	MonsterParts*		GetPartsByIndex(int nIndex);
#ifdef PRE_ADD_MONSTER_PARTS_UI_TRIGGER
	int					GetPartsIndexFromPartsID( const UINT uiTableID );
#endif
private:
	bool	_AddMonsterParts( const MonsterParts::_Info& info );
	
	void	_OnRefreshParts( DnActorHandle hActor, MonsterParts* pParts, int nPartsIndex );
	void	_OnDestroyParts( DnActorHandle hActor, MonsterParts* pParts, int nPartsIndex );


protected:
	DNVector(MonsterParts)	m_Parts;
};
