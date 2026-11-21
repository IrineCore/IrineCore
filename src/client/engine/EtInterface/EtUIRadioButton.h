#pragma once
#include "etuicheckbox.h"

class CEtUIRadioButton : public CEtUICheckBox
{
public:
	CEtUIRadioButton( CEtUIDialog *pParent );
	virtual ~CEtUIRadioButton(void);

protected:
	SUICoord m_IconCoord;
	bool m_bEnableDragMode;		// Note : Whether tabs can be used while dragging an item or skill
	bool m_bIgnoreAdjustAlpha;

protected:
	virtual void SetCheckedInternal( bool bChecked, bool bClearGroup, bool bFromInput, bool bSound );

public:
	int GetTabID()												{ return m_Property.RadioButtonProperty.nTabID; }
	void EnableDragMode( bool bEnable )							{ m_bEnableDragMode = bEnable; }
	void SetChecked( bool bChecked, bool bClearGroup = true, bool bSound = true )	{ SetCheckedInternal( bChecked, bClearGroup, false, bSound ); }

	void SetButtonGroup( UINT nButtonGroup )					{ m_Property.RadioButtonProperty.nButtonGroup = nButtonGroup; }
	UINT GetButtonGroup()										{ return m_Property.RadioButtonProperty.nButtonGroup; }

	void SetIgnoreAdjustAlpha( bool bIgnore )					{ m_bIgnoreAdjustAlpha = bIgnore; }

	virtual void SetText( const std::wstring &strText , bool bFit = false, float fScaleX = 1.0f, float fScaleY = 1.0f) override;

public:
	virtual void Initialize( SUIControlProperty *pProperty );
	virtual bool HandleKeyboard( UINT uMsg, WPARAM wParam, LPARAM lParam );
	virtual bool HandleMouse( UINT uMsg, float fX, float fY, WPARAM wParam, LPARAM lParam );
	virtual void OnHotkey();
	virtual void UpdateRects(); 
	virtual void Render( float fElapsedTime );

	virtual void SetButtonRects( int nAllign );
};
