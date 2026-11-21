#include "stdafx.h"
#include "DnAIBotBuildData.h"
#include "DNUserSession.h"
#include "DNGameRoom.h"
#include "DNUserItem.h"
#include "DNGameDataManager.h"
#include "DnTableDB.h"
#include "DnParts.h"
#include "DnWeapon.h"
#include "DnGlyph.h"
#include "DnSkill.h"

#include <map>
#include <set>
#include <algorithm>

extern CDNGameDataManager* g_pDataManager;

namespace AIBot
{
	static const BotJobSpec g_BotJobSpecs[] =
	{
		// Core Tank
		{ 41, 20, 4, RoleType::Tank,       CDnWeapon::Flail,     CDnWeapon::Shield,   L"Bot_Guardian"  },
		{ 26, 12, 1, RoleType::Tank,       CDnWeapon::Hammer,    CDnWeapon::Gauntlet, L"Bot_Destroyer" },
		// Core Healer / Cure
		{ 43, 22, 4, RoleType::Healer,     CDnWeapon::Wand,      CDnWeapon::Shield,   L"Bot_Saint"     },
		{ 51, 49, 5, RoleType::Healer,     CDnWeapon::BubbleGun, CDnWeapon::Glove,    L"Bot_Physician" },
		// Core Ice Stacker
		{ 36, 17, 3, RoleType::IceStacker, CDnWeapon::Staff,     CDnWeapon::Orb,      L"Bot_Elestra"   },
		// Core Buffers / Support
		{ 32, 15, 2, RoleType::Support,    CDnWeapon::SmallBow,  CDnWeapon::Arrow,    L"Bot_Windwlkr"  },
		{ 59, 57, 6, RoleType::Support,    CDnWeapon::Chakram,   CDnWeapon::Charm,    L"Bot_SpirDncr"  },
		{ 37, 18, 3, RoleType::Support,    CDnWeapon::Staff,     CDnWeapon::Book,     L"Bot_Smasher"   },
		{ 38, 18, 3, RoleType::Support,    CDnWeapon::Staff,     CDnWeapon::Puppet,   L"Bot_Majesty"   },
		{ 56, 54, 6, RoleType::Support,    CDnWeapon::Fan,       CDnWeapon::Charm,    L"Bot_SoulEater" },
		// Core DPS
		{ 48, 46, 5, RoleType::Dps,        CDnWeapon::Cannon,    CDnWeapon::Glove,    L"Bot_GearMstr"  },
		{ 44, 22, 4, RoleType::Dps,        CDnWeapon::Wand,      CDnWeapon::Shield,   L"Bot_Inquis"    },
		{ 24, 11, 1, RoleType::Dps,        CDnWeapon::Sword,     CDnWeapon::Gauntlet, L"Bot_Moonlord"  },
		{ 29, 14, 2, RoleType::Dps,        CDnWeapon::BigBow,    CDnWeapon::Arrow,    L"Bot_Sniper"    },
		{ 35, 17, 3, RoleType::Dps,        CDnWeapon::Staff,     CDnWeapon::Orb,      L"Bot_Saleana"   },
		{ 47, 46, 5, RoleType::Dps,        CDnWeapon::Cannon,    CDnWeapon::Glove,    L"Bot_ShootStr"  },
		{ 23, 11, 1, RoleType::Dps,        CDnWeapon::Sword,     CDnWeapon::Gauntlet, L"Bot_Gladiator" },
		{ 25, 12, 1, RoleType::Dps,        CDnWeapon::Axe,       CDnWeapon::Gauntlet, L"Bot_Barbarian" },
		{ 42, 20, 4, RoleType::Dps,        CDnWeapon::Mace,      CDnWeapon::Shield,   L"Bot_Crusader"  },
		{ 30, 14, 2, RoleType::Dps,        CDnWeapon::CrossBow,  CDnWeapon::Arrow,    L"Bot_Artillery" },
		{ 31, 15, 2, RoleType::Dps,        CDnWeapon::SmallBow,  CDnWeapon::Arrow,    L"Bot_Tempest"   },
		{ 50, 49, 5, RoleType::Dps,        CDnWeapon::BubbleGun, CDnWeapon::Glove,    L"Bot_Adept"     },
		{ 55, 54, 6, RoleType::Dps,        CDnWeapon::Fan,       CDnWeapon::Charm,    L"Bot_DarkSumnr" },
		{ 58, 57, 6, RoleType::Dps,        CDnWeapon::Chakram,   CDnWeapon::Charm,    L"Bot_BladeDncr" },
	};

	const BotJobSpec* FindJobSpec(BYTE cJobID)
	{
		for (size_t i = 0; i < _countof(g_BotJobSpecs); ++i)
		{
			if (g_BotJobSpecs[i].cSecondJob == cJobID || g_BotJobSpecs[i].cFirstJob == cJobID)
				return &g_BotJobSpecs[i];
		}
		for (size_t i = 0; i < _countof(g_BotJobSpecs); ++i)
		{
			if (g_BotJobSpecs[i].cBaseClass == cJobID)
				return &g_BotJobSpecs[i];
		}
		return NULL;
	}

	RoleType GetJobRole(BYTE cJobID)
	{
		const BotJobSpec* pSpec = FindJobSpec(cJobID);
		return pSpec ? pSpec->Role : RoleType::Dps;
	}

	void SelectSquadSpecs(BYTE cHumanJobID, int nBotCount, std::vector<const BotJobSpec*>& outSpecs)
	{
		outSpecs.clear();
		if (nBotCount <= 0)
			return;

		const BotJobSpec* pHumanSpec = FindJobSpec(cHumanJobID);
		RoleType humanRole = pHumanSpec ? pHumanSpec->Role : RoleType::Dps;

		std::set<BYTE> usedSecondJobs;
		if (pHumanSpec)
			usedSecondJobs.insert(pHumanSpec->cSecondJob);

		auto tryAddJob = [&](BYTE cSecondJob) -> bool {
			if ((int)outSpecs.size() >= nBotCount)
				return false;
			if (usedSecondJobs.count(cSecondJob))
				return false;
			for (size_t i = 0; i < _countof(g_BotJobSpecs); ++i)
			{
				if (g_BotJobSpecs[i].cSecondJob == cSecondJob)
				{
					usedSecondJobs.insert(cSecondJob);
					outSpecs.push_back(&g_BotJobSpecs[i]);
					return true;
				}
			}
			return false;
		};

		// Priority order for Cap 60 4-man & 8-man nest meta:
		// 1. Tank (Guardian or Destroyer)
		// 2. Main Healer (Saint)
		// 3. Ice Stacker (Elestra)
		// 4. Support / Buff / Off-Healer (Physician, Spirit Dancer / Windwalker, Smasher)
		// 5. High DPS mix (GearMaster, Inquisitor, Moonlord, Sniper, Saleana, ShootingStar, Barbarian, DarkSummoner...)
		if (humanRole != RoleType::Tank)
		{
			if (!tryAddJob(41)) // Guardian
				tryAddJob(26);  // Destroyer
		}
		if (humanRole != RoleType::Healer)
		{
			if (!tryAddJob(43)) // Saint
				tryAddJob(51);  // Physician
		}
		if (humanRole != RoleType::IceStacker)
		{
			tryAddJob(36); // Elestra
		}

		static const BYTE kMetaPriority[] = {
			51, // Physician (Heals + Cure + Disease)
			32, // Windwalker (Spirit Boost)
			59, // Spirit Dancer (Genie + Dance buffs)
			48, // GearMaster (Towers + Ducks)
			37, // Smasher (Beyond Time + Laser)
			44, // Inquisitor (Curative / Consecration / Burst)
			24, // Moonlord
			29, // Sniper
			35, // Saleana
			26, // Destroyer (Armor Break + Howl)
			47, // Shooting Star
			55, // Dark Summoner
			23, // Gladiator
			25, // Barbarian
			42, // Crusader
			56, // Soul Eater
			38, // Majesty
			58, // Blade Dancer
			30, // Artillery
			31, // Tempest
			50, // Adept
			41, // Guardian
			43, // Saint
			36, // Elestra
		};

		for (size_t i = 0; i < _countof(kMetaPriority) && (int)outSpecs.size() < nBotCount; ++i)
			tryAddJob(kMetaPriority[i]);

		// Safety fallback if somehow nBotCount exceeds table
		while ((int)outSpecs.size() < nBotCount)
			outSpecs.push_back(&g_BotJobSpecs[(int)outSpecs.size() % _countof(g_BotJobSpecs)]);
	}

	namespace
	{
		bool ParseJobListMatch(const TItemData* pItem, const BotJobSpec* pSpec)
		{
			if (!pItem || !pSpec)
				return false;
			if (pItem->nNeedJobClassList.empty())
				return true;
			for (size_t i = 0; i < pItem->nNeedJobClassList.size(); ++i)
			{
				int j = pItem->nNeedJobClassList[i];
				if (j == 0 || j == pSpec->cBaseClass || j == pSpec->cFirstJob || j == pSpec->cSecondJob)
					return true;
			}
			return false;
		}

		bool DoesJobHierarchyInclude(const BotJobSpec* pSpec, int nJobID)
		{
			if (!pSpec || nJobID <= 0)
				return false;
			return (nJobID == pSpec->cBaseClass || nJobID == pSpec->cFirstJob || nJobID == pSpec->cSecondJob);
		}

		int MapPartsEnumToEquipSlot(int nParts)
		{
			switch (nParts)
			{
			case CDnParts::Helmet:   return EQUIP_HELMET;
			case CDnParts::Body:     return EQUIP_BODY;
			case CDnParts::Leg:      return EQUIP_LEG;
			case CDnParts::Hand:     return EQUIP_HAND;
			case CDnParts::Foot:     return EQUIP_FOOT;
			case CDnParts::Necklace: return EQUIP_NECKLACE;
			case CDnParts::Earring:  return EQUIP_EARRING;
			case CDnParts::Ring:     return EQUIP_RING1;
			case CDnParts::Ring2:    return EQUIP_RING2;
			default:                 return -1;
			}
		}

