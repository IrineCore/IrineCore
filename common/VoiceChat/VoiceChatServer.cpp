#include "stdafx.h"
#ifdef _USE_VOICECHAT
#include "VoiceChatServer.h"
#include "CriticalSection.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif


CDNVoiceChatBase::CDNVoiceChatBase()
{
}

CDNVoiceChatBase::~CDNVoiceChatBase()
{
	Finalize();
}

ICE_ERROR CDNVoiceChatBase::Initialize(const char * pGameName, const char * pAudioServerIP, unsigned short nPort)
{
	ICE_ERROR IceError = ICE_ERROR_ICE_NULL;

	//create and initialize an ICE object
	m_pICE = ICE_create();
	if(!m_pICE) return IceError;

	//create the ICE_CONFIG structure
	ICE_CONFIG config;
	memset(&config, 0, sizeof(ICE_CONFIG));

	config.mode = ICE_MODE_REMOTESERVERS;
	config.gsname = pGameName;
	config.cs.address = pAudioServerIP;
	config.cs.port = nPort;

	IceError = ICE_init(m_pICE, &config, ICE_SERVER_API_VERSION);
	
	return IceError;
}

void CDNVoiceChatBase::Finalize()
{
	ICE_destroy(m_pICE);
}

ICE_ERROR CDNVoiceChatBase::CreateVoiceChannel(UINT nVoiceChannelID, int nHearingRange)
{
	ICE_ERROR IceRet = ICE_ERROR_UNKNOWN;
	if (nHearingRange == 0)		//non spartial
		IceRet = ICE_create_channel_nonspatial(m_pICE, nVoiceChannelID);
	else						// spartial
		//IceRet = ICE_create_channel_spatial(m_pICE, nVoiceChannelID, nHearingRange, NULL, 0);
		IceRet = ICE_create_channel_spatial(m_pICE, nVoiceChannelID, nHearingRange, HearingPoint, 6);
	return IceRet;
}

ICE_ERROR CDNVoiceChatBase::DestroyVoiceChannel(UINT nChannelID)
{
	return ICE_destroy_channel(m_pICE, nChannelID);
}

ICE_ERROR CDNVoiceChatBase::IceConnect(UINT nUserID, const char * pUserName, const char * pUserIP)
{
	return ICE_connect_adv(m_pICE, pUserName, nUserID, ICE_str2ip(pUserIP));
}

ICE_ERROR CDNVoiceChatBase::IceDisconnect(UINT nUserID)
{
	return ICE_disconnect(m_pICE, nUserID);
}

ICE_ERROR CDNVoiceChatBase::SetPremium(UINT nUserID)
{
	return ICE_set_premium(m_pICE, nUserID, 1);
}

ICE_ERROR CDNVoiceChatBase::ListenJoin(UINT nChannelID, UINT nUserID)
{
	return ICE_channel_talk_join(m_pICE, nUserID, nChannelID, 0, 0);
}

ICE_ERROR CDNVoiceChatBase::ListenLeave(UINT nChannelID, UINT nUserID)
{
	return ICE_channel_listen_leave(m_pICE, nUserID, nChannelID);;
}

ICE_ERROR CDNVoiceChatBase::TalkJoin(UINT nChannelID, UINT nUserID)
{
	return ICE_channel_listen_join(m_pICE, nUserID, nChannelID, 0);
}

ICE_ERROR CDNVoiceChatBase::TalkLeave(UINT nChannelID, UINT nUserID)
{
	return ICE_channel_talk_leave(m_pICE, nUserID, nChannelID);
}

ICE_ERROR CDNVoiceChatBase::MuteOneToOne(UINT nUserID, UINT nMuteUserID, bool bIsMute)
{
	return ICE_mute_one_to_one(m_pICE, nUserID, nMuteUserID, bIsMute == true ? 1 : 0);
}

//ICE_ERROR CDNVoiceChatBase::MuteOneToMany(UINT nUserID, const UINT * pMuterUserIDArr, const BYTE * pMuteFlagArr, const UINT nLen)
//{
//	if (!pMuterUserIDArr || !pMuteFlagArr) return ICE_ERROR_UNKNOWN;
//	return ICE_mute_one_to_many(m_pICE, nUserID, pMuterUserIDArr, pMuteFlagArr, nLen);
//}

ICE_ERROR CDNVoiceChatBase::ComplaintRequest(UINT nComplainerUserID, UINT nHasComplainee, UINT nComplaineeUserID, const char * pCategory, const char * pSubject, const char * pMsg)
{
	if (!pCategory || !pSubject || !pMsg) return ICE_ERROR_UNKNOWN;
	return ICE_complaint_request(m_pICE, nComplainerUserID, nHasComplainee, nComplaineeUserID, pCategory, pSubject, pMsg);
}

ICE_ERROR CDNVoiceChatBase::SetUserPos(UINT nPlayerID, int nX, int nY, int nZ, USHORT nRotation)
{
	return ICE_position(m_pICE, nPlayerID, nX, nZ, nY, nRotation);
}

ICE_ERROR CDNVoiceChatBase::SetChannelProperty(UINT nChannelID, const ICE_CHAN_PROPERTY Ice_Prop, const void * pVal, const unsigned int nLen)
{
	if (!pVal) return ICE_ERROR_UNKNOWN;
	return ICE_set_channel_property(m_pICE, nChannelID, Ice_Prop, pVal, nLen);
}

ICE_ERROR CDNVoiceChatBase::GetChannelProperty(UINT nVoiceChannelID, ICE_CHAN_PROPERTY Ice_Prop, void * pVal, unsigned int * pSize)
{
	if (!pVal || !pSize) return ICE_ERROR_UNKNOWN;
	return ICE_get_channel_property(m_pICE, nVoiceChannelID, Ice_Prop, pVal, pSize);
}

ICE_ERROR CDNVoiceChatBase::GetUserStatus(UINT nUserID, ICE_player_status * pStatus)
{
	if (!pStatus) return ICE_ERROR_UNKNOWN;
	return ICE_get_player_status(m_pICE, nUserID, pStatus);
}
#endif