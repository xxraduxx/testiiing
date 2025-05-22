#pragma once
#include "afxcmn.h"


#include "EtTriggerElement.h"

// CTriggerPropertiesPaneView form view

class CTEtTriggerObject;
//class CEtTriggerElement;
class CTriggerPropertiesPaneView : public CFormView
{
	DECLARE_DYNCREATE(CTriggerPropertiesPaneView)

protected:
	CTriggerPropertiesPaneView();           // protected constructor used by dynamic creation
	virtual ~CTriggerPropertiesPaneView();

protected:
	bool m_bActivate;
	CRichEditCtrl m_DescriptionCtrl;
	CXTTreeCtrl m_TreeCtrl;
	CImageList m_ImageList;
	CMenu *m_pContextMenu;
	HTREEITEM m_hRootTree;
	HTREEITEM m_hConditionTree;
	HTREEITEM m_hActionTree;
	HTREEITEM m_hEventTree;
	CTypedPtrList<CPtrList, HTREEITEM> m_SelectItemList;
	CString m_szPrevLabelString;
	bool m_bEnable;
	bool m_bLastAddElement;
	CTEtTriggerElement *m_pOriginalElement;

	CTEtTriggerObject *m_pTriggerObj;
	CImageList *m_pDragImage;
	bool m_bDragging;
	HTREEITEM m_hDrag;
	HTREEITEM m_hDrop;

	CTEtTriggerObject *m_pClipObject;
	std::vector<CEtTriggerElement *> m_pVecClipElementList;
	int m_nClipElementType;
	bool m_bClipObjectCut;
	bool m_bCopyDrag;

	struct OperatorTreeStruct {
		HTREEITEM hti;
		CEtTriggerElement::OperatorTypeEnum Type;
		int nValue[4];
	};
	std::vector<OperatorTreeStruct> m_VecOperatorList[2];
protected:
	void CalcSelectItem();
	void AddElement( int nType );
	bool OpenElementDialog( CTEtTriggerElement *pElement, bool bAddElement = false );

	bool CheckDeleteOperatorElement( HTREEITEM hti );
	bool IsOperatorTreeItem( HTREEITEM hti, int nConditionAction );
	OperatorTreeStruct *GetOperatorStruct( CTEtTriggerElement *pElement, int nConditionAction );
	HTREEITEM GetOperatorTreeHandle( CTEtTriggerElement *pElement, int nConditionAction );
	CEtTriggerElement::OperatorTypeEnum GetOperatorType( HTREEITEM hti, int nConditionAction );
	int GetOperatorIndex( HTREEITEM hti, int nConditionAction );
	int GetOperatorValue( HTREEITEM hti, int nConditionAction, int nIndex );
	void SetOperatorValue( HTREEITEM hti, int nConditionAction, int nIndex, int nValue );
	CTEtTriggerElement *GetOperatorPrevElement( HTREEITEM hti );
	void GetOperatorElementList( CEtTriggerElement::OperatorTypeEnum Type, int nOperatorIndex, int nConditionAction, std::vector<CTEtTriggerElement*> &VecResult );

	OperatorTreeStruct *InsertOperator( HTREEITEM hParent, HTREEITEM hInsertAfter, int nConditionAction, CEtTriggerElement::OperatorTypeEnum Type, int nValue, CTEtTriggerElement *pElement = NULL );

public:
	enum { IDD = IDD_TRIGGERPROPERTIESPANEVIEW };
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
	afx_msg void OnPaint();
	afx_msg void OnTriggerpropertiesAddaction();
	afx_msg void OnTriggerpropertiesAddcondition();
	afx_msg void OnTriggerpropertiesAddevent();
	afx_msg LRESULT OnPickObject( WPARAM wParam, LPARAM lParam );
	afx_msg void OnTriggerpropertiesProperties();
	afx_msg LRESULT OnEnablePane( WPARAM wParam, LPARAM lParam );
	afx_msg void OnTriggerpropertiesDelete();
	afx_msg void OnTriggerpropertiesCopy();
	afx_msg void OnTriggerpropertiesCut();
	afx_msg void OnTriggerpropertiesPaste();

	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnEnChangeRichedit21();
	afx_msg void OnEnUpdateRichedit21();
	afx_msg void OnTvnBegindragTree1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnTriggerpropertiesAddOperator_OR();
	afx_msg void OnTriggerpropertiesAddOperator_Random();
};