		int MapPartsEnumToCashSlot(int nParts)
		{
			switch (nParts)
			{
			case CDnParts::CashHelmet:   return CASHEQUIP_HELMET;
			case CDnParts::CashBody:     return CASHEQUIP_BODY;
			case CDnParts::CashLeg:      return CASHEQUIP_LEG;
			case CDnParts::CashHand:     return CASHEQUIP_HAND;
			case CDnParts::CashFoot:     return CASHEQUIP_FOOT;
			case CDnParts::CashNecklace: return CASHEQUIP_NECKLACE;
			case CDnParts::CashEarring:  return CASHEQUIP_EARRING;
			case CDnParts::CashRing:     return CASHEQUIP_RING1;
			case CDnParts::CashRing2:    return CASHEQUIP_RING2;
			case CDnParts::CashWing:     return CASHEQUIP_WING;
			case CDnParts::CashTail:     return CASHEQUIP_TAIL;
			case CDnParts::CashFaceDeco: return CASHEQUIP_FACEDECO;
			case CDnParts::CashFairy:    return CASHEQUIP_FAIRY;
			default:                     return -1;
			}
		}

		bool DoesCashSlotRequireBodySkin(int nCashSlot)
		{
			switch (nCashSlot)
			{
			case CASHEQUIP_HELMET:
			case CASHEQUIP_BODY:
			case CASHEQUIP_LEG:
			case CASHEQUIP_HAND:
			case CASHEQUIP_FOOT:
			case CASHEQUIP_WING:
			case CASHEQUIP_TAIL:
				return true;
			default:
				return false;
			}
		}

		DWORD PackColorR10G10B10(DWORD dwSource, float fIntensity)
		{
			int nR = (int)(fIntensity * LOBYTE(HIWORD(dwSource)));
			int nG = (int)(fIntensity * HIBYTE(LOWORD(dwSource)));
			int nB = (int)(fIntensity * LOBYTE(LOWORD(dwSource)));
			if (nR < 0) nR = 0; else if (nR > 1023) nR = 1023;
			if (nG < 0) nG = 0; else if (nG > 1023) nG = 1023;
			if (nB < 0) nB = 0; else if (nB > 1023) nB = 1023;
			return (DWORD)((nR << 20) | (nG << 10) | nB);
		}

		struct ClassAppearancePool
		{
			int nDefaultBody;
			int nDefaultLeg;
			int nDefaultHand;
			int nDefaultFoot;
			std::vector<int> vecFaces;
			std::vector<int> vecHairs;
			std::vector<DWORD> vecHairColors;
			std::vector<DWORD> vecEyeColors;
			std::vector<DWORD> vecSkinColors;

			ClassAppearancePool()
				: nDefaultBody(0), nDefaultLeg(0), nDefaultHand(0), nDefaultFoot(0)
			{}
		};

		struct EquipCandidate
		{
			int nItemID;
			int nRank;
			int nSetItemID;
			int nLevelLimit;
		};

		struct CashCandidate
		{
			int nItemID;
			char cEquipType;
			int nSetItemID;
		};

		class CBotCatalog
		{
		public:
			static CBotCatalog& GetInstance()
			{
				static CBotCatalog s_Instance;
				s_Instance.EnsureInitialized();
				return s_Instance;
			}

			const ClassAppearancePool* GetAppearancePool(BYTE cBaseClass) const
			{
				if (cBaseClass < 1 || cBaseClass > 10)
					return NULL;
				return &m_AppearancePool[cBaseClass];
			}

			int PickBestGear(const BotJobSpec* pSpec, int nEquipSlot, char cPreferredWeaponType, bool bRandomTieBreak, int nExcludeItemID = 0) const
			{
				if (!pSpec || nEquipSlot < 0 || nEquipSlot >= EQUIPMAX)
					return 0;

				const int nSearchSlot = (nEquipSlot == EQUIP_RING2) ? EQUIP_RING1 : nEquipSlot;
				const std::vector<EquipCandidate>& pool = m_GearPool[nSearchSlot];

				int nBestScore = -1;
				std::vector<int> bestIDs;
				for (size_t i = 0; i < pool.size(); ++i)
				{
					const EquipCandidate& cand = pool[i];
					if (nExcludeItemID > 0 && cand.nItemID == nExcludeItemID)
						continue;
					const TItemData* pItem = g_pDataManager->GetItemData(cand.nItemID);
					if (!ParseJobListMatch(pItem, pSpec))
						continue;

					int nScore = cand.nRank * 10000 + cand.nLevelLimit * 100;
					if (cand.nSetItemID > 0)
						nScore += 500;

					if (nSearchSlot == EQUIP_WEAPON1 || nSearchSlot == EQUIP_WEAPON2)
					{
						TWeaponData* pWeap = g_pDataManager->GetWeaponData(cand.nItemID);
						if (!pWeap)
							continue;
						if (nSearchSlot == EQUIP_WEAPON1 && cPreferredWeaponType >= 0 && pWeap->cEquipType == cPreferredWeaponType)
							nScore += 2000;
						else if (nSearchSlot == EQUIP_WEAPON2 && cPreferredWeaponType >= 0 && pWeap->cEquipType == cPreferredWeaponType)
							nScore += 2000;
					}

					if (nScore > nBestScore)
					{
						nBestScore = nScore;
						bestIDs.clear();
						bestIDs.push_back(cand.nItemID);
					}
					else if (nScore == nBestScore)
					{
						bestIDs.push_back(cand.nItemID);
					}
				}

				if (bestIDs.empty())
					return (nExcludeItemID > 0) ? PickBestGear(pSpec, nEquipSlot, cPreferredWeaponType, bRandomTieBreak, 0) : 0;
				return bestIDs[bRandomTieBreak ? (rand() % (int)bestIDs.size()) : 0];
			}

			int PickRandomCashSetID(const BotJobSpec* pSpec) const
			{
				if (!pSpec || pSpec->cBaseClass < 1 || pSpec->cBaseClass > 10)
					return 0;
				const std::vector<int>& sets = m_CompleteCashSets[pSpec->cBaseClass];
				if (sets.empty())
					return 0;
				return sets[rand() % (int)sets.size()];
			}

			int PickRandomCashPart(const BotJobSpec* pSpec, int nCashSlot, int nPreferredSetID = 0, int nExcludeItemID = 0) const
			{
				if (!pSpec || pSpec->cBaseClass < 1 || pSpec->cBaseClass > 10)
					return 0;
				const int nSearchSlot = (nCashSlot == CASHEQUIP_RING2) ? CASHEQUIP_RING1 : nCashSlot;
				const std::vector<CashCandidate>& pool = m_CashPartsPool[pSpec->cBaseClass][nSearchSlot];
				if (pool.empty())
					return 0;

				std::vector<int> setMatches;
				std::vector<int> anyMatches;
				for (size_t i = 0; i < pool.size(); ++i)
				{
					if (nExcludeItemID > 0 && pool[i].nItemID == nExcludeItemID)
						continue;
					const TItemData* pItem = g_pDataManager->GetItemData(pool[i].nItemID);
					if (!ParseJobListMatch(pItem, pSpec))
						continue;
					anyMatches.push_back(pool[i].nItemID);
					if (nPreferredSetID > 0 && pool[i].nSetItemID == nPreferredSetID)
						setMatches.push_back(pool[i].nItemID);
				}
				if (!setMatches.empty())
					return setMatches[rand() % (int)setMatches.size()];
				if (!anyMatches.empty())
					return anyMatches[rand() % (int)anyMatches.size()];
				if (nExcludeItemID > 0)
					return PickRandomCashPart(pSpec, nCashSlot, nPreferredSetID, 0);
				return 0;
			}

			int PickRandomCashWeapon(const BotJobSpec* pSpec, char cWeaponType) const
			{
				if (!pSpec || cWeaponType < 0 || cWeaponType >= 32)
					return 0;
				const std::vector<CashCandidate>& pool = m_CashWeaponByType[(int)cWeaponType];
				if (pool.empty())
					return 0;

				std::vector<int> jobMatches;
				std::vector<int> fallbackMatches;
				for (size_t i = 0; i < pool.size(); ++i)
				{
					const TItemData* pItem = g_pDataManager->GetItemData(pool[i].nItemID);
					if (!pItem)
						continue;
					fallbackMatches.push_back(pool[i].nItemID);
					if (ParseJobListMatch(pItem, pSpec))
						jobMatches.push_back(pool[i].nItemID);
				}
				if (!jobMatches.empty())
					return jobMatches[rand() % (int)jobMatches.size()];
				if (!fallbackMatches.empty())
					return fallbackMatches[rand() % (int)fallbackMatches.size()];
				return 0;
			}

			const std::vector<int>& GetStatGlyphPool() const { return m_StatGlyphPool; }
			const std::vector<int>& GetSkillGlyphPool() const { return m_SkillGlyphPool; }
			const std::vector<int>& GetSpecialGlyphPool() const { return m_SpecialGlyphPool; }
			const std::vector<int>& GetDragonGlyphPool() const { return m_DragonGlyphPool; }

		private:
			CBotCatalog() : m_bInitialized(false) {}

