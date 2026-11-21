/*//===============================================================

	<< BIT SET EX (1) >>

	PRGM : B4nFter

	FILE : BITSETEX1.HPP
DESC : Bitset class implementation (static, fixed-size, fast)
	INIT BUILT DATE : 2004. 06. 06
	LAST BUILT DATE : 2008. 05. 20

Copyright (c) 2002~2009 B4nFter (b4nfter@gmail.com) and Co-Writers. All Rights Reserved.

	P.S.>
- Designed to support both normal bit operations and bit flag operations simultaneously (see class declaration for method distinction)
- Limited to TTYPE size, but simplifies operations for higher speed

*///===============================================================


#pragma once


///////////////////////////////////////////////////////////////////
//	INCLUDE
///////////////////////////////////////////////////////////////////

#include "CommonMacros.h"
#include "BaseSet.h"
#include "DebugUtil.h"
#include "SafeStringMacros.h"


///////////////////////////////////////////////////////////////////
//	DEFINE
///////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////
//	DECLARATION
///////////////////////////////////////////////////////////////////

//*---------------------------------------------------------------
// TYPE : CLASS
// NAME : CBitSetEx1
//*---------------------------------------------------------------
// DESC : Bitset class (static, fixed-size, fast)
// PRGM : B4nFter
//*---------------------------------------------------------------
template<INT TMAX, class TTYPE>
class CBitSetEx1
{
public:
	typedef TTYPE TP_TYPE;

public:
	explicit CBitSetEx1(TTYPE pBitSet = static_cast<TTYPE>(0));

	// Bitwise operation methods
	BOOL Get(ULONG pBitPos);
	BOOL IsSet() const;
	VOID Set(ULONG pBitPos, BOOL pBitVal);
	VOID SetAll(BOOL pBitVal);
	UINT GetMax() const { return TMAX; }
	UINT GetBitSize() const { return sizeof(TP_TYPE)*CHAR_BIT; }

	// Flag operation method (flag start value must be 1)
	BOOL IsSet(TTYPE pBitSet) const;
	BOOL IsReset(TTYPE pBitSet) const;
	BOOL IsSetOther(TTYPE pBitSet) const;	// Whether flags other than the specified flag are SET
	VOID Set(TTYPE pBitSet);
	VOID Set();
	VOID Reset(TTYPE pBitSet);
	VOID Reset();
	TP_TYPE GetRaw() const;

	BOOL operator[](ULONG pBitPos);

private:
	TTYPE m_BitSet;
};


///////////////////////////////////////////////////////////////////
//	IMPLEMENTATION
///////////////////////////////////////////////////////////////////

