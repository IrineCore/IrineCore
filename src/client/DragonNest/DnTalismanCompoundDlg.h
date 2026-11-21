#pragma once

#if defined(PRE_MOD_ITEM_COMPOUND_DIALOG_RENEWAL) && defined(PRE_ADD_TALISMAN_SYSTEM)

#include "DnCompoundBase.h"

class CDnTalismanCompoundDlg : public CDnCompoundBase
{
public:
	CDnTalismanCompoundDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL, bool bAutoCursor = false );
	virtual ~CDnTalismanCompoundDlg();

	enum {
		eTalismanNormal  = 24,
		eTalismanSpecial = 26,
	};

protected:
	CEtUITreeCtl *m_pTreeCtrlTalisman;
	std::map< int, std::vector<CDnCompoundBase::ItemGroupStruct*> > m_MapCategoryList;

	bool m_bRemote;
	CDnCompoundBase::ItemGroupStruct m_RemoteCompoundItem;

public:
	virtual void InitialUpdate();
	virtual void Initialize( bool bShow );
	virtual void Show( bool bShow );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );
	virtual void Reset();

	virtual void SetCompoundTable( int nType );
	virtual void UpdateGroupList( std::vector< CDnCompoundBase::ItemGroupStruct* >& pVecResultList );
	virtual void SetRemoteCompoundDlg( CDnCompoundBase::ItemGroupStruct& CompoundGroupData );

	bool IsExistItem( int nKey, int nExistitem );
	void CompoundGroupListClear();
	bool SetTalismanList();
};

#endif // PRE_MOD_ITEM_COMPOUND_DIALOG_RENEWAL && PRE_ADD_TALISMAN_SYSTEM