			void EnsureInitialized()
			{
				if (m_bInitialized || !g_pDataManager)
					return;

				LoadAppearanceTable();

				DNTableFileFormat* pPartsSox = GetDNTable(CDnTableDB::TPARTS);
				DNTableFileFormat* pWeaponSox = GetDNTable(CDnTableDB::TWEAPON);
				if (!pPartsSox || !pWeaponSox)
					return;

				const std::map<int, TItemData*>& itemMap = g_pDataManager->GetItemDataMap();
				for (std::map<int, TItemData*>::const_iterator it = itemMap.begin(); it != itemMap.end(); ++it)
				{
					const TItemData* pItem = it->second;
					if (!pItem)
						continue;

					if (pItem->IsCash)
					{
						IndexCashItem(pItem, pPartsSox, pWeaponSox);
						continue;
					}

					if (pItem->nType == ITEMTYPE_GLYPH)
					{
						IndexGlyphItem(pItem);
						continue;
					}

					if (pItem->cLevelLimit == 60 && pItem->cRank >= ITEMRANK_A && pItem->cRank <= ITEMRANK_SS)
					{
						IndexNormalEquipItem(pItem, pPartsSox, pWeaponSox);
					}
				}

				BuildCompleteCashSetIndex();
				m_bInitialized = true;
			}

			void LoadAppearanceTable()
			{
				DNTableFileFormat* pSox = GetDNTable(CDnTableDB::TDEFAULTCREATE);
				if (!pSox)
					return;

				char szLabel[64] = { 0 };
				for (int i = 0; i < pSox->GetItemCount(); ++i)
				{
					int nRowID = pSox->GetItemID(i);
					DNTableCell* pClassCell = pSox->GetFieldFromLablePtr(nRowID, "_ClassID");
					if (!pClassCell)
						continue;
					int nClassID = pClassCell->GetInteger();
					if (nClassID < 1 || nClassID > 10)
						continue;

					ClassAppearancePool& pool = m_AppearancePool[nClassID];
					DNTableCell* pBody = pSox->GetFieldFromLablePtr(nRowID, "_DefaultBody");
					DNTableCell* pLeg  = pSox->GetFieldFromLablePtr(nRowID, "_DefaultLeg");
					DNTableCell* pHand = pSox->GetFieldFromLablePtr(nRowID, "_DefaultHand");
					DNTableCell* pFoot = pSox->GetFieldFromLablePtr(nRowID, "_DefaultFoot");
					if (pBody) pool.nDefaultBody = pBody->GetInteger();
					if (pLeg)  pool.nDefaultLeg  = pLeg->GetInteger();
					if (pHand) pool.nDefaultHand = pHand->GetInteger();
					if (pFoot) pool.nDefaultFoot = pFoot->GetInteger();

					for (int j = 0; j < DEFAULTPARTSMAX; ++j)
					{
						sprintf_s(szLabel, "_Face%d", j + 1);
						DNTableCell* pF = pSox->GetFieldFromLablePtr(nRowID, szLabel);
						if (pF && pF->GetInteger() > 0)
							pool.vecFaces.push_back(pF->GetInteger());

						sprintf_s(szLabel, "_Hair%d", j + 1);
						DNTableCell* pH = pSox->GetFieldFromLablePtr(nRowID, szLabel);
						if (pH && pH->GetInteger() > 0)
							pool.vecHairs.push_back(pH->GetInteger());
					}

					for (int j = 0; j < DEFAULTHAIRCOLORMAX; ++j)
					{
						sprintf_s(szLabel, "_HairColor%dR", j + 1);
						DNTableCell* pR = pSox->GetFieldFromLablePtr(nRowID, szLabel);
						sprintf_s(szLabel, "_HairColor%dG", j + 1);
						DNTableCell* pG = pSox->GetFieldFromLablePtr(nRowID, szLabel);
						sprintf_s(szLabel, "_HairColor%dB", j + 1);
						DNTableCell* pB = pSox->GetFieldFromLablePtr(nRowID, szLabel);
						if (pR && pG && pB)
						{
							int r = pR->GetInteger(), g = pG->GetInteger(), b = pB->GetInteger();
							if (r > 0 || g > 0 || b > 0)
							{
								DWORD dwARGB = ((255U << 24) | (((DWORD)r & 0xff) << 16) | (((DWORD)g & 0xff) << 8) | ((DWORD)b & 0xff));
								pool.vecHairColors.push_back(PackColorR10G10B10(dwARGB, 1.1f));
							}
						}
					}

					for (int j = 0; j < DEFAULTSKINCOLORMAX; ++j)
					{
						sprintf_s(szLabel, "_SkinColor%dA", j + 1);
						DNTableCell* pA = pSox->GetFieldFromLablePtr(nRowID, szLabel);
						sprintf_s(szLabel, "_SkinColor%dR", j + 1);
						DNTableCell* pR = pSox->GetFieldFromLablePtr(nRowID, szLabel);
						sprintf_s(szLabel, "_SkinColor%dG", j + 1);
						DNTableCell* pG = pSox->GetFieldFromLablePtr(nRowID, szLabel);
						sprintf_s(szLabel, "_SkinColor%dB", j + 1);
						DNTableCell* pB = pSox->GetFieldFromLablePtr(nRowID, szLabel);
						if (pA && pR && pG && pB)
						{
							int a = (int)(pA->GetFloat() * 255.f);
							int r = (int)(pR->GetFloat() * 255.f);
							int g = (int)(pG->GetFloat() * 255.f);
							int b = (int)(pB->GetFloat() * 255.f);
							if (a > 0)
							{
								DWORD dwSkin = ((((DWORD)a & 0xff) << 24) | (((DWORD)r & 0xff) << 16) | (((DWORD)g & 0xff) << 8) | ((DWORD)b & 0xff));
								pool.vecSkinColors.push_back(dwSkin);
							}
						}
					}

					for (int j = 0; j < DEFAULTEYECOLORMAX; ++j)
					{
						sprintf_s(szLabel, "_EyeColor%dR", j + 1);
						DNTableCell* pR = pSox->GetFieldFromLablePtr(nRowID, szLabel);
						sprintf_s(szLabel, "_EyeColor%dG", j + 1);
						DNTableCell* pG = pSox->GetFieldFromLablePtr(nRowID, szLabel);
						sprintf_s(szLabel, "_EyeColor%dB", j + 1);
						DNTableCell* pB = pSox->GetFieldFromLablePtr(nRowID, szLabel);
						if (pR && pG && pB)
						{
							int r = pR->GetInteger(), g = pG->GetInteger(), b = pB->GetInteger();
							if (r > 0 || g > 0 || b > 0)
							{
								DWORD dwARGB = ((255U << 24) | (((DWORD)r & 0xff) << 16) | (((DWORD)g & 0xff) << 8) | ((DWORD)b & 0xff));
								pool.vecEyeColors.push_back(PackColorR10G10B10(dwARGB, 1.0f));
							}
						}
					}
				}
			}

			void BuildCompleteCashSetIndex()
			{
				// Find cohesive cash costume sets per class that cover Helmet, Body, Leg, Hand, and Foot
				for (int cls = 1; cls <= 6; ++cls)
				{
					std::map<int, int> setSlotMask;
					static const int kCoreSlots[5] = { CASHEQUIP_HELMET, CASHEQUIP_BODY, CASHEQUIP_LEG, CASHEQUIP_HAND, CASHEQUIP_FOOT };
					for (int s = 0; s < 5; ++s)
					{
						const std::vector<CashCandidate>& pool = m_CashPartsPool[cls][kCoreSlots[s]];
						for (size_t i = 0; i < pool.size(); ++i)
						{
							if (pool[i].nSetItemID > 0)
								setSlotMask[pool[i].nSetItemID] |= (1 << s);
						}
					}
					for (std::map<int, int>::const_iterator it = setSlotMask.begin(); it != setSlotMask.end(); ++it)
					{
						if (it->second == 0x1F)
							m_CompleteCashSets[cls].push_back(it->first);
					}
				}
			}

			void IndexNormalEquipItem(const TItemData* pItem, DNTableFileFormat* pPartsSox, DNTableFileFormat* pWeaponSox)
			{
				if (pItem->nType == ITEMTYPE_PARTS)
				{
					TPartData* pPart = g_pDataManager->GetPartData(pItem->nItemID);
					if (!pPart)
						return;
					int nSlot = MapPartsEnumToEquipSlot(pPart->nParts);
					if (nSlot < 0 || nSlot >= EQUIPMAX)
						return;
					int nSetID = pPartsSox->IsExistItem(pItem->nItemID) ? pPartsSox->GetFieldFromLablePtr(pItem->nItemID, "_SetItemID")->GetInteger() : 0;
					EquipCandidate cand = { pItem->nItemID, pItem->cRank, nSetID, pItem->cLevelLimit };
					m_GearPool[nSlot].push_back(cand);
				}
				else if (pItem->nType == ITEMTYPE_WEAPON)
				{
					TWeaponData* pWeap = g_pDataManager->GetWeaponData(pItem->nItemID);
					if (!pWeap)
						return;
					int nSetID = pWeaponSox->IsExistItem(pItem->nItemID) ? pWeaponSox->GetFieldFromLablePtr(pItem->nItemID, "_SetItemID")->GetInteger() : 0;
					EquipCandidate cand = { pItem->nItemID, pItem->cRank, nSetID, pItem->cLevelLimit };
					bool bIsMain = IsMainWeaponEquipType(pWeap->cEquipType);
					m_GearPool[bIsMain ? EQUIP_WEAPON1 : EQUIP_WEAPON2].push_back(cand);
				}
			}

			static bool HasSkinCellValue(DNTableCell* pCell)
			{
				if (!pCell)
					return false;
				return (pCell->GetInteger() > 0);
			}

