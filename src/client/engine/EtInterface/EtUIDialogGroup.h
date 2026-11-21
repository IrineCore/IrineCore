#pragma once

class CEtUIDialogGroup
{
public:
	typedef std::pair<DWORD, CEtUIDialog*>	ID_DIALOG_PAIR;

	// Note : Better to change the vector to a map later.
	//
	typedef std::vector<ID_DIALOG_PAIR>		DIALOG_VEC;
	typedef DIALOG_VEC::iterator			DIALOG_VEC_ITER;

public:
	CEtUIDialogGroup(void);
	virtual ~CEtUIDialogGroup(void);

protected:
	DIALOG_VEC m_vecDialog;
	CEtUIDialog *m_pShowDialog;
	DWORD m_dwShowDialogID;

public:
	// Note : Function to add a dialog without initializing it
	//
	void SetDialog( DWORD dwDialogID, CEtUIDialog *pDialog );
	// Note : Initialize the dialog internally.
	//
	void AddDialog( DWORD dwDialogID, CEtUIDialog *pDialog );
	bool ChangeDialog( DWORD dwDialogID, CEtUIDialog *pDialog );
	bool DelDialog( DWORD dwDialogID );

	bool IsShowDialog( DWORD dwDialogID ) const;
	void ShowDialog( DWORD dwDialogID, bool bShow, bool bCloseAll = true );
	void CloseAllDialog();

	CEtUIDialog* GetDialog( DWORD dwDialogID );
	CEtUIDialog* GetDialogFromIndex( int nIndex );
	CEtUIDialog* GetShowDialog()			{ return m_pShowDialog; }
	const CEtUIDialog* GetShowDialog() const{ return m_pShowDialog; }
	DWORD GetShowDialogID()					{ return m_dwShowDialogID; }

	DIALOG_VEC& GetDialogList()				{ return m_vecDialog; }
	int GetDialogCount()					{ return (int)m_vecDialog.size(); }

	void MoveDialog( float fX, float fY );
};