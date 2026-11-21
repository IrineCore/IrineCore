#include "stdafx.h"
#include "DnRestraintTask.h"
#include "DnInterface.h"
#include "DnMessageBox.h"
#include "SyncTimer.h"
#include "DnInterfaceString.h"
#include <mmsystem.h>

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 

CDnRestraintTask::CDnRestraintTask() : CTaskListener(true)
{
}

CDnRestraintTask::~CDnRestraintTask()
{
}

void CDnRestraintTask::OnDispatchMessage(int nMainCmd, int nSubCmd, char *pData, int nSize)
{
	switch (nMainCmd)
	{
	case SC_RESTRAINT: OnRecvRestraintMessage(nSubCmd, pData, nSize); break;
	}
}

void CDnRestraintTask::OnRecvRestraintMessage(int nSubCmd, char * pData, int nSize)
{

	switch (nSubCmd)
	{
	case eRestraint::SC_RESTRAINTADD: OnRecvRestraintAdd((SCRestraintAdd*)pData); break;
	case eRestraint::SC_RESTRAINTDEL: OnRecvRestraintDel((SCRestraintDel*)pData); break;
	case eRestraint::SC_RESTRAINTRETCODE: OnRecvRestraintRetCode((SCRestraintRet*)pData); break;
	case eRestraint::SC_RESTRAINTLIST: OnRecvRestraintList((SCRestraintList*)pData); break;
	}
}

void CDnRestraintTask::OnRecvRestraintAdd(SCRestraintAdd * pPacket)
{
	if (IsExistsRestraint(pPacket->Restraint.nRestraintKey))
	{
		return;
	}
	m_RestraintList.push_back(pPacket->Restraint);

	// Also use system message when added.
	CheckRestraint( (eRestraintType)pPacket->Restraint.nRestraintType, false, true );
}

void CDnRestraintTask::OnRecvRestraintDel(SCRestraintDel * pPacket)
{
	std::vector <TRestraint>::iterator ii; 
	for (ii = m_RestraintList.begin(); ii != m_RestraintList.end(); ii++)
	{
		if ((*ii).nRestraintKey == pPacket->nRestraintKey)
		{
			m_RestraintList.erase(ii);
			return;
		}
	}
}

void CDnRestraintTask::OnRecvRestraintRetCode(SCRestraintRet * pPacket)
{
	GetInterface().ServerMessageBox(pPacket->nRestraintRet);
}

void CDnRestraintTask::OnRecvRestraintList(SCRestraintList * pPacket)
{
	//std::vector <TRestraint> m_RestraintList;
	m_RestraintList.clear();

	for (int i = 0; i < pPacket->cCount; i++)
	{
		m_RestraintList.push_back(pPacket->restraint[i]);
		CheckRestraint( (eRestraintType)pPacket->restraint[i].nRestraintType, false, true );
	}
}

bool CDnRestraintTask::IsExistsRestraint(UINT nKey)
{
	std::vector <TRestraint>::iterator ii; 
	for (ii = m_RestraintList.begin(); ii != m_RestraintList.end(); ii++)
		if ((*ii).nRestraintKey == nKey)
			return true;
	return false;
}

bool CDnRestraintTask::IsRestrainting( eRestraintType Type, TRestraint &Data )
{
	std::vector<TRestraint>::iterator iter;
	for( iter = m_RestraintList.begin(); iter != m_RestraintList.end(); ++iter ) {
		if( (*iter).nRestraintType == Type ) {

			// Get the current time, and if it is within the range,
			if( CSyncTimer::IsActive() ) {
				if( (*iter)._tBegineTime < CSyncTimer::GetInstance().GetCurTime() && (*iter)._tEndTime > CSyncTimer::GetInstance().GetCurTime() ) {
					Data = (*iter);
					return true;
				}
			}
			else {
#ifndef _FINAL_BUILD
				//_ASSERT(0&&"DnRestraintTask. Why isn't the sync timer being used.")
#endif
				time_t Time;
				time(&Time);
				if( (*iter)._tBegineTime <= Time && (*iter)._tEndTime >= Time ) {
					Data = (*iter);
					return true;
				}
			}
		}
	}
	return false;
}

bool CDnRestraintTask::CheckRestraint( eRestraintType Type, bool bMessageBox, bool bChatMsg )
{
	// Check first to see if it triggers
	WCHAR wszTemp[256];
	std::wstring wszBeginTime, wszEndTime;
	TRestraint Data;
	memset( &Data, 0, sizeof(TRestraint) );
	bool bCheck = IsRestrainting( Type, Data );
	if( bCheck ) {
		DN_INTERFACE::STRING::GetTimeText( wszBeginTime, Data._tBegineTime );
		DN_INTERFACE::STRING::GetTimeText( wszEndTime, Data._tEndTime );

		{
			switch( Type )
			{
			case _RESTRAINTTYPE_BLOCK:
				break;
			case _RESTRAINTTYPE_CHAT:
				swprintf_s( wszTemp, _countof(wszTemp), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4404 ), Data.wszRestraintReason, wszBeginTime.c_str(), wszEndTime.c_str() );
				if( bMessageBox ) GetInterface().MessageBox( wszTemp );
				if( bChatMsg ) GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", wszTemp, true );
				break;
			case _RESTRAINTTYPE_TRADE:
				if( bMessageBox ) {
					swprintf_s( wszTemp, _countof(wszTemp), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4407 ) );
					GetInterface().MessageBox( wszTemp );
				}
				if( bChatMsg ) {
					swprintf_s( wszTemp, _countof(wszTemp), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4406 ), Data.wszRestraintReason, wszBeginTime.c_str(), wszEndTime.c_str() );
					GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", wszTemp, true );
				}
				break;
			}
		}
	}
	return bCheck;
}

