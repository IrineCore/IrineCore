#pragma once
#include "DnSlotButton.h"

class CDnRadioMsgButton : public CDnSlotButton
{
public:
	CDnRadioMsgButton( CEtUIDialog *pParent );
	virtual ~CDnRadioMsgButton(void);

protected:
	//SUICoord m_ShortCutUV;
	//SUICoord m_ShortCutCoord;

	// Use a separate variable to reduce coupling with parent.
	EtTextureHandle m_hRadioMsgIcon;
	EtTextureHandle m_hRadioMsgIconList[RADIOMSG_TEXTURE_COUNT];

public:
	// For a standard SlotButton, create a derived MIInventoryItem object and call SetItem.
	// However, this approach seems to keep increasing class coupling, which is not good.
	// Since radio message objects only have a few fields anyway, receive them directly.
	// So do not use the m_pItem member.
	void SetRadioMsgInfo( int nRadioMsgID );

public:
	virtual void Initialize( SUIControlProperty *pProperty );
	virtual void Render( float fElapsedTime );
	virtual void RenderDrag( float fElapsedTime );
};