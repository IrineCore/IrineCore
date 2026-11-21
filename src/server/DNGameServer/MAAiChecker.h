
#pragma once

/*#####################################################################################################################
MAAiChecker is loaded once at startup, so it must not store reference values in member variables.
Base value must depend on hActor or hTarget.
#####################################################################################################################*/

/*#####################################################################################################################
	ActorChecker
#####################################################################################################################*/

class CMAAiActorChecker
{
public:
	CMAAiActorChecker() {}
	virtual ~CMAAiActorChecker() {}

	virtual bool bIsCheck( DnActorHandle hActor, int iParam ) = 0;
	virtual bool bIsAnnounceSkill(){ return false; }
	virtual bool bIsRateChecker(){ return false; }
	virtual bool bIsWaitOrderChecker(){ return false; }
};

// Weapon check
class CMAAiActorWeaponChecker: public CMAAiActorChecker
{
public:
	CMAAiActorWeaponChecker( const UINT uiIndex):m_uiWeaponIndex(uiIndex){}
	virtual bool bIsCheck( DnActorHandle hActor, int iParam );
private:
	const UINT m_uiWeaponIndex;
};

// Check part destruction
class CMAAiActorPartsDestroyChecker:public CMAAiActorChecker
{
public:
	CMAAiActorPartsDestroyChecker( std::vector<UINT>& vTableID ):m_vPartsTableID(vTableID){}
	virtual bool bIsCheck( DnActorHandle hActor, int iParam );

private:

	std::vector<UINT>	m_vPartsTableID;
};

// Check part existence
class CMAAiActorPartsExistChecker:public CMAAiActorChecker
{
public:
	CMAAiActorPartsExistChecker( std::vector<UINT>& vTableID ):m_vPartsTableID(vTableID){}
	virtual bool bIsCheck( DnActorHandle hActor, int iParam );

private:

	std::vector<UINT>	m_vPartsTableID;
};

// Check Fly state
class CMAAiActorFlyStateChecker:public CMAAiActorChecker
{
public:
	virtual bool bIsCheck( DnActorHandle hActor, int iParam );
};

class CMAAiActorFly2StateChecker:public CMAAiActorChecker
{
public:
	virtual bool bIsCheck( DnActorHandle hActor, int iParam );
};

// Check Ground state
class CMAAiActorGroundStateChecker:public CMAAiActorChecker
{
public:
	virtual bool bIsCheck( DnActorHandle hActor, int iParam );
};

// Check UnderGround state
class CMAAiActorUnderGroundStateChecker:public CMAAiActorChecker
{
public:
	virtual bool bIsCheck( DnActorHandle hActor, int iParam );
};

// Check HPPercent
class CMAAiActorHPPercentChecker:public CMAAiActorChecker
{
public:
	CMAAiActorHPPercentChecker( const int iHPPercent ):m_iHPPercent(iHPPercent){}
	virtual bool bIsCheck( DnActorHandle hActor, int iParam );
private:
	int m_iHPPercent;
};

// Check HPPercent range (Min <= val <= Max)
class CMAAiActorHPPercentCheckerRange:public CMAAiActorChecker
{
public:
	CMAAiActorHPPercentCheckerRange( const int iHPMinPercent, const int iHPMaxPercent ):m_iHPMinPercent(iHPMinPercent),m_iHPMaxPercent(iHPMaxPercent){}
	virtual bool bIsCheck( DnActorHandle hActor, int iParam );
private:
	int m_iHPMinPercent;
	int m_iHPMaxPercent;
};

// Check SP
class CMAAiActorSPChecker:public CMAAiActorChecker
{
public:
	CMAAiActorSPChecker( const int iSP ):m_iSP(iSP){}
	virtual bool bIsCheck( DnActorHandle hActor, int iParam );
private:
	int m_iSP;
};

// Check skills currently in use
class CMAAiActorUsedSkillChecker:public CMAAiActorChecker
{
public:
#ifdef __COVERITY__
#else
#endif
	CMAAiActorUsedSkillChecker( std::vector<UINT>& vTableID ):m_vSkillTableID(vTableID){}
	virtual bool bIsCheck( DnActorHandle hActor, int iParam );
private:
	std::vector<UINT>	m_vSkillTableID;
};

// Check skills not currently in use
class CMAAiActorNotUsedSkillChecker:public CMAAiActorChecker
{
public:
#ifdef __COVERITY__
#else
#endif
	CMAAiActorNotUsedSkillChecker( std::vector<UINT>& vTableID ):m_vSkillTableID(vTableID){}
	virtual bool bIsCheck( DnActorHandle hActor, int iParam );
private:
	std::vector<UINT>	m_vSkillTableID;
};