#ifdef PRE_ADD_ANTI_CHAT_SPAM
bool CDnRestraintTask::CheckUSAChatSpam( const wstring& strChat )
{
	return m_ChatSpamChecker.Check( strChat );
}

bool CDnChatSpamChecker::IsPunct( wchar_t Char )
{
	switch( Char )
	{
		case L'^':
		case L'-':
		case L'!':
		case L'\"':
		case L'#':
		case L'$':
		case L'%':
		case L'&':
		case L'\'':
		case L'(':
		case L')':
		case L'*':
		case L'+':
		case L',':
		case L'.':
		case L'/':
		case L':':
		case L';':
		case L'<':
		case L'=':
		case L'>':
		case L'?':
		case L'@':
		case L'[':
		case L'\\':
		case L']':
		case L'_':
		case L'`':
		case L'{':
		case L'|':
		case L'}':
		case L'~':
			return true;
			break;
	}

	return false;
}

void CDnChatSpamChecker::TokenizeExceptPunct( wstring& strSource, set<wstring>& setResult )
{
	// Convert all special characters to spaces.
	wchar_t CharSpace = L' ';
	wstring::size_type NowPos = 0;
	wstring::size_type LastPos = 0;
	for( int i = 0; i < (int)strSource.length(); ++i )
	{
		wchar_t Char = strSource.at( i );
		if( IsPunct(Char) )
			strSource.at( NowPos ) = CharSpace;

		++NowPos;
	}

	// Ignore if the very first character is a delimiter
	std::wstring::size_type lastPos = strSource.find_first_not_of( CharSpace, 0 );
	// Find the first character that is not a delimiter
	std::wstring::size_type pos = strSource.find_first_of( CharSpace, lastPos );

	while( std::wstring::npos != pos || std::wstring::npos != lastPos )
	{
		// Token found, so add it to vector
		setResult.insert( strSource.substr( lastPos, pos - lastPos ) );
		// Skip delimiters. Note "not_of"
		lastPos = strSource.find_first_not_of( CharSpace, pos );
		// Find the next non-delimiter character
		pos = strSource.find_first_of( CharSpace, lastPos );
	}
}

// Return true if detected as spam.
bool CDnChatSpamChecker::Check( const wstring& strChat )
{
	bool bIsSpammer = false;
	DWORD dwNowTime = timeGetTime();

	// First, a fixed time interval from the last time entered in the list 
	// Remove string if it exceeds the limit.
	list<S_CHAT_INFO>::iterator iter = m_listChatHistory.begin();
	for( iter; iter != m_listChatHistory.end(); )
	{
		if( SPAM_CHECK_TIME < dwNowTime - iter->dwTime )
		{
			// If spam count exceeds the threshold, even when time expires 
			// Subtract only the spam count and keep it active.
			if( SPAM_JUDGEMENT_COUNT <= iter->iSpamCount )
			{
				iter->iSpamCount -= SPAM_JUDGEMENT_COUNT;
				iter->dwTime = dwNowTime;
			}
			else
			{
				iter = m_listChatHistory.erase( iter );
			}
		}
		else
		{
			++iter;
		}
	}

	wstring strChatToCheck = strChat;
	ToLowerW( strChatToCheck );
	
	vector<wstring> vlTokens;
	set<wstring> setWords;
	TokenizeExceptPunct( strChatToCheck, setWords );

	bool bIsSpamChat = false;
	if( false == m_listChatHistory.empty() )
	{
		iter = m_listChatHistory.begin();
		for( iter; iter != m_listChatHistory.end(); ++iter )
		{
			// First check if there is a matching string.
			S_CHAT_INFO& Chat = *iter;
			if( Chat.strSentence == strChat )
			{
				Chat.iSpamCount++;
				Chat.dwTime = dwNowTime;
				bIsSpamChat = true;
			}
			else
			{
				// Otherwise check if there are matching strings above a certain ratio per same word.
				vector<wstring> vlSameWords;
				set_intersection( Chat.setWords.begin(), Chat.setWords.end(),
								  setWords.begin(), setWords.end(), 
								  back_inserter(vlSameWords) );

				if( false == vlSameWords.empty() )
				{
					int iWholeWordsCount = (int)Chat.setWords.size();
					int iSameWordsCount = (int)vlSameWords.size();
					float fSameRatio = (float)iSameWordsCount / (float)iWholeWordsCount;
					if( SAME_WORDS_TOLERANCE <= fSameRatio )
					{
						Chat.iSpamCount++;
						Chat.dwTime = dwNowTime;
						bIsSpamChat = true;
					}
				}
			}

			// Determined as similar chat spam exceeding the threshold..
			if( bIsSpamChat && (SPAM_JUDGEMENT_COUNT <= Chat.iSpamCount) )
			{
				bIsSpammer = true;
			}
		}
	}

	// Sort by update time so the most recently entered chat string comes first.
	m_listChatHistory.sort();

	// If not classified as spam, prepend it to the front of the list.
	if( false == bIsSpamChat )
	{
		S_CHAT_INFO Chat;
		Chat.dwTime = dwNowTime;
		Chat.setWords = setWords;
		Chat.strSentence = strChat;
		m_listChatHistory.push_front( Chat );
	}

	return bIsSpammer;
}
#endif // #ifdef PRE_ADD_ANTI_CHAT_SPAM