#pragma once
#include "OwnerShip.h"

class MIInventoryItem : public GiveOwnerShip
{
public:
	enum InvenItemTypeEnum {
		Item,
		Skill,
		RadioMsg,
		Gesture,
		SecondarySkill,
		ItemTypeEnum_Amount = 5,
	};
	enum ItemConditionEnum {
		Usable,
		Unusable,
		NotEnoughCondition,
		Toggle,
	};

	MIInventoryItem( InvenItemTypeEnum Type );
	virtual ~MIInventoryItem();
	tstring m_szDescription;

protected:
	InvenItemTypeEnum m_Type;
	int m_nIconImageIndex;
	int m_nSlotIndex;	
	CASHITEM_SN	m_CashItemSerial;

protected:
	void SetIconImageIndex( int nValue );
//	virtual bool IsEnable() { return true; }

public:
	InvenItemTypeEnum GetType() const { return m_Type; }
	int GetIconImageIndex() const { return m_nIconImageIndex; }

	int GetSlotIndex() const { return m_nSlotIndex; }
	void SetSlotIndex( int nValue ) { m_nSlotIndex = nValue; }

	virtual bool IsQuickSlotItem() { return true; }	
	bool	IsCashItem() const		{ return (m_CashItemSerial > 0); }
	int		GetCashItemSN() const	{ return m_CashItemSerial; }
	void	SetCashItemSN(CASHITEM_SN sn)	{ _ASSERT(sn); m_CashItemSerial = sn; }


public:
	virtual int GetClassID() const { return 0; }
	virtual int GetOverlapCount() const { return 0; }
	virtual const TCHAR *GetDescription() { return m_szDescription.c_str(); }
	virtual float GetCoolTime() { return 0.f; }



	virtual ItemConditionEnum GetItemCondition() { return Usable; }

	virtual int GetClickedEquipPage() { return 0; }
	virtual bool IsEnableTooltip() { return true; }
};
