// EtWorldPainter.h : EtWorldPainter ���� ���α׷��� ���� �� ��� ����
//
#pragma once

#ifndef __AFXWIN_H__
	#error "PCH�� ���� �� ������ �����ϱ� ���� 'stdafx.h'�� �����մϴ�."
#endif

#include "resource.h"       // �� ��ȣ�Դϴ�.


// CEtWorldPainterApp:
// �� Ŭ������ ������ ���ؼ��� EtWorldPainter.cpp�� �����Ͻʽÿ�.
//
class CPreLogoDlg;
class CEtWorldPainterApp : public CWinApp
{
public:
	CEtWorldPainterApp();


// �������Դϴ�.
public:
	virtual BOOL InitInstance();

protected:

// �����Դϴ�.
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnIdle(LONG lCount);
};

extern CEtWorldPainterApp theApp;
