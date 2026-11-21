#include "StdAfx.h"
#include "DnSecondarySkillRecipeItem.h"


#ifdef PRE_ADD_SECONDARY_SKILL

CDnSecondarySkillRecipeItem::CDnSecondarySkillRecipeItem() : m_nDurability( 0 )
{
}

CDnSecondarySkillRecipeItem::~CDnSecondarySkillRecipeItem()
{
}

void CDnSecondarySkillRecipeItem::GetTItemInfo( TItemInfo &Info )
{
	// Although this is a copy item registered to another window, cSlotIndex is also required.
	// Since this copy item's info may also be used when requesting a packet..
	Info.cSlotIndex = m_nSlotIndex;
	Info.Item.nItemID = m_nClassID;
	Info.Item.wCount = m_nOverlapCount;
	Info.Item.cLevel = m_cEnchantLevel;
	Info.Item.cPotential = m_cPotentialIndex;
	Info.Item.cOption = m_cOptionIndex;
	Info.Item.cSealCount = m_cSealCount;
	Info.Item.bSoulbound = (m_Reversion == ITEMREVERSION_BELONG) ? m_bSoulBound : true;
	Info.Item.nSerial = m_nSerialID;
	Info.Item.wDur = m_nDurability;
}
#endif // PRE_ADD_SECONDARY_SKILL