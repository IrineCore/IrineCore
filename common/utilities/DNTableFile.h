
#pragma once

#include "StringUtil.h"

class CStream;

class CDNTableFile
{
public:
	struct HeaderSize
	{
		enum eCode
		{
			Version = sizeof(short),
			ReserveLen = sizeof(short),
			FieldCount = sizeof(short),
			DataCount = sizeof(int),
			FieldNameLen = sizeof(short),
			FieldType = sizeof(char),
		};
	};

	struct FieldType
	{
		enum eCode
		{
			NA	= 0,
			STRING = 1,
			BOOL = 2,
			INT = 3,
			PER = 4,
			FLOAT = 5,

			PRIMARY_KEY = 255,
		};
	};

#pragma pack(push, 1)
	class Cell
	{
		public:

			Cell()
			{
				m_uCell.iValue = 0;
			}
			~Cell(){}

			void SetString( char* pString ){ m_uCell.pValue = pString; }
			char* GetString() const
			{
				if( !this ) {
					ASSERT( 0 && "GetString() - Null" );
					return "";
				}
				return m_uCell.pValue;
			}

			void SetInteger( int iValue ){ m_uCell.iValue = iValue; }
			int GetInteger() const
			{ 
				if( !this ) 
				{
					ASSERT( 0 && "GetInteger() - Null" );
					return 0;
				}
				return m_uCell.iValue;
			}

			bool GetBool() const
			{
				return GetInteger() ? true : false;
			}

			void SetFloat( float fValue ){ m_uCell.fValue = fValue; }
			float GetFloat() const
			{
				if( !this ) 
				{
					ASSERT( 0 && "GetFloat() - Null" );
					return 0.f;
				}
				return m_uCell.fValue;
			}

		private:

			union uCell
			{
				char* pValue;
				int	  iValue;
				float fValue;
			};

			uCell m_uCell;
	};

#pragma pack(pop)

	CDNTableFile();
	CDNTableFile( const char *szFileName );
	virtual ~CDNTableFile();

	void SetLoadName( const char* pszFileName ){ m_strFileName=pszFileName; }
	void GetLoadName( std::string& strFileName ){ strFileName=m_strFileName; }
	bool SetGenerationInverseLabel( char* pszStr );
	bool Load( std::vector<CStream*>& VecStream, bool bDontUseItemIDData = false );
	bool Load( const char* pszFileName, bool bResetData=true );

	bool IsExistItem( int iItemID );
	int GetIDXprimary( int iItemID ){ return _GetStartCellIndexFromItemID(iItemID); }
	
	int GetFieldNum( const char* pszFieldName ){ return _GetFieldIndexFromFieldName( pszFieldName ); }
	bool GetFieldFromLable( int iItemID, const char* pszFieldName, Cell& Cell );
	Cell* GetFieldPtr( int iItemID, int iFieldNum );
	Cell* GetFieldFromLablePtr( int iItemID, const char* pszFieldName );
	Cell* GetFieldFromLablePtr( int iStartCellIndex, int iFieldNum );
	int GetItemCount(){ return static_cast<int>(m_mItemIDStartCellIndex.size()); }
	int GetItemID( UINT uiIndex );
	int GetArrayIndex( int iItemID );
	int GetFieldCount(){ return m_nFieldCount; }
	
	int GetItemIDListFromField( const char *szFieldLabel, int nValue, std::vector<int> &nVecList, bool bClearList = true );
	std::vector<std::string>* GetVecGenerationInverseLabelList() { return &m_vGenerationInverseLabel; }

	int GetItemIDFromField( const char *pszFieldName, const char *szValue );
	int GetItemIDFromField( const char* pszFieldName, int iValue );
	int GetItemIDFromFieldCaseFree( const char* pszFieldName, const char* pszValue );

private:
	void _Reset();
	bool _Load( CStream* pStream, bool bResetData=true );
	int _GetStartCellIndexFromItemID( int iItemID );
	int	_GetFieldIndexFromFieldName( const char* pszFieldName );

	short m_nFieldCount;
	int m_iDataCount;
	
	typedef std::map<std::string, std::vector<int>> TDStringItemID;
	typedef std::map<int, std::vector<int>> TDIntItemID;
	typedef std::map<float, std::vector<int>> TDFloatItemID;
	typedef std::map<std::string,int> TDFieldNameFieldIndex;
	typedef std::map<int,int> TDItemIDCellIndex;

	std::vector<std::string> m_vLabel;
	std::vector<int> m_vItemID;
	
	TDStringItemID* m_pmStringItemID;
	TDIntItemID* m_pmIntItemID;
	TDFloatItemID* m_pmFloatItemID;
	TDFieldNameFieldIndex m_mFieldNameFieldIndex;
	TDItemIDCellIndex m_mItemIDStartCellIndex;

	std::vector<Cell> m_vCellRepository;
	std::vector<std::string> m_vGenerationInverseLabel;

	StringPool<char> m_StringPool;
	std::string m_strFileName;

public:
	bool m_bDontUseItemIDData;
	bool m_bAutoUnload;
	float m_fHoldingTime;
};

typedef CDNTableFile DNTableFileFormat;
typedef CDNTableFile::Cell DNTableCell;