			void IndexCashItem(const TItemData* pItem, DNTableFileFormat* pPartsSox, DNTableFileFormat* pWeaponSox)
			{
				// Allow both permanent and period cash items (bots receive bEternity=true)
				if (pItem->nType == ITEMTYPE_PARTS)
				{
					TPartData* pPart = g_pDataManager->GetPartData(pItem->nItemID);
					if (!pPart || !pPart->nSubPartsList.empty())
						return;
					int nCashSlot = MapPartsEnumToCashSlot(pPart->nParts);
					if (nCashSlot < 0 || nCashSlot >= CASHEQUIPMAX)
						return;
					if (!pPartsSox->IsExistItem(pItem->nItemID))
						return;

					int nSetID = 0;
					DNTableCell* pSetCell = pPartsSox->GetFieldFromLablePtr(pItem->nItemID, "_SetItemID");
					if (pSetCell)
						nSetID = pSetCell->GetInteger();

					bool bHasSkinID = false;
					DNTableCell* pSkinIdCell = pPartsSox->GetFieldFromLablePtr(pItem->nItemID, "_SkinID");
					if (pSkinIdCell && pSkinIdCell->GetInteger() > 0)
						bHasSkinID = true;

					bool bRequireBodySkin = DoesCashSlotRequireBodySkin(nCashSlot);
					int nTargetSlot = (nCashSlot == CASHEQUIP_RING2) ? CASHEQUIP_RING1 : nCashSlot;

					char szLabel[32];
					for (int cls = 1; cls <= 6; ++cls)
					{
						sprintf_s(szLabel, "_Player%d_SkinName", cls);
						bool bHasClassSkin = HasSkinCellValue(pPartsSox->GetFieldFromLablePtr(pItem->nItemID, szLabel));
						if (bRequireBodySkin && !bHasClassSkin && !bHasSkinID)
							continue;

						// For non-body accessories (Necklace/Earring/Ring/FaceDeco/Fairy), check class restriction if any
						if (!bRequireBodySkin && !pItem->nNeedJobClassList.empty())
						{
							bool bClassOk = false;
							for (size_t j = 0; j < pItem->nNeedJobClassList.size(); ++j)
							{
								int nJob = pItem->nNeedJobClassList[j];
								if (nJob == 0 || nJob == cls)
								{
									bClassOk = true;
									break;
								}
								int nBase = g_pDataManager->GetBaseClass(nJob);
								if (nBase >= 0 && (nBase + 1) == cls)
								{
									bClassOk = true;
									break;
								}
							}
							if (!bClassOk)
								continue;
						}

						CashCandidate cand = { pItem->nItemID, 0, nSetID };
						m_CashPartsPool[cls][nTargetSlot].push_back(cand);
					}
				}
				else if (pItem->nType == ITEMTYPE_WEAPON)
				{
					TWeaponData* pWeap = g_pDataManager->GetWeaponData(pItem->nItemID);
					if (!pWeap || !pWeaponSox->IsExistItem(pItem->nItemID))
						return;
					if (pWeap->cEquipType < 0 || pWeap->cEquipType >= 32)
						return;

					bool bHasSkin = HasSkinCellValue(pWeaponSox->GetFieldFromLablePtr(pItem->nItemID, "_SkinName"))
						|| HasSkinCellValue(pWeaponSox->GetFieldFromLablePtr(pItem->nItemID, "_SkinNameSub1"));
					if (!bHasSkin)
						return;

					int nSetID = 0;
					DNTableCell* pSetCell = pWeaponSox->GetFieldFromLablePtr(pItem->nItemID, "_SetItemID");
					if (pSetCell)
						nSetID = pSetCell->GetInteger();

					// Index directly by cEquipType (avoids 0-based GetBaseClass mismatch!)
					CashCandidate cand = { pItem->nItemID, pWeap->cEquipType, nSetID };
					m_CashWeaponByType[(int)pWeap->cEquipType].push_back(cand);
				}
			}

			void IndexGlyphItem(const TItemData* pItem)
			{
				int nGlyphType = g_pDataManager->GetGlyphType(pItem->nItemID);
				if (nGlyphType == CDnGlyph::Enchant)
				{
					// Include Lv.50-60 Rare/Epic/Unique Enhancement Heraldries (Lv.60 Epic ranked highest!)
					if (pItem->cLevelLimit >= 50 && pItem->cLevelLimit <= 60 && pItem->cRank >= ITEMRANK_B && pItem->cRank <= ITEMRANK_SS)
						m_StatGlyphPool.push_back(pItem->nItemID);
				}
				else if (nGlyphType == CDnGlyph::PassiveSkill)
				{
					if (pItem->cLevelLimit <= 60 && pItem->cRank >= ITEMRANK_B && pItem->cRank <= ITEMRANK_SS)
						m_SkillGlyphPool.push_back(pItem->nItemID);
				}
				else if (nGlyphType == CDnGlyph::ActiveSkill)
				{
					// Special Skill Heraldries (GLYPH_SPECIALSKILL - e.g. Manticore/Apocalypse/Sea Dragon/Typhoon Kim/Professor K!)
					if (pItem->cLevelLimit <= 60 && pItem->cRank >= ITEMRANK_B && pItem->cRank <= ITEMRANK_SS)
						m_SpecialGlyphPool.push_back(pItem->nItemID);
				}
#if defined(PRE_ADD_DRAGON_FELLOWSHIP_GLYPH)
				else if (nGlyphType == CDnGlyph::DragonFellowship)
				{
					if (pItem->cLevelLimit <= 60)
						m_DragonGlyphPool.push_back(pItem->nItemID);
				}
#endif
			}

			static bool IsMainWeaponEquipType(char cEquipType)
			{
				switch (cEquipType)
				{
				case CDnWeapon::Sword:
				case CDnWeapon::Axe:
				case CDnWeapon::Hammer:
				case CDnWeapon::SmallBow:
				case CDnWeapon::BigBow:
				case CDnWeapon::CrossBow:
				case CDnWeapon::Staff:
				case CDnWeapon::Mace:
				case CDnWeapon::Flail:
				case CDnWeapon::Wand:
				case CDnWeapon::Cannon:
				case CDnWeapon::BubbleGun:
				case CDnWeapon::Fan:
				case CDnWeapon::Chakram:
					return true;
				default:
					return false;
				}
			}

			bool m_bInitialized;
			ClassAppearancePool m_AppearancePool[11];
			std::vector<EquipCandidate> m_GearPool[EQUIPMAX];
			std::vector<CashCandidate> m_CashPartsPool[11][CASHEQUIPMAX];
			std::vector<int> m_CompleteCashSets[11];
			std::vector<CashCandidate> m_CashWeaponByType[32];
			std::vector<int> m_StatGlyphPool;
			std::vector<int> m_SkillGlyphPool;
			std::vector<int> m_SpecialGlyphPool;
			std::vector<int> m_DragonGlyphPool;
		};

		char PickBestEnchantLevel(int nEnchantID, int nDesiredLevel)
		{
			if (nEnchantID <= 0)
				return 0;
			int nMaxLv = g_pDataManager->GetEnchantMaxLevel(nEnchantID);
			int nLv = min(nDesiredLevel, nMaxLv);
			while (nLv > 0)
			{
				if (g_pDataManager->GetEnchantLevelData(nEnchantID, (char)nLv) != NULL)
					return (char)nLv;
				--nLv;
			}
			return 0;
		}

		char PickBestPotentialIndex(int nPotentialID, const BotJobSpec* pSpec)
		{
			if (nPotentialID <= 0 || !pSpec)
				return 0;
			TPotentialData* pPotential = g_pDataManager->GetPotentialData(nPotentialID);
			if (!pPotential || pPotential->pVecItemData.empty())
				return 0;

			DNTableFileFormat* pPotentialSox = GetDNTable(CDnTableDB::TPOTENTIAL);
			if (!pPotentialSox)
				return 1;

			bool bMagicClass = (pSpec->cBaseClass == 3 || pSpec->cBaseClass == 4 || pSpec->cSecondJob == 24 || pSpec->cSecondJob == 30 || pSpec->cSecondJob == 50 || pSpec->cSecondJob == 51 || pSpec->cSecondJob == 55 || pSpec->cSecondJob == 56);
			int nBestIdx = 1;
			double fBestScore = -1.0;

			for (size_t i = 0; i < pPotential->pVecItemData.size(); ++i)
			{
				int nRowID = pPotential->pVecItemData[i]->nItemID;
				if (!pPotentialSox->IsExistItem(nRowID))
					continue;
				double fScore = 0.0;
				char szLabel[32];
				for (int s = 0; s < 16; ++s)
				{
					sprintf_s(szLabel, "_State%d", s + 1);
					int nStateType = pPotentialSox->GetFieldFromLablePtr(nRowID, szLabel)->GetInteger();
					if (nStateType == -1)
						break;
					sprintf_s(szLabel, "_State%dValue", s + 1);
					double fVal = atof(pPotentialSox->GetFieldFromLablePtr(nRowID, szLabel)->GetString());
					// Score key stats: Str(0)/Agi(1)/Int(2)/Vit(3)/PAtk(4-5,25-26)/MAtk(6-7,27-28)/Crit(12,30)/MaxHP(16,32)/Final(29)
					if (nStateType == 3 || nStateType == 12 || nStateType == 16 || nStateType == 29 || nStateType == 30 || nStateType == 32)
						fScore += fVal * 2.0 + 10.0;
					else if (!bMagicClass && (nStateType == 0 || nStateType == 1 || nStateType == 4 || nStateType == 5 || nStateType == 25 || nStateType == 26))
						fScore += fVal * 3.0 + 25.0;
					else if (bMagicClass && (nStateType == 2 || nStateType == 6 || nStateType == 7 || nStateType == 27 || nStateType == 28))
						fScore += fVal * 3.0 + 25.0;
					else
						fScore += 1.0;
				}
				if (fScore > fBestScore)
				{
					fBestScore = fScore;
					nBestIdx = (int)i + 1;
				}
			}
			return (char)nBestIdx;
		}

