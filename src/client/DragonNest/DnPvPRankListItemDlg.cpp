#include "Stdafx.h"

#ifdef PRE_ADD_PVP_RANKING

#include "DNPacket.h"

#include "DnInterface.h"
#include "DnInterfaceString.h"
#include "DnPvPRankListItemDlg.h"


#define INCREASE_MARKER L"\u25B2"
#define DECREASE_MARKER L"\u25BC"


CDnPvPRankListItemDlg::CDnPvPRankListItemDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback, bool bAutoCursor )
: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback, bAutoCursor )
,m_pStaticRank(NULL)
,m_pStaticLevel(NULL)
,m_pStaticGuild(NULL)
,m_pStaticName(NULL)
,m_pStaticResult(NULL)
,m_pStaticPoint(NULL)
,m_pJobIcon(NULL)
,m_pTextureControl(NULL)
{

}

void CDnPvPRankListItemDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName("RankListDlg.ui").c_str(), bShow );	
}

void CDnPvPRankListItemDlg::InitialUpdate()
{
	m_pStaticRank = GetControl<CEtUIStatic>("ID_TEXT_RANK");  // Rank.
	m_pStaticLevel = GetControl<CEtUIStatic>("ID_TEXT_LEVEL"); // Level.
	m_pStaticGuild = GetControl<CEtUIStatic>("ID_TEXT_GUILD"); // Guild.
	m_pStaticName = GetControl<CEtUIStatic>("ID_TEXT_NAME");  // Name.
	m_pStaticResult = GetControl<CEtUIStatic>("ID_TEXT_RESULT");// Result.
	m_pStaticPoint = GetControl<CEtUIStatic>("ID_TEXT_COUNT"); // Rating.
	m_pJobIcon = GetControl<CDnJobIconStatic>("ID_STATIC_CLASS"); // Job icon.
	m_pTextureControl = GetControl<CEtUITextureControl>("ID_TEXTUREL_RANKMARK"); // Colosseum rank icon.
}

void CDnPvPRankListItemDlg::Show( bool bShow )
{

}


void CDnPvPRankListItemDlg::SetInfoColosseum( TPvPRanking * pInfo )
{	
	if( pInfo->wszCharName == NULL || wcslen( pInfo->wszCharName ) < 1 || pInfo->biRank == 0 )
		return;

	wchar_t str[256] = {0,};
	
	std::wstring sign( L"-" );	
	/*if( pInfo->iChangedRank > 0 )
		sign.assign( INCREASE_MARKER );
	else if( pInfo->iChangedRank < 0 )
		sign.assign( DECREASE_MARKER );*/	

	if( pInfo->iChangedRank < 0 )
		sign.assign( DECREASE_MARKER );
	if( pInfo->iChangedRank == 1 )
		sign.assign( INCREASE_MARKER );
	if( pInfo->iChangedRank == 2 )
		sign.assign( L"N" );


	// Rank.
	_i64tow( pInfo->biRank, str, 10 );
	std::wstring strRank( str );
	strRank.append( L"(" );
	strRank.append( sign );
	strRank.append( L")" );
	m_pStaticRank->SetText( strRank.c_str() );
	//m_pStaticRank->SetText( L"WhyNotWorking" );

	//wchar_t sign = '-';
	//if( (pInfo->biRank - pInfo->iChangedRank) > 0 )
	//	sign = INCREASE_MARKER;
	//else if( (pInfo->biRank - pInfo->iChangedRank) < 0 )
	//	sign = DECREASE_MARKER;

	//// Rank.
	//_i64tow( pInfo->biRank, str, 10 );
	//swprintf_s( str, 256, L"%s(%c)", str, sign );	
	//m_pStaticRank->SetText( str );


	memset( str, 0, 256 );

	// Level.
	_itow( pInfo->iLevel, str, 10 );	
	m_pStaticLevel->SetText( str );
	memset( str, 0, 256 );
		
	// Job icon.
	m_pJobIcon->SetIconID( pInfo->cJobCode );
	m_pJobIcon->SetTooltipText( DN_INTERFACE::STRING::GetJobString( pInfo->cJobCode ) );
	
	// Guild.
	m_pStaticGuild->SetText( pInfo->wszGuildName );

	// Colosseum rank icon.
	GetInterface().SetPvPClassIcon( m_pTextureControl, pInfo->cPvPLevel );

	// Name.
	m_pStaticName->SetText( pInfo->wszCharName );

	// Result.
	// Remove Kill/Death % - #68053.
	/*int nPersent = 0;
	if( pInfo->iDeath == 0 )
		nPersent = pInfo->iKill * 100;
	else if( pInfo->iDeath > 0 )
		nPersent = (int)( ( (float)pInfo->iKill / (float)pInfo->iDeath ) * 100 );	
	swprintf_s( str, 256, L"%d / %d (%d%%)", pInfo->iKill, pInfo->iDeath, nPersent );*/	
	swprintf_s( str, 256, L"%d / %d", pInfo->iKill, pInfo->iDeath );
	m_pStaticResult->SetText( str );
	memset( str, 0, 256 );

	// Point.
	_itow( pInfo->uiExp, str, 10 );	
	m_pStaticPoint->SetText( str );

}


