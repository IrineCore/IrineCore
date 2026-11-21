#include "StdAfx.h"
#include "DnInterfaceString.h"
#include "DnTableDB.h"
#include "TimeSet.h"
#include "DnUIString.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

namespace DN_INTERFACE
{
	namespace STRING
	{
		UINT CLASS_2_INDEX( int nJobID )
		{
			DNTableFileFormat*  pSox = GetDNTable(CDnTableDB::TJOB);
			if( !pSox ) return UINT_MAX;
			if( pSox->IsExistItem( nJobID ) )
				return pSox->GetFieldFromLablePtr( nJobID, "_JobName" )->GetInteger();
			return UINT_MAX;
		}

		LPCWSTR GET_NAME( const char *szName )
		{
			static TCHAR szStr[256];
			wsprintf( szStr, L"" );
			MultiByteToWideChar( CP_ACP, 0, szName, -1, szStr, (int)strlen(szName) );

			return szStr;
		}

		LPCWSTR GetClassString( int nClassID )
		{
			return GetEtUIXML().GetUIString( CEtUIXML::idCategory1, CLASS_2_INDEX( nClassID ) );
		}
		LPCWSTR GetJobString( int nJobID )
		{
			DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TJOB );
			if( !pSox ) return L"";
			int nJobStringID = pSox->GetFieldFromLablePtr( nJobID, "_JobName" )->GetInteger();
			return GetEtUIXML().GetUIString( CEtUIXML::idCategory1, nJobStringID );
		}

		void GetLocationText( std::wstring &wszStr, TCommunityLocation &LocInfo )
		{
			switch(LocInfo.cServerLocation)
			{
			case _LOCATION_NONE:	wszStr = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4305 );	return;	// Offline
			case _LOCATION_MOVE:	wszStr = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4306 );	return;	// Transferring server
			case _LOCATION_LOGIN:	wszStr = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4309 );	return;	// Logging in
			case _LOCATION_VILLAGE:
				{
					std::wstring mapName;
					DNTableFileFormat*  pSox = GetDNTable(CDnTableDB::TMAP);
					int nStringTableID = pSox->GetFieldFromLablePtr( LocInfo.nMapIdx, "_MapNameID" )->GetInteger();
					mapName = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, nStringTableID );

					if( mapName.empty() || LocInfo.nChannelID == 0 || LocInfo.nChannelID == -1 )
						_ASSERT(0);
					wszStr = FormatW(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4307 ), mapName.c_str(), LocInfo.nChannelID);	// (Channel:%d)
				}
				return;

			case _LOCATION_GAME:
				{
					DNTableFileFormat*  pSox = GetDNTable(CDnTableDB::TMAP);
					int nStringTableID = pSox->GetFieldFromLablePtr( LocInfo.nMapIdx, "_MapNameID" )->GetInteger();
					wszStr = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, nStringTableID );
				}
				return;
			default:	wszStr = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4308 );	return;	// No info
			}
		}

		void GetDayText( std::wstring &wszStr, __time64_t time )
		{
			DBTIMESTAMP DbTime;
			CTimeSet::ConvertTimeT64ToDbTimeStamp_LC( time, &DbTime );

			// Use Jan 1, 2009 format instead of 2009-1-1.
			//WCHAR wszTemp[32] = {0,};
			//swprintf_s( wszTemp, _countof(wszTemp), L"%d-%d-%d", DbTime.year, DbTime.month, DbTime.day );
			//wszStr = wszTemp;

			char szBuffer[256];
			//sprintf_s( szBuffer, _countof(szBuffer), "%d,%d,%d", DbTime.year, DbTime.month, DbTime.day );
			_snprintf_s(szBuffer, _countof(szBuffer), _TRUNCATE, "%d,%d,%d", DbTime.year, DbTime.month, DbTime.day );
			MakeUIStringUseVariableParam( wszStr, 191, szBuffer );
		}

		void GetDayTextSlash( eDayTextFormat eFormat, std::wstring &wszStr, __time64_t time, wchar_t seperator/* = '/'*/ )
		{
			DBTIMESTAMP DbTime;
			CTimeSet::ConvertTimeT64ToDbTimeStamp_LC( time, &DbTime );

			WCHAR wszTemp[32] = {0,};
			switch( eFormat )
			{
				case FORMAT_YY_MM_DD:
					//swprintf_s( wszTemp, _countof(wszTemp), L"%d/%d/%d", DbTime.year, DbTime.month, DbTime.day );
					_snwprintf_s(wszTemp, _countof(wszTemp), _TRUNCATE, L"%d%c%d%c%d", DbTime.year, seperator, DbTime.month, seperator, DbTime.day);
					break;;
				case FORMAT_MM_DD_YY:
					//swprintf_s( wszTemp, _countof(wszTemp), L"%d/%d/%d", DbTime.month, DbTime.day, DbTime.year );
					_snwprintf_s(wszTemp, _countof(wszTemp), _TRUNCATE, L"%d%c%d%c%d", DbTime.month, seperator, DbTime.day, seperator, DbTime.year);
					break;;
				case FORMAT_DD_MM_YY:
					//swprintf_s( wszTemp, _countof(wszTemp), L"%d/%d/%d", DbTime.day, DbTime.month, DbTime.year );
					_snwprintf_s(wszTemp, _countof(wszTemp), _TRUNCATE, L"%d%c%d%c%d", DbTime.day, seperator, DbTime.month, seperator, DbTime.year);
					break;;
			}
			
			wszStr = wszTemp;
		}

		void GetTimeText( std::wstring &wszStr, __time64_t time )
		{
			DBTIMESTAMP DbTime;
			CTimeSet::ConvertTimeT64ToDbTimeStamp_LC( time, &DbTime );

			char szBuffer[256];
			_snprintf_s(szBuffer, _countof(szBuffer), _TRUNCATE, "%d,%d,%d,%02d,%02d", DbTime.year, DbTime.month, DbTime.day, DbTime.hour, DbTime.minute);
			MakeUIStringUseVariableParam( wszStr, 192, szBuffer );
		}

