#pragma once


// CColorListCtrl

class CColorListCtrl : public CXTListCtrl
{
	DECLARE_DYNAMIC(CColorListCtrl)

public:
	CColorListCtrl();
	virtual ~CColorListCtrl();

	int m_nColorColumnIndex;

protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual void DrawItem(LPDRAWITEMSTRUCT /*lpDrawItemStruct*/);
	void DrawText( CDC *pDC, CString &szString, DWORD dwCol, DWORD dwBg, RECT Rect, UINT nDT );
	void DrawFrameBorder(CDC* pDC, LPRECT rect);
};


