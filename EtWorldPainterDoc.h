// EtWorldPainterDoc.h : CEtWorldPainterDoc Ŭ������ �������̽�
//


#pragma once


class CEtWorldPainterDoc : public CDocument
{
protected: // serialization������ ��������ϴ�.
	CEtWorldPainterDoc();
	DECLARE_DYNCREATE(CEtWorldPainterDoc)

// Ư���Դϴ�.
public:

// �۾��Դϴ�.
public:

// �������Դϴ�.
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);

// �����Դϴ�.
public:
	virtual ~CEtWorldPainterDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// ������ �޽��� �� �Լ�
protected:
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnProjectConnect();
	afx_msg void OnProjectSave();
	afx_msg void OnProjectSetting();
	afx_msg void OnProjectDisconnect();
};