		void BuildBotItem(int nItemID, INT64 nSerial, const BotJobSpec* pSpec, int nEnchantTarget, TItem& outItem)
		{
			memset(&outItem, 0, sizeof(TItem));
			const TItemData* pItemData = g_pDataManager->GetItemData(nItemID);
			if (!pItemData)
				return;

			outItem.nItemID = nItemID;
			outItem.nSerial = nSerial;
			outItem.wCount = 1;
			outItem.nLifespan = LIFESPANMAX;
			outItem.bEternity = true;
			outItem.nRandomSeed = (rand() << 16) ^ rand();
			if (outItem.nRandomSeed == 0)
				outItem.nRandomSeed = 1337;

			if (pItemData->nType == ITEMTYPE_WEAPON)
			{
				TWeaponData* pWep = g_pDataManager->GetWeaponData(nItemID);
				if (pWep)
					outItem.wDur = (USHORT)pWep->nDurability;
			}
			else if (pItemData->nType == ITEMTYPE_PARTS)
			{
				TPartData* pPart = g_pDataManager->GetPartData(nItemID);
				if (pPart)
					outItem.wDur = (USHORT)pPart->nDurability;
			}

			if (!pItemData->IsCash)
			{
				outItem.cLevel = PickBestEnchantLevel(pItemData->nEnchantID, nEnchantTarget);
				outItem.cOption = PickBestPotentialIndex(pItemData->nTypeParam[0], pSpec);
				outItem.cPotential = PickBestPotentialIndex(pItemData->nTypeParam[1], pSpec);
			}
		}

		void PopulateBotAppearance(CDNUserSession* pBotSession, const BotJobSpec* pSpec, int nBotSlotIndex, INT64& serialSeq)
		{
			const CBotCatalog& catalog = CBotCatalog::GetInstance();
			const ClassAppearancePool* pApp = catalog.GetAppearancePool(pSpec->cBaseClass);
			if (!pApp)
				return;

			TCharacterStatus* pStatus = pBotSession->GetStatusData();
			pStatus->nDefaultBody = pApp->nDefaultBody;
			pStatus->nDefaultLeg  = pApp->nDefaultLeg;
			pStatus->nDefaultHand = pApp->nDefaultHand;
			pStatus->nDefaultFoot = pApp->nDefaultFoot;

			DWORD dwSeed = GetTickCount() ^ ((DWORD)(nBotSlotIndex + 1) * 2654435761U) ^ (DWORD)rand();

			pStatus->dwHairColor = !pApp->vecHairColors.empty()
				? pApp->vecHairColors[(dwSeed + 3) % pApp->vecHairColors.size()]
				: PackColorR10G10B10(0xFF403020, 1.1f);
			pStatus->dwEyeColor = !pApp->vecEyeColors.empty()
				? pApp->vecEyeColors[(dwSeed + 7) % pApp->vecEyeColors.size()]
				: PackColorR10G10B10(0xFF305080, 1.0f);
			pStatus->dwSkinColor = !pApp->vecSkinColors.empty()
				? pApp->vecSkinColors[(dwSeed + 11) % pApp->vecSkinColors.size()]
				: 0xFF7F7F7F;

			TItem item;
			if (!pApp->vecFaces.empty())
			{
				int nFaceID = pApp->vecFaces[(dwSeed + 1) % pApp->vecFaces.size()];
				BuildBotItem(nFaceID, --serialSeq, pSpec, 0, item);
				pBotSession->GetItem()->SetDirectEquip(EQUIP_FACE, item);
			}
			if (!pApp->vecHairs.empty())
			{
				int nHairID = pApp->vecHairs[(dwSeed + 5) % pApp->vecHairs.size()];
				BuildBotItem(nHairID, --serialSeq, pSpec, 0, item);
				pBotSession->GetItem()->SetDirectEquip(EQUIP_HAIR, item);
			}
		}

		void PopulateBotEquipment(CDNUserSession* pBotSession, const BotJobSpec* pSpec, INT64& serialSeq)
		{
			const CBotCatalog& catalog = CBotCatalog::GetInstance();
			TItem item;

			static const int kNormalSlots[] = {
				EQUIP_HELMET, EQUIP_BODY, EQUIP_LEG, EQUIP_HAND, EQUIP_FOOT,
				EQUIP_NECKLACE, EQUIP_EARRING, EQUIP_RING1, EQUIP_RING2
			};
			int nNormalRing1ID = 0;
			for (size_t i = 0; i < _countof(kNormalSlots); ++i)
			{
				int nSlot = kNormalSlots[i];
				int nExclude = (nSlot == EQUIP_RING2) ? nNormalRing1ID : 0;
				int nItemID = catalog.PickBestGear(pSpec, nSlot, -1, nSlot >= EQUIP_NECKLACE, nExclude);
				if (nItemID > 0)
				{
					if (nSlot == EQUIP_RING1) nNormalRing1ID = nItemID;
					BuildBotItem(nItemID, --serialSeq, pSpec, 12, item);
					pBotSession->GetItem()->SetDirectEquip(nSlot, item);
				}
			}

			char cEquippedMainType = pSpec->cMainWeaponType;
			char cEquippedSubType = pSpec->cSubWeaponType;

			int nMainWeapID = catalog.PickBestGear(pSpec, EQUIP_WEAPON1, cEquippedMainType, false);
			if (nMainWeapID > 0)
			{
				BuildBotItem(nMainWeapID, --serialSeq, pSpec, 12, item);
				pBotSession->GetItem()->SetDirectEquip(EQUIP_WEAPON1, item);
				if (TWeaponData* pW = g_pDataManager->GetWeaponData(nMainWeapID))
					cEquippedMainType = pW->cEquipType;
			}

			int nSubWeapID = catalog.PickBestGear(pSpec, EQUIP_WEAPON2, cEquippedSubType, false);
			if (nSubWeapID > 0)
			{
				BuildBotItem(nSubWeapID, --serialSeq, pSpec, 12, item);
				pBotSession->GetItem()->SetDirectEquip(EQUIP_WEAPON2, item);
				if (TWeaponData* pW = g_pDataManager->GetWeaponData(nSubWeapID))
					cEquippedSubType = pW->cEquipType;
			}

			// Randomize cohesive Cash Costume Set + ALL Cash Accessories (Necklace, Earring, Ring1/2, Wing, Tail, Decal, Spirit)
			int nPreferredSetID = catalog.PickRandomCashSetID(pSpec);
			static const int kCashSlots[] = {
				CASHEQUIP_HELMET, CASHEQUIP_BODY, CASHEQUIP_LEG, CASHEQUIP_HAND, CASHEQUIP_FOOT,
				CASHEQUIP_NECKLACE, CASHEQUIP_EARRING, CASHEQUIP_RING1, CASHEQUIP_RING2,
				CASHEQUIP_WING, CASHEQUIP_TAIL, CASHEQUIP_FACEDECO, CASHEQUIP_FAIRY
			};
			int nCashRing1ID = 0;
			for (size_t i = 0; i < _countof(kCashSlots); ++i)
			{
				int nSlot = kCashSlots[i];
				int nExclude = (nSlot == CASHEQUIP_RING2) ? nCashRing1ID : 0;
				int nCashID = catalog.PickRandomCashPart(pSpec, nSlot, nPreferredSetID, nExclude);
				if (nCashID > 0)
				{
					if (nSlot == CASHEQUIP_RING1) nCashRing1ID = nCashID;
					BuildBotItem(nCashID, --serialSeq, pSpec, 0, item);
					pBotSession->GetItem()->SetDirectCashEquip(nSlot, item);
					pBotSession->SetViewCashEquipBitmap(nSlot, true);
				}
			}

			int nCashWep1 = catalog.PickRandomCashWeapon(pSpec, cEquippedMainType);
			if (nCashWep1 > 0)
			{
				BuildBotItem(nCashWep1, --serialSeq, pSpec, 0, item);
				pBotSession->GetItem()->SetDirectCashEquip(CASHEQUIP_WEAPON1, item);
				pBotSession->SetViewCashEquipBitmap(CASHEQUIP_WEAPON1, true);
			}
			int nCashWep2 = catalog.PickRandomCashWeapon(pSpec, cEquippedSubType);
			if (nCashWep2 > 0)
			{
				BuildBotItem(nCashWep2, --serialSeq, pSpec, 0, item);
				pBotSession->GetItem()->SetDirectCashEquip(CASHEQUIP_WEAPON2, item);
				pBotSession->SetViewCashEquipBitmap(CASHEQUIP_WEAPON2, true);
			}

			pBotSession->SetViewCashEquipBitmap(HIDEHELMET_BITINDEX, false);
		}

