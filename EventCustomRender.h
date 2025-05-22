#pragma once

class CTEtWorldEventArea;
class CEventCustomRender
{
public:
	CEventCustomRender( const char *szEventControlName ) { m_szEventControlName = szEventControlName; m_pEventArea = NULL; }
	virtual ~CEventCustomRender() {}

protected:
	std::string m_szEventControlName;
	CTEtWorldEventArea *m_pEventArea;
	static std::vector<CEventCustomRender *> s_pVecList;

public:
	static void RegisterClass();
	static void UnregisterClass();
	static CEventCustomRender *AllocCustomRender( const char *szControlName );

	void SetEventArea( CTEtWorldEventArea *pArea ) { m_pEventArea = pArea; }
	virtual CEventCustomRender *Clone() { return new CEventCustomRender( m_szEventControlName.c_str() ); }

	virtual void Initialize() {}
	virtual void OnSelect() {}
	virtual void OnUnselect() {}
	virtual void OnModify() {}

	const char *GetEventControlName() { return m_szEventControlName.c_str(); }
};