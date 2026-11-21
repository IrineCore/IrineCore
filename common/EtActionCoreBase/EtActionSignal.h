#pragma once

#include "MemPool.h"

class CEtActionSignal : public TBoostMemoryPool< CEtActionSignal >
{
public:
	CEtActionSignal();
	virtual ~CEtActionSignal();

protected:
	short m_nSignalIndex;
	short m_nSignalListArrayIndex;	// SignalList ArrayIndex
	short m_nStartFrame;
	short m_nEndFrame;
	char m_cOrder;

	void *m_pData;

	std::vector<short> m_nVec2Index;
	std::vector<short> m_nVec3Index;
	std::vector<short> m_nVec4Index;
	std::vector<short> m_nVecStrIndex;

#ifdef WIN64
	std::vector<int> m_nVec2Index64;
	std::vector<int> m_nVec3Index64;
	std::vector<int> m_nVec4Index64;
	std::vector<int> m_nVecStrIndex64;
#endif

protected:
#ifdef WIN64
	char IsPointerTable( int nIndex );
	void IncreasePointerTableIndex( int nValue );
#endif

public:
	bool LoadSignal( CStream *pStream );
	static bool SkipSignal( CStream *pStream );
	void *GetData() { return m_pData; }

	__forceinline bool CheckSignal( float fPrev, float fCur )
	{
		if( ( fCur >= ( float )m_nStartFrame ) && ( fCur < ( float )m_nEndFrame ) )
		{
			if( ( ( int )fPrev == ( int )fCur ) && ( fCur != 0.0f ) )
			{
				return false;
			}
			else
			{
				return true;
			}
		}
		if( ( fCur >= ( float )m_nEndFrame ) && ( fPrev < ( float )m_nStartFrame ) )
		{
			return true;
		}

		return false;
	}	

	int		GetSignalIndex() { return m_nSignalIndex; }
	void	SetSignalListArrayIndex( const int iIdx ){ m_nSignalListArrayIndex = iIdx; }
	int		GetSignalListArrayIndex() const { return m_nSignalListArrayIndex; }
	int		GetStartFrame() { return m_nStartFrame; }
	int		GetEndFrame() { return m_nEndFrame; }
	int		GetOrder() { return m_cOrder; }
	void	SetStartFrame( int nValue ) { m_nStartFrame = nValue; }
	void	SetEndFrame( int nValue ) { m_nEndFrame = nValue; }

	// Functions to avoid using if possible
	// For pointers, to maintain compatibility with previously saved files
	// Use these functions when Null handling is required
	// Add Table.
	void InsertVec2Table( EtVector2 *pPtr, EtVector2 &vVec );
	void InsertVec3Table( EtVector3 *pPtr, EtVector3 &vVec );
	void InsertVec4Table( EtVector4 *pPtr, EtVector4 &vVec );
	void InsertStrTable( char *pPtr, std::string &szStr );
};