#pragma once

#define NAV_FILE_STRING	"Eternity Engine Navigation Mesh File 0.1"
#define NAV_FILE_VERSION	10
#define NAV_HEADER_RESERVED	( 1024 - sizeof( SNavFileHeader ) )

struct SNavFileHeader
{
	char szHeaderString[ 256 ];
	int nVersion;
	int nTriangleCount;
};

class CEtNavFile
{
public:
	CEtNavFile(void);
	virtual ~CEtNavFile(void);

protected:
	std::vector< EtVector3 > m_vecPoint;

public:
	void Load( CStream *pStream );
};
