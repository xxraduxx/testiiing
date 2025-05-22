#pragma once

#include "EtMatrixEx.h"
#include "EtObject.h"
#include "AxisRenderObject.h"

class CTEtWorldSoundEnvi;
class CAxisLockRenderObject : public CEtObject {
public:
	CAxisLockRenderObject( CTEtWorldSoundEnvi *pEnvi );
	virtual ~CAxisLockRenderObject();

protected:

	float m_fScale;
	CTEtWorldSoundEnvi *m_pEnvi;

public:
	void Initialize();
	void Destroy();
	void DrawAxis();

	int CheckAxis( EtVector3 &vOrig, EtVector3 &vDir, EtVector3 &vOffset );
	void MoveAxis( int nX, int nY );
	void SetScale( float fValue ) { m_fScale = fValue; }

	virtual void Render();
	virtual void GetExtent( EtVector3 &Origin, EtVector3 &Extent );

};