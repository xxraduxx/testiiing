#pragma once
#include "afxcmn.h"


#include "EditLabelTreeCtrl.h"

// CTriggerPaneView form view

class CEtTriggerObject;
class CTriggerPaneView : public CFormView
{
	DECLARE_DYNCREATE(CTriggerPaneView)

protected:
	CTriggerPaneView();           // protected constructor used by dynamic creation
	virtual ~CTriggerPaneView();

protected:
	bool m_bActivate;

protected:
	CEditLabelTreeCtrl m_TreeCtrl;
	CImageList m_ImageList;
	CMenu *m_pContextMenu;
	HTREEITEM m_hRootTree;
	CTypedPtrList<CPtrList, HTREEITEM> m_SelectItemList;
	CString m_szPrevLabelString;
	CImageList *m_pDragImage;
	bool m_bDragging;
	HTREEITEM m_hDrag;
	HTREEITEM m_hDrop;
	std::vector<CEtTriggerObject *> m_pVecClipObjectList;
	bool m_bClipObjectCut;
	bool m_bCopyDrag;

protected:
	void CalcSelectItem();

public:
	enum { IDD = IDD_TRIGGERPANEVIEW };
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual void OnInitialUpdate();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg LRESULT OnRefresh( WPARAM wParam, LPARAM lParam );
	afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
	afx_msg void OnTriggerlistAddcategory();
	afx_msg void OnTriggerlistAddtrigger();
	afx_msg void OnTvnBeginlabeleditTree1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTvnEndlabeleditTree1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTvnSelchangedTree1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg LRESULT OnEnablePane( WPARAM wParam, LPARAM lParam );
public:
	afx_msg void OnTriggerlistDelete();
public:
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnTvnBegindragTree1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnTriggerlistCopy();
	afx_msg void OnTriggerlistCut();
	afx_msg void OnTriggerlistPaste();
	afx_msg void OnTvnKeydownTree1(NMHDR *pNMHDR, LRESULT *pResult);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};