void CDnPvPRankListItemDlg::SetInfoLadder( TPvPLadderRanking2 * pInfo )
{	
	if( pInfo->wszCharName == NULL || wcslen( pInfo->wszCharName ) < 1 || pInfo->biRank == 0 )
		return;

	wchar_t str[256] = {0,};

	std::wstring sign( L"-" );	
	/*if( pInfo->iChangedRank > 0 )
		sign.assign( INCREASE_MARKER );
	else if( pInfo->iChangedRank < 0 )
		sign.assign( DECREASE_MARKER );*/

	if( pInfo->iChangedRank < 0 )
		sign.assign( DECREASE_MARKER );
	if( pInfo->iChangedRank == 1 )
		sign.assign( INCREASE_MARKER );
	if( pInfo->iChangedRank == 2 )
		sign.assign( L"N" );

	

	// Rank.
	_i64tow( pInfo->biRank, str, 10 );
	std::wstring strRank( str );
	strRank.append( L"(" );
	strRank.append( sign );
	strRank.append( L")" );
	m_pStaticRank->SetText( strRank.c_str() );


	memset( str, 0, 256 );

	// Level.
	_itow( pInfo->iLevel, str, 10 );	
	m_pStaticLevel->SetText( str );
	memset( str, 0, 256 );

	// Job icon.
	m_pJobIcon->SetIconID( pInfo->cJobCode );
	m_pJobIcon->SetTooltipText( DN_INTERFACE::STRING::GetJobString( pInfo->cJobCode ) );

	// Guild.
	m_pStaticGuild->SetText( pInfo->wszGuildName );

	// Colosseum rank icon.
	GetInterface().SetPvPClassIcon( m_pTextureControl, pInfo->cPvPLevel );

	// Name.
	m_pStaticName->SetText( pInfo->wszCharName );

	// Result.
	int nPersent = 0;
	int denominator = pInfo->iWin + pInfo->iLose;
	if( denominator > 0 )
		nPersent = (int)( ( (float)pInfo->iWin / (float)denominator ) * 100 );	
	
	swprintf_s( str, 256, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 126039 ), pInfo->iWin, pInfo->iLose, pInfo->iDraw ); // "%d W %d L %d D"
	std::wstring strResult( str );
	swprintf_s( str, 256, L" %s %d%%", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 126177 ), nPersent );  // "Win Rate :"
	strResult.append( str );
	m_pStaticResult->SetText( strResult.c_str() );
	memset( str, 0, 256 );
  
	// Point.
	_itow( pInfo->iPvPLadderGradePoint, str, 10 );	
	m_pStaticPoint->SetText( str );

}


