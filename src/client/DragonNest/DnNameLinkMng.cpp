#include "stdafx.h"
#include "DnNameLinkMng.h"
#include "DnInterfaceString.h"
#include "DnWeapon.h"
#include "DnParts.h"
#include "EtUIMan.h"
#include "DnItemTask.h"
#include "DnTableDB.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnNameLinkMng::CDnNameLinkMng()
{
	m_pLinkItem = NULL;
}

WCHAR CDnNameLinkMng::GetNameLinkSeperator() const
{
	return 0xff00;
}

void CDnNameLinkMng::MakeItemNameLinkInfo(std::wstring& res, ITEMCLSID classId, int enchantLevel, int seed, int durability, 
										  int potentialIndex, int optionIndex, int sealCount
										  , bool bSoulBound, bool bEternity
#ifdef PRE_ADD_DMIX_DESIGNNAME
										  , ITEMCLSID lookItemId
#endif
#ifdef PRE_ADD_EXCHANGE_POTENTIAL
										  , int potentialMoveCount
#endif
										  )
{
#ifdef PRE_ADD_DMIX_DESIGNNAME
	#ifdef PRE_ADD_EXCHANGE_POTENTIAL
		res = FormatW(L"%ci%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d;", GetNameLinkSeperator(), classId, enchantLevel, seed, durability, potentialIndex, optionIndex, sealCount, bSoulBound ? 1:0, bEternity ? 1:0, lookItemId, potentialMoveCount); // count : 11
	#else
		res = FormatW(L"%ci%d:%d:%d:%d:%d:%d:%d:%d:%d:%d;", GetNameLinkSeperator(), classId, enchantLevel, seed, durability, potentialIndex, optionIndex, sealCount, bSoulBound ? 1:0, bEternity ? 1:0, lookItemId); // count : 10
	#endif
#else
	#ifdef PRE_ADD_EXCHANGE_POTENTIAL
		res = FormatW(L"%ci%d:%d:%d:%d:%d:%d:%d:%d:%d:%d;", GetNameLinkSeperator(), classId, enchantLevel, seed, durability, potentialIndex, optionIndex, sealCount, bSoulBound ? 1:0, bEternity ? 1:0, potentialMoveCount); // count : 10
	#else
		res = FormatW(L"%ci%d:%d:%d:%d:%d:%d:%d:%d:%d;", GetNameLinkSeperator(), classId, enchantLevel, seed, durability, potentialIndex, optionIndex, sealCount, bSoulBound ? 1:0, bEternity ? 1:0); // count : 9
	#endif
#endif
}

//	todo by kalliste : Extract to strategy if count grows
void CDnNameLinkMng::MakeNameLinkString_Item(std::wstring& res, const CDnItem& item)
{
	WCHAR sep = GetNameLinkSeperator();
	const CEtUINameLinkMgr& linkMgr = EtInterface::GetNameLinkMgr();

	linkMgr.AppendTag(res, CEtUINameLinkMgr::eNLTAG_START);

	int durability = 0;
	if (item.GetItemType() == ITEMTYPE_WEAPON)
	{
		const CDnWeapon* weapon = dynamic_cast<const CDnWeapon*>(&item);
		durability = weapon->GetDurability();
	}
	else if (item.GetItemType() == ITEMTYPE_PARTS)
	{
		const CDnParts* parts = dynamic_cast<const CDnParts*>(&item);
		durability = parts->GetDurability();
	}

	std::wstring itemInfoString;
#ifdef PRE_ADD_DMIX_DESIGNNAME
	#ifdef PRE_ADD_EXCHANGE_POTENTIAL
		MakeItemNameLinkInfo(itemInfoString, item.GetClassID(), item.GetEnchantLevel(), item.GetSeed(), durability, 
			item.GetPotentialIndex(), item.GetOptionIndex(), item.GetSealCount(), item.IsSoulbBound(), item.IsEternityItem(), item.GetLookItemID(), item.GetPotentialMoveCount());
	#else
		MakeItemNameLinkInfo(itemInfoString, item.GetClassID(), item.GetEnchantLevel(), item.GetSeed(), durability, 
			item.GetPotentialIndex(), item.GetOptionIndex(), item.GetSealCount(), item.IsSoulbBound(), item.IsEternityItem(), item.GetLookItemID());
	#endif
#else
	#ifdef PRE_ADD_EXCHANGE_POTENTIAL
		MakeItemNameLinkInfo(itemInfoString, item.GetClassID(), item.GetEnchantLevel(), item.GetSeed(), durability, 
			item.GetPotentialIndex(), item.GetOptionIndex(), item.GetSealCount(), item.IsSoulbBound(), item.IsEternityItem(), item.GetPotentialMoveCount());
	#else
		MakeItemNameLinkInfo(itemInfoString, item.GetClassID(), item.GetEnchantLevel(), item.GetSeed(), durability, 
			item.GetPotentialIndex(), item.GetOptionIndex(), item.GetSealCount(), item.IsSoulbBound(), item.IsEternityItem());
	#endif
#endif

#ifdef PRE_ADD_BESTFRIEND
	// For best friend ring, item name is "Best Friend Ring with %s", so display "Best Friend Ring" when linked.
	// : Reason - The best friend name of the user who linked the item is only known after receiving SCEnterUser packet. 
	//  However, since chat is also displayed to out-of-range users who didn't receive SCEnterUser, there is no way to fill the best friend's name in "%s".
	if ( item.GetItemType() == ITEMTYPE_PARTS && item.GetTypeParam(0) == 10 && item.GetTypeParam(1) == 10 )
		res += FormatW(_T("%cc%08x%s[%s]%cr"), sep, DN_INTERFACE::STRING::ITEM::RANK_2_COLOR(item.GetItemRank()), itemInfoString.c_str(), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4350 ), sep);
	else
		res += FormatW(_T("%cc%08x%s[%s]%cr"), sep, DN_INTERFACE::STRING::ITEM::RANK_2_COLOR(item.GetItemRank()), itemInfoString.c_str(), item.GetName(), sep);
