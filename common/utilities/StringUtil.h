#pragma once

#include <deque>
#include <string>

// Because std::string dynamically allocates all strings of size 16 or larger 
// Since most filenames exceed 16 characters, frequent dynamic allocations during filename checks have high overhead.
// Use a custom class with a reasonably large buffer size for filename strings.

// typedef std::string CFileNameString; // Enable this if issues occur during use.

class CFileNameString
{
public:
	enum {
		INTERNAL_BUFFER_SIZE = 50,			// Most filenames are under 100 chars; dynamically allocate only if over 100.
	};
	typedef size_t size_type;

private:
	char m_szInternalBuffer[ INTERNAL_BUFFER_SIZE + 1 ];
	char *m_dynamicBuffer;	
	size_type m_nSize;
	size_type m_nCapacity;

private:
	void _Assign( const char *pString ) 
	{
		size_type nLen = strlen( pString );
		if( nLen <= INTERNAL_BUFFER_SIZE ) {
			if( nLen == 0 ) {
				m_szInternalBuffer[0]='\0';
			}
			else {				
				strcpy_s( m_szInternalBuffer, pString );				
			}
		}
		else {
			m_szInternalBuffer[ 0 ] = '\0';
			if( m_nCapacity < nLen ) {
				if( m_nCapacity != 0 ) {
					delete [] m_dynamicBuffer;
				}
				m_dynamicBuffer = new char[ nLen + 1];
				m_nCapacity = nLen;
			}
			strcpy_s( m_dynamicBuffer, nLen+1, pString );
		}
		m_nSize = nLen;
	}

	void _Reset() {
		m_szInternalBuffer[ 0 ] = '\0';				
		m_dynamicBuffer = NULL;
		m_nSize = 0;
		m_nCapacity = 0;
	}

	void _Destroy() {
		if( m_dynamicBuffer ) {
			delete [] m_dynamicBuffer;
			m_dynamicBuffer = NULL;
			m_nCapacity = 0;
		}
	}

public:
	CFileNameString() {
		_Reset();		
	}

	CFileNameString( const char *pString ) {
		_Reset();
		_Assign( pString );
	}

	CFileNameString( std::string &szString ) {
		_Reset();
		_Assign( szString.c_str() );
	}
	CFileNameString( const CFileNameString &szString )
	{
		_Reset();
		_Assign( szString.c_str() );
	}

	~CFileNameString() {
		_Destroy();
	}

public:
	void operator = ( const CFileNameString &rhs )
	{
		_Assign( rhs.c_str() );
	}

	void operator = ( const char *pString )
	{
		_Assign( pString );
	}

	void operator = ( char *pString ) 
	{
		_Assign( pString );
	}

	void operator = ( std::string &szString ) 
	{
		_Assign( szString.c_str() );
	}

	CFileNameString& operator += ( const CFileNameString& rhs ) 
	{
		std::string szFullString = (char*)c_str();
		szFullString += (char*)rhs.c_str();
		_Assign( (char*)szFullString.c_str() );
		return *this;
	}

	CFileNameString operator + ( const CFileNameString& rhs ) 
	{
		std::string szFullString = (char*)c_str();
		szFullString += (char*)rhs.c_str();		
		return CFileNameString( szFullString.c_str() ) ;
	}
	size_t size()
	{
		return m_nSize;
	}

	const char * c_str() const
	{
		return ( m_nSize <= INTERNAL_BUFFER_SIZE ) ? m_szInternalBuffer : m_dynamicBuffer;
	}

	char * begin()
	{
		return const_cast<char*>(c_str());
	}

	char * end()
	{
		return const_cast<char*>(c_str()) + m_nSize;
	}

	const char & operator [] ( unsigned int index ) const
	{
		return c_str()[ index ];
	}

	const bool operator < ( const CFileNameString & rhs ) const
	{
		return (strcmp( c_str(), rhs.c_str() )	< 0 );
	}

	bool operator == ( char *rhs )
	{
		return (strcmp( c_str(), rhs ) == 0 );
	}

	bool operator == ( CFileNameString &rhs )
	{
		return (strcmp( c_str(), rhs.c_str() ) == 0 );
	}

	bool empty() 
	{ 
		return ( m_nSize == 0 );
	}
};

template <class T, int _PoolSize = 10240 >
class StringPool
{
public:
	StringPool() : m_pBuffer( NULL ), m_nAllocSize(0), m_nSize(0) {}
	~StringPool() {
		Clear();
	} 

private:
	std::vector<T*> m_BufferList;
	T *m_pBuffer;
	size_t m_nAllocSize;
	size_t m_nSize;

public:
	void Clear() {
		typename std::vector<T*>::iterator it = m_BufferList.begin();
		while( it != m_BufferList.end() ) {
			delete [] (*it);
			++it;
		}
		m_BufferList.clear();
	}