void CDnPvPRankListItemDlg::SetInfoUserColosseum( TPvPRankingDetail * pInfo )
{	
	if( pInfo->wszCharName == NULL || wcslen( pInfo->wszCharName ) < 1 || pInfo->biRank == 0 )
		return;

	m_ClassRank.jobCode = pInfo->cJobCode;
	m_ClassRank.biChangedClassRank = pInfo->iChangedClassRank;
	m_ClassRank.biChangedSubClassRank = pInfo->iChangedSubClassRank;
	m_ClassRank.biClassRank = pInfo->biClassRank;
	m_ClassRank.biSubClassRank = pInfo->biSubClassRank;
	
	wchar_t str[256] = {0,};
	
	std::wstring sign( L"-" );	
	//if( (pInfo->biRank - pInfo->iChangedRank) > 0 )
	//	sign.assign( INCREASE_MARKER );
	//else if( (pInfo->biRank - pInfo->iChangedRank) < 0 )
	//	sign.assign( DECREASE_MARKER );
	if( pInfo->iChangedRank < 0 )
		sign.assign( DECREASE_MARKER );
	if( pInfo->iChangedRank == 1 )
		sign.assign( INCREASE_MARKER );
	if( pInfo->iChangedRank == 2 )
		sign.assign( L"N" );

	// Rank.
	_i64tow( pInfo->biRank, str, 10 );
	std::wstring strRank( str );
	strRank.append( L"(" );
	strRank.append( sign );
	strRank.append( L")" );
	m_pStaticRank->SetText( strRank.c_str() );
	SetUserRank();

	//wchar_t sign = '-';
	//if( (pInfo->biRank - pInfo->iChangedRank) > 0 )
	//	sign = INCREASE_MARKER;
	//else if( (pInfo->biRank - pInfo->iChangedRank) < 0 )
	//	sign = DECREASE_MARKER;

	//// Rank.
	//_i64tow( pInfo->biRank, str, 10 );
	//swprintf_s( str, 256, L"%s(%c)", str, sign );	
	//m_pStaticRank->SetText( str );


	memset( str, 0, 256 );

	// Level.
	_itow( pInfo->iLevel, str, 10 );	
	m_pStaticLevel->SetText( str );
	memset( str, 0, 256 );
		
	// Job icon.
	m_pJobIcon->SetIconID( pInfo->cJobCode );
	m_pJobIcon->SetTooltipText( DN_INTERFACE::STRING::GetJobString( pInfo->cJobCode ) );
	
	// Guild.
	m_pStaticGuild->SetText( pInfo->wszGuildName );

	// Colosseum rank icon.
	GetInterface().SetPvPClassIcon( m_pTextureControl, pInfo->cPvPLevel );

	// Name.
	m_pStaticName->SetText( pInfo->wszCharName );

	// Result.
	// Remove Kill/Death % - #68053.
	/*int nPersent = 0;
	if( pInfo->iDeath == 0 )
		nPersent = pInfo->iKill * 100;
	else if( pInfo->iDeath > 0 )
		nPersent = (int)( ( (float)pInfo->iKill / (float)pInfo->iDeath ) * 100 );	
	swprintf_s( str, 256, L"%d / %d (%d%%)", pInfo->iKill, pInfo->iDeath, nPersent );*/	
	swprintf_s( str, 256, L"%d / %d", pInfo->iKill, pInfo->iDeath );
	m_pStaticResult->SetText( str );
	memset( str, 0, 256 );

	// Point.
	_itow( pInfo->uiExp, str, 10 );	
	m_pStaticPoint->SetText( str );

}