#ifdef PRE_ADD_EQUIPLOCK
		void GetTimeText2( std::wstring &wszStr, __time64_t time, int messageId )
		{
			DBTIMESTAMP DbTime;
			CTimeSet::ConvertTimeT64ToDbTimeStamp_LC( time, &DbTime );

			wszStr = FormatW(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, messageId), DbTime.year, DbTime.month, DbTime.day, DbTime.hour, DbTime.minute);
		}
#endif

		void GetStageDifficultyText(std::wstring& wszStr, int idx)
		{
#ifdef PRE_PARTY_DB
			switch(idx)
#else
			switch(idx-1)
#endif
			{
				case Dungeon::Difficulty::Easy: wszStr = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 2370 ); break;	//	UISTRING : Easy
				case Dungeon::Difficulty::Normal: wszStr = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 2371 ); break;	//	UISTRING : Normal
				case Dungeon::Difficulty::Hard: wszStr = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 2372 ); break;	//	UISTRING : Hard
				case Dungeon::Difficulty::Master: wszStr = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 2373 ); break;	//	UISTRING : Master
				case Dungeon::Difficulty::Abyss: wszStr = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 2374 ); break;	//	UISTRING : Abyss
			}
		}

		void GetMapName(std::wstring& wszStr, int mapIdx)
		{
			DNTableFileFormat*  pSox = GetDNTable(CDnTableDB::TMAP);
			int nStringTableID = pSox->GetFieldFromLablePtr( mapIdx, "_MapNameID" )->GetInteger();
			wszStr = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, nStringTableID );
		}

		DWORD GetStageDifficultyColor(int difficultyIdx)
		{
			if (difficultyIdx >= 0)
			{
				switch (difficultyIdx)
				{
#ifdef PRE_PARTY_DB
				case Dungeon::Difficulty::Easy : return difficultycolor::EASY;
				case Dungeon::Difficulty::Normal : return difficultycolor::NORMAL;
				case Dungeon::Difficulty::Hard : return difficultycolor::HARD;
				case Dungeon::Difficulty::Master : return difficultycolor::MASTER;
				case Dungeon::Difficulty::Abyss : return difficultycolor::ABYSS;
#else
				case 1 : return difficultycolor::EASY;
				case 2 : return difficultycolor::NORMAL;
				case 3 : return difficultycolor::HARD;
				case 4 : return difficultycolor::MASTER;
				case 5 : return difficultycolor::ABYSS;
#endif
				}
			}

			return textcolor::WHITE;
		}

		void GetProfileText( std::wstring &wszStr, TProfile &Profile )
		{
#ifdef PRE_FIX_USA_PROFILE
			wszStr = FormatW( L"%s", Profile.wszGreeting );
#else
			std::wstring wszGender;
			if( Profile.cGender == 1 ) wszGender = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 8139 );
			else if( Profile.cGender == 2 ) wszGender = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 8140 );
			wszStr = FormatW( L"%s / %s", wszGender.c_str(), Profile.wszGreeting );
