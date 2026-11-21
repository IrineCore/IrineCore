#pragma once
#include "CSVWriter.h"
#include "DnDamageBase.h"
#include "Singleton.h"

// Rotha DPS meter
// Implemented quickly for now due to a rush
// Structurally, many parts would be handled cleanly if moved into a table 
// Plan to review and modify later.

#ifdef PRE_ADD_EXPORT_DPS_INFORMATION

class CDnDPSReporter  : public CSingleton<CDnDPSReporter>
{
public:

	enum eReport
	{
		MaxBlowCount = 10,
		MaxUserCount = 4,
		MaxElementCount = 4,
	};

	struct sBasicData
	{
		int nTime;                  // Time
		std::wstring strCharName;   // Character name
		std::wstring strSkillName;  // Skill name or projectile
		int nSkillLevel;            // Skill level
		int nWeaponTableID;         // Weapon / projectile table ID
		int nCostMP;                // Consumed mana

		char cAttackType;            // Damage type
		int nAttackElement;         // Element type
		INT64 nDamage;                // Damage
		int nHitProb;               // Signal damage
		int nSkillProb;             // Skill damage
		int nSkillAbsolute;         // Skill absolute value
		bool bCritical;             // Critical
		bool bCritcalRes;           // Crit resistance

		float fSkillCoolTime;       // Cooldown
		float fSkillDelay;          // Skill delay            
		int nAverageDamage;         // Average damage
		int nWeaponDamage;          // Attack power

		int nAreaMonsterCount;      // Number of nearby monsters at that time

		INT64 nMaxHP;
		int nMaxSP;
		INT64 nCurrentHP;             // Current HP
		int nCurrentSP;             // Current mana

		int arrPlayerBuff[MaxBlowCount];    // Buffs/debuffs currently applied to the player
		int arrPlayerDebuff[MaxBlowCount];

	
		void clear()
		{
			nTime = 0;                  
			strCharName.clear();
			strSkillName.clear();

			nSkillLevel = 0;          
			nWeaponTableID = 0;
			nCostMP = 0;       

			cAttackType = 0;       
			nAttackElement = 0;
			nDamage = 0;     
			nHitProb = 0;
			nSkillProb = 0;
			nSkillAbsolute = 0;
 
			bCritical = false;
			bCritcalRes = false;   

			fSkillCoolTime = 0.f;
			fSkillDelay = 0.f;
			nAverageDamage = 0;    
			nWeaponDamage = 0;   

			nAreaMonsterCount = 0;

			nMaxHP = 0;
			nMaxSP = 0;
			nCurrentHP = 0;   
			nCurrentSP = 0;   

			for(int i=0; i<MaxBlowCount; i++)
			{
				arrPlayerBuff[i] = 0;
				arrPlayerDebuff[i] = 0;
			}
		}
	};

	struct sTargetData
	{
		std::wstring strTargetName;    // Target name
		int nTargetID;                 // Target ID
		int nTargetUniqueID;           // Target unique ID
		
		INT64 nTargetHP;                 // HP
		int nTargetPhysicalDamage;     // Physical Attack
		int nTargetMagicDamage;        // Magic attack
		int nTargetPhysicalDefense;    // Physical Defense
		int nTargetMagicalDefense;     // Magic defense
		int nTargetCritical;           // Crit
		int nTargetCriticalRes;        // Crit resistance

		int arrElementalDefense[MaxElementCount];
		int arrTargetBuff[MaxBlowCount];    // Buffs/debuffs currently applied to target
		int arrTargetDebuff[MaxBlowCount];

		void clear()
		{
			strTargetName.clear();
			nTargetID = 0;   
			nTargetUniqueID = 0;
			nTargetHP = 0;
			nTargetPhysicalDamage = 0;   
			nTargetMagicDamage = 0;
			nTargetPhysicalDefense = 0; 
			nTargetMagicalDefense = 0;
			nTargetCritical = 0;
			nTargetCriticalRes = 0;   

			for(int i=0; i<MaxElementCount; i++)
				arrElementalDefense[i] = 0;

			for(int i=0; i<MaxBlowCount; i++)
			{
				arrTargetBuff[i] = 0;
				arrTargetDebuff[i] = 0;
			}
		}
	};

	CSVWriter *m_pCSVWriter_BASE; // Keep as member since info may be collected and printed.
	CSVWriter *m_pCSVWriter_SKILL;
	CSVWriter *m_pCSVWriter_USER; 

	sBasicData m_sBaseData;
	sTargetData m_sTargetData;

	std::vector<INT64> m_vecEnableUserList;
	bool m_bValidated;

public:

	CDnDPSReporter();
	virtual ~CDnDPSReporter();

	int GetEnabledUserCount();
	bool EnableUser( INT64 nDBID );
	bool IsEnabledUser( INT64 nDBID );
	void DisableAllUser();

	void StartReport();
	void EndReport();
	void ReportDPS();

	void ClearData();

	void ApplyBaseData( CDnDamageBase::SHitParam *HitParam , float fDamage);
	void ApplyTargetData( DnActorHandle hActor );
	void ApplyDotDamageData(int nDamage, DnActorHandle hActor , DnBlowHandle hFromBlow );
	
	void ApplyAreaMonsterCount(int nCount);

	void ReportBaseInfo();
	void ReportTargetInfo();
	void ReportUserInfo( DnActorHandle hActor );
	void ReportSkillInfo( DnSkillHandle hSkill ,  int nAreaMonsterCount );

	void MakeBaseColumnName(std::wstring &str);
	void MakeTargetColumnName(std::wstring &str);
	void MakeUserInfoColumName(std::wstring &str);
	void MakeSkillInfoColumName(std::wstring &str);

};

extern CDnDPSReporter g_DPSReporter;

#endif