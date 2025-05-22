#pragma once

#include "EtMatrixEx.h"
#include "EtObject.h"
#include "AxisRenderObject.h"

class CPointAxisRenderObject : public CEtCustomRender {
public:
	CPointAxisRenderObject( EtVector3 *pPoint );
	virtual ~CPointAxisRenderObject();

public:
	static int m_nSelectAxis;
	static bool m_bInitialize;

	static CAxisRenderObject::VertexFormat *m_pAxis[3];
	static CAxisRenderObject::VertexFormat *m_pAxisBack[3];
	static CAxisRenderObject::VertexFormat *m_pSelect[3];

	static int m_nPrimitiveCount[3];
	static float s_fAxisLength;
	static float s_fAxisPos;
	static float s_fAxisRadius;

	float m_fScale;
	static int s_nRefCount;
	EtVector3 *m_pPoint;

public:
	void Initialize();
	void Destroy();
	void DrawAxis();

	int CheckAxis( EtVector3 &vOrig, EtVector3 &vDir );
	void MoveAxis( int nX, int nY, float fSpeed );
	void SetScale( float fValue ) { m_fScale = fValue; }

	void Show( bool bShow ) { CEtCustomRender::Enable( bShow ); }
	bool IsShow() { return CEtCustomRender::IsEnable(); }

	virtual void RenderCustom( float fElapsedTime );

};