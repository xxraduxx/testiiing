#pragma once

#include "EventCustomRender.h"
#include "EtVolumeFog.h"

class CECRVolumeFog : public CEventCustomRender
{
public:
	CECRVolumeFog( const char *szEventControlName );
	virtual ~CECRVolumeFog();

protected:
	EtVolumeFogHandle m_hVolumeFog;

public:
	virtual CEventCustomRender *Clone() { return new CECRVolumeFog( m_szEventControlName.c_str() ); }

	virtual void Initialize();
	virtual void OnSelect();
	virtual void OnUnselect();
	virtual void OnModify();

};