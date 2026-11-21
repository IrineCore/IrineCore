#pragma once
#include "DnItem.h"
#include "DnSkill.h"
#include "DnTooltipDlg.h"
#include "DnWeapon.h"
#include "DnParts.h"

namespace DN_INTERFACE
{
	namespace STRING
	{
		enum eDayTextFormat{ FORMAT_YY_MM_DD = 0, FORMAT_MM_DD_YY, FORMAT_DD_MM_YY  };

		UINT CLASS_2_INDEX( int nJobID );

		LPCWSTR GET_NAME( const char *szName );
		LPCWSTR GetClassString( int nClassID );
		LPCWSTR GetJobString( int nJobID );
		void GetLocationText( std::wstring &wszStr, TCommunityLocation &Loc );
		void GetDayTextSlash( eDayTextFormat eFormat, std::wstring &wszStr, __time64_t time, wchar_t seperator = '/' );		// 2009-1-1
		void GetDayText( std::wstring &wszStr, __time64_t time );			// Jan 1, 2009 (Currently used for guild and cash item duration)
		void GetTimeText( std::wstring &wszStr, __time64_t time );			// Jan 1, 2009 1:01 (Used for sanctions)
#ifdef PRE_ADD_EQUIPLOCK
		void GetTimeText2( std::wstring &wszStr, __time64_t time, int messageId );
#endif
		void GetStageDifficultyText(std::wstring& wszStr, int idx);
		void GetMapName(std::wstring& wszStr, int mapIdx);
		DWORD GetStageDifficultyColor(int difficultyIdx);
		void GetProfileText( std::wstring &wszStr, TProfile &Profile );

		namespace SKILL
		{
			UINT STATIC_2_INDEX( emTOOLTIP_STATIC emStatic );
			UINT STYPE_2_INDEX( CDnSkill::SkillTypeEnum emType );
			UINT DTYPE_2_INDEX( CDnSkill::DurationTypeEnum emType );
			UINT UTYPE_2_INDEX( CDnSkill::TargetTypeEnum emType );

			LPCWSTR GET_STATIC( emTOOLTIP_STATIC emStatic );
			LPCWSTR GET_TYPE( CDnSkill::SkillTypeEnum emType );
			LPCWSTR GET_DURATION_TYPE( CDnSkill::DurationTypeEnum emType );
			LPCWSTR GET_USING_TYPE( CDnSkill::TargetTypeEnum emType );
			LPCWSTR GET_DECREASE_ITEM( int nItemID );
		}

		namespace ITEM
		{
			DWORD RANK_2_COLOR( eItemRank emRank );
			UINT STATIC_2_INDEX( emTOOLTIP_STATIC emStatic );
			UINT ITYPE_2_INDEX( eItemTypeEnum emType );
			UINT RTYPE_2_INDEX( CDnItem::ItemReversionEnum emType );
			UINT RANK_2_INDEX( eItemRank emRank );
			eItemRank INDEX_2_RANK( int UIStringID );
			UINT EQUIPTYPE_2_INDEX( CDnWeapon::EquipTypeEnum emType );
			UINT PARTSTYPE_2_INDEX( CDnParts::PartsTypeEnum emType );
			UINT CASHPARTSTYPE_2_INDEX( CDnParts::PartsTypeEnum emType );

			LPCWSTR GET_STATIC( emTOOLTIP_STATIC emStatic );
			LPCWSTR GET_REVERSION_TYPE( CDnItem::ItemReversionEnum emType );
			LPCWSTR GetItemTypeString( eItemTypeEnum emType );
			LPCWSTR GetEquipString( CDnWeapon::EquipTypeEnum emType );
			LPCWSTR GetCashEquipString( CDnWeapon::EquipTypeEnum emType );
			LPCWSTR GetPartsString( CDnParts::PartsTypeEnum emType );
			LPCWSTR GetCashPartsString( CDnParts::PartsTypeEnum emType );
			LPCWSTR GetRankString( eItemRank emRank );
		}
	}

	namespace UTIL
	{
		void GetMoneyFormat( INT64 nMoney, std::wstring &strFormat );
		void GetMoneyFormatUseStr( INT64 nMoney, std::wstring &strFormat );

		bool CheckSpecialCharacter( std::wstring &strSrc );
		bool CheckAccount( std::wstring &strName );
		bool CheckChat( std::wstring &strChat );
		void CheckChat( std::wstring &strChat, wchar_t wcTrans );	// Replace forbidden word parts with the character passed as an argument.

		void GetValue_2_String( int nValue, std::wstring &strString );
		std::wstring GetAddCommaString( INT64 nValue );
#ifdef PRE_ADD_ITEM_GAINTABLE
		void String2Wstring(std::wstring& result, const std::string& source);
#endif
	}
}

#if defined(PRE_ADD_ENGLISH_STRING)
#define TILDE L"-"
#else
#define TILDE L"~"
#endif