//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CBitSetEx1::CBitSetEx1
//*---------------------------------------------------------------
// DESC : Constructor (requires arguments)
// PARM :	1 . init size
//			2 . init value
// RETV : N/A
// PRGM : B4nFter
//*---------------------------------------------------------------
template<INT TMAX, class TTYPE>
inline CBitSetEx1<TMAX, TTYPE>::CBitSetEx1(TTYPE pBitSet) : m_BitSet(pBitSet)
{
	DN_ASSERT(!CDebugUtil::IsOverflow(static_cast<TTYPE>(0), static_cast<TTYPE>(TMAX)),	"Overflow!");
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CBitSetEx1::Get
//*---------------------------------------------------------------
// DESC : Get the bit value at a specific position
// PARM : N/A
// RETV : bit value at specific position (0/1)
// PRGM : B4nFter
//*---------------------------------------------------------------
template<INT TMAX, class TTYPE>
inline BOOL CBitSetEx1<TMAX, TTYPE>::Get(ULONG pBitPos)
{
	if (!CHECK_LIMIT(pBitPos, GetBitSize())) {
		DN_RETURN(FALSE);
	}

	return (m_BitSet >> pBitPos) & 1;
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CBitSetEx1::IsSet
//*---------------------------------------------------------------
// DESC : Check if bitset value is non-zero
// PARM : N/A
// RETV : TRUE - success / FALSE - failure
// PRGM : B4nFter
//*---------------------------------------------------------------
template<INT TMAX, class TTYPE>
inline BOOL CBitSetEx1<TMAX, TTYPE>::IsSet() const
{
	return (0 != m_BitSet);
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CBitSetEx1::IsSet
//*---------------------------------------------------------------
// DESC : Check if specified bit in bitset is non-zero
// PARM : 1 . pBitSet - Specific value
// RETV : TRUE - success / FALSE - failure
// PRGM : B4nFter
//*---------------------------------------------------------------
template<INT TMAX, class TTYPE>
inline BOOL CBitSetEx1<TMAX, TTYPE>::IsSet(TTYPE pBitSet) const
{
	DN_ASSERT((0 != pBitSet) && (pBitSet <= TMAX),	"Invalid!");
	return (0 != (pBitSet & m_BitSet));
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CBitSetEx1::IsReset
//*---------------------------------------------------------------
// DESC : Check if specified bit in bitset is 0
// PARM : 1 . pBitSet - Specific value
// RETV : TRUE - success / FALSE - failure
// PRGM : B4nFter
//*---------------------------------------------------------------
template<INT TMAX, class TTYPE>
inline BOOL CBitSetEx1<TMAX, TTYPE>::IsReset(TTYPE pBitSet) const
{
	DN_ASSERT((0 != pBitSet) && (pBitSet <= TMAX),	"Invalid!");
	return (0 == (pBitSet & m_BitSet));
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CBitSetEx1::IsSetOther
//*---------------------------------------------------------------
// DESC : Check if remaining bits other than specified bit in bitset are non-zero
// PARM : 1 . pBitSet - Specific value
// RETV : TRUE - success / FALSE - failure
// PRGM : B4nFter
//*---------------------------------------------------------------
template<INT TMAX, class TTYPE>
inline BOOL CBitSetEx1<TMAX, TTYPE>::IsSetOther(TTYPE pBitSet) const
{
	DN_ASSERT((0 != pBitSet) && (pBitSet <= TMAX),	"Invalid!");
	return (0 != ((~pBitSet) & m_BitSet));
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CBitSetEx1::Set
//*---------------------------------------------------------------
// DESC : Set entire bitset to specified bit value
// PARM : 1 . pBitSet - Specific value
// RETV : N/A
// PRGM : B4nFter
//*---------------------------------------------------------------
template<INT TMAX, class TTYPE>
inline VOID CBitSetEx1<TMAX, TTYPE>::Set(TTYPE pBitSet)
{
	DN_ASSERT((0 != pBitSet) && (pBitSet <= TMAX),	"Invalid!");
	m_BitSet |= (pBitSet);
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CBitSetEx1::Set
//*---------------------------------------------------------------
// DESC : Set all bits in bitset to 1
// PARM : N/A
// RETV : N/A
// PRGM : B4nFter
//*---------------------------------------------------------------
template<INT TMAX, class TTYPE>
inline VOID CBitSetEx1<TMAX, TTYPE>::Set()
{
	SetAll(1);
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CBitSetEx1::Set
//*---------------------------------------------------------------
// DESC : Set the bit at a specific position to a specific bit value
// PARM : 1 . pBitPos - Bit position
//			2 . pBitVal - bit value to set
// RETV : N/A
// PRGM : B4nFter
//*---------------------------------------------------------------
template<INT TMAX, class TTYPE>
inline VOID CBitSetEx1<TMAX, TTYPE>::Set(ULONG pBitPos, BOOL pBitVal)
{
	if (!CHECK_LIMIT(pBitPos, GetBitSize())) {
		DN_ASSERT(0,	"Invalid!");
		return;
	}

	if (pBitVal) {
		m_BitSet |= (static_cast<TTYPE>(1) << pBitPos);
	}
	else {
		m_BitSet &= (~(static_cast<TTYPE>(1) << pBitPos));
	}
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CBitSetEx1::SetAll
//*---------------------------------------------------------------
// DESC : Set entire bitset to specified bit value
// PARM : N/A
// RETV : N/A
// PRGM : B4nFter
//*---------------------------------------------------------------
template<INT TMAX, class TTYPE>
inline VOID CBitSetEx1<TMAX, TTYPE>::SetAll(BOOL pBitVal)
{
	if (pBitVal) {
		::memset(&m_BitSet, 0xFF, sizeof(m_BitSet));
	}
	else {
		::memset(&m_BitSet, 0x00, sizeof(m_BitSet));
	}
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CBitSetEx1::Reset
//*---------------------------------------------------------------
// DESC : Initialize bitset
// PARM : N/A
// RETV : N/A
// PRGM : B4nFter
//*---------------------------------------------------------------
template<INT TMAX, class TTYPE>
VOID CBitSetEx1<TMAX, TTYPE>::Reset()
{
	m_BitSet = static_cast<TTYPE>(0);
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CBitSetEx1::Reset
//*---------------------------------------------------------------
// DESC : Set bitset to bitwise inverse of specified bitset value
// PARM : 
// RETV : N/A
// PRGM : B4nFter
//*---------------------------------------------------------------
template<INT TMAX, class TTYPE>
VOID CBitSetEx1<TMAX, TTYPE>::Reset(TTYPE pBitSet)
{
	DN_ASSERT((0 != pBitSet) && (pBitSet <= TMAX),	"Invalid!");
	m_BitSet &= ~(pBitSet);
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CBitSetEx1::GetRaw
//*---------------------------------------------------------------
// DESC : Get actual value of bitset
// PARM : N/A
// RETV : actual value of type TTYPE
// PRGM : B4nFter
//*---------------------------------------------------------------
template<INT TMAX, class TTYPE>
typename CBitSetEx1<TMAX, TTYPE>::TP_TYPE CBitSetEx1<TMAX, TTYPE>::GetRaw() const
{
	return m_BitSet;
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CBitSetEx1::operator[]
//*---------------------------------------------------------------
// DESC : Get the value of a specific bit
// PARM : N/A
// RETV : value of specific bit
// PRGM : B4nFter
//*---------------------------------------------------------------
template<INT TMAX, class TTYPE>
inline BOOL CBitSetEx1<TMAX, TTYPE>::operator[](ULONG pBitPos)
{
	return Get(pBitPos);
}
