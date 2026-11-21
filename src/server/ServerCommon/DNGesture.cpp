#include "stdafx.h"
#include "DNGesture.h"

CDNGesture::CDNGesture(CDNUserSession * pSession)
{
	m_pSendManager = (CDNUserSendManager*)pSession;
	m_MapEffectItemGestureList.clear();
}

void CDNGesture::AddCashGesture( int nGestureID )
{
	m_MapCashGestureList[nGestureID] = true;
}

void CDNGesture::DelCashGesture( int nGestureID )
{
	m_MapCashGestureList.erase( nGestureID );
}

void CDNGesture::GetCashGestureList( TGesture * pGestureArr )
{
	int idx = 0;
	for( std::map <int, bool>::iterator iter=m_MapCashGestureList.begin(); iter != m_MapCashGestureList.end(); iter++ )
	{
		if( idx >= GESTUREMAX )	break;
		pGestureArr[idx].nGestureID = iter->first;
		idx++;
	}
}

bool CDNGesture::HasCashGesture( int nGestureID )
{
	std::map <int, bool>::iterator iter = m_MapCashGestureList.find( nGestureID );
	if( iter == m_MapCashGestureList.end() )	return false;

	return iter->second;
}

void CDNGesture::AddEffectItemGesture( int nGestureID )
{
	m_MapEffectItemGestureList[nGestureID] = true;
}

void CDNGesture::DelEffectItemGesture( int nGestureID )
{
	m_MapEffectItemGestureList.erase( nGestureID );
}

void CDNGesture::GetEffectItemGestureList( TGesture * pGestureArr )
{
	int idx = 0;
	for( std::map <int, bool>::iterator iter=m_MapEffectItemGestureList.begin(); iter != m_MapEffectItemGestureList.end(); iter++ )
	{
		if( idx >= GESTUREMAX )	break;
		pGestureArr[idx].nGestureID = iter->first;
		idx++;
	}
}

bool CDNGesture::HasEffectItemGesture( int nGestureID )
{
	std::map <int, bool>::iterator iter = m_MapEffectItemGestureList.find( nGestureID );
	if( iter == m_MapEffectItemGestureList.end() )	return false;

	return iter->second;
}