#endif
		}

		namespace SKILL
		{
			UINT STATIC_2_INDEX( emTOOLTIP_STATIC emStatic )
			{
				switch( emStatic )
				{
				case emTOOLTIP_STATIC::SKILL_DISTANCE:			return 1130;
				case emTOOLTIP_STATIC::SKILL_DESCRIPTION:		return 1131;
				case emTOOLTIP_STATIC::SKILL_NEXT_DESCRIPTION:	return 1132;
				default: ASSERT(0&&"SKILL::STATIC_2_INDEX");
				}

				return UINT_MAX;
			}

			UINT STYPE_2_INDEX( CDnSkill::SkillTypeEnum emType )
			{
				switch( emType )
				{
				case CDnSkill::SkillTypeEnum::Active:	return 1100;
				case CDnSkill::SkillTypeEnum::Passive:
				case CDnSkill::SkillTypeEnum::AutoPassive:
					return 1101;
				case CDnSkill::SkillTypeEnum::EnchantPassive: return 1214;
				case CDnSkill::SkillTypeEnum::AutoActive:	return 1000041145;
				default: ASSERT(0&&"SKILL::STYPE_2_INDEX");
				}

				return UINT_MAX;
			}

			UINT DTYPE_2_INDEX( CDnSkill::DurationTypeEnum emType )
			{
				switch( emType )
				{
				case CDnSkill::DurationTypeEnum::Instantly:		return 1110;
				case CDnSkill::DurationTypeEnum::Buff:			return 1111;
				case CDnSkill::DurationTypeEnum::Debuff:		return 1112;
				case CDnSkill::DurationTypeEnum::TimeToggle:	return 1113;
				case CDnSkill::DurationTypeEnum::ActiveToggle:	return 1114;
				case CDnSkill::DurationTypeEnum::ActiveToggleForSummon:	return 1110;
				case CDnSkill::DurationTypeEnum::Aura:			return 1114;
				case CDnSkill::DurationTypeEnum::SummonOnOff:	return 1110;
				case CDnSkill::DurationTypeEnum::StanceChange:	return 1111;
				default: ASSERT(0&&"SKILL::DTYPE_2_INDEX");
				}

				return UINT_MAX;
			}

			UINT UTYPE_2_INDEX( CDnSkill::TargetTypeEnum emType )
			{
				switch( emType )
				{
				case CDnSkill::TargetTypeEnum::Self:	return 1120;
				case CDnSkill::TargetTypeEnum::Friend:	return 1121;
				case CDnSkill::TargetTypeEnum::Party:	return 1121;
				case CDnSkill::TargetTypeEnum::Enemy:	return 1122;
				default: ASSERT(0&&"SKILL::UTYPE_2_INDEX");
				}

				return UINT_MAX;
			}

			LPCWSTR GET_STATIC( emTOOLTIP_STATIC emStatic )
			{
				return GetEtUIXML().GetUIString( CEtUIXML::idCategory1, STATIC_2_INDEX( emStatic ) );
			}

			LPCWSTR GET_NAME( const char *szName )
			{
				static TCHAR szStr[256];
				wsprintf( szStr, L"" );
				MultiByteToWideChar( CP_ACP, 0, szName, -1, szStr, (int)strlen(szName) );
				return szStr;
			}

			LPCWSTR GET_TYPE( CDnSkill::SkillTypeEnum emType )
			{
				return GetEtUIXML().GetUIString( CEtUIXML::idCategory1, STYPE_2_INDEX( emType ) );
			}

			LPCWSTR GET_DURATION_TYPE( CDnSkill::DurationTypeEnum emType )
			{
				return GetEtUIXML().GetUIString( CEtUIXML::idCategory1, DTYPE_2_INDEX( emType ) );
			}

			LPCWSTR GET_USING_TYPE( CDnSkill::TargetTypeEnum emType )
			{
				return GetEtUIXML().GetUIString( CEtUIXML::idCategory1, UTYPE_2_INDEX( emType ) );
			}

			LPCWSTR GET_DECREASE_ITEM( int nItemID )
			{
				DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TITEM );
				if( !pSox ) return NULL;

				return GetEtUIXML().GetUIString( CEtUIXML::idCategory1, pSox->GetFieldFromLablePtr( nItemID, "_NameID" )->GetInteger() );
			}
		}

		namespace ITEM
		{
			DWORD RANK_2_COLOR( eItemRank emRank )
			{
				switch( emRank )
				{
				case ITEMRANK_D:	return itemcolor::NORMAL;
				case ITEMRANK_C:	return itemcolor::MAGIC;
				case ITEMRANK_B:	return itemcolor::RARE;
				case ITEMRANK_A:	return itemcolor::EPIC;
				case ITEMRANK_S:	return itemcolor::UNIQUE;
				case ITEMRANK_SS:	return itemcolor::HEROIC;
				case ITEMRANK_SSS:  return itemcolor::NAMED;
				default: ASSERT(0&&"ITEM::RANK_2_COLOR");
				}

				return textcolor::WHITE;
			}

			UINT RANK_2_INDEX( eItemRank emRank )
			{
				switch( emRank )
				{
				case ITEMRANK_D:	return 2270;
				case ITEMRANK_C:	return 2271;
				case ITEMRANK_B:	return 2272;
				case ITEMRANK_A: return 2273;
				case ITEMRANK_S: return 2274;
				case ITEMRANK_SS: return 2275;
				case ITEMRANK_SSS: return 1000003496;
				default: ASSERT(0&&"ITEM::RANK_2_INDEX");
				}

				return UINT_MAX;
			}

			eItemRank INDEX_2_RANK( int UIStringID )
			{
				switch( UIStringID )
				{
				case 2270: return ITEMRANK_D;
				case 2271: return ITEMRANK_C;
				case 2272: return ITEMRANK_B;
				case 2273: return ITEMRANK_A;
				case 2274: return ITEMRANK_S;
				case 2275: return ITEMRANK_SS;
				default: ASSERT(0&&"ITEM::INDEX_2_RANK");
				}

				return ITEMRANK_MAX;
			}

			UINT ITYPE_2_INDEX( eItemTypeEnum emType )
			{
				switch( emType )
				{
				case ITEMTYPE_WEAPON:	return 2100;
				case ITEMTYPE_PARTS:	return 2101;
				case ITEMTYPE_NORMAL:	return 2102;
				case ITEMTYPE_SKILL:	return 2103;
				case ITEMTYPE_JEWEL:	return 2104;
				case ITEMTYPE_PLATE:	return 2105;
				case ITEMTYPE_INSTANT:
				default: ASSERT(0&&"ITEM::ITYPE_2_INDEX");
				}

				return UINT_MAX;
			}

			UINT STATIC_2_INDEX( emTOOLTIP_STATIC emStatic )
			{
				switch( emStatic )
				{
				case emTOOLTIP_STATIC::ITEM_DURABLE:		return 2244;
				case emTOOLTIP_STATIC::ITEM_UNDESTRUCTION:	return 2131;
				case emTOOLTIP_STATIC::ITEM_ATTACK:			return 2132;
				case emTOOLTIP_STATIC::ITEM_DEFENSE:		return 2133;
					//
				case emTOOLTIP_STATIC::ITEM_STRENGTH:		return 5009;
				case emTOOLTIP_STATIC::ITEM_AGILITY:		return 5010;
				case emTOOLTIP_STATIC::ITEM_INTELLIGENCE:	return 5011;
				case emTOOLTIP_STATIC::ITEM_STAMINA:		return 5012;
				case emTOOLTIP_STATIC::ITEM_SPIRIT:			return 5074;
					//
				case emTOOLTIP_STATIC::ITEM_STIFFNESS_P:	return 5021;
				case emTOOLTIP_STATIC::ITEM_STIFFNESS_R:	return 5024;
				case emTOOLTIP_STATIC::ITEM_STUN_P:			return 5022;
				case emTOOLTIP_STATIC::ITEM_STUN_R:			return 5025;
				case emTOOLTIP_STATIC::ITEM_CRITICAL_P:		return 5023;
				case emTOOLTIP_STATIC::ITEM_CRITICAL_R:		return 5026;
					//
				case emTOOLTIP_STATIC::ITEM_FIRE_A:			return 5029;
				case emTOOLTIP_STATIC::ITEM_ICE_A:			return 5030;
				case emTOOLTIP_STATIC::ITEM_LIGHT_A:		return 5027;
				case emTOOLTIP_STATIC::ITEM_DARK_A:			return 5028;
					//
				case emTOOLTIP_STATIC::ITEM_FIRE_R:			return 5033;
				case emTOOLTIP_STATIC::ITEM_ICE_R:			return 5034;
				case emTOOLTIP_STATIC::ITEM_LIGHT_R:		return 5031;
				case emTOOLTIP_STATIC::ITEM_DARK_R:			return 5032;
					//
				case emTOOLTIP_STATIC::ITEM_MOVE:			return 2180;
				case emTOOLTIP_STATIC::ITEM_HP:				return 2181;
				case emTOOLTIP_STATIC::ITEM_SP:				return 2182;
					//
				case emTOOLTIP_STATIC::ITEM_JUDGMENT:		return 2190;
				case emTOOLTIP_STATIC::ITEM_COOLTIME:		return 2278;

				case emTOOLTIP_STATIC::ITEM_SAFEZONEMOVE:	return 5076;
				case emTOOLTIP_STATIC::ITEM_MAXHP:			return 5063;
				case emTOOLTIP_STATIC::ITEM_MAXSP:			return 5064;

				case emTOOLTIP_STATIC::ITEM_ATTACK_P:		return 5018;
				case emTOOLTIP_STATIC::ITEM_ATTACK_M:		return 5019;
				case emTOOLTIP_STATIC::ITEM_DEFENSE_P:		return 5020;
				case emTOOLTIP_STATIC::ITEM_DEFENSE_M:		return 5021;

				case emTOOLTIP_STATIC::ITEM_RECOVERSP:		return 2256;

				case emTOOLTIP_STATIC::ITEM_ADDEXP:			return 2299;

				default: ASSERT(0&&"ITEM::STATIC_2_INDEX");
				}

				return UINT_MAX;
			}

			UINT RTYPE_2_INDEX( CDnItem::ItemReversionEnum emType )
			{
				switch( emType )
				{
				case CDnItem::ItemReversionEnum::Belong:		return 2111;
				case CDnItem::ItemReversionEnum::Trade:			return 2111;
				case CDnItem::ItemReversionEnum::GuildBelong:	return 2213;
#if defined(PRE_ADD_SERVER_WAREHOUSE)
				case CDnItem::ItemReversionEnum::OnlyTradeWorldStorage: return 2111;
#endif // #if defined(PRE_ADD_SERVER_WAREHOUSE)
				//case CDnItem::ItemReversionEnum::Gain:	return 2111;
				default: ASSERT(0&&"ITEM::RTYPE_2_INDEX");
				}

				return UINT_MAX;
			}

			UINT EQUIPTYPE_2_INDEX( CDnWeapon::EquipTypeEnum emType )
			{
				switch( emType )
				{
				case CDnWeapon::Sword:		return 2191;
				case CDnWeapon::Gauntlet:	return 2192;
				case CDnWeapon::Axe:		return 2193;
				case CDnWeapon::Hammer:		return 2194;
				case CDnWeapon::SmallBow:	return 2195;
				case CDnWeapon::BigBow:		return 2196;
				case CDnWeapon::CrossBow:	return 2197;
				case CDnWeapon::Staff:		return 2198;
				case CDnWeapon::Book:		return 2199;
				case CDnWeapon::Orb:		return 2200;
				case CDnWeapon::Puppet:		return 2201;
				case CDnWeapon::Mace:		return 2202;
				case CDnWeapon::Flail:		return 2203;
				case CDnWeapon::Wand:		return 2204;
				case CDnWeapon::Shield:		return 2205;
				case CDnWeapon::Arrow:		return 2207;
				case CDnWeapon::Cannon:		return 1000032445;
				case CDnWeapon::BubbleGun:	return 1000032446;
				case CDnWeapon::Glove:		return 1000032447;
				case CDnWeapon::Fan:		return 1000041664;
				case CDnWeapon::Chakram:	return 1000041665;
				case CDnWeapon::Charm:		return 1000041666;
				case CDnWeapon::Scimiter:	return 2165;
				case CDnWeapon::Dagger:		return 2166;
				case CDnWeapon::Crook:		return 2167;
				case CDnWeapon::Spear:		return 1000072229;
				case CDnWeapon::Bracelet:	return 1000072230;
				case CDnWeapon::KnuckleGear:return 1000088607;
				case CDnWeapon::Claw:		return 1000088608;
				//default: ASSERT(0&&"EQUIPTYPE_2_INDEX");
				}
#pragma message("[DONE]   -----> DnInterfaceString 473 ADD ItemType")
				return UINT_MAX;
			}
			UINT CASHEQUIPTYPE_2_INDEX( CDnWeapon::EquipTypeEnum emType )
			{
				switch( emType )
				{
				case CDnWeapon::Sword:
				case CDnWeapon::Axe:
				case CDnWeapon::Hammer:
				case CDnWeapon::SmallBow:
				case CDnWeapon::BigBow:
				case CDnWeapon::CrossBow:
				case CDnWeapon::Mace:
				case CDnWeapon::Flail:
				case CDnWeapon::Wand:
				case CDnWeapon::Staff:
				case CDnWeapon::Cannon:
				case CDnWeapon::BubbleGun:
				case CDnWeapon::Chakram:
				case CDnWeapon::Fan:
				case CDnWeapon::Scimiter:
				case CDnWeapon::Dagger:
				case CDnWeapon::Spear:
				case CDnWeapon::KnuckleGear:
					return 2208;
				case CDnWeapon::Gauntlet:
				case CDnWeapon::Arrow:
				case CDnWeapon::Shield:
				case CDnWeapon::Book:
				case CDnWeapon::Orb:
				case CDnWeapon::Puppet:
				case CDnWeapon::Glove:
				case CDnWeapon::Charm:
				case CDnWeapon::Crook:
				case CDnWeapon::Bracelet:
				case CDnWeapon::Claw:
					return 2209;
				}

				return UINT_MAX;
			}

			UINT PARTSTYPE_2_INDEX( CDnParts::PartsTypeEnum emType )
			{
				switch( emType )
				{
				case CDnParts::Face:		return 2220;
				case CDnParts::Hair:		return 2221;
				case CDnParts::Helmet:		return 2222;
				case CDnParts::Body:		return 2223;
				case CDnParts::Leg:			return 2224;
				case CDnParts::Hand:		return 2225;
				case CDnParts::Foot:		return 2226;
				case CDnParts::Necklace:	return 2227;
				case CDnParts::Earring:		return 2228;
				case CDnParts::Ring:		return 2229;
				case CDnParts::Ring2:		return 2229;
				default: ASSERT(0&&"ITEM::PARTSTYPE_2_INDEX");
				}

				return UINT_MAX;
			}
			UINT CASHPARTSTYPE_2_INDEX( CDnParts::PartsTypeEnum emType )
			{
				switch( emType )
				{
				case CDnParts::CashHelmet:		return 2222;
				case CDnParts::CashBody:		return 2223;
				case CDnParts::CashLeg:			return 2224;
				case CDnParts::CashHand:		return 2225;
				case CDnParts::CashFoot:		return 2226;
				case CDnParts::CashNecklace:	return 2227;
				case CDnParts::CashEarring:		return 2228;
				case CDnParts::CashRing:		return 2229;
				case CDnParts::CashRing2:		return 2229;
				case CDnParts::CashWing:		return 2232;
				case CDnParts::CashTail:		return 2233;
				case CDnParts::CashFaceDeco:	return 2234;
				case CDnParts::CashFairy:		return 2231;
				default: ASSERT(0&&"ITEM::CASHPARTSTYPE_2_INDEX");
				}

				return UINT_MAX;
			}

			LPCWSTR GET_STATIC( emTOOLTIP_STATIC emStatic )
			{
				return GetEtUIXML().GetUIString( CEtUIXML::idCategory1, STATIC_2_INDEX( emStatic ) );
			}

			LPCWSTR GET_REVERSION_TYPE( CDnItem::ItemReversionEnum emType )
			{
				return GetEtUIXML().GetUIString( CEtUIXML::idCategory1, RTYPE_2_INDEX( emType ) );
			}

			LPCWSTR GetItemTypeString( eItemTypeEnum emType )
			{
				return GetEtUIXML().GetUIString( CEtUIXML::idCategory1, ITYPE_2_INDEX( emType ) );
			}

			LPCWSTR GetEquipString( CDnWeapon::EquipTypeEnum emType )
			{
				return GetEtUIXML().GetUIString( CEtUIXML::idCategory1, EQUIPTYPE_2_INDEX( emType ) );
			}
			LPCWSTR GetCashEquipString( CDnWeapon::EquipTypeEnum emType )
			{
				return GetEtUIXML().GetUIString( CEtUIXML::idCategory1, CASHEQUIPTYPE_2_INDEX( emType ) );
			}

			LPCWSTR GetPartsString( CDnParts::PartsTypeEnum emType )
			{
				return GetEtUIXML().GetUIString( CEtUIXML::idCategory1, PARTSTYPE_2_INDEX( emType ) );
			}
			LPCWSTR GetCashPartsString( CDnParts::PartsTypeEnum emType )
			{
				return GetEtUIXML().GetUIString( CEtUIXML::idCategory1, CASHPARTSTYPE_2_INDEX( emType ) );
			}

			LPCWSTR GetRankString( eItemRank emRank )
			{
				return GetEtUIXML().GetUIString( CEtUIXML::idCategory1, RANK_2_INDEX( emRank ) );
			}
		}
	}

	namespace UTIL
	{
		void GetMoneyFormat( INT64 nMoney, std::wstring &strFormat  )
		{
			strFormat.clear();

			if( nMoney <= 0 )
			{
				return;
			}

			int nGold = int(nMoney/10000);
			int nSilver = int((nMoney%10000)/100);
			int nCopper = int(nMoney%100);

			wchar_t szTemp[256]={0};

			if( nMoney < 100 )
			{
				swprintf_s( szTemp, 256, L"%dC", nCopper );
			}
			else if( nMoney < 10000 )
			{
				swprintf_s( szTemp, 256, L"%dS %dC", nSilver, nCopper );
			}
			else
			{
				swprintf_s( szTemp, 256, L"%dG %dS %dC", nGold, nSilver, nCopper );
			}

			strFormat = szTemp;
		}

		// Return as a formatted string in gold, silver, and copper.
		void GetMoneyFormatUseStr( INT64 nMoney, std::wstring &strFormat )
		{
			strFormat.clear();

			if( nMoney <= 0 )
			{
				return;
			}

			int nGold = int(nMoney/10000);
			int nSilver = int((nMoney%10000)/100);
			int nCopper = int(nMoney%100);

			// Can't write this any better...
			wchar_t szTemp[256]={0};
			wchar_t szTempSub[256]={0};
			wchar_t szTempGold[256]={0};
			wchar_t szTempSilver[256]={0};
			wchar_t szTempCopper[256]={0};

			if( nGold != 0 )
				swprintf_s( szTempGold, 256, L"%d%s", nGold, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 507 ) );

			if( nSilver != 0 )
				swprintf_s( szTempSilver, 256, L"%d%s", nSilver, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 508 ) );

			if( nCopper != 0 )
				swprintf_s( szTempCopper, 256, L"%d%s", nCopper, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 509 ) );

			if( nGold != 0 && nSilver != 0 )
				swprintf_s( szTempSub, 256, L"%s %s", szTempGold, szTempSilver );
			else
				swprintf_s( szTempSub, 256, L"%s%s", szTempGold, szTempSilver );

			if( szTempSub[0] != '\0' && nCopper != 0 )
				swprintf_s( szTemp, 256, L"%s %s", szTempSub, szTempCopper );
			else
				swprintf_s( szTemp, 256, L"%s%s", szTempSub, szTempCopper );

			strFormat = szTemp;
		}

		bool CheckSpecialCharacter( std::wstring &strSrc )
		{
			std::wstring::size_type index;
			index = strSrc.find_first_of( L"~!@#$%^&*+|:?><,.;[]{}()\n\t\v\b\r\a\\\?\'\" " );

			if( index != string::npos )
			{
				return true;
			}

			return false;
		}

		bool CheckAccount( std::wstring &strName )
		{
			std::vector<std::wstring> &vecAccountFilter = GetEtUIXML().GetAccountFilter();
			std::wstring::size_type index;

			std::wstring strTemp(strName);
			std::transform ( strTemp.begin(), strTemp.end(), strTemp.begin(), towlower );

			for( int i=0; i<(int)vecAccountFilter.size(); i++ )
			{
				index = strTemp.find( vecAccountFilter[i] );
				if( index != string::npos )
				{
					return true;
				}
			}

			return false;
		}

		bool CheckChat( std::wstring &strChat )
		{
			std::vector<std::wstring> &vecChatFilter = GetEtUIXML().GetChatFilter();
			std::vector<std::wstring> &vecWhiteList = GetEtUIXML().GetWhiteList();
			std::wstring::size_type index;

			std::wstring strTemp(strChat);
			std::transform ( strTemp.begin(), strTemp.end(), strTemp.begin(), towlower );

			std::wstring::size_type found;
			do
			{
				std::wstring startTag;
				startTag = FormatW(L"%cs", 0xff00);
				found = strTemp.find(startTag.c_str());
				if (found != std::wstring::npos)
				{
					std::wstring endTag;
					endTag = FormatW(L"%ce", 0xff00);
					std::wstring::size_type foundEnd = strTemp.find(endTag.c_str(), found + 1);
					if (foundEnd == std::wstring::npos)
						break;

					strTemp.erase(found, (foundEnd + endTag.size()) - found);
					found = foundEnd + 1;
				}
			}while(found != std::wstring::npos);

			for( int i=0; i<(int)vecChatFilter.size(); i++ )
			{
				index = strTemp.find( vecChatFilter[i] );
				if( index != string::npos )
				{
					bool bWhiteList = false;
					for( int k=0; k<(int)vecWhiteList.size(); k++ )
					{
						std::string::size_type findOffsetSafeWord = vecWhiteList[k].find( vecChatFilter[i] );

						// If it is not a word in WhiteList,
						if( std::string::npos != findOffsetSafeWord )
						{
							// nip is a forbidden word and snip is a safe word. Handle it as below.
							if( index >= findOffsetSafeWord )
							{
								if( wcsncmp( &strChat[index - findOffsetSafeWord], vecWhiteList[k].c_str(), vecWhiteList[k].size() ) == 0 )
								{
									bWhiteList = true;
									break;
								}
							}
						}
					}
					if( !bWhiteList )
					{
						return true;
					}
				}
			}

			return false;
		}

		void CheckChat( std::wstring &strChat, wchar_t wcTrans )
		{
			std::vector<std::wstring> &vecChatFilter = GetEtUIXML().GetChatFilter();
			std::vector<std::wstring> &vecWhiteList  = GetEtUIXML().GetWhiteList();
#ifdef PRE_FIX_54701
			std::vector<std::wstring> &vecReplaceFilter = GetEtUIXML().GetReplaceFilter();
			int nChangeWordLength = 0;
			static std::wstring strTemp;
			strTemp = strChat;
#else
			std::wstring strTemp(strChat);
#endif
			
			std::string::size_type findOffset;
			std::transform( strTemp.begin(), strTemp.end(), strTemp.begin(), towlower );

			for( int i=0; i<(int)vecChatFilter.size(); i++ )
			{
				findOffset = 0;

				// Search the source string until the target substring is no longer found
				while( true )
				{
					std::wstring startTag;
					startTag = FormatW(L"%cs", 0xff00);
					std::string::size_type tagFoundStart = strTemp.find(startTag.c_str(), findOffset);
					if (tagFoundStart != std::wstring::npos)
					{
						std::wstring endTag;
						endTag = FormatW(L"%ce", 0xff00);
						std::wstring::size_type tagFoundEnd = strTemp.find(endTag.c_str(), tagFoundStart);
						if (tagFoundEnd != std::wstring::npos)
							findOffset = (tagFoundEnd + endTag.size());// - tagFoundStart;
					}

					// Search
					findOffset = strTemp.find( vecChatFilter[i], findOffset );

					// When not found
					if( std::string::npos == findOffset )
						break;

					// When found
					// First compare against WhiteList to check if it is a safe word,
					bool bWhiteList = false;
					std::string::size_type addOffset;
					for( int k=0; k<(int)vecWhiteList.size(); k++ )
					{
						std::string::size_type findOffsetSafeWord = vecWhiteList[k].find( vecChatFilter[i] );

						// If it is not a word in WhiteList, (check from the front)
						if( std::string::npos != findOffsetSafeWord )
						{
							// nip is a forbidden word and snip is a safe word. Handle it as below.
							if( findOffset >= findOffsetSafeWord )
							{
								if( wcsncmp( &strTemp[findOffset - findOffsetSafeWord], vecWhiteList[k].c_str(), vecWhiteList[k].size() ) == 0 )
								{
									bWhiteList = true;
									addOffset = vecWhiteList[k].size() - findOffsetSafeWord;
									break;
								}
							}
						}

						// Next, check from the back
						findOffsetSafeWord = vecWhiteList[k].rfind( vecChatFilter[i] );
						if( std::string::npos != findOffsetSafeWord )
						{
							if( findOffset >= findOffsetSafeWord )
							{
								if( wcsncmp( &strTemp[findOffset - findOffsetSafeWord], vecWhiteList[k].c_str(), vecWhiteList[k].size() ) == 0 )
								{
									bWhiteList = true;
									addOffset = vecWhiteList[k].size() - findOffsetSafeWord;
									break;
								}
							}
						}
					}
					if( bWhiteList )
					{
						findOffset = findOffset + addOffset;
						continue;
					}

#ifdef PRE_FIX_54701
					// Replace filtered word with "ReplacementWord" instead of '*'.
					if( !vecReplaceFilter.empty() && (int)vecReplaceFilter.size() > i )
					{
						strTemp.replace(findOffset, vecChatFilter[i].length(), vecReplaceFilter[i].c_str());
						nChangeWordLength = (int)vecReplaceFilter[i].size();
					}
					else
					{
						// If ReplaceWord buffer is shorter than vecChatFilter buffer, fall back to the previous filtering method.
						for (int j = 0; j<(int)vecChatFilter[i].size(); ++j)
						{
							strTemp[findOffset+j] = wcTrans;
						}
					}

					findOffset = findOffset + nChangeWordLength;
#else
					// Previous filtering method. (Replace words with '*')
					for (int j = 0; j<(int)vecChatFilter[i].size(); ++j)
					{
						strChat[findOffset+j] = wcTrans;
					}

					// Move current search position
					++findOffset;
#endif
				} // end of while
			} // end of for ( vecChatFilter.size() )

#ifdef PRE_FIX_54701
			// Since 'ass' is not the only word registered in the xml file, there is no guarantee Find succeeds only once.
			// Usually the same word comes in 2-3 times. (Searches vecChatFileter from start to end)
			// Directly modifying &strChat as in the old method causes the following issue.

			// strChat : you ass ok
			// strTemp(strChat); strTemp.Find( ... );

			// Search using "strTemp", but apply changes from search results to "strChat"!
			// strTemp still has unchanged "ass", while strChat is already changed to "GemsCool". Word start and end offsets differ.

			// Offset 0123456789 ...
			// 1.1. you ass ok -> changed to You GemsCool ok.    offset ( start : 4 , Length of ass : 3 )
			// 1.2. you GemsCool ok -> changed to You GemGemsCool ok. offset ( start : 4 , Length of ass : 3 )
			// Because this issue occurs, replace strTemp words immediately to avoid duplicate checks, then pass to strChat at the end.

			// The old method only replaced with '*', keeping word length unchanged so no issue occurred.
			if(nChangeWordLength > 0)
				strChat = strTemp;
#endif
		}


		void GetValue_2_String( int nValue, std::wstring &strString )
		{
			// Handle only for domestic build.
#if defined(_RDEBUG)
			static bool s_bOnce = false;
			static wchar_t wszM1[10][64];
			static wchar_t wszM2[4][64];
			static wchar_t wszM3[4][64];

			if (s_bOnce == false)
			{
				wsprintf( wszM1[0], L"");
				wsprintf( wszM2[0], L"");
				wsprintf( wszM3[0], L"");

				int j;
				for( j=1; j <= 9; ++j)
					wsprintf( wszM1[j], GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 510+j ) );
				for( j=1; j <= 3; ++j)
					wsprintf( wszM2[j], GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 519+j ) );
				for( j=1; j <= 3; ++j)
					wsprintf( wszM3[j], GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 522+j ) );
				s_bOnce = true;
			}

			wchar_t wszBuffer[256] = {0};
			_itow_s( nValue, wszBuffer, _countof(wszBuffer), 10 );

			int nValueLen = (int)wcslen( wszBuffer );
			bool bFlag(false);
			int nNumber, nDiv, nMod;
			std::wstring strNumber;
			strString.clear();

			for( int i=0; i<nValueLen; i++ )
			{
				nDiv = (nValueLen-(i+1)) / 4;
				nMod = (nValueLen-(i+1)) % 4;

				strNumber = wszBuffer[i];
				nNumber = _wtoi(strNumber.c_str());

				if( nNumber != 0 )
				{
					bFlag = true;
					strString += wszM1[nNumber];
					strString += wszM2[nMod];
				}

				if( nMod == 0 && bFlag )
				{
					bFlag = false;
					strString += wszM3[nDiv];
				}
			}
#endif
		}

		std::wstring GetAddCommaString( INT64 nValue )
		{
			std::wstring strString;
			std::wstring strValue = FormatW( L"%d", nValue );
			int nLen = static_cast<int>( strValue.length() );
			
			if( nValue >= 0 && nLen < 4 )
				return strValue;

			if( nValue < 0 && nLen < 5 )
				return strValue;

			strString.reserve( nLen<<1 );
			std::wstring::const_iterator itBeg = strValue.begin();
			int nEnd = nLen % 3;

			if( nEnd )
			{
				strString.append( itBeg, itBeg + nEnd );
				strString += L",";
			}

			while( nEnd < nLen )
			{
				strString.append( itBeg+nEnd, itBeg+nEnd+3 );
				if( ( nEnd += 3 ) < nLen )
					strString += L",";
			}

			return strString;
		}

#ifdef PRE_ADD_ITEM_GAINTABLE
		void String2Wstring(std::wstring& result, const std::string& source)
		{
			std::wstring str(source.length(), L' ');
			std::copy(source.begin(), source.end(), str.begin());
			result = str;
		}
#endif
	}
}