	void Reserve( size_t nSize ) {		
		if( m_pBuffer == NULL ) {
			m_pBuffer = new T[ _PoolSize ];
			m_BufferList.push_back( m_pBuffer );
		}
		else {
			if( m_nSize + nSize > _PoolSize ) {
				m_pBuffer = new T[ _PoolSize ];
				m_BufferList.push_back( m_pBuffer );
				m_nSize = 0;
			}
		}		
	}

	T *Alloc( T *pSource , size_t nLength ) {		
		ASSERT( (nLength+1 < _PoolSize) && (nLength >= 0));
		if( nLength == 0 ) {
			static T cNullTerminate = '\0';
			return &cNullTerminate;
		}
		Reserve( (nLength + 1) );
		if( m_nSize > nLength+1 ) {
			if( memcmp( m_pBuffer + m_nSize - (nLength+1) , pSource, (nLength+1)*sizeof(T) ) == 0 ) {
				return m_pBuffer + m_nSize - (nLength+1);
			}
		}
		memcpy_s( m_pBuffer + m_nSize, (_PoolSize - m_nSize)*sizeof(T), pSource, sizeof(T) * nLength );
		m_pBuffer[ m_nSize + nLength ] = 0;
		T *pResult = m_pBuffer + m_nSize;
		m_nSize += (nLength+1);
		return pResult;
	}
};

bool ToMultiString(IN const std::wstring& wstr, OUT std::string& str );
bool ToWideString(IN const std::string& str, OUT std::wstring& wstr );

bool ToMultiString(IN WCHAR* wstr, OUT std::string& str );
bool ToWideString(IN CHAR* str, OUT std::wstring& wstr );

// Convert to lowercase
void ToLowerA(std::string& str);
void ToLowerA( CFileNameString& str);
void ToLowerW(std::wstring& str);

// Convert to uppercase
void ToUpperA(std::string& str);
void ToUpperW(std::wstring& str);

std::wstring FormatW( const WCHAR* fmt, ... );
std::string FormatA( const CHAR* fmt, ... );

/*
When splitting a string by whitespace.. roughly write it like this.
	std::vector<std::string> tokens;
	std::string str("-IP 127.0.0.1 -Port 5000", );
	Tokenize( str, tokens, " ");
*/

void TokenizeA( const std::string& str, std::vector<std::string>& tokens, const std::string& delimiters = " " );
void TokenizeW( const std::wstring& str, std::vector<std::wstring>& tokens, const std::wstring& delimiters = L" " );
void TokenizeW( const std::wstring& str, std::deque<std::wstring>& tokens, const std::wstring& delimiters = L" " );
void TokenizeW_NewLine( const std::wstring& str, std::vector<std::wstring>& tokens, const std::wstring& delimiters = L" " );

/*
	std::vector<std::string> tokens;
	std::string str("{user_nick} {user_class}", );
Tokenize( str, tokens, "{", "}" ); // last argument specifies whether to include delimiters
*/
void TokenizeW( const std::wstring& str, std::vector<std::wstring>& tokens, const std::wstring& szBegin, const std::wstring& szEnd, bool bIncludeDelimiters );
void TokenizeA( const std::string& str, std::vector<std::string>& tokens, const std::string& szBegin, const std::string& szEnd, bool bIncludeDelimiters  );


// Remove all whitespace from the string.
void RemoveSpaceA(std::string& str);
void RemoveSpaceW(std::wstring& str);

// Remove del from the string.
void RemoveStringA(std::string& str, const std::string& del);
void RemoveStringW(std::wstring& str, const std::wstring& del);

// Replace all occurrences of szOld in str with szNew.
void AllReplaceW( std::wstring& str, std::wstring& szOld, std::wstring& szNew );
void AllReplaceA( std::string& str, std::string& szOld, std::string& szNew );

// Trim leading and trailing whitespace from string
void TrimString( char *pString );


// Gold - insert comma ',' every 3 digits.
std::wstring GoldToMoneyString( INT64 nGold );

// Comma string.
std::wstring AtoCommaString( int nValue );


// : Gold acquired - do not print units that are 0 (e.g. 1 Gold 0 Silver 1 Copper => 1 Gold 1 Copper).
void GoldToString( std::wstring & rStr, INT64 coin, const wchar_t * pStrGold, const wchar_t * pStrSilver, const wchar_t * pStrCopper );
bool IsExistString( const std::wstring& wszString, const std::wstring& wszFindString );