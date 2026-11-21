#pragma once
#include "EtUITreeCtl.h"
#include "EtUIQuestTreeItem.h"


class CEtUIQuestTreeCtl : public CEtUITreeCtl
{
public:
	CEtUIQuestTreeCtl( CEtUIDialog *pParent );
	virtual ~CEtUIQuestTreeCtl(void);

public:
	virtual void Initialize( SUIControlProperty *pProperty );
};


//
//class CEtUIQuestTreeCtl : public CEtUITreeCtl
//{
//public:
//	CEtUIQuestTreeCtl( CEtUIDialog *pParent );
//	virtual ~CEtUIQuestTreeCtl(void);
//
//protected:
//	virtual void RenderTree( TREEITEM_LIST &VecTree );
//	//virtual void RenderButton( CTreeItem *pItem );
//	void RenderState( CTreeItem *pItem );
//	void RenderText( CTreeItem *pItem );
//	void RenderBackImg( CTreeItem *pItem );
//	void RenderFrame( CTreeItem *pItem );
//	void RenderSelFrame( CTreeItem *pItem );
//
//public:
//	/*CEtUIQuestTreeItem* AddQuestItem( DWORD dwType, LPCWSTR szText, DWORD dwColor = textcolor::WHITE );
//	CEtUIQuestTreeItem* AddQuestChildItem( CEtUIQuestTreeItem *pTreeItem, DWORD dwType, LPCWSTR szText, DWORD dwColor = textcolor::WHITE );*/
//
//public:
//	virtual void Initialize( SUIControlProperty *pProperty );
//	virtual int GetScrollBarTemplate() { return m_Property.QuestTreeControlProperty.nScrollBarTemplate; }
//	virtual void SetScrollBarTemplate( int nIndex ) { m_Property.QuestTreeControlProperty.nScrollBarTemplate = nIndex; }
//
//	// Select the last tree node.
//	void SelectLastTree();
//};