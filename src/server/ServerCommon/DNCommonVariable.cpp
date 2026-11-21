#include "stdafx.h"
#include "DNCommonVariable.h"
#include "DNUserSession.h"
#include "DNDBConnection.h"

CDNCommonVariable::CDNCommonVariable(CDNUserSession *pSession)
{
	m_mVariableData.clear();
	m_pSession = pSession;
}

CDNCommonVariable::~CDNCommonVariable(void)
{
}

bool CDNCommonVariable::InitializeData( const TAGetListVariableReset* pA )
{
	for( int i=0 ; i<pA->cCount ; ++i )
	{
		if( m_mVariableData.find( pA->Data[i].Type ) == m_mVariableData.end() )
		{
			std::pair<std::map<CommonVariable::Type::eCode, CommonVariable::Data>::iterator,bool> Ret = m_mVariableData.insert( std::make_pair(pA->Data[i].Type, pA->Data[i]) );
			if( Ret.second == false )
				return false;
		}
	}
	return true;
}

bool CDNCommonVariable::IncData( CommonVariable::Type::eCode Type, __time64_t tDate/*=0*/)
{
	time_t Time;
	if( 0 != tDate )
		Time = tDate;
	else
		time(&Time);

	std::map<CommonVariable::Type::eCode, CommonVariable::Data>::iterator itor = m_mVariableData.find( Type );
	if( itor == m_mVariableData.end() )
	{
		CommonVariable::Data Data;
		memset( &Data, 0, sizeof(Data) );

		Data.Type = Type;
		std::pair<std::map<CommonVariable::Type::eCode, CommonVariable::Data>::iterator,bool> Ret = m_mVariableData.insert( std::make_pair(Type, Data ) );
		if( Ret.second == false )
			return false;
		itor = Ret.first;
	}

	(*itor).second.biValue++;
	(*itor).second.tLastModifyDate = Time;

	m_pSession->SendModCommonVariableData((*itor).second );
	m_pSession->GetDBConnection()->QueryModVariableReset( m_pSession, Type, (*itor).second.biValue, Time );

	return true;
}

bool CDNCommonVariable::ModData( CommonVariable::Type::eCode Type, INT64 biValue, __time64_t tDate/*=0*/)
{
	time_t Time;
	if( 0 != tDate )
		Time = tDate;
	else
		time(&Time);

	std::map<CommonVariable::Type::eCode, CommonVariable::Data>::iterator itor = m_mVariableData.find( Type );
	if( itor == m_mVariableData.end() )
	{
		CommonVariable::Data Data;
		memset( &Data, 0, sizeof(Data) );

		Data.Type = Type;
		std::pair<std::map<CommonVariable::Type::eCode, CommonVariable::Data>::iterator,bool> Ret = m_mVariableData.insert( std::make_pair(Type, Data ) );
		if( Ret.second == false )
			return false;
		itor = Ret.first;
	}

	(*itor).second.biValue = biValue;
	(*itor).second.tLastModifyDate = Time;

	m_pSession->SendModCommonVariableData((*itor).second );
	m_pSession->GetDBConnection()->QueryModVariableReset( m_pSession, Type, biValue, Time );
	return true;
}

bool CDNCommonVariable::GetDataValue( CommonVariable::Type::eCode Type, INT64& biValue )
{
	std::map<CommonVariable::Type::eCode, CommonVariable::Data>::iterator itor = m_mVariableData.find( Type );
	if( itor == m_mVariableData.end() )
		return false;

	biValue = (*itor).second.biValue;
	return true;
}

bool CDNCommonVariable::GetDataModDate(CommonVariable::Type::eCode Type, __time64_t& tDate)
{
	std::map<CommonVariable::Type::eCode, CommonVariable::Data>::iterator itor = m_mVariableData.find( Type );
	if( itor == m_mVariableData.end() )
		return false;

	tDate = (*itor).second.tLastModifyDate;
	return true;
}

void CDNCommonVariable::SendCommonVariableData(CommonVariable::Type::eCode Type)
{
	std::map<CommonVariable::Type::eCode,CommonVariable::Data>::iterator itor = m_mVariableData.find( Type );
	if( itor != m_mVariableData.end() )
	{
		m_pSession->SendModCommonVariableData((*itor).second);
	}				
}