// rate
class CMAAiActorSkillRateChecker:public CMAAiActorChecker
{
public:
	CMAAiActorSkillRateChecker( const int iRate ):m_iRate(iRate){}
	virtual bool bIsCheck( DnActorHandle hActor, int iParam );
	virtual bool bIsRateChecker(){ return true; }

private:
	int m_iRate;
};

// LimitCount
class CMAAiActorSkillLimitCountChecker:public CMAAiActorChecker
{
public:
	CMAAiActorSkillLimitCountChecker( const int iCount ):m_iLimitCount(iCount){}
	virtual bool bIsCheck( DnActorHandle hActor, int iParam );
private:
	int m_iLimitCount;
};

class CMAAiActorEncounterTickChecker:public CMAAiActorChecker
{
public:
	CMAAiActorEncounterTickChecker( const int iTick ):m_iCheckTick(iTick){}

	virtual bool bIsCheck( DnActorHandle hActor, int iParam );
private:

	int m_iCheckTick;
};

class CMAAiActorLimitTickChecker:public CMAAiActorChecker
{
public:
	CMAAiActorLimitTickChecker( const int iTick ):m_iCheckTick(iTick){}

	virtual bool bIsCheck( DnActorHandle hActor, int iParam );
private:

	int m_iCheckTick;
};

class CMAAiActorNotifyAnnounceChecker:public CMAAiActorChecker
{
public:
	CMAAiActorNotifyAnnounceChecker( const int iRate, const int iGap ):m_iRate(iRate),m_iTickGap(iGap){}
	virtual bool bIsCheck( DnActorHandle hActor, int iParam );
	virtual bool bIsAnnounceSkill(){ return true; }

private:
	int m_iRate;
	int m_iTickGap;
};

class CMAAiActorGlobalCoolTimeChecker:public CMAAiActorChecker
{
public:
	CMAAiActorGlobalCoolTimeChecker( const int iIndex ):m_iCoolTimeIndex(iIndex){}
	virtual bool bIsCheck( DnActorHandle hActor, int iParam );

private:
	int m_iCoolTimeIndex;
};

// NoAggro 
class CMAAiActorNoAggroChecker:public CMAAiActorChecker
{
public:
	virtual bool bIsCheck( DnActorHandle hActor, int iParam );
};

// WaitOrder
class CMAAiActorWaitOrderChecker:public CMAAiActorChecker
{
public:
	CMAAiActorWaitOrderChecker( const int iIndex ):m_iSkillID(iIndex){}
	virtual bool bIsWaitOrderChecker(){ return true; }
	virtual bool bIsCheck( DnActorHandle hActor, int iParam );
	
	int GetSkillID(){ return m_iSkillID; }
private:
	int m_iSkillID;
};

// SkillToggle
class CMAAiActorSkillToggleChecker:public CMAAiActorChecker
{
public:
	CMAAiActorSkillToggleChecker( const int iIndex, int iLevel ):m_iSkillID(iIndex),m_iSkillLevel(iLevel){}
	virtual bool bIsCheck( DnActorHandle hActor, int iParam );

#if defined(PRE_FIX_64312)
	int GetSkillID() { return m_iSkillID; }
#endif // PRE_FIX_64312
private:
	int m_iSkillID;
	int m_iSkillLevel;
};

// PassiveSkill
class CMAAiActorPassiveSkillChecker:public CMAAiActorChecker
{
public:
	CMAAiActorPassiveSkillChecker( const int iIndex, int iLevel ):m_iSkillID(iIndex),m_iSkillLevel(iLevel){}
	virtual bool bIsCheck( DnActorHandle hActor, int iParam );

#if defined(PRE_FIX_64312)
	int GetSkillID() { return m_iSkillID; }
#endif // PRE_FIX_64312
private:
	int m_iSkillID;
	int m_iSkillLevel;
};

/*#####################################################################################################################
	TargetActorChecker
#####################################################################################################################*/

class CMAAiTargetActorChecker
{
public:
	virtual bool bIsCheck( DnActorHandle hActor, DnActorHandle hTarget ) = 0;
	virtual ~CMAAiTargetActorChecker() {}

protected:
	int GetTargetDegree( DnActorHandle hActor, DnActorHandle hTarget, const EtVector3& AxisVector );
};

// LF
class CMAAi_LF_DirectionChecker: public CMAAiTargetActorChecker
{
public:
	virtual bool bIsCheck( DnActorHandle hActor, DnActorHandle hTarget );
};

