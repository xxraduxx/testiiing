#pragma once


// CEditLabelListCtrl

class CEditLabelListCtrl : public CXTListCtrl
{
	DECLARE_DYNAMIC(CEditLabelListCtrl)

public:
	CEditLabelListCtrl();
	virtual ~CEditLabelListCtrl();

	UINT GetSelectedItem();
protected:
	DECLARE_MESSAGE_MAP()

	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);

};