		void PopulateBotSkillsAndGlyphs(CDNUserSession* pBotSession, const BotJobSpec* pSpec, int nTargetLevel, INT64& serialSeq)
		{
			TSkillGroup* pSkillGroup = pBotSession->GetSkillData();
			memset(pSkillGroup, 0, sizeof(TSkillGroup));

			std::vector<std::pair<int, BYTE>> learnedSkills;
			std::set<int> usedExclusiveIDs;

			// Pass 1: 2nd-job skills first (ensures 2nd-job ultimate & EX choices claim ExclusiveID slots first)
			// Pass 2: 1st-job skills, then Pass 3: base-class skills
			const int jobPriority[3] = { pSpec->cSecondJob, pSpec->cFirstJob, pSpec->cBaseClass };
			for (int pass = 0; pass < 3; ++pass)
			{
				int nJobFilter = jobPriority[pass];
				int nTreeCount = g_pDataManager->GetNumSkillTreeData();
				for (int idx = 0; idx < nTreeCount; ++idx)
				{
					TSkillTreeData* pTree = g_pDataManager->GetSkillTreeDataByIndex(idx);
					if (!pTree || pTree->nSkillID <= 0)
						continue;
					TSkillData* pSkill = g_pDataManager->GetSkillData(pTree->nSkillID);
					if (!pSkill || pSkill->nNeedJobID != nJobFilter)
						continue;

					// Verify weapon compatibility when a skill explicitly restricts weapon type
					bool bWeaponMatch = (pSkill->cNeedWeaponType[0] == -1 && pSkill->cNeedWeaponType[1] == -1)
						|| (pSkill->cNeedWeaponType[0] == pSpec->cMainWeaponType || pSkill->cNeedWeaponType[0] == pSpec->cSubWeaponType)
						|| (pSkill->cNeedWeaponType[1] == pSpec->cMainWeaponType || pSkill->cNeedWeaponType[1] == pSpec->cSubWeaponType);
					if (!bWeaponMatch)
						continue;

					if (pSkill->nExclusiveID > 0 && usedExclusiveIDs.count(pSkill->nExclusiveID))
						continue;

					int nMaxLearnLv = 0;
					for (size_t l = 0; l < pSkill->vLevelDataList.size(); ++l)
					{
						const TSkillLevelData& lvData = pSkill->vLevelDataList[l];
						if (lvData.cLevelLimit <= nTargetLevel && lvData.cSkillLevel <= pSkill->nMaxLevel)
						{
							if (lvData.cSkillLevel > nMaxLearnLv)
								nMaxLearnLv = lvData.cSkillLevel;
						}
					}
					if (nMaxLearnLv <= 0)
						continue;

					if (pSkill->nExclusiveID > 0)
						usedExclusiveIDs.insert(pSkill->nExclusiveID);

					learnedSkills.push_back(std::make_pair(pSkill->nSkillID, (BYTE)nMaxLearnLv));
				}
			}

			// Sort learned skills chronologically (base -> 1st -> 2nd job) so prerequisites satisfy order checks
			std::set<int> learnedIDs;
			int nWriteIdx = 0;
			for (int pass = 2; pass >= 0 && nWriteIdx < SKILLMAX; --pass)
			{
				int nJobFilter = jobPriority[pass];
				for (size_t i = 0; i < learnedSkills.size() && nWriteIdx < SKILLMAX; ++i)
				{
					TSkillData* pSkill = g_pDataManager->GetSkillData(learnedSkills[i].first);
					if (!pSkill || pSkill->nNeedJobID != nJobFilter)
						continue;
					if (learnedIDs.count(pSkill->nSkillID))
						continue;
					learnedIDs.insert(pSkill->nSkillID);

					pSkillGroup->SkillList[nWriteIdx].nSkillID = pSkill->nSkillID;
					pSkillGroup->SkillList[nWriteIdx].cSkillLevel = learnedSkills[i].second;
					pSkillGroup->SkillList[nWriteIdx].nCoolTime = 0;
					pSkillGroup->SkillList[nWriteIdx].bLock = false;
					++nWriteIdx;
				}
			}

			// Equip ALL Heraldry Slots:
			// 1) 8 Enhancement Heraldries (GLYPH_ENCHANT1..8) - deduplicated by real nTypeParam[1] family
			// 2) 4 Skill Heraldries (GLYPH_SKILL1..4) - prioritized for 2nd/1st job learned skills
			// 3) 1 Special Skill Heraldry (GLYPH_SPECIALSKILL)
			// 4) 3 Cash Expansion Heraldries (GLYPH_CASH1..3) - unlocked & filled with extra Skill/Enhancement crests
			// 5) 4 Dragon Expedition Heraldries (GLYPH_DRAGON1..4) when PRE_ADD_DRAGON_FELLOWSHIP_GLYPH is active
			const CBotCatalog& catalog = CBotCatalog::GetInstance();
			DNTableFileFormat* pItemSox = GetDNTable(CDnTableDB::TITEM);
			bool bMagicClass = (pSpec->cBaseClass == 3 || pSpec->cBaseClass == 4 || pSpec->cSecondJob == 24 || pSpec->cSecondJob == 30 || pSpec->cSecondJob == 50 || pSpec->cSecondJob == 51 || pSpec->cSecondJob == 55 || pSpec->cSecondJob == 56);

			std::vector<std::pair<int, int>> rankedStatGlyphs;
			const std::vector<int>& statGlyphs = catalog.GetStatGlyphPool();
			for (size_t i = 0; i < statGlyphs.size(); ++i)
			{
				int nGlyphID = statGlyphs[i];
				const TItemData* pItemData = g_pDataManager->GetItemData(nGlyphID);
				if (!pItemData || !pItemSox || !pItemSox->IsExistItem(nGlyphID))
					continue;
				int nState1 = pItemSox->GetFieldFromLablePtr(nGlyphID, "_State1")->GetInteger();
				int nScore = pItemData->cLevelLimit * 1000 + pItemData->cRank * 200;
				if (nState1 == 12 || nState1 == 29 || nState1 == 3 || nState1 == 16) nScore += 80; // Crit / FD / Vit / HP
				if (!bMagicClass && (nState1 == 0 || nState1 == 1 || nState1 == 4 || nState1 == 5 || nState1 == 25 || nState1 == 26)) nScore += 120;
				if (bMagicClass && (nState1 == 2 || nState1 == 6 || nState1 == 7 || nState1 == 27 || nState1 == 28)) nScore += 120;
				rankedStatGlyphs.push_back(std::make_pair(nScore, nGlyphID));
			}
			std::sort(rankedStatGlyphs.begin(), rankedStatGlyphs.end(), [](const std::pair<int, int>& a, const std::pair<int, int>& b) {
				return a.first > b.first;
			});

			std::set<int> usedGlyphTypeParams;
			int nEnhSlot = GLYPH_ENCHANT1;
			for (size_t i = 0; i < rankedStatGlyphs.size() && nEnhSlot <= GLYPH_ENCHANT8; ++i)
			{
				int nGlyphID = rankedStatGlyphs[i].second;
				const TItemData* pItemData = g_pDataManager->GetItemData(nGlyphID);
				if (!pItemData) continue;
				int nFamily = pItemData->nTypeParam[1];
				if (nFamily > 0 && usedGlyphTypeParams.count(nFamily))
					continue;
				if (nFamily > 0) usedGlyphTypeParams.insert(nFamily);
				TItem glyphItem;
				BuildBotItem(nGlyphID, --serialSeq, pSpec, 0, glyphItem);
				pBotSession->GetItem()->SetDirectGlyph(nEnhSlot++, glyphItem);
			}

			std::vector<std::pair<int, int>> rankedSkillGlyphs;
			const std::vector<int>& skillGlyphs = catalog.GetSkillGlyphPool();
			for (size_t i = 0; i < skillGlyphs.size(); ++i)
			{
				int nGlyphID = skillGlyphs[i];
				TGlyphSkillData* pGlyphData = g_pDataManager->GetGlyphSkillData(nGlyphID);
				const TItemData* pItemData = g_pDataManager->GetItemData(nGlyphID);
				if (!pGlyphData || !pItemData || !learnedIDs.count(pGlyphData->nSkillID))
					continue;
				TSkillData* pSkData = g_pDataManager->GetSkillData(pGlyphData->nSkillID);
				int nScore = pItemData->cLevelLimit * 100 + pItemData->cRank * 50;
				if (pSkData && pSkData->nNeedJobID == pSpec->cSecondJob) nScore += 300;
				else if (pSkData && pSkData->nNeedJobID == pSpec->cFirstJob) nScore += 150;
				rankedSkillGlyphs.push_back(std::make_pair(nScore, nGlyphID));
			}
			std::sort(rankedSkillGlyphs.begin(), rankedSkillGlyphs.end(), [](const std::pair<int, int>& a, const std::pair<int, int>& b) {
				return a.first > b.first;
			});

			int nSkillSlot = GLYPH_SKILL1;
			std::set<int> usedSkillCrests;
			size_t nSkillCursor = 0;
			for (; nSkillCursor < rankedSkillGlyphs.size() && nSkillSlot <= GLYPH_SKILL4; ++nSkillCursor)
			{
				int nGlyphID = rankedSkillGlyphs[nSkillCursor].second;
				TGlyphSkillData* pGlyphData = g_pDataManager->GetGlyphSkillData(nGlyphID);
				const TItemData* pItemData = g_pDataManager->GetItemData(nGlyphID);
				if (!pGlyphData || !pItemData || usedSkillCrests.count(pGlyphData->nSkillID))
					continue;
				if (pItemData->nTypeParam[1] > 0 && usedGlyphTypeParams.count(pItemData->nTypeParam[1]))
					continue;
				usedSkillCrests.insert(pGlyphData->nSkillID);
				if (pItemData->nTypeParam[1] > 0) usedGlyphTypeParams.insert(pItemData->nTypeParam[1]);
				TItem glyphItem;
				BuildBotItem(nGlyphID, --serialSeq, pSpec, 0, glyphItem);
				pBotSession->GetItem()->SetDirectGlyph(nSkillSlot++, glyphItem);
			}

			// Special Skill Heraldry (GLYPH_SPECIALSKILL)
			const std::vector<int>& specialGlyphs = catalog.GetSpecialGlyphPool();
			int nBestSpecialID = 0;
			int nBestSpecialScore = -1;
			for (size_t i = 0; i < specialGlyphs.size(); ++i)
			{
				int nGlyphID = specialGlyphs[i];
				const TItemData* pItemData = g_pDataManager->GetItemData(nGlyphID);
				if (!pItemData || !ParseJobListMatch(pItemData, pSpec))
					continue;
				int nScore = pItemData->cLevelLimit * 100 + pItemData->cRank * 50 + (rand() % 25);
				if (nScore > nBestSpecialScore)
				{
					nBestSpecialScore = nScore;
					nBestSpecialID = nGlyphID;
				}
			}
			if (nBestSpecialID > 0)
			{
				const TItemData* pSpecItem = g_pDataManager->GetItemData(nBestSpecialID);
				if (pSpecItem && pSpecItem->nTypeParam[1] > 0) usedGlyphTypeParams.insert(pSpecItem->nTypeParam[1]);
				TItem glyphItem;
				BuildBotItem(nBestSpecialID, --serialSeq, pSpec, 0, glyphItem);
				pBotSession->GetItem()->SetDirectGlyph(GLYPH_SPECIALSKILL, glyphItem);
			}

			// Unlock & fill all 3 Extra Cash Heraldry Slots (GLYPH_CASH1..3)
			TCashGlyphData* pCashGlyphExpire = pBotSession->GetItem()->GetGlyphExpireDate();
			if (pCashGlyphExpire)
			{
				__time64_t tFuture = _time64(NULL) + 86400LL * 365LL;
				for (int c = 0; c < 3; ++c)
				{
					pCashGlyphExpire[c].cActiveGlyph = (BYTE)(c + 1);
					pCashGlyphExpire[c].tGlyphExpireDate = tFuture;
				}
			}
			int nCashGlyphSlot = GLYPH_CASH1;
			for (; nSkillCursor < rankedSkillGlyphs.size() && nCashGlyphSlot <= GLYPH_CASH3; ++nSkillCursor)
			{
				int nGlyphID = rankedSkillGlyphs[nSkillCursor].second;
				TGlyphSkillData* pGlyphData = g_pDataManager->GetGlyphSkillData(nGlyphID);
				const TItemData* pItemData = g_pDataManager->GetItemData(nGlyphID);
				if (!pGlyphData || !pItemData || usedSkillCrests.count(pGlyphData->nSkillID))
					continue;
				if (pItemData->nTypeParam[1] > 0 && usedGlyphTypeParams.count(pItemData->nTypeParam[1]))
					continue;
				usedSkillCrests.insert(pGlyphData->nSkillID);
				if (pItemData->nTypeParam[1] > 0) usedGlyphTypeParams.insert(pItemData->nTypeParam[1]);
				TItem glyphItem;
				BuildBotItem(nGlyphID, --serialSeq, pSpec, 0, glyphItem);
				pBotSession->GetItem()->SetDirectGlyph(nCashGlyphSlot++, glyphItem);
			}
			for (size_t i = 0; i < rankedStatGlyphs.size() && nCashGlyphSlot <= GLYPH_CASH3; ++i)
			{
				int nGlyphID = rankedStatGlyphs[i].second;
				const TItemData* pItemData = g_pDataManager->GetItemData(nGlyphID);
				if (!pItemData) continue;
				if (pItemData->nTypeParam[1] > 0 && usedGlyphTypeParams.count(pItemData->nTypeParam[1]))
					continue;
				if (pItemData->nTypeParam[1] > 0) usedGlyphTypeParams.insert(pItemData->nTypeParam[1]);
				TItem glyphItem;
				BuildBotItem(nGlyphID, --serialSeq, pSpec, 0, glyphItem);
				pBotSession->GetItem()->SetDirectGlyph(nCashGlyphSlot++, glyphItem);
			}

#if defined(PRE_ADD_DRAGON_FELLOWSHIP_GLYPH)
			const std::vector<int>& dragonGlyphs = catalog.GetDragonGlyphPool();
			int nDragonSlot = GLYPH_DRAGON1;
			for (size_t i = 0; i < dragonGlyphs.size() && nDragonSlot <= GLYPH_DRAGON4; ++i)
			{
				int nGlyphID = dragonGlyphs[i];
				const TItemData* pItemData = g_pDataManager->GetItemData(nGlyphID);
				if (!pItemData || !ParseJobListMatch(pItemData, pSpec))
					continue;
				if (pItemData->nTypeParam[1] > 0 && usedGlyphTypeParams.count(pItemData->nTypeParam[1]))
					continue;
				if (pItemData->nTypeParam[1] > 0) usedGlyphTypeParams.insert(pItemData->nTypeParam[1]);
				TItem glyphItem;
				BuildBotItem(nGlyphID, --serialSeq, pSpec, 0, glyphItem);
				pBotSession->GetItem()->SetDirectGlyph(nDragonSlot++, glyphItem);
			}
#endif
		}
		bool IsBotNameUsedInRoom(CDNGameRoom* pRoom, const WCHAR* wszCandidate)
		{
			if (!pRoom || !wszCandidate)
				return false;
			for (UINT i = 0; i < pRoom->GetUserCount(); ++i)
			{
				CDNUserSession* pSess = pRoom->GetUserData(i);
				if (pSess && pSess->GetCharacterName() && _wcsicmp(pSess->GetCharacterName(), wszCandidate) == 0)
					return true;
			}
			return false;
		}

