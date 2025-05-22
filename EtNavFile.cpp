#include "StdAfx.h"
#include "EtNavFile.h"

CEtNavFile::CEtNavFile(void)
{
}

CEtNavFile::~CEtNavFile(void)
{
}

void CEtNavFile::Load( CStream *pStream )
{
	SNavFileHeader Header;
	pStream->Read( &Header, sizeof( SNavFileHeader ) );
	pStream->Seek( NAV_HEADER_RESERVED, SEEK_CUR );
	m_vecPoint.resize( Header.nTriangleCount * 3 );
	pStream->Read( &m_vecPoint[ 0 ], Header.nTriangleCount * 3 * sizeof( EtVector3 ) );
}