// FL
class CMAAi_FL_DirectionChecker: public CMAAiTargetActorChecker
{
public:
	virtual bool bIsCheck( DnActorHandle hActor, DnActorHandle hTarget );
};

// FR
class CMAAi_FR_DirectionChecker: public CMAAiTargetActorChecker
{
public:
	virtual bool bIsCheck( DnActorHandle hActor, DnActorHandle hTarget );
};

// RF
class CMAAi_RF_DirectionChecker: public CMAAiTargetActorChecker
{
public:
	virtual bool bIsCheck( DnActorHandle hActor, DnActorHandle hTarget );
};

// RB
class CMAAi_RB_DirectionChecker: public CMAAiTargetActorChecker
{
public:
	virtual bool bIsCheck( DnActorHandle hActor, DnActorHandle hTarget );
};

// BR
class CMAAi_BR_DirectionChecker: public CMAAiTargetActorChecker
{
public:
	virtual bool bIsCheck( DnActorHandle hActor, DnActorHandle hTarget );
};

// BL
class CMAAi_BL_DirectionChecker: public CMAAiTargetActorChecker
{
public:
	virtual bool bIsCheck( DnActorHandle hActor, DnActorHandle hTarget );
};

// LB
class CMAAi_LB_DirectionChecker: public CMAAiTargetActorChecker
{
public:
	virtual bool bIsCheck( DnActorHandle hActor, DnActorHandle hTarget );
};

// TargetHPPercent
class CMAAiTargetActorHPPercentChecker: public CMAAiTargetActorChecker
{
public:
	CMAAiTargetActorHPPercentChecker( const int iHPPercent):m_iHPPercent(iHPPercent){}
	virtual bool bIsCheck( DnActorHandle hActor, DnActorHandle hTarget );
private:
	int m_iHPPercent;
};

// TargetBuff
class CMAAiTargetActorBuffChecker:public CMAAiTargetActorChecker
{
public:
	virtual bool bIsCheck( DnActorHandle hActor, DnActorHandle hTarget );
};

// TargetDeBuff
class CMAAiTargetActorDeBuffChecker:public CMAAiTargetActorChecker
{
public:
	virtual bool bIsCheck( DnActorHandle hActor, DnActorHandle hTarget );
};

class CMAAiTargetActorBlowChecker:public CMAAiTargetActorChecker
{
public:
	CMAAiTargetActorBlowChecker( std::vector<UINT>& vTableID, bool bIsNoBlowCheck=false ):m_vTableID(vTableID),m_bIsNoBlowCheck(bIsNoBlowCheck){}
	virtual bool bIsCheck( DnActorHandle hActor, DnActorHandle hTarget );
private:
	std::vector<UINT>	m_vTableID;
	bool	m_bIsNoBlowCheck;
};

//#################################################################################################
// MAAiCheckerManager
//#################################################################################################

class CMAAiCheckerManager
{
public:
#ifdef __COVERITY__
#else
#endif
	CMAAiCheckerManager()
	{
		m_bIgnoreRateChecker = false;
	}
	virtual ~CMAAiCheckerManager();

	// Checker
	bool			bIsActorChecker( DnActorHandle hActor, int iParam=0 ) const;
	bool			bIsTargetActorChecker( DnActorHandle hActor, DnActorHandle hTarget ) const;
	// Loader
	bool			bLoadChecker( lua_tinker::table& t );
	virtual bool	bOnAddLoadChecker( lua_tinker::table& t ) = 0;
	
	bool			bIsAnnounceSkill();
	void			IgnoreRateChecker( bool bFlag ){ m_bIgnoreRateChecker = bFlag; }

#if defined(PRE_FIX_64312)
	std::vector<CMAAiActorChecker*>& GetActorCheckerList() { return m_vActorChecker; }
#endif // PRE_FIX_64312
protected:

	std::vector<CMAAiTargetActorChecker*>	m_vTargetActorChecker;
	std::vector<CMAAiTargetActorChecker*>	m_vTargetActorDirectionChecker;
	std::vector<CMAAiActorChecker*>			m_vActorChecker;

	bool m_bIgnoreRateChecker;
};

class CMAAiActionCheckerManager:public CMAAiCheckerManager
{
public:
	virtual bool	bOnAddLoadChecker( lua_tinker::table& t );
};

class CMAAiSkillCheckerManager:public CMAAiCheckerManager
{
public:
	virtual bool	bOnAddLoadChecker( lua_tinker::table& t );
};