		void GenerateRandomBotName(const BotJobSpec* pSpec, CDNGameRoom* pRoom, int nBotSlotIndex, WCHAR* wszOut, size_t cchOut)
		{
			static const WCHAR* kMaleNames[] = {
				L"Liam", L"Marcus", L"Kael", L"Ren", L"Dante", L"Noah", L"Leon", L"Zane",
				L"Lucas", L"Cole", L"Kai", L"Evan", L"Owen", L"Dylan", L"Asher", L"Jude",
				L"Soren", L"Caleb", L"Axel", L"Ryan", L"Theo", L"Vance", L"Nolan", L"Milo",
				L"Gavin", L"Ezra", L"Riven", L"Jayce", L"Aiden", L"Troy", L"Sora", L"Kenji",
				L"Kuro", L"Luka", L"Vex", L"Ryu", L"Jace", L"Cain", L"Finn", L"Rowan"
			};
			static const WCHAR* kFemaleNames[] = {
				L"Jane", L"Aria", L"Luna", L"Chloe", L"Maya", L"Elena", L"Yuki", L"Hana",
				L"Iris", L"Nora", L"Freya", L"Mina", L"Zoe", L"Kira", L"Claire", L"Vera",
				L"Sera", L"Lina", L"Rina", L"Faye", L"Yuna", L"Emi", L"Tessa", L"Naomi",
				L"Alice", L"Ruby", L"Vivi", L"Aoi", L"Mei", L"Rhea", L"Kanna", L"Miyu",
				L"Saya", L"Nadia", L"Celine", L"Ivy", L"Gwen", L"Nana", L"Hazel", L"Sora"
			};

			bool bMaleClass = (pSpec && (pSpec->cBaseClass == 1 || pSpec->cBaseClass == 4));
			const WCHAR** pPool = bMaleClass ? kMaleNames : kFemaleNames;
			size_t nPoolCount = bMaleClass ? _countof(kMaleNames) : _countof(kFemaleNames);

			static LONG s_NameCounter = 0;
			DWORD dwSeed = GetTickCount()
				^ (DWORD)(InterlockedIncrement(&s_NameCounter) * 2654435761U)
				^ (pRoom ? (pRoom->GetRoomID() * 2246822519U) : 0U)
				^ ((DWORD)(nBotSlotIndex + 1) * 3266489917U);
			dwSeed ^= (dwSeed >> 16);
			dwSeed *= 0x7feb352dU;
			dwSeed ^= (dwSeed >> 15);
			dwSeed *= 0x846ca68bU;
			dwSeed ^= (dwSeed >> 16);

			size_t nStart = (size_t)(dwSeed % nPoolCount);
			for (size_t attempt = 0; attempt < nPoolCount; ++attempt)
			{
				const WCHAR* pPicked = pPool[(nStart + attempt) % nPoolCount];
				swprintf_s(wszOut, cchOut, L"[BOT]%s", pPicked);
				if (!IsBotNameUsedInRoom(pRoom, wszOut))
					return;
			}

			// Fallback with slot suffix if somehow exhausted
			swprintf_s(wszOut, cchOut, L"[BOT]%s%d", pPool[nStart], nBotSlotIndex + 1);
		}
	}

	bool PopulateBotSession(CDNUserSession* pBotSession, const BotJobSpec* pSpec, int nBotSlotIndex, int nTargetLevel)
	{
		if (!pBotSession || !pSpec || !g_pDataManager)
			return false;

		pBotSession->SetAIBot(true);
		WCHAR wszBotName[NAMELENMAX] = { 0 };
		GenerateRandomBotName(pSpec, pBotSession->GetGameRoom(), nBotSlotIndex, wszBotName, _countof(wszBotName));
		pBotSession->SetCharacterName(wszBotName);

		TCharacterStatus* pStatus = pBotSession->GetStatusData();
		pStatus->cClass = (char)pSpec->cBaseClass;
		pStatus->cJob = pSpec->cSecondJob;
		pStatus->cLevel = (BYTE)nTargetLevel;
		pStatus->nExp = g_pDataManager->GetExp(pSpec->cBaseClass, (BYTE)nTargetLevel);
		memset(pStatus->cJobArray, 0, sizeof(pStatus->cJobArray));
		pStatus->cJobArray[0] = pSpec->cBaseClass;
		pStatus->cJobArray[1] = pSpec->cFirstJob;
		pStatus->cJobArray[2] = pSpec->cSecondJob;
		pStatus->wFatigue = 0;
		pStatus->cRebirthCoin = 0;

		static LONG s_BotItemSerialCounter = 0;
		INT64 serialSeq = -1000000LL * (nBotSlotIndex + 1) - (INT64)InterlockedIncrement(&s_BotItemSerialCounter) * 100LL;

		PopulateBotAppearance(pBotSession, pSpec, nBotSlotIndex, serialSeq);
		PopulateBotEquipment(pBotSession, pSpec, serialSeq);
		PopulateBotSkillsAndGlyphs(pBotSession, pSpec, nTargetLevel, serialSeq);

		pBotSession->SetLoadingComplete(true);
		pBotSession->SetSessionState(SESSION_STATE_READY_TO_SYNC);
		return true;
	}
}

#include "DNUserSession.h"
#include "DnTableDB.h"
#include "DnActor.h"
#include "DnWorld.h"

bool CDNGameRoom::HasAIBots()
{
	for( int i = 0; i < (int)m_VecMember.size(); ++i )
	{
		if( m_VecMember[i].pSession && m_VecMember[i].pSession->bIsAIBot() )
			return true;
	}
	return false;
}