void CDnPvPRankListItemDlg::SetInfoUserLadder( TPvPLadderRankingDetail * pInfo )
{	
	if( pInfo->wszCharName == NULL || wcslen( pInfo->wszCharName ) < 1 || pInfo->biRank == 0 )
		return;

	m_ClassRank.jobCode = pInfo->cJobCode;
	m_ClassRank.biChangedClassRank = pInfo->iChangedClassRank;
	m_ClassRank.biChangedSubClassRank = pInfo->iChangedSubClassRank;
	m_ClassRank.biClassRank = pInfo->biClassRank;
	m_ClassRank.biSubClassRank = pInfo->biSubClassRank;
	
	wchar_t str[256] = {0,};

	std::wstring sign( L"-" );	
	//if( (pInfo->biRank - pInfo->iChangedRank) > 0 )
	//	sign.assign( INCREASE_MARKER );
	//else if( (pInfo->biRank - pInfo->iChangedRank) < 0 )
	//	sign.assign( DECREASE_MARKER );
	if( pInfo->iChangedRank < 0 )
		sign.assign( DECREASE_MARKER );
	if( pInfo->iChangedRank == 1 )
		sign.assign( INCREASE_MARKER );
	if( pInfo->iChangedRank == 2 )
		sign.assign( L"N" );

	// Rank.
	_i64tow( pInfo->biRank, str, 10 );
	std::wstring strRank( str );
	strRank.append( L"(" );
	strRank.append( sign );
	strRank.append( L")" );
	m_pStaticRank->SetText( strRank.c_str() );
	SetUserRank();

	memset( str, 0, 256 );

	// Level.
	_itow( pInfo->iLevel, str, 10 );	
	m_pStaticLevel->SetText( str );
	memset( str, 0, 256 );

	// Job icon.
	m_pJobIcon->SetIconID( pInfo->cJobCode );
	m_pJobIcon->SetTooltipText( DN_INTERFACE::STRING::GetJobString( pInfo->cJobCode ) );

	// Guild.
	m_pStaticGuild->SetText( pInfo->wszGuildName );

	// Colosseum rank icon.
	GetInterface().SetPvPClassIcon( m_pTextureControl, pInfo->cPvPLevel );

	// Name.
	m_pStaticName->SetText( pInfo->wszCharName );

	// Result.
	int nPersent = 0;
	int denominator = pInfo->iWin + pInfo->iLose;
	if( denominator > 0 )
		nPersent = (int)( ( (float)pInfo->iWin / (float)denominator ) * 100 );	
	
	swprintf_s( str, 256, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 126039 ), pInfo->iWin, pInfo->iLose, pInfo->iDraw ); // "%d W %d L %d D"
	std::wstring strResult( str );
	swprintf_s( str, 256, L" %s %d%%", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 126177 ), nPersent );  // "Win Rate :"
	strResult.append( str );
	m_pStaticResult->SetText( strResult.c_str() );
	memset( str, 0, 256 );
  
	// Point.
	_itow( pInfo->iPvPLadderGradePoint, str, 10 );	
	m_pStaticPoint->SetText( str );

}

void CDnPvPRankListItemDlg::SetUserRank()
{
	if( !m_pStaticRank )
		return;

	int job = m_ClassRank.jobCode;	
	int parentJob = job;

	int changeClass = 1;
	while( true )
	{
		parentJob = GetParentJob( parentJob );
		if( parentJob == 0 )
			break;

		++changeClass;
	}

	const wchar_t * pStr = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 120204 ); // "- %s Rank #%d"

	m_strTooltip.clear();
	// 1st job class.
	if( changeClass == 1 )
	{
		m_strTooltip.append( FormatW( pStr, DN_INTERFACE::STRING::GetJobString( job ), m_ClassRank.biClassRank ) );
	}

	// 2nd job
	else if( changeClass == 2 )
	{
		m_strTooltip.append( FormatW( pStr, DN_INTERFACE::STRING::GetJobString( GetParentJob(job) ), m_ClassRank.biClassRank ) );
		m_strTooltip.append( L"\n" );
		m_strTooltip.append( FormatW( pStr, DN_INTERFACE::STRING::GetJobString( job ), m_ClassRank.biSubClassRank ) );
	}

	// 3rd job.
	else if( changeClass == 3 )
	{
		int paJob = GetParentJob(job);
		m_strTooltip.append( FormatW( pStr, DN_INTERFACE::STRING::GetJobString( GetParentJob( paJob ) ), m_ClassRank.biClassRank ) );
		m_strTooltip.append( L"\n" );
		m_strTooltip.append( FormatW( pStr, DN_INTERFACE::STRING::GetJobString( paJob ), m_ClassRank.biSubClassRank ) );
	}

	m_pStaticRank->SetTooltipText( m_strTooltip.c_str() );
}


// Return parent job name of job.
int CDnPvPRankListItemDlg::GetParentJob( int job )
{
	DNTableFileFormat * pTable = GetDNTable( CDnTableDB::TJOB );
	if( !pTable )
		return 0;

	DNTableCell * pCell = NULL;

	int nParentJob = 0;
	int nSize = pTable->GetItemCount();
	for( int i=0; i<nSize; ++i )
	{
		int nID = pTable->GetItemID( i );
		if( nID != job )
			continue;
				
		pCell = pTable->GetFieldFromLablePtr( nID, "_ParentJob" );
		if( pCell && pCell->GetInteger() > 0 )
		{
			nParentJob = pCell->GetInteger();
			break;
		}		
	}

	return nParentJob;	
}


#endif // PRE_ADD_PVP_RANKING