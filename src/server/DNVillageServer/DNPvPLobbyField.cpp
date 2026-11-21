
#include "stdafx.h"
#include "DNPvPLobbyField.h"
#include "DNUserSession.h"
#include "DNPvPRoomManager.h"
#include "DNPvPRoom.h"
#include "DNMasterConnection.h"
#include "DNLadderSystemManager.h"
#include "DNLadderRoom.h"

CDNPvPLobbyField::CDNPvPLobbyField()
{

}

CDNPvPLobbyField::~CDNPvPLobbyField()
{

}

int CDNPvPLobbyField::SendFieldMessage(CDNBaseObject *pSender, USHORT wMsg, TBaseData *pSenderData, TParamData *pParamData)
{
	switch(wMsg)
	{
		case FM_CREATE:
		{
			if( !EnterObject(pSender) )
			{
				return -1;
			}
			break;
		}
		case FM_DESTROY:
		{
			if( !LeaveObject(pSender) )
			{
				return -1;
			}
			break;
		}
		case FM_CHAT:
		{
			CDNUserSession* pSession = static_cast<CDNUserSession*>(pSender);

			if( !pSession )
			{
				return -1;
			}

			UINT32 uiPvPIndex = pSession->GetPvPIndex();

			// Lobby state
			if( uiPvPIndex == 0 && pSession->bIsLadderUser() == false )
			{
#if !defined( _FINAL_BUILD )
				std::vector<std::wstring> tokens;
				if( pParamData )
				{
					if (wcslen( pParamData->wszChatMsg ) > 0 )
					{
						TokenizeW( pParamData->wszChatMsg, tokens, L" " );

						if( !tokens.empty() )
						{
							if( wcsicmp( tokens[0].c_str(), L"/userlist") == 0 )
							{
								if( g_pMasterConnection && g_pMasterConnection->GetActive() )
								{
									CSPVP_WAITUSERLIST TxPacket;
									memset( &TxPacket, 0, sizeof(TxPacket) );

									TxPacket.SortType	= PvPCommon::WaitUserList::SortType::Null;
									TxPacket.unPage		= 0;
									TxPacket.bIsAscend	= false;

									g_pMasterConnection->SendPvPWaitUserList( pSession->GetAccountDBID(), pSession->GetChannelID(), &TxPacket );
								}
							}
							if( wcsicmp( tokens[0].c_str(), L"/ladderinviteconfirm") == 0 && tokens.size() >= 3 )
							{
								LadderSystem::CS_INVITE_CONFIRM TxPacket;
								memset( &TxPacket, 0, sizeof(TxPacket) );

								TxPacket.bAccept = _ttoi(tokens[1].c_str()) ? true : false;
								_wcscpy( TxPacket.wszCharName, _countof(TxPacket.wszCharName), tokens[2].c_str(), (int)wcslen(tokens[2].c_str()) );

								pSession->OnRecvPvPMessage( ePvP::CS_LADDER_INVITE_CONFIRM, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
							}
						}
					}
				}
#endif // #if !defined( _FINAL_BUILD )

				for( UINT i=0 ; i<m_FieldBlocks.size() ; ++i )
					m_FieldBlocks[i]->SendPvPLobbyMessage( pSender, wMsg, pSenderData, pParamData );
			}
			// PvP room state
			else if( uiPvPIndex > 0 )
			{
#if !defined( _FINAL_BUILD )
				std::vector<std::wstring> tokens;
				if( pParamData )
				{
					if (wcslen( pParamData->wszChatMsg ) > 0 )
					{
						TokenizeW( pParamData->wszChatMsg, tokens, L" " );

						if( !tokens.empty() )
						{
						}
					}
				}
#endif // #if !defined( _FINAL_BUILD )
				CDNPvPRoom* pPvPRoom = CDNPvPRoomManager::GetInstance().GetPvPRoom( pSession->GetChannelID(), uiPvPIndex );
				if( !pPvPRoom )
				{
					_DANGER_POINT();
					return -1;
				}

				if (pParamData)
					pPvPRoom->SendChat( pParamData->eChatType, pParamData->nChatSize, pSender->wszName(), pParamData->wszChatMsg );
			}
			else if( pSession->bIsLadderUser() )
			{

				LadderSystem::CRoom* pRoom = LadderSystem::CManager::GetInstance().GetRoomPtr( pSession->GetCharacterName() );
				if( pRoom == NULL )
				{
					_DANGER_POINT();
					return -1;
				}

				if (pParamData)
					pRoom->SendChat( pParamData->eChatType, pParamData->nChatSize, pSender->wszName(), pParamData->wszChatMsg );
			}
			break;
		}
	}

	return 1;
}
