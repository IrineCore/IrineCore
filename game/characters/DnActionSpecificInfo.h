#pragma once
#include "Singleton.h"
#include "SignalHeader.h"


// Detailed action-related info to use globally.
// Add and use as needed. 
// Currently a projectile signal in an arbitrary action of an arbitrary action file, 
// Holds the count of SendAction_Weapon signals,
// Read-only until server application terminates after being saved once in CDnActionBase::CheckPreSignal()
// Therefore, there is no thread synchronization.
// Used only on game server; action files must be loaded with PreLoad option when game server application starts 
// All must be loaded.
class CDnActionSpecificInfo : public CSingleton<CDnActionSpecificInfo>
{
public:
	struct S_WEAPONACTION_INFO
	{
		int iWeaponIndex;
		int iFrame;		// Frame info where weapon action signal is embedded
		std::string strActionName;
		
		S_WEAPONACTION_INFO( void ) : iWeaponIndex( 0 ), iFrame( 0 ) {};
	};

	struct S_PROJECTILE_COUNT_INFO
	{
		// Action index, projectile signal count
		map<int, int> mapMaxProjectileCountInAction;

		// Action index, weapon index used by projectile.
		map<int, multiset<int> > mapUsingProjectileWeaponTableIDs;

		// Store the frame where the projectile signal of the action is placed.
		map<int, deque<int> > mapProjectileSignalFrameOffset;

		// Extract default shoot action cooltime based on input signal (actions containing Shoot_ string)
		// Based on the input signal with the earliest frame, excluding Jump and Pick actions.
		map<int, DWORD> mapBasicShootActionCoolTime;

		// Action index, SendAction_Weapon signal count
		map<int, vector<S_WEAPONACTION_INFO> > mapSendActionWeapon;

		void Clear( void ) 
		{ 
			mapMaxProjectileCountInAction.clear();
			mapUsingProjectileWeaponTableIDs.clear();
			mapSendActionWeapon.clear();
			mapBasicShootActionCoolTime.clear();
		};
	};

	struct S_SKILLCHAIN_INPUT_INFO
	{
		// Action index that has an input signal with skill chain input enabled
		set<int> setSkillChainAction;

		// For each action, collect the target action names specified in input signals with skill chain input enabled.
		map<int, vector<string> > mapCanChainToThisAction;

		void Clear( void ) { setSkillChainAction.clear(); mapCanChainToThisAction.clear(); };
	};

	struct S_PASSIVESKILL_SIGNAL_INFO
	{
		int iSkillID;
		string strChangeActionName;
		string strEXSkillChangeActionName;
	};

	struct S_INPUTHASPASSIVESKILL_INFO
	{
		// If each action contains inputhaspassiveskill, save the required signal info.
		map<int, vector<S_PASSIVESKILL_SIGNAL_INFO> > mapPassiveSkillInfo;

		void Clear( void ) { mapPassiveSkillInfo.clear(); };
	};

	struct S_BASIC_ATTACK_INPUT_SIGNAL_INFO
	{
		string strChangeActionName;
		DWORD dwStartFrame;
		DWORD dwEndFrame;

		S_BASIC_ATTACK_INPUT_SIGNAL_INFO( void ) : dwStartFrame( 0 ), dwEndFrame( 0 ) {};
	};

	struct S_BASIC_ATTACK_INFO
	{
		map<int, vector<S_BASIC_ATTACK_INPUT_SIGNAL_INFO> > mapBasicAttackInfo;
		void Clear( void ) { mapBasicAttackInfo.clear(); };
	};

private:
	map<std::string, S_PROJECTILE_COUNT_INFO*> m_mapProjectileSignalInfoByActionFile;
	S_PROJECTILE_COUNT_INFO* m_pCurrentLoadSignalInfo;

	map<std::string, S_SKILLCHAIN_INPUT_INFO*> m_mapSkillChainActionSetByActionFile;

	map<std::string, S_INPUTHASPASSIVESKILL_INFO*> m_mapPassiveSkillInfoByActionFile;

	map<std::string, S_BASIC_ATTACK_INFO*> m_mapBasicAttackInfoByActionFile;

public:
	CDnActionSpecificInfo( void );
	~CDnActionSpecificInfo( void );

	void AddProjectileSignalInfo( const std::string& strFullPath, const S_PROJECTILE_COUNT_INFO& ProjectileSignalInfo );
	const S_PROJECTILE_COUNT_INFO* FindProjectileSignalInfo( const std::string& strFullPath );

	void AddSkillChainActionSet( const std::string& strFullPath, const S_SKILLCHAIN_INPUT_INFO& SkillChainInputInfo );
	const S_SKILLCHAIN_INPUT_INFO* FindSkillChainActionSet( const std::string& strFullPath );

	void AddPassiveSkillInfo( const std::string& strFullPath, const S_INPUTHASPASSIVESKILL_INFO& PassiveSkillInfo );
	const S_INPUTHASPASSIVESKILL_INFO* FindPassiveSkillInfo( const std::string& strFullPath );

	void AddBasicAttackActionInfo( const std::string& strFullPath, const S_BASIC_ATTACK_INFO& BasicAttackInfo );
	const S_BASIC_ATTACK_INFO* FindBasicAttackInfo( const std::string& strFullPath );
};
