#include "Stdafx.h"
#include "DNVoiceChatTask.h"
#include "VoiceChatSendPacket.h"
#include "DnActor.h"
#include "GameOption.h"
#include "DnInterface.h"
#include "DnGaugeDlg.h"
#ifdef _USE_VOICECHAT
#include "VoiceChatClient.h"
#endif
#include "DnPartyTask.h"
#include "DnItemTask.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 

CDnVoiceChatTask::CDnVoiceChatTask() : CTaskListener(true)
{
	memset(m_szVoiceChatIp, 0, sizeof(m_szVoiceChatIp));
	m_wControlPort = m_wAudioPort = m_nUniqueVoiceID = 0;
	m_bInit = false;
	m_pVoiceChatOptDlg = NULL;
	m_bUseAutoAdjustVolume = false;
	m_fAutoAdjustVolume = 0.1f;
	m_bAutoAdjustVolumeMode = false;
}

CDnVoiceChatTask::~CDnVoiceChatTask()
{
}

bool CDnVoiceChatTask::Initialize()
{
	return true;
}

void CDnVoiceChatTask::Finalize()
{
}

void CDnVoiceChatTask::OnDispatchMessage(int nMainCmd, int nSubCmd, char *pData, int nSize)
{
	switch (nMainCmd)
	{
#ifdef _USE_VOICECHAT
	case SC_VOICECHAT: OnRecvVoiceChatMessage(nSubCmd, pData, nSize); break;
#endif
	}
}

void CDnVoiceChatTask::ReqVoiceAvailable(bool bFlag)
{
	if( m_bInit && bFlag )
		SendVoiceChatAvailable(true);
	else
		SendVoiceChatAvailable(false);
}

void CDnVoiceChatTask::ReqUserMute(UINT nSessionID, bool bFlag)
{
	if (m_bInit == false)
	{
		_ASSERT(0);
		return;
	}

	SendVoiceMute(nSessionID, bFlag);		//bflag : true mute, false unmute;
}

void CDnVoiceChatTask::ReqUserComplaint(UINT nSessionID)
{
	if (m_bInit == false)
	{
		_ASSERT(0);
		return;
	}

	SendVoiceComplaint(nSessionID);
}

void CDnVoiceChatTask::MuteMyMic(bool bMute)
{
#ifdef _USE_VOICECHAT
	if (CVoiceChatClient::IsActive())
		CVoiceChatClient::GetInstance().MuteMyMic(bMute);
#endif
}

void CDnVoiceChatTask::SetRotation(EtVector2 vLook)
{
#ifdef _USE_VOICECHAT
	if (CVoiceChatClient::IsActive())
	{
		int nVoiceRotate = (int)EtToDegree(acos(EtVec2Dot(&EtVector2(0.f, 1.f), &vLook)));
		if( vLook.x > 0.0f )
			nVoiceRotate = 360 - nVoiceRotate;
		nVoiceRotate = (nVoiceRotate + 90 + 11) % 360;	// MoveDir is Look plus 15 degrees. Changed to 11 due to correction error with server.
		CVoiceChatClient::GetInstance().SetRotation(nVoiceRotate);
	}
#endif
}

#ifdef _USE_VOICECHAT
bool CDnVoiceChatTask::CheckVoiceFont()
{
	if( !CDnActor::s_hLocalActor ) return false;
	if( !CDnInterface::IsActive() ) return false;
	if( !CGameOption::IsActive() ) return false;
	if( !CVoiceChatClient::IsActive() ) return false;

	int nType = CGameOption::GetInstance().m_nVoiceFont;
	float fPitch = CGameOption::GetInstance().m_fVoicePitch;
	float fTimbre = CGameOption::GetInstance().m_fVoiceTimbre;

	// Access item task and apply voice modulation item if present.
	bool bPremium = false;
	std::vector<CDnItem*> pVecResult;
	if( CDnItemTask::IsActive() && GetItemTask().FindItemFromItemType( ITEMTYPE_VOICEFONT, ST_INVENTORY_CASH, pVecResult ) )
		bPremium = true;

	if( bPremium )
		CVoiceChatClient::GetInstance().SetVoiceFont( (CVoiceChatClient::eVoiceFontType)nType, fPitch, fTimbre );
	else
		CVoiceChatClient::GetInstance().SetVoiceFont( (CVoiceChatClient::eVoiceFontType)0, 1.0f, 1.0f );

	return bPremium;
}
#endif

void CDnVoiceChatTask::OnRecvVoiceChatMessage(int nSubCmd, char *pData, int nSize)
{
	switch (nSubCmd)
	{
	case eVoiceChat::SC_VOICECHATINFO: OnRecvSystemVoiceChatInfo((SCVoiceChatInfo*)pData);	break;	// Once on login
	case eVoiceChat::SC_TALKINGINFO: OnRecvTalkingInfo((SCTalkingInfo*)pData); break;
	case eVoiceChat::SC_VOICEMEMBERINFO: OnRecvVoiceMemberInfo((SCVoiceMemberInfo*)pData); break;
	}
}

void CDnVoiceChatTask::OnRecvSystemVoiceChatInfo(SCVoiceChatInfo *pPacket)
{
	_strcpy(m_szVoiceChatIp, _countof(m_szVoiceChatIp), pPacket->szVoiceChatIp, (int)strlen(pPacket->szVoiceChatIp));
	m_wControlPort = pPacket->wControlPort;
	m_wAudioPort = pPacket->wAudioPort;
	m_nUniqueVoiceID = pPacket->nAccountDBID;

	InitVoiceChat();

	if( m_bInit && CGameOption::GetInstance().m_bVoiceChat )
		SendVoiceChatAvailable(true);
	else
		SendVoiceChatAvailable(false);
}

