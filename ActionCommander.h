#pragma once

#include "Singleton.h"
#include "UnionValueProperty.h"

class CActionElement;
class CActionCommander {
public:
	CActionCommander();
	virtual ~CActionCommander();

protected:
	std::vector<CActionElement *> m_pVecElement;
	int m_nCurIndex;

public:
	void Reset();
	bool IsCanUndo();
	bool IsCanRedo();
	int GetUndoCount();
	int GetRedoCount();
	CString GetUndoDesc( int nIndex );
	CString GetRedoDesc( int nIndex );

	bool Redo();
	bool Undo();

	void Add( CActionElement *pElement );


};

class CActionElement {
public:
	CActionElement( CActionCommander *pCommander ) { m_pCommander = pCommander; }
	virtual ~CActionElement() {}

protected:
	CString m_szDesc;
	CActionCommander *m_pCommander;

public:
	void SetDesc( const char *szStr ) { m_szDesc = szStr; }
	CString GetDesc() { return m_szDesc; }

	virtual bool Redo() { return true; }
	virtual bool Undo() { return true; }

	void AddAction();
};

extern CActionCommander s_BrushActionCommander;
extern CActionCommander s_PropActionCommander;
extern CActionCommander s_NaviActionCommander;
extern CActionCommander s_SoundActionCommander;
extern CActionCommander s_EventActionCommander;
