#pragma once
#include "EtUIHtmlTextBox.h"

// For NpcAnswer, unlike normal HtmlTextBox
// Custom implementation to handle two-line selection and indexing for long dialog lines.
class CDnNpcAnswerHtmlTextBox : public CEtUIHtmlTextBox
{
public:
	CDnNpcAnswerHtmlTextBox(CEtUIDialog *pParent);
	virtual ~CDnNpcAnswerHtmlTextBox(void);

protected:
	int m_nAnswerIndex;

	// Sound played when selecting a text line.
	// Used only in NPC dialog answers, so do not add sound to control properties and
	// Set it inside the code like this.
	int m_nSelectionSoundIndex;

#define NPC_ANSWER_ARROW
#ifdef NPC_ANSWER_ARROW
	// Arrow position
	int m_nArrowLineIndex;
	float m_fOffsetTime;
#endif

#define NPC_ANSWER_KEYBOARD_SELECT
#ifdef NPC_ANSWER_KEYBOARD_SELECT
	int m_nKeyboardSelectLineIndex;
#endif

protected:
	virtual void BeginParse(DWORD dwAppData, bool &bAbort);
	virtual void StartTag(CLiteHTMLTag *pTag, DWORD dwAppData, bool &bAbort);
	virtual void EndTag(CLiteHTMLTag *pTag, DWORD dwAppData, bool &bAbort);
	virtual void Characters(const CStringW &rText, DWORD dwAppData, bool &bAbort);
	virtual void EndParse(DWORD dwAppData, bool bIsAborted);

	virtual void RenderText( float fElapsedTime );
#ifdef NPC_ANSWER_ARROW
	virtual void RenderWord( float fElapsedTime, CWord& sWord, bool bRollOver = false, int nBorderFlag = 0 );
#endif

public:
	virtual void Initialize( SUIControlProperty *pProperty );
#ifdef NPC_ANSWER_KEYBOARD_SELECT
	virtual bool HandleKeyboard( UINT uMsg, WPARAM wParam, LPARAM lParam );
#endif
	virtual bool HandleMouse( UINT uMsg, float fX, float fY, WPARAM wParam, LPARAM lParam );
#ifdef NPC_ANSWER_ARROW
	virtual void Render( float fElapsedTime );
#endif

#ifdef PRE_ADD_AIM_ASSIST_BUTTON
	void GetLinePos( std::vector<EtVector2> &vecSmartMovePos );
#endif
};