#include "StdAfx.h"
#include "PropEditWizardCommon.h"

#include "resource.h"

#include "PropEdit_SelectEditType.h"
#include "PropEdit_SelectType.h"
#include "PropEdit_CreateType.h"

#include "PropEdit_Finish.h"
#include "PropEdit_CreateParameter.h"
#include "PropEdit_SelectParameter.h"

int CPropEditWizardCommon::s_nStartType = -1;

CString CPropEditWizardCommon::s_szCurrentSignalName;
CPropSignalItem *CPropEditWizardCommon::s_pCurrentSignalItem = NULL;

CString CPropEditWizardCommon::s_szCurrentPropertyName;
CUnionValueProperty *CPropEditWizardCommon::s_pCurrentPropertyVariable = NULL;


SignalPropEditWizardStruct g_SignalPropEditWizardStructDefine[] = {
	{ CPropEditWizardCommon::SELECT_EDIT_TYPE, RUNTIME_CLASS( CPropEdit_SelectEditType ), NULL },
	{ CPropEditWizardCommon::CREATE_SIGNAL, RUNTIME_CLASS( CPropEdit_CreateType ), NULL },
	{ CPropEditWizardCommon::SELECT_SIGNAL, RUNTIME_CLASS( CPropEdit_SelectType ), NULL },
	{ CPropEditWizardCommon::FINISH, RUNTIME_CLASS( CPropEdit_Finish ), NULL },
	{ CPropEditWizardCommon::CREATE_PARAMETER, RUNTIME_CLASS( CPropEdit_CreateParameter ), NULL },
	{ CPropEditWizardCommon::SELECT_PARAMETER, RUNTIME_CLASS( CPropEdit_SelectParameter ), NULL },
	{ CPropEditWizardCommon::UNKNOWN_TYPE, NULL, NULL },
};

CWnd *CPropEditWizardCommon::GetWizardWindow( CPropEditWizardCommon::VIEW_TYPE Type )
{
	int nCount = sizeof(g_SignalPropEditWizardStructDefine) / sizeof(SignalPropEditWizardStruct);
	for( int i=0; i<nCount; i++ ) {
		if( g_SignalPropEditWizardStructDefine[i].ViewType == Type ) {
			if( g_SignalPropEditWizardStructDefine[i].pThis == NULL ) continue;
			CWnd *pWnd = ((CFrameWnd*)g_SignalPropEditWizardStructDefine[i].pThis)->GetWindow(GW_CHILD);
			return pWnd;
		}
	}
	return NULL;
}