int CDNGameRoom::ResolveTargetMapIndex( int nMapIdx, int nGateIdx, int nGateSelect, bool bDirectConnect )
{
	if( bDirectConnect || nMapIdx <= 0 || nGateIdx < 1 || nGateIdx > DNWORLD_GATE_COUNT )
		return nMapIdx;

	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TMAP );
	if( !pSox || !pSox->IsExistItem( nMapIdx ) )
		return nMapIdx;

	char szLabel[64];
	sprintf_s( szLabel, "_Gate%d_MapIndex_txt", nGateIdx );
	DNTableCell* pCell = pSox->GetFieldFromLablePtr( nMapIdx, szLabel );
	if( !pCell )
		return nMapIdx;

	char* pszGateMapIndexs = pCell->GetString();
	if( !pszGateMapIndexs || !pszGateMapIndexs[0] )
		return nMapIdx;

	std::vector<std::string> vGateMapIndexs;
	TokenizeA( pszGateMapIndexs, vGateMapIndexs, ";" );
	if( nGateSelect >= 0 && nGateSelect < (int)vGateMapIndexs.size() )
	{
		int nResolved = atoi( vGateMapIndexs[nGateSelect].c_str() );
		if( nResolved > 0 )
			return nResolved;
	}
	return nMapIdx;
}

int CDNGameRoom::GetTargetMapAIBotCount( int nTargetMapIdx, bool bContinueStage, int nCurrentBots, TDUNGEONDIFFICULTY StageDifficulty )
{
	if( bIsPvPRoom() || bIsFarmRoom() || nTargetMapIdx <= 0 )
		return 0;

	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TMAP );
	if( !pSox || !pSox->IsExistItem( nTargetMapIdx ) )
		return 0;

	EWorldEnum::MapTypeEnum MapType = (EWorldEnum::MapTypeEnum)pSox->GetFieldFromLablePtr( nTargetMapIdx, "_MapType" )->GetInteger();
	if( MapType != EWorldEnum::MapTypeDungeon )
		return 0;

	EWorldEnum::MapSubTypeEnum MapSubType = (EWorldEnum::MapSubTypeEnum)pSox->GetFieldFromLablePtr( nTargetMapIdx, "_MapSubType" )->GetInteger();
	if( MapSubType == EWorldEnum::MapSubTypePvPVillage || MapSubType == EWorldEnum::MapSubTypePvPLobby )
		return 0;

	// Do not summon AI bots on Easy or Normal regular stages (Hero allies assist there instead).
	// Hard/Master/Abyss stages, Nests, Dark Lair, and Chaos dungeons continue to summon AI bots.
	if( MapSubType == EWorldEnum::MapSubTypeNone &&
		( StageDifficulty == Dungeon::Difficulty::Easy || StageDifficulty == Dungeon::Difficulty::Normal ) )
		return 0;

	int nEnterTableID = pSox->GetFieldFromLablePtr( nTargetMapIdx, "_EnterConditionTableID" )->GetInteger();
	if( nEnterTableID <= 0 )
	{
		// Multi-stage sub-maps (e.g. Nest/Raid Stage 2+, Dungeon Boss rooms) omit _EnterConditionTableID; preserve squad!
		if( bContinueStage && nCurrentBots > 0 )
			return nCurrentBots;
		return 0;
	}

	DNTableFileFormat* pDungeonSOX = GetDNTable( CDnTableDB::TDUNGEONENTER );
	if( !pDungeonSOX || !pDungeonSOX->IsExistItem( nEnterTableID ) )
		return ( bContinueStage && nCurrentBots > 0 ) ? nCurrentBots : 0;

	int nMaxParty = pDungeonSOX->GetFieldFromLablePtr( nEnterTableID, "_PartyOneNumMax" )->GetInteger();
	if( nMaxParty == 1 || nMaxParty == 0 )
		return 0;
	if( nMaxParty < 0 )
		nMaxParty = 4;
	if( nMaxParty > PARTYCOUNTMAX )
		nMaxParty = PARTYCOUNTMAX;

	return nMaxParty - 1;
}

void CDNGameRoom::DespawnAndRemoveAIBots( bool bNotifyClients )
{
	if( !HasAIBots() )
		return;

	for( int i = (int)m_VecMember.size() - 1; i >= 0; --i )
	{
		CDNUserSession* pBot = m_VecMember[i].pSession;
		if( !pBot || !pBot->bIsAIBot() )
			continue;

		if( bNotifyClients )
		{
			SCPartyOut OutPkt;
			memset( &OutPkt, 0, sizeof(OutPkt) );
			OutPkt.nSessionID = pBot->GetSessionID();
			OutPkt.nRetCode = ERROR_NONE;
			OutPkt.cKickKind = -1;
			for( DWORD m = 0; m < m_VecMember.size(); ++m )
			{
				if( m_VecMember[m].pSession && !m_VecMember[m].pSession->bIsAIBot() )
					m_VecMember[m].pSession->SendPacket( SC_PARTY, eParty::SC_PARTYOUT, reinterpret_cast<char*>(&OutPkt), sizeof(OutPkt), _RELIABLE );
			}
		}

		DnActorHandle hActor = pBot->GetActorHandle();
		if( hActor )
		{
			hActor->RemoveOctreeNode();
			SAFE_RELEASE_SPTR( hActor );
			pBot->SetActorHandle( CDnActor::Identity() );
		}

		m_VecMember[i].ReleaseEquipInventory();
		CheckCurrentItemLooterIdx( &m_VecMember[i] );

		int nBotMemberIdx = pBot->GetPartyMemberIndex();
		if( nBotMemberIdx >= 0 && nBotMemberIdx < PARTYMAX && m_nPartyMemberIndex[nBotMemberIdx] == pBot->GetSessionID() )
			m_nPartyMemberIndex[nBotMemberIdx] = 0;
		pBot->SetPartyMemberIndex( -1 );

		m_UserList.remove( pBot );
		m_mCharDBIDMember.erase( pBot->GetCharacterDBID() );
		m_VecMember.erase( m_VecMember.begin() + i );

		if( m_bFinalizeRoom )
			delete pBot;
		else
			m_vDeleteSession.push_back( pBot );
	}

	m_iPartMemberCnt = (int)m_VecMember.size();
}

bool CDNGameRoom::EnsureAIBotsForMap( int nTargetMapIdx, bool bContinueStage, TDUNGEONDIFFICULTY StageDifficulty )
{
	if( GetRealUserCountWithoutGM() != 1 )
	{
		if( HasAIBots() )
		{
			DespawnAndRemoveAIBots( bContinueStage );
			return true;
		}
		return false;
	}

	CDNUserSession* pHuman = NULL;
	for( DWORD i = 0; i < (DWORD)m_VecMember.size(); ++i )
	{
		if( m_VecMember[i].pSession && !m_VecMember[i].pSession->bIsAIBot() && !m_VecMember[i].pSession->bIsGMTrace() )
		{
			pHuman = m_VecMember[i].pSession;
			break;
		}
	}
	if( !pHuman )
		return false;

	int nCurrentBots = 0;
	for( DWORD i = 0; i < (DWORD)m_VecMember.size(); ++i )
	{
		if( m_VecMember[i].pSession && m_VecMember[i].pSession->bIsAIBot() )
			++nCurrentBots;
	}

	int nWantedBots = GetTargetMapAIBotCount( nTargetMapIdx, bContinueStage, nCurrentBots, StageDifficulty );

	if( nWantedBots <= 0 )
	{
		if( nCurrentBots > 0 )
		{
			DespawnAndRemoveAIBots( bContinueStage );
			return true;
		}
		return false;
	}

	if( nCurrentBots == nWantedBots )
		return false;

	if( nCurrentBots > 0 )
		DespawnAndRemoveAIBots( bContinueStage );

	std::vector<const AIBot::BotJobSpec*> vecSpecs;
	AIBot::SelectSquadSpecs( pHuman->GetUserJob(), nWantedBots, vecSpecs );

	// Ensure human leader owns a valid party slot index so gate/ready packets stay aligned
	int nHumanIdx = pHuman->GetPartyMemberIndex();
	if( nHumanIdx < 0 || nHumanIdx >= PARTYMAX )
	{
		nHumanIdx = 0;
		pHuman->SetPartyMemberIndex( nHumanIdx );
	}
	if( m_nPartyMemberIndex[nHumanIdx] == 0 )
		m_nPartyMemberIndex[nHumanIdx] = pHuman->GetSessionID();

	for( int k = 0; k < (int)vecSpecs.size(); ++k )
	{
		UINT nBotSID = 0x70000001U + ( ( GetRoomID() & 0x0FFFU ) << 4 ) + (UINT)k;
		UINT nBotAID = nBotSID;
		INT64 biBotCID = (INT64)nBotSID;

		CDNUserSession* pBotSession = new IBoostPoolDNGameSession( nBotSID, m_pGameServer, this );
		if( !pBotSession )
			continue;

		pBotSession->SetAIBot( true );
		pBotSession->PreInitializeUser( L"AIBot", nBotAID, nBotSID, biBotCID,
			pHuman->GetTeam(), pHuman->GetWorldSetID(), pHuman->GetVillageID(), false, true, 0, NULL );
		AIBot::PopulateBotSession( pBotSession, vecSpecs[k], k, 60 );
		pBotSession->SetMapIndex( nTargetMapIdx );
		pBotSession->SetSessionState( SESSION_STATE_READY_TO_SYNC );
		pBotSession->SetLoadingComplete( true );

		int nBotMemberIdx = GetLeftMemberIndex();
		if( nBotMemberIdx < 0 || nBotMemberIdx >= PARTYMAX )
		{
			for( int s = 0; s < PARTYMAX; ++s )
			{
				if( m_nPartyMemberIndex[s] == 0 )
				{
					nBotMemberIdx = s;
					break;
				}
			}
		}
		if( nBotMemberIdx < 0 || nBotMemberIdx >= PARTYMAX )
			nBotMemberIdx = ( k + 1 < PARTYMAX ) ? ( k + 1 ) : ( PARTYMAX - 1 );
		pBotSession->SetPartyMemberIndex( nBotMemberIdx );
		m_nPartyMemberIndex[nBotMemberIdx] = nBotSID;

		AddPartyStruct( pBotSession, false );
		m_UserList.push_back( pBotSession );
	}

	m_iPartMemberCnt = (int)m_VecMember.size();
	return true;
}