#else
	res += FormatW(_T("%cc%08x%s[%s]%cr"), sep, DN_INTERFACE::STRING::ITEM::RANK_2_COLOR(item.GetItemRank()), itemInfoString.c_str(), item.GetName(), sep);
#endif // PRE_ADD_BESTFRIEND

	linkMgr.AppendTag(res, CEtUINameLinkMgr::eNLTAG_END);
}

void CDnNameLinkMng::MakeNameLinkString_Item(std::wstring& res, const TItem& itemInfo)
{
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TITEM );
	if (pSox == NULL || pSox->IsExistItem( itemInfo.nItemID ) == false) 
		return;

	WCHAR sep = GetNameLinkSeperator();
	const CEtUINameLinkMgr& linkMgr = EtInterface::GetNameLinkMgr();

	linkMgr.AppendTag(res, CEtUINameLinkMgr::eNLTAG_START);

	std::wstring itemInfoString;
#ifdef PRE_ADD_DMIX_DESIGNNAME
	#ifdef PRE_ADD_EXCHANGE_POTENTIAL
		MakeItemNameLinkInfo(itemInfoString, itemInfo.nItemID, itemInfo.cLevel, itemInfo.nRandomSeed, itemInfo.wDur, itemInfo.cPotential, itemInfo.cOption, itemInfo.cSealCount, itemInfo.bSoulbound, itemInfo.bEternity, itemInfo.nLookItemID, itemInfo.cPotentialMoveCount);
	#else
		MakeItemNameLinkInfo(itemInfoString, itemInfo.nItemID, itemInfo.cLevel, itemInfo.nRandomSeed, itemInfo.wDur, itemInfo.cPotential, itemInfo.cOption, itemInfo.cSealCount, itemInfo.bSoulbound, itemInfo.bEternity, itemInfo.nLookItemID);
	#endif
#else
	#ifdef PRE_ADD_EXCHANGE_POTENTIAL
		MakeItemNameLinkInfo(itemInfoString, itemInfo.nItemID, itemInfo.cLevel, itemInfo.nRandomSeed, itemInfo.wDur, itemInfo.cPotential, itemInfo.cOption, itemInfo.cSealCount, itemInfo.bSoulbound, itemInfo.bEternity, itemInfo.cPotentialMoveCount);
	#else
		MakeItemNameLinkInfo(itemInfoString, itemInfo.nItemID, itemInfo.cLevel, itemInfo.nRandomSeed, itemInfo.wDur, itemInfo.cPotential, itemInfo.cOption, itemInfo.cSealCount, itemInfo.bSoulbound, itemInfo.bEternity);
	#endif
#endif

	eItemRank rank = (eItemRank)pSox->GetFieldFromLablePtr( itemInfo.nItemID, "_Rank" )->GetInteger();
	tstring itemName = CDnItem::GetItemFullName(itemInfo.nItemID);
	res += FormatW(_T("%cc%08x%s[%s]%cr"), sep, DN_INTERFACE::STRING::ITEM::RANK_2_COLOR(rank), itemInfoString.c_str(), itemName.c_str(), sep);

	linkMgr.AppendTag(res, CEtUINameLinkMgr::eNLTAG_END);
}

