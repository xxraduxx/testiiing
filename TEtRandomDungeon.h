#pragma once
#include "EtRandomDungeon.h"
#include "Singleton.h"

class CEtMazeMask;
class CTEtRandomDungeon : public CEtRandomDungeon, public CSingleton<CTEtRandomDungeon>
{
public:
	CTEtRandomDungeon();
	virtual ~CTEtRandomDungeon();

protected:
	struct ObjectStruct {
		EtObjectHandle ***m_pObjectArray;
		int **m_pArrayCount;
		ObjectStruct() {
			m_pObjectArray = NULL;
			m_pArrayCount = NULL;
		}
	};
	std::vector<ObjectStruct> m_VecRenderList;

protected:
	void DrawBlock1( int nOffsetX, int nOffsetY, int nX, int nY, int nZ, CDC *pDC );
	void DrawBlock2( int nOffsetX, int nOffsetY, int nX, int nY, int nZ, CDC *pDC );
	void DrawBlock3( int nOffsetX, int nOffsetY, int nX, int nY, int nZ, CDC *pDC );

public:
	virtual bool Initialize( int nSeed = 0 );
	virtual void Destroy();
	bool GenerateMaze( int nWidth, int nHeight, int nLevel, int nSparseness, int nRandomness, int nDeadendsRemove );
	CEtMazeMask *GetMazeMask();
	CEtMaze *GetMaze();

	void DrawMazeToDC( int nX, int nY, int nLevel, int nDrawType, CDC *pDC );
	void RenderDungeon();
};