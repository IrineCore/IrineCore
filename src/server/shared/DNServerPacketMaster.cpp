
#include "stdafx.h"
#include "DNServerPacketMaster.h"
#include "Util.h"

void MAGuildChat::Set(int nManagedID, const TGuildUID pGuildUID, UINT nAccountDBID, INT64 nCharacterDBID, LPCWSTR lpwszChatMsg, short nLen)
{
	DN_ASSERT(pGuildUID.IsSet(),	"Invalid!");
	DN_ASSERT(0 != nAccountDBID,	"Invalid!");
	DN_ASSERT(0 != nCharacterDBID,	"Invalid!");
	DN_ASSERT(NULL != lpwszChatMsg,	"Invalid!");
	DN_ASSERT(0 < nLen,				"Invalid!");

	m_nManagedID = nManagedID;
	m_GuildUID = pGuildUID;
	m_nAccountDBID = nAccountDBID;
	m_nCharacterDBID = nCharacterDBID;

	_wcscpy( m_wszChatMsg, _countof(m_wszChatMsg), lpwszChatMsg, nLen );
	m_nLen = nLen;
}
