#pragma once

#include "EtMatrixEx.h"
#include "EtObject.h"

class CEtWorldProp;
class CAxisRenderObject : public CEtCustomRender {
public:
	CAxisRenderObject( CEtWorldProp *pProp );
	virtual ~CAxisRenderObject();

	struct VertexFormat
	{
		EtVector3 vPos;
		DWORD dwColor;
	};

public:
	static int m_nSelectAxis;
	static bool m_bInitialize;

	static VertexFormat *m_pAxis[3];
	static VertexFormat *m_pAxisBack[3];
	static VertexFormat *m_pSelect[3];

	static int m_nPrimitiveCount[3];
	static float s_fAxisLength;
	static float s_fAxisPos;
	static float s_fAxisRadius;

	EtMatrix m_WorldMat;
	float m_fScale;
	static int s_nRefCount;
	CEtWorldProp *m_pProp;

	static int CreateCone( VertexFormat **pVertex, DWORD dwColor, float fRadius, float fLength, int nSegment, int nAxis, float fAxisPos );
public:
	void Initialize();
	void Destroy();
	void DrawAxis();

	int CheckAxis( EtVector3 &vOrig, EtVector3 &vDir );
	void MoveAxis( int nX, int nY, float fSpeed );
	void SetScale( float fValue ) { m_fScale = fValue; }

	virtual void RenderCustom( float fElapsedTime );
	void Update( EtMatrix *pMatrix );
	void Show( bool bShow ) { CEtCustomRender::Enable( bShow ); }
	bool IsShow() { return CEtCustomRender::IsEnable(); }

};