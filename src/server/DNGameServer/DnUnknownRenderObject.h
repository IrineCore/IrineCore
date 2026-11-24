#pragma once

#include "MultiSmartPtr.h"
#include "Timer.h"
#include "MultiElement.h"

template < class T, int MAXCOUNT >
class CDnUnknownRenderObject :  virtual public CMultiElement, public CMultiSmartPtrBase< T, MAXCOUNT >
{
public:
	CDnUnknownRenderObject( CMultiRoom *pRoom, bool bProcess ) 
		: CMultiSmartPtrBase< T, MAXCOUNT >( pRoom )
	{
		m_bDestroy = false;
		m_bProcess = bProcess;
		if( bProcess )
			STATIC_INSTANCE(s_pVecProcessList).push_back( static_cast< T *>( this ) );
	}
	virtual ~CDnUnknownRenderObject()
	{
		SetProcess( false );
	}

	virtual void Process( LOCAL_TIME LocalTime, float fDelta ) {}

	void SetProcess( bool bFlag ) 
	{
		if( m_bProcess == bFlag ) return;
		m_bProcess = bFlag;

		if( m_bProcess == false ) {
			for( DWORD i=0; i<STATIC_INSTANCE(s_pVecProcessList).size(); i++ ) {
				if( STATIC_INSTANCE(s_pVecProcessList)[i] == this ) {
					STATIC_INSTANCE(s_pVecProcessList).erase( STATIC_INSTANCE(s_pVecProcessList).begin() + i );
					break;
				}
			}
		}
		else {
			STATIC_INSTANCE(s_pVecProcessList).push_back( static_cast< T *>( this ) );
		}
	}
	bool IsProcess() { return m_bProcess; }

	bool IsDestroy() { return m_bDestroy; }
	void SetDestroy() { m_bDestroy = true; }

protected:
	bool m_bProcess;
	bool m_bDestroy;

public:
	static std::vector<T *> s_pVecProcessList[MAXCOUNT];
};

#define DECL_DN_SMART_PTR_STATIC( name, MAXCOUNT, count ) \
	template<> std::vector<name *> CDnUnknownRenderObject<name, MAXCOUNT>::s_pVecProcessList[MAXCOUNT]{}; \
	DECL_MULTISMART_PTR_STATIC( name, MAXCOUNT, count ) 
