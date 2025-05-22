#pragma once



// CSectorInfoPaneView 폼 뷰입니다.
#include "PropertyGridCreator.h"
#include "ActionCommander.h"

class CTEtWorldSector;
class CSectorInfoPaneView : public CFormView, public CPropertyGridImp
{
	DECLARE_DYNCREATE(CSectorInfoPaneView)

protected:
	CSectorInfoPaneView();           // 동적 만들기에 사용되는 protected 생성자입니다.
	virtual ~CSectorInfoPaneView();

public:
	enum { IDD = IDD_SECTORINFOPANEVIEW };
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif
protected:
	bool m_bActivate;
	DWORD m_dwShaderParamCount;
	std::vector< CUnionValueProperty *> m_pVecPropertyList;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()

public:
	virtual void OnInitialUpdate();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg LRESULT OnRefresh( WPARAM wParam, LPARAM lParam );
	virtual void OnSetValue( CUnionValueProperty *pVariable, DWORD dwIndex );
	virtual void OnChangeValue( CUnionValueProperty *pVariable, DWORD dwIndex );
	virtual void OnSelectChangeValue( CUnionValueProperty *pVariable, DWORD dwIndex );
	afx_msg LRESULT OnGridNotify(WPARAM, LPARAM);
};


class CActionElementModifyHeightMultiply : public CActionElement {
public:
	CActionElementModifyHeightMultiply( CTEtWorldSector *pSector );
	virtual ~CActionElementModifyHeightMultiply();

protected:
	CTEtWorldSector *m_pSector;
	float m_fHeight;
	float m_fPrevHeight;

public:
	bool Redo();
	bool Undo();
	void ModifyHeightMultiply( float fHeight );
};

class CActionElementChangeBlockType : public CActionElement {
public:
	CActionElementChangeBlockType( CTEtWorldSector *pSector );
	virtual ~CActionElementChangeBlockType();

protected:
	CTEtWorldSector *m_pSector;
	std::vector<int> m_nVecBlockList;
	std::vector<int> m_nVecBlockType;
	std::vector<int> m_nVecPrevBlockType;

public:
	bool Redo();
	bool Undo();
	void ChangeBlockType( std::vector<int> &nVecList, std::vector<int> &nVecTypeList, std::vector<int> &nVecPrevTypeList );
};
