#include "stdafx.h"
#include "DnPlayerDoNotEnterChecker.h"
#include "DnWorld.h"
#include "DnPlayerActor.h"
#include "DNUserSession.h"

CDnPlayerDoNotEnterChecker::CDnPlayerDoNotEnterChecker( CDnPlayerActor* pActor )
: IDnPlayerChecker( pActor )
{
	m_nInvalidCount = 0;
	m_nValidCount = 0;
	m_LastCheckTime = 0;
	m_nCheckerCount = 0;
}

CDnPlayerDoNotEnterChecker::~CDnPlayerDoNotEnterChecker()
{
}

void CDnPlayerDoNotEnterChecker::Process( LOCAL_TIME LocalTime, float fDelta )
{
	if( m_LastCheckTime == 0 ) m_LastCheckTime = LocalTime;
	else if( m_LastCheckTime > LocalTime ) m_LastCheckTime = LocalTime;

	// Check once per second
	if( LocalTime - m_LastCheckTime > 1000 ) 
	{
		m_LastCheckTime = LocalTime; // Since it could trigger when the server slows down, do not do m_nLastCheckTime -= 1000 and just set it to the current time.
		if( IsInvalidPosition() == true ) 
		{
			m_nValidCount = 0;
			m_nInvalidCount++;
			if( m_nInvalidCount == 1 ) 
			{
				if( m_pActor && m_pActor->GetUserSession() )
				{
					g_Log.Log( LogType::_HACK, m_pActor->GetUserSession(), L"HackChecker(DoNotEnter) : CharName=%s InvalidCount=%d, ValidCount=%d MapIndex=%d\n", m_pActor->GetUserSession()->GetCharacterName(), m_nInvalidCount, m_nValidCount, m_pActor->GetUserSession()->GetMapIndex() );					
				}
				
				m_pActor->OnInvalidPlayerChecker( 5 );
				m_nCheckerCount += 5;
			}
		}
		else 
		{
			m_nValidCount++;
		}
	}

	if( m_nValidCount > 10 ) 
	{
		m_nInvalidCount = 0;
		m_nCheckerCount = 0;
	}
}

bool CDnPlayerDoNotEnterChecker::IsInvalidPosition()
{
	if( !m_pActor ) return false;
	if( !CDnWorld::IsActive(m_pActor->GetRoom()) ) return false;
	int nBlockSize = CDnWorld::GetInstance(m_pActor->GetRoom()).GetAttributeBlockSize( m_pActor->GetPosition()->x, m_pActor->GetPosition()->z );
	int nBlockCount = 0;

	EtVector3 vPos = *m_pActor->GetPosition();

	EtVector2 vSize;
	CDnWorld::GetInstance(m_pActor->GetRoom()).CalcWorldSize( vSize );
	vSize /= 2.f;
	
	if( vPos.x <= -vSize.x || vPos.x >= vSize.x || vPos.z <= -vSize.y || vPos.z >= vSize.y ) return true;

	char cAttr = CDnWorld::GetInstance(m_pActor->GetRoom()).GetAttribute( vPos );
	if( ( cAttr & 0x0f ) == 1 || ( cAttr & 0x0f ) == 2 ) 
	{
		nBlockCount++;

		for( int i=-1; i<2; i++ ) 
		{
			for( int j=-1; j<2; j++ ) 
			{
				if( i == 0 && j == 0 ) continue;

				vPos = *m_pActor->GetPosition() + EtVector3( (float)( nBlockSize * j ), 0.f, (float)( nBlockSize * i ) );
				cAttr = CDnWorld::GetInstance(m_pActor->GetRoom()).GetAttribute( vPos );
				
				if( ( cAttr & 0x0f ) == 1 || ( cAttr & 0x0f ) == 2 ||
					( vPos.x <= -vSize.x || vPos.x >= vSize.x || vPos.z <= -vSize.y || vPos.z >= vSize.y ) ) 
				{
					nBlockCount++;
					if( nBlockCount >= 9 )
					{		
						if( m_nInvalidCount == 0 ) 
						{
							EtVector3 vPrevPos = *m_pActor->GetPosition();
							g_Log.Log( LogType::_HACK, m_pActor->GetUserSession(), L"HackChecker(DoNotEnter:Position) : Prev Position X:%f Y:%f Z:%f\n", vPrevPos.x, vPrevPos.y, vPrevPos.z );
							g_Log.Log( LogType::_HACK, m_pActor->GetUserSession(), L"HackChecker(DoNotEnter:Position) : Calc Position X:%f Y:%f Z:%f\n", vPos.x, vPos.y, vPos.z );
						}						
						return true;
					}
				}
			}
		}
	}
	return false;
}

bool CDnPlayerDoNotEnterChecker::IsInvalidPlayer()
{
	return ( m_nInvalidCount > 0 ) ? true : false;
}

void CDnPlayerDoNotEnterChecker::ResetInvalid()
{
	m_nInvalidCount = 0;
	if( m_pActor && m_pActor->GetUserSession() ) 
	{
		m_pActor->OnInvalidPlayerChecker( -m_nCheckerCount );
		m_nCheckerCount = 0;
		g_Log.Log( LogType::_HACK, m_pActor->GetUserSession(), L"[CDnPlayerDoNotEnterChecker] Player stuck in restricted area; prior log is valid. CharName=%s\n", m_pActor->GetUserSession()->GetCharacterName() );
	}
}