void CDnVoiceChatTask::OnRecvTalkingInfo(SCTalkingInfo * pPacket)
{
#ifdef _USE_VOICECHAT
	if( !CDnActor::s_hLocalActor ) return;
	if( !CDnInterface::IsActive() ) return;
	if( !CGameOption::IsActive() ) return;
	if( !CGameOption::GetInstance().m_bVoiceChat ) return;

	bool bPartySpeaking = false;
	for (int i = 0; i < pPacket->cCount; i++)
	{
		if( pPacket->Talking[i].nSessionID == CDnActor::s_hLocalActor->GetUniqueID() )
		{
			if( !GetInterface().GetPlayerGauge() ) return;
			GetInterface().GetPlayerGauge()->UpdateVoiceSpeaking( (pPacket->Talking[i].cTalking) ? true : false );
		}
		else
		{
			// Find party member dialog and call the function above
			GetInterface().RefreshPartySpeaking( pPacket->Talking[i].nSessionID, (pPacket->Talking[i].cTalking) ? true : false );
			bPartySpeaking = (pPacket->Talking[i].cTalking) ? true : false;
		}
	}

	if( m_bUseAutoAdjustVolume )
	{
		if( bPartySpeaking )
		{
			float fTotalVolume = m_fAutoAdjustVolume;
			CEtSoundEngine::GetInstance().SetMasterVolume( "BGM", fTotalVolume * CGameOption::GetInstance().GetVolumeBGM() );
			CEtSoundEngine::GetInstance().SetMasterVolume( "3D", fTotalVolume * CGameOption::GetInstance().GetVolumeBGM() );
			CEtSoundEngine::GetInstance().SetMasterVolume( "2D", fTotalVolume * CGameOption::GetInstance().GetVolumeBGM() );
			CEtSoundEngine::GetInstance().SetMasterVolume( "VOICE", fTotalVolume * CGameOption::GetInstance().GetVolumeBGM() );
			CEtSoundEngine::GetInstance().SetMasterVolume( "NULL", fTotalVolume );
			m_bAutoAdjustVolumeMode = true;
		}
		else
		{
			CGameOption::GetInstance().ApplySound();
			m_bAutoAdjustVolumeMode = false;
		}
	}
#endif
}

void CDnVoiceChatTask::Process( LOCAL_TIME LocalTime, float fDelta )
{
	if( m_bAutoAdjustVolumeMode )
	{
		// If a party member leaves while speaking, the end-of-speech notification can no longer be received, so
		if( CDnPartyTask::IsActive() && CDnPartyTask::GetInstance().GetPartyCount() == 1 )
		{
			CGameOption::GetInstance().ApplySound();
			m_bAutoAdjustVolumeMode = false;
		}
	}
}

void CDnVoiceChatTask::OnRecvVoiceMemberInfo(SCVoiceMemberInfo * pPacket)
{
#ifdef _USE_VOICECHAT
	if( !CDnActor::s_hLocalActor ) return;
	if( !CDnInterface::IsActive() ) return;
	if( !CGameOption::IsActive() ) return;

	// Find the mute list,
	std::vector<UINT> vecMuteSessionID;
	for (int k = 0; k < PARTYCOUNTMAX; k++)
	{
		if (pPacket->nVoiceMutedList[k])
			vecMuteSessionID.push_back(pPacket->nVoiceMutedList[k]);
	}

	for (int i = 0; i < pPacket->cCount; i++)
	{
		if( pPacket->Info[i].nSessionID == CDnActor::s_hLocalActor->GetUniqueID() )
		{
			if( !GetInterface().GetPlayerGauge() ) return;
			GetInterface().GetPlayerGauge()->UpdateVoiceButtonMode( pPacket->Info[i].cVoiceAvailable ? true : false );
		}
		else
		{
			// Find party member dialog and call the function above
			bool bMute = false;
			for (int j = 0; j < (int)vecMuteSessionID.size(); j++)
			{
				if (pPacket->Info[i].nSessionID == vecMuteSessionID[j])
				{
					bMute = true;
					break;
				}
			}
			GetInterface().RefreshPartyVoiceButtonMode( pPacket->Info[i].nSessionID, (pPacket->Info[i].cVoiceAvailable) ? true : false, bMute );
		}
	}
#endif
}

bool CDnVoiceChatTask::InitVoiceChat()
{
#ifdef _USE_VOICECHAT
	if( !CDnActor::s_hLocalActor )
		return false;
	
	if( CVoiceChatClient::GetInstancePtr() )
		delete CVoiceChatClient::GetInstancePtr();

	if( !CVoiceChatClient::CreateInstance() )
		return false;

	char szTemp[ 1024 ];
	WideCharToMultiByte( CP_ACP, 0, CDnActor::s_hLocalActor->GetName(), -1, szTemp, 1024, NULL, NULL );
	if( CVoiceChatClient::GetInstance().Initialize( m_szVoiceChatIp, m_wAudioPort, szTemp, m_nUniqueVoiceID ) )
	{
		m_bInit = true;
		CheckVoiceFont();
		return true;
	}
	else
	{
		if( CVoiceChatClient::GetInstancePtr() )
			delete CVoiceChatClient::GetInstancePtr();

		m_bInit = false;
	}
#endif
	return false;
}