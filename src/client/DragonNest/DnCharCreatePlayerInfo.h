#pragma once
#include "DnCustomDlg.h"


#ifdef PRE_MOD_SELECT_CHAR

class CDnCharCreatePlayerInfo : public CDnCustomDlg
{
public:
	CDnCharCreatePlayerInfo( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL  );
	virtual ~CDnCharCreatePlayerInfo();

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );
	virtual void Process( float fElapsedTime );
	virtual void Show( bool bShow );

public:
	void SetJobID( int nJobClassID, int nJobExplanationID );
	void PlayMovie( const std::string& fileName );
protected:
	CDnMovieControl*	m_pMovieCtrl;
};

#endif // PRE_MOD_SELECT_CHAR