void CDnNameLinkMng::MakeSkillNameLinkInfo(std::wstring& res, const CDnSkill& itemInfo)
{
	res = FormatW(L"%ck%d:%d;", GetNameLinkSeperator(), itemInfo.GetClassID(), itemInfo.GetLevel()); // count : 7
}

void CDnNameLinkMng::MakeNameLinkString_Skill(std::wstring& res, const CDnSkill& skill)
{
	WCHAR sep = GetNameLinkSeperator();
	const CEtUINameLinkMgr& linkMgr = EtInterface::GetNameLinkMgr();

	linkMgr.AppendTag(res, CEtUINameLinkMgr::eNLTAG_START);

	std::wstring itemInfoString;
	MakeSkillNameLinkInfo(itemInfoString, skill);
	res += FormatW(_T("%s[%s]%cr"), itemInfoString.c_str(), skill.GetName(), sep);

	linkMgr.AppendTag(res, CEtUINameLinkMgr::eNLTAG_END);
}

void CDnNameLinkMng::MakeNameLinkString_UserName(std::wstring& res, const std::wstring& userName)
{
	WCHAR sep = GetNameLinkSeperator();
	const CEtUINameLinkMgr& linkMgr = EtInterface::GetNameLinkMgr();

	linkMgr.AppendTag(res, CEtUINameLinkMgr::eNLTAG_START);

	res += FormatW(_T("%cn%s"), sep, userName.c_str());

	linkMgr.AppendTag(res, CEtUINameLinkMgr::eNLTAG_END);
}

CDnItem* CDnNameLinkMng::MakeItem(const std::wstring& tagString)
{
	OnParseTextItemInfo(tagString);
	m_pLinkItem = GetItemTask().CreateItem(m_MakeLinkItemInfo);
	return m_pLinkItem;
}

bool CDnNameLinkMng::OnParseTextItemInfo(const std::wstring& argString)
{
	ZeroMemory(&m_MakeLinkItemInfo, sizeof(TItemInfo));
	int soulBound = 0;
	int eternity = 0;
	int nLevel = 0, nDur = 0, nPotential = 0, nOption = 0, nSealCount = 0, nPotentialMoveCount = 0;
#ifdef PRE_ADD_DMIX_DESIGNNAME
	#ifdef PRE_ADD_EXCHANGE_POTENTIAL
		swscanf_s(argString.c_str(), L"%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d", &m_MakeLinkItemInfo.Item.nItemID, &nLevel, &m_MakeLinkItemInfo.Item.nRandomSeed, 
			&nDur, &nPotential, &nOption, &nSealCount, &soulBound, &eternity, &m_MakeLinkItemInfo.Item.nLookItemID, &nPotentialMoveCount);
	#else
		swscanf_s(argString.c_str(), L"%d:%d:%d:%d:%d:%d:%d:%d:%d:%d", &m_MakeLinkItemInfo.Item.nItemID, &nLevel, &m_MakeLinkItemInfo.Item.nRandomSeed, 
			&nDur, &nPotential, &nOption, &nSealCount, &soulBound, &eternity, &m_MakeLinkItemInfo.Item.nLookItemID);
	#endif
#else
	#ifdef PRE_ADD_EXCHANGE_POTENTIAL
		swscanf_s(argString.c_str(), L"%d:%d:%d:%d:%d:%d:%d:%d:%d:%d", &m_MakeLinkItemInfo.Item.nItemID, &nLevel, &m_MakeLinkItemInfo.Item.nRandomSeed, 
			&nDur, &nPotential, &nOption, &nSealCount, &soulBound, &eternity, &nPotentialMoveCount);
	#else
		swscanf_s(argString.c_str(), L"%d:%d:%d:%d:%d:%d:%d:%d:%d", &m_MakeLinkItemInfo.Item.nItemID, &nLevel, &m_MakeLinkItemInfo.Item.nRandomSeed, 
			&nDur, &nPotential, &nOption, &nSealCount, &soulBound, &eternity);
	#endif
#endif
	m_MakeLinkItemInfo.Item.cLevel = static_cast<char>(nLevel);
	m_MakeLinkItemInfo.Item.wDur = static_cast<USHORT>(nDur);
	m_MakeLinkItemInfo.Item.cPotential = static_cast<char>(nPotential);
	m_MakeLinkItemInfo.Item.cOption = static_cast<char>(nOption);
	m_MakeLinkItemInfo.Item.cSealCount = static_cast<char>(nSealCount);
	m_MakeLinkItemInfo.Item.cPotentialMoveCount = static_cast<char>(nPotentialMoveCount);
	m_MakeLinkItemInfo.Item.bSoulbound = (soulBound != 0);
	m_MakeLinkItemInfo.Item.bEternity = (eternity != 0);

	return true;
}