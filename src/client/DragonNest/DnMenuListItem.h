#pragma once
#include <list>

class CDnMenuListItem
{

public:

	// MenuItem is divided into Main and Sub belonging to Main.
	enum EITEMTYPE
	{
		EITEMTYPE_MAIN = 0,
		EITEMTYPE_SUB,
	};

	bool m_bExpand;
	
private:

	CEtUIControl * m_pParentControl;
	
	CDnMenuListItem * m_pParentItem;
	std::vector<CDnMenuListItem *> m_vSubItem;

	EITEMTYPE m_ItemType;

	float m_fLineSpace;
	
	bool m_bEllipsis;
	bool m_bIsSelected;
	bool m_bIsOver;
	bool m_bPress;
	
	bool m_bShow;
	bool m_bGetOutArea; // Outside MenuList Clipping area - if TRUE, exclude from Render.

	std::wstring m_strText;
	std::wstring m_strOrigText;


	// Value.
	void *	m_pData;
	int		m_nValue;
	INT64	m_nValue64;
	float	m_fValue;
	std::wstring m_strValue;
	std::string m_strValue2;


public:
	SUICoord m_uiCoord;			// Note : Item coordinates	
	SUICoord m_BaseCoord;
	SUICoord m_SelectCoord;
	SUICoord m_OverCoord;

	SUICoord m_strCoord;		// Note : Coordinates where the text will be rendered
	SUICoord m_strBaseCoord;
	SUICoord m_strOverCoord;
	SUICoord m_strSelectCoord;

	float m_strBaseCoord_X;	
	float m_strOverCoord_X;
	float m_strSelectCoord_X;

	DWORD m_TextColor;

private:

	SUICoord & CrrStrCoord();

public:

	CDnMenuListItem();
	~CDnMenuListItem();

	void Initialize( CEtUIControl * pControl, EITEMTYPE type, bool bShow, int nDepth=1 );


	void DeleteSubItem();


	//void SetUICoord( SUICoord & uiCoord );
	void SetText( LPCWSTR szText, DWORD dwColor=textcolor::WHITE );
	std::wstring & GetText(){
		return m_strText;
	}
	std::wstring & GetTextOrg(){
		return m_strOrigText;
	}

	bool IsWithEllipsis(){
		return m_bEllipsis;
	}

	//bool SetTextWithEllipsis( const wchar_t *wszText, const wchar_t *wszSymbol );
	
	void SetUICoordX( float fX );
	void SetUICoordY( float fY );
	SUICoord& GetCoord(){
		return m_uiCoord; 
	}

	void SetParentItem( CDnMenuListItem * pParent ){
		m_pParentItem = pParent;
	}
	CDnMenuListItem * GetParentItem(){
		return m_pParentItem;
	}

	void AddItem( CDnMenuListItem * pItem );


	EITEMTYPE GetType(){
		return m_ItemType;
	}

	// bChild : if true, apply to child nodes as well.
	void Show( bool bShow, bool bChild=false );
	bool IsShow(){
		return m_bShow;
	}	

	bool IsSelected(){
		return m_bIsSelected;
	}
	void SetSelect( bool bSelect ){
		m_bIsSelected = bSelect;
		m_strCoord = CrrStrCoord(); // ( bSelect == true ? m_strSelectCoord : m_strBaseCoord ); // Text Coord.	
		SetText( m_strOrigText.c_str(), m_TextColor );
	}
	
	void ShowItems( bool bShow );

	bool IsOver(){
		return m_bIsOver;
	}
	void SetOver( bool bOver ){
		m_bIsOver = bOver;		
		m_strCoord = CrrStrCoord();
		SetText( m_strOrigText.c_str(), m_TextColor );
	}

	bool IsPress(){
		return m_bPress;
	}
	void SetPress( bool bPress ){
		m_bPress = bPress;
	}
	
	void SetGetOutArea( bool b ){
		m_bGetOutArea = b;
	}
	bool IsGetOutArea(){
		return m_bGetOutArea;
	}

	void SetTextColor( DWORD color ){
		m_TextColor = color;
	}
	DWORD GetTextColor(){
		return m_TextColor;
	}
	
	CDnMenuListItem * GetClickItem( float fX, float fY );

	int GetSubItemSize();

	CDnMenuListItem * GetSubItem( int idx ){
		if( idx < (int)m_vSubItem.size() )
			return m_vSubItem[ idx ];
		return NULL;
	}

	std::vector<CDnMenuListItem *> & GetSubItems(){
		return m_vSubItem;
	}
	
	CDnMenuListItem * GetFrontSubItem();
	CDnMenuListItem * GetEndSubItem();

	


	// Value.
	void SetItemData( void *pData )							{ m_pData = pData; }
	void* GetItemData()										{ return m_pData; }
	void SetItemValueInt( int nValue )						{ m_nValue = nValue; }
	int GetItemValueInt()									{ return m_nValue; }
	void SetItemValueFloat( float fValue )					{ m_fValue = fValue; }
	float GetItemValueFloat()								{ return m_fValue; }
	void SetItemValueString( const std::wstring &strValue )	{ m_strValue = strValue; }
	std::wstring &GetItemValueString()						{ return m_strValue; }
	void SetItemValueStringA( const std::string &strValue )	{ m_strValue2 = strValue; }
	std::string &GetItemValueStringA()						{ return m_strValue2; }
	void SetItemValueInt64( INT64 nValue )					{ m_nValue64 = nValue; }
	INT64 GetItemValueInt64()								{ return m_nValue64; }

};