#pragma once

class CDnSmartMoveCursor
{
public:
	CDnSmartMoveCursor();

protected:
	CEtUIControl *m_pControl;
	CEtUIDialog *m_pParentDlg;

	// Destination position
	float m_fMovePosX;
	float m_fMovePosY;

	// Previous position to remember
	float m_fPrePosX;
	float m_fPrePosY;

public:
	void SetControl( CEtUIControl* pControl );
	void MoveCursor();
	void ReturnCursor();
};

// When entering cursor mode from non-cursor mode (like the main menu), the smart move above does not work properly.
// Looking into the reason,
// Cursor movement is naturally disabled while cursor is hidden,
// Even if GetEtDevice()->ShowCursor( true ); is called, cursor movement does not work in that frame.
// That is, the cursor becomes visible starting from the frame after that frame is Presented, and cursor movement happens then.
//
// So in Process, we decided to move the cursor on the next frame, making the function differ from the above.
class CDnSmartMoveCursorEx
{
public:
	void SetControl( CEtUIControl* pControl );
	void MoveCursor();
	void Process();

	CDnSmartMoveCursorEx();

protected:
	CEtUIControl *m_pControl;
	CEtUIDialog *m_pParentDlg;
	bool m_bReadyMove;
};