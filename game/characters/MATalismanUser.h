#pragma once

#if defined(PRE_ADD_TALISMAN_SYSTEM)

#include "DnTalisman.h"

class MATalismanUser
{
public:
	MATalismanUser();
	virtual ~MATalismanUser();

protected:
	DnTalismanHandle m_hTalisman[TALISMAN_MAX];
	bool m_bSelfDelete[TALISMAN_MAX];
	float m_fTalismanEfficiency[TALISMAN_MAX];
	CDnState m_TalismanScaledState[TALISMAN_MAX];

	void ScaleStateByEfficiency( CDnState &dst, CDnState *src, float fEfficiency );

public:
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );

	virtual bool AttachTalisman( DnTalismanHandle hTalisman, int nSlotIndex, float fEfficiency, bool bDelete = false );
	virtual bool DetachTalisman( int nSlotIndex );

	DnTalismanHandle GetTalisman( int nSlotIndex ) { return ( nSlotIndex >= 0 && nSlotIndex < TALISMAN_MAX ) ? m_hTalisman[nSlotIndex] : CDnTalisman::Identity(); }
	float GetTalismanEfficiency( int nSlotIndex ) const { return ( nSlotIndex >= 0 && nSlotIndex < TALISMAN_MAX ) ? m_fTalismanEfficiency[nSlotIndex] : 1.0f; }
	CDnState *GetTalismanScaledState( int nSlotIndex );
};

#endif // PRE_ADD_TALISMAN_SYSTEM
