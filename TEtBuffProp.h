#pragma once

#include "TEtWorldActProp.h"

class CTEtBuffProp : public CTEtWorldActProp {
public:
	CTEtBuffProp();
	virtual ~CTEtBuffProp();

protected:
	void DrawBuffRange();
	float GetRange();

public:
	virtual bool Render( LOCAL_TIME LocalTime );
};