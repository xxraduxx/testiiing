#include "StdAfx.h"
#include "TEtRandomDungeon.h"
#include "EtMaze.h"
#include "EtMazeMask.h"
#include "FileServer.h"
#include "EtMatrixEx.h"


CTEtRandomDungeon g_RDungeon;
CTEtRandomDungeon::CTEtRandomDungeon()
{
}

CTEtRandomDungeon::~CTEtRandomDungeon()
{
	Destroy();
}

bool CTEtRandomDungeon::Initialize( int nSeed )
{
	m_VecRenderList.clear();
	CEtRandomDungeon::Initialize( nSeed );
	return true;
}

void CTEtRandomDungeon::Destroy()
{
	for( DWORD z=0; z<m_VecRenderList.size(); z++ ) {
		if( m_VecRenderList[z].m_pObjectArray ) {
			for( int i=0; i<m_DungeonSize.nX; i++ ) {
				for( int j=0; j<m_DungeonSize.nY; j++ ) {
					for( int k=0; k<m_VecRenderList[z].m_pArrayCount[i][j]; k++ ) {
						SAFE_RELEASE_SPTR( m_VecRenderList[z].m_pObjectArray[i][j][k] );
					}
					if( m_VecRenderList[z].m_pObjectArray[i][j] ) 
						SAFE_DELETEA( m_VecRenderList[z].m_pObjectArray[i][j] );
				}
				SAFE_DELETEA( m_VecRenderList[z].m_pObjectArray[i] );
				SAFE_DELETEA( m_VecRenderList[z].m_pArrayCount[i] );
			}
			SAFE_DELETEA( m_VecRenderList[z].m_pObjectArray );
			SAFE_DELETEA( m_VecRenderList[z].m_pArrayCount );

		}
	}
	SAFE_DELETE_VEC( m_VecRenderList );
	CEtRandomDungeon::Destroy();
}

CEtMazeMask *CTEtRandomDungeon::GetMazeMask()
{
	return m_pMazeMask;
}

CEtMaze *CTEtRandomDungeon::GetMaze()
{
	return m_pMaze;
}

void CTEtRandomDungeon::DrawBlock1( int nOffsetX, int nOffsetY, int nX, int nY, int nZ, CDC *pDC )
{
	int nBlockSize = 15;
	CRect rcRect;
	rcRect.left = nOffsetX + ( nX * nBlockSize );
	rcRect.right = rcRect.left + nBlockSize;
	rcRect.top = nOffsetY + ( nY * nBlockSize );
	rcRect.bottom = rcRect.top + nBlockSize;
	pDC->FillSolidRect( &rcRect, RGB( 128, 128, 128 ) );

	int nAttr = m_pMaze->GetMazeAt( nX, nY, nZ );
	if( nAttr != 0 ) {
		CRect rcTemp = rcRect;
		rcTemp.DeflateRect( 1, 1, 1, 1 );
		pDC->FillSolidRect( &rcTemp, RGB( 255, 255, 255 ) );

		if( nAttr & CEtMaze::MD_EAST ) {
			CPoint p[2];
			CPen Pen( PS_SOLID, 1, RGB( 255, 0, 0 ) );
			pDC->SelectObject( &Pen );

			p[0].x = rcTemp.left;
			p[0].y = rcTemp.top + rcTemp.Height() / 2;
			p[1].x = rcTemp.right;
			p[1].y = rcTemp.top + rcTemp.Height() / 2;
			pDC->Polyline( p, 2 );

			p[0].x = rcTemp.right;
			p[0].y = rcTemp.top + rcTemp.Height() / 2;
			p[1].x = rcTemp.right - 4;
			p[1].y = rcTemp.top + 4;
			pDC->Polyline( p, 2 );

			p[0].x = rcTemp.right;
			p[0].y = rcTemp.top + rcTemp.Height() / 2;
			p[1].x = rcTemp.right - 4;
			p[1].y = rcTemp.bottom - 4;
			pDC->Polyline( p, 2 );

		}

		if( nAttr & CEtMaze::MD_WEST ) {
			CPoint p[2];
			CPen Pen( PS_SOLID, 1, RGB( 255, 0, 0 ) );
			pDC->SelectObject( &Pen );

			p[0].x = rcTemp.left;
			p[0].y = rcTemp.top + rcTemp.Height() / 2;
			p[1].x = rcTemp.right;
			p[1].y = rcTemp.top + rcTemp.Height() / 2;
			pDC->Polyline( p, 2 );

			p[0].x = rcTemp.left;
			p[0].y = rcTemp.top + rcTemp.Height() / 2;
			p[1].x = rcTemp.left + 4;
			p[1].y = rcTemp.top + 4;
			pDC->Polyline( p, 2 );

			p[0].x = rcTemp.left;
			p[0].y = rcTemp.top + rcTemp.Height() / 2;
			p[1].x = rcTemp.left + 4;
			p[1].y = rcTemp.bottom - 4;
			pDC->Polyline( p, 2 );

		}
		if( nAttr & CEtMaze::MD_SOUTH ) {
			CPoint p[2];
			CPen Pen( PS_SOLID, 1, RGB( 255, 0, 0 ) );
			pDC->SelectObject( &Pen );

			p[0].x = rcTemp.left + rcTemp.Width() / 2;
			p[0].y = rcTemp.top;
			p[1].x = rcTemp.left + rcTemp.Width() / 2;
			p[1].y = rcTemp.bottom;
			pDC->Polyline( p, 2 );

			p[0].x = rcTemp.left + rcTemp.Width() / 2;
			p[0].y = rcTemp.bottom;
			p[1].x = rcTemp.left + 4;
			p[1].y = rcTemp.bottom - 4;
			pDC->Polyline( p, 2 );

			p[0].x = rcTemp.left + rcTemp.Width() / 2;
			p[0].y = rcTemp.bottom;
			p[1].x = rcTemp.right - 4;
			p[1].y = rcTemp.bottom - 4;
			pDC->Polyline( p, 2 );

		}
		if( nAttr & CEtMaze::MD_NORTH ) {
			CPoint p[2];
			CPen Pen( PS_SOLID, 1, RGB( 255, 0, 0 ) );
			pDC->SelectObject( &Pen );

			p[0].x = rcTemp.left + rcTemp.Width() / 2;
			p[0].y = rcTemp.top;
			p[1].x = rcTemp.left + rcTemp.Width() / 2;
			p[1].y = rcTemp.bottom;
			pDC->Polyline( p, 2 );

			p[0].x = rcTemp.left + rcTemp.Width() / 2;
			p[0].y = rcTemp.top;
			p[1].x = rcTemp.left + 4;
			p[1].y = rcTemp.top + 4;
			pDC->Polyline( p, 2 );

			p[0].x = rcTemp.left + rcTemp.Width() / 2;
			p[0].y = rcTemp.top;
			p[1].x = rcTemp.right - 4;
			p[1].y = rcTemp.top + 4;
			pDC->Polyline( p, 2 );

		}
		if( nAttr & CEtMaze::MD_UP ) {
			CRect rcTemp2 = rcTemp;
			rcTemp2.DeflateRect( 3, 3, 3, 3 );
			rcTemp2.right = rcTemp2.left + 3;
			pDC->FillSolidRect( &rcTemp2, RGB( 0, 0, 255 ) );
		}
		if( nAttr & CEtMaze::MD_DOWN ) {
			CRect rcTemp2 = rcTemp;
			rcTemp2.DeflateRect( 3, 3, 3, 3 );
			rcTemp2.left = rcTemp2.right - 3;
			pDC->FillSolidRect( &rcTemp2, RGB( 255, 0, 0 ) );
		}


	}
}

void CTEtRandomDungeon::DrawBlock2( int nOffsetX, int nOffsetY, int nX, int nY, int nZ, CDC *pDC )
{
	int nBlockSize = 15;
	CRect rcRect;
	rcRect.left = nOffsetX + ( nX * nBlockSize );
	rcRect.right = rcRect.left + nBlockSize;
	rcRect.top = nOffsetY + ( nY * nBlockSize );
	rcRect.bottom = rcRect.top + nBlockSize;
	pDC->FillSolidRect( &rcRect, RGB( 128, 128, 128 ) );


	int nAttr = m_pMaze->GetMazeAt( nX, nY, nZ );
	if( nAttr != 0 ) {
		if( nAttr & CEtMaze::MD_EAST ) {
			CRect rcTemp = rcRect;
			rcTemp.left += rcTemp.Width() / 2;
			rcTemp.top += 4;
			rcTemp.bottom -= 4;

			pDC->FillSolidRect( &rcTemp, RGB( 255, 255, 255 ) );

		}

		if( nAttr & CEtMaze::MD_WEST ) {
			CRect rcTemp = rcRect;
			rcTemp.right -= rcTemp.Width() / 2;
			rcTemp.top += 4;
			rcTemp.bottom -= 4;

			pDC->FillSolidRect( &rcTemp, RGB( 255, 255, 255 ) );
		}
		if( nAttr & CEtMaze::MD_NORTH ) {
			CRect rcTemp = rcRect;
			rcTemp.bottom -= rcTemp.Width() / 2;
			rcTemp.left += 4;
			rcTemp.right -= 4;

			pDC->FillSolidRect( &rcTemp, RGB( 255, 255, 255 ) );
		}
		if( nAttr & CEtMaze::MD_SOUTH ) {
			CRect rcTemp = rcRect;
			rcTemp.top += rcTemp.Width() / 2;
			rcTemp.left += 4;
			rcTemp.right -= 4;

			pDC->FillSolidRect( &rcTemp, RGB( 255, 255, 255 ) );
		}
	}
}

void CTEtRandomDungeon::DrawBlock3( int nOffsetX, int nOffsetY, int nX, int nY, int nZ, CDC *pDC )
{
	int nBlockSize = 15;
	CRect rcRect;
	rcRect.left = nOffsetX + ( nX * nBlockSize );
	rcRect.right = rcRect.left + nBlockSize;
	rcRect.top = nOffsetY + ( nY * nBlockSize );
	rcRect.bottom = rcRect.top + nBlockSize;
	pDC->FillSolidRect( &rcRect, RGB( 128, 128, 128 ) );


	int nAttr = GetDungeonAt( nX, nY, nZ );
	if( nAttr != 0 ) {
		if( nAttr & BA_PASSAGE ) {
			if( nAttr & BA_WEST ) {
				CRect rcTemp = rcRect;
				rcTemp.right -= rcTemp.Width() / 2;
				rcTemp.top += 4;
				rcTemp.bottom -= 4;

				pDC->FillSolidRect( &rcTemp, RGB( 255, 255, 255 ) );
			}
			if( nAttr & BA_EAST ) {
				CRect rcTemp = rcRect;
				rcTemp.left += rcTemp.Width() / 2;
				rcTemp.top += 4;
				rcTemp.bottom -= 4;

				pDC->FillSolidRect( &rcTemp, RGB( 255, 255, 255 ) );
			}
			if( nAttr & BA_NORTH ) {
				CRect rcTemp = rcRect;
				rcTemp.bottom -= rcTemp.Width() / 2;
				rcTemp.left += 4;
				rcTemp.right -= 4;

				pDC->FillSolidRect( &rcTemp, RGB( 255, 255, 255 ) );
			}
			if( nAttr & BA_SOUTH ) {
				CRect rcTemp = rcRect;
				rcTemp.top += rcTemp.Width() / 2;
				rcTemp.left += 4;
				rcTemp.right -= 4;

				pDC->FillSolidRect( &rcTemp, RGB( 255, 255, 255 ) );
			}
			if( nAttr & BA_UP ) {
				CRect rcTemp2 = rcRect;
				rcTemp2.DeflateRect( 3, 5, 3, 5 );
				rcTemp2.right = rcTemp2.left + 4;
				pDC->FillSolidRect( &rcTemp2, RGB( 0, 0, 255 ) );
			}
			if( nAttr & BA_DOWN ) {
				CRect rcTemp2 = rcRect;
				rcTemp2.DeflateRect( 3, 5, 3, 5 );
				rcTemp2.left = rcTemp2.right - 4;
				pDC->FillSolidRect( &rcTemp2, RGB( 255, 0, 0 ) );
			}
		}
		if( nAttr & BA_ROOM ) {
			CRect rcTemp = rcRect;
			pDC->FillSolidRect( &rcTemp, RGB( 100, 100, 255 ) );
		}
		if( nAttr & BA_DOOR ) {
			if( nAttr & BA_WEST ) {
				CRect rcTemp = rcRect;
				rcTemp.right -= rcTemp.Width() / 2 + 3;
				rcTemp.top += 4;
				rcTemp.bottom -= 4;

				pDC->FillSolidRect( &rcTemp, RGB( 255, 0, 255 ) );
			}
			if( nAttr & BA_EAST ) {
				CRect rcTemp = rcRect;
				rcTemp.left += rcTemp.Width() / 2 + 3;
				rcTemp.top += 4;
				rcTemp.bottom -= 4;

				pDC->FillSolidRect( &rcTemp, RGB( 255, 0, 255 ) );
			}
			if( nAttr & BA_NORTH ) {
				CRect rcTemp = rcRect;
				rcTemp.bottom -= rcTemp.Width() / 2 + 3;
				rcTemp.left += 4;
				rcTemp.right -= 4;

				pDC->FillSolidRect( &rcTemp, RGB( 255, 0, 255 ) );
			}
			if( nAttr & BA_SOUTH ) {
				CRect rcTemp = rcRect;
				rcTemp.top += rcTemp.Width() / 2 + 3;
				rcTemp.left += 4;
				rcTemp.right -= 4;

				pDC->FillSolidRect( &rcTemp, RGB( 255, 0, 255 ) );
			}
		}
	}
}


void CTEtRandomDungeon::DrawMazeToDC( int nX, int nY, int nLevel, int nDrawType, CDC *pDC )
{
	if( m_pMaze == NULL ) return;
	for( int i=0; i<m_pMaze->GetHeight(); i++ ) {
		for( int j=0; j<m_pMaze->GetWidth(); j++ ) {
			switch( nDrawType ) {
				case 0:
					DrawBlock1( nX, nY, j, i, nLevel, pDC );
					break;
				case 1:
					DrawBlock2( nX, nY, j, i, nLevel, pDC );
					break;
				case 2:
					DrawBlock3( nX, nY, j, i, nLevel, pDC );
					break;
			}
		}
	}
	if( nDrawType == 2 ) {
		for( int i=0; i<GetRoomCount(); i++ ) {
			if( GetRoom(i)->GetPosition()->nZ != nLevel ) continue;
			CString szStr;
			int x = nX + ( GetRoom(i)->GetPosition()->nX * 15 ) + 10;
			int y = nY + ( GetRoom(i)->GetPosition()->nY * 15 ) + 10;
			szStr.Format( "r%d", i );
			pDC->TextOut( x, y, szStr );
		}
	}

}


bool CTEtRandomDungeon::GenerateMaze( int nWidth, int nHeight, int nLevel, int nSparseness, int nRandomness, int nDeadendsRemove )
{
	CEtRandomDungeon::CreateMazeMask( nWidth, nHeight, nLevel );

	/*
	// GenerateMaze 가 불러지기 전에 고정 방을 Add 시켜야 한다.
	CEtRandomDungeonRoom *pRoom = new CEtRandomDungeonRoom();
	pRoom->Initialize( 8, 8, 0, 4, 4 );
	pRoom->AddDoor( 3, 2, CEtRandomDungeonRoom::DD_EAST );
	if( AddConstraintRoom( pRoom, true ) == -1 ) SAFE_DELETE( pRoom );
	*/
	/*
	CEtRandomDungeonRoom *pRoom = new CEtRandomDungeonRoom();
	pRoom->Initialize( 1, 1, 0, 1, 1 );
	pRoom->AddDoor( 0, 0, CEtRandomDungeonRoom::DD_EAST );
	if( AddConstraintRoom( pRoom, true ) == -1 ) SAFE_DELETE( pRoom );
	*/

	/*
	pRoom = new CEtRandomDungeonRoom();
	pRoom->Initialize( 8, 8, 1, 4, 4 );
	pRoom->AddDoor( 3, 2, CEtRandomDungeonRoom::DD_EAST );
	if( AddConstraintRoom( pRoom, true ) == -1 ) SAFE_DELETE( pRoom );
	*/

	/*
	pRoom = new CEtRandomDungeonRoom();
	pRoom->Initialize( 8, 8, 2, 4, 4 );
	pRoom->AddDoor( 3, 2, CEtRandomDungeonRoom::DD_EAST );
	if( AddConstraintRoom( pRoom, true ) == -1 ) SAFE_DELETE( pRoom );
	*/


	m_pMazeMask->AddRoad( 2, 2, -1, (CEtMaze::Maze_Direction)( CEtMaze::MD_WEST | CEtMaze::MD_EAST | CEtMaze::MD_NORTH | CEtMaze::MD_SOUTH ) );
	m_pMazeMask->AddRoad( 1, 2, -1, (CEtMaze::Maze_Direction)( CEtMaze::MD_EAST ) );
	m_pMazeMask->AddRoad( 3, 2, -1, (CEtMaze::Maze_Direction)( CEtMaze::MD_WEST ) );
	m_pMazeMask->AddRoad( 2, 1, -1, (CEtMaze::Maze_Direction)( CEtMaze::MD_SOUTH ) );
	m_pMazeMask->AddRoad( 2, 3, -1, (CEtMaze::Maze_Direction)( CEtMaze::MD_NORTH ) );
	/*
	m_pMazeMask->AddRoad( 9, 10, -1, (CEtMaze::Maze_Direction)( CEtMaze::MD_EAST ) );
	m_pMazeMask->AddRoad( 11, 10, -1, (CEtMaze::Maze_Direction)( CEtMaze::MD_WEST ) );
	m_pMazeMask->AddRoad( 10, 9, -1, (CEtMaze::Maze_Direction)( CEtMaze::MD_SOUTH ) );
	m_pMazeMask->AddRoad( 10, 11, -1, (CEtMaze::Maze_Direction)( CEtMaze::MD_NORTH ) );
	m_pMazeMask->AddRoad( 10, 10, -1, (CEtMaze::Maze_Direction)( CEtMaze::MD_WEST | CEtMaze::MD_EAST | CEtMaze::MD_NORTH | CEtMaze::MD_SOUTH ) );

	m_pMazeMask->AddRoad( 1, 1, 0, (CEtMaze::Maze_Direction)( CEtMaze::MD_UP | CEtMaze::MD_SOUTH ) );
//	m_pMazeMask->AddRoad( 1, 2, 0, (CEtMaze::Maze_Direction)( CEtMaze::MD_NORTH | CEtMaze::MD_SOUTH ) );

	m_pMazeMask->AddRoad( 1, 1, 1, (CEtMaze::Maze_Direction)( CEtMaze::MD_DOWN | CEtMaze::MD_SOUTH ) );
//	m_pMazeMask->AddRoad( 1, 2, 1, (CEtMaze::Maze_Direction)( CEtMaze::MD_NORTH | CEtMaze::MD_SOUTH ) );
	m_pMazeMask->AddBlock( 0, 0, 0, 1, 1 );
	m_pMazeMask->AddBlock( 2, 0, 0, 1, 1 );
	m_pMazeMask->AddBlock( 0, 1, 0, 1, 1 );
	m_pMazeMask->AddBlock( 2, 1, 0, 1, 1 );
	m_pMazeMask->AddBlock( 1, 0, 0, 1, 1 );

	m_pMazeMask->AddBlock( 0, 0, 1, 1, 1 );
	m_pMazeMask->AddBlock( 2, 0, 1, 1, 1 );
	m_pMazeMask->AddBlock( 0, 1, 1, 1, 1 );
	m_pMazeMask->AddBlock( 2, 1, 1, 1, 1 );
	m_pMazeMask->AddBlock( 1, 0, 1, 1, 1 );

	m_pMazeMask->AddRoad( 6, 16, 0, (CEtMaze::Maze_Direction)( CEtMaze::MD_UP | CEtMaze::MD_SOUTH ) );
	m_pMazeMask->AddRoad( 6, 16, 1, (CEtMaze::Maze_Direction)( CEtMaze::MD_DOWN | CEtMaze::MD_SOUTH ) );
	m_pMazeMask->AddBlock( 5, 15, 0, 1, 1 );
	m_pMazeMask->AddBlock( 7, 15, 0, 1, 1 );
	m_pMazeMask->AddBlock( 5, 16, 0, 1, 1 );
	m_pMazeMask->AddBlock( 7, 16, 0, 1, 1 );
	m_pMazeMask->AddBlock( 6, 15, 0, 1, 1 );

	m_pMazeMask->AddBlock( 5, 15, 1, 1, 1 );
	m_pMazeMask->AddBlock( 7, 15, 1, 1, 1 );
	m_pMazeMask->AddBlock( 5, 16, 1, 1, 1 );
	m_pMazeMask->AddBlock( 7, 16, 1, 1, 1 );
	m_pMazeMask->AddBlock( 6, 15, 1, 1, 1 );



	m_pMazeMask->AddRoad( 18, 15, 1, (CEtMaze::Maze_Direction)( CEtMaze::MD_UP | CEtMaze::MD_WEST ) );
	m_pMazeMask->AddRoad( 18, 15, 2, (CEtMaze::Maze_Direction)( CEtMaze::MD_DOWN | CEtMaze::MD_WEST ) );
	m_pMazeMask->AddBlock( 18, 14, 1, 1, 1 );
	m_pMazeMask->AddBlock( 19, 14, 1, 1, 1 );
	m_pMazeMask->AddBlock( 19, 15, 1, 1, 1 );
	m_pMazeMask->AddBlock( 19, 16, 1, 1, 1 );
	m_pMazeMask->AddBlock( 18, 16, 1, 1, 1 );

	m_pMazeMask->AddBlock( 18, 14, 2, 1, 1 );
	m_pMazeMask->AddBlock( 19, 14, 2, 1, 1 );
	m_pMazeMask->AddBlock( 19, 15, 2, 1, 1 );
	m_pMazeMask->AddBlock( 19, 16, 2, 1, 1 );
	m_pMazeMask->AddBlock( 18, 16, 2, 1, 1 );
	*/


	m_pMazeMask->AddStartBlock( 0, 0, 0 );

	CEtRandomDungeon::GenerateMaze( nWidth, nHeight, nLevel, nSparseness, nRandomness, nDeadendsRemove );


	CalcDungeonArray();

//	GenerateRandomRoom( 2, 0, 0, 1, 1 );
//	GenerateRandomRoom( 15, 2, 6, 2, 6 );
//	GenerateRandomRoom();

	// Test
	CString szFolder, szFullName;
	MatrixEx CrossVec;

	szFolder.Format( "%s\\Resource\\RandomRes\\", CFileServer::GetInstance().GetWorkingFolder() );

//	/*
	char *szFileNameList[] = {
		"Corridor_I_Opened.skn",
		"Corridor_I_Closed.skn",
		"Corridor_L.skn",
		"Corridor_T.skn",
		"Corridor_X.skn",
		"Roomset_Base.skn",
		"Roomset_Sealing.skn",
		"Roomset_wall_Closed.skn",
		"Roomset_wall_Open.skn",
	};
	float fRotationList[] = {
		90.f,
		0.f,
		-90.f,
		0.f,
		0.f,
		0.f,
	};
//	*/
	/*ww
	char *szFileNameList[] = {
		"Block_I.skn",
		"Block_I_end.skn",
		"Block_L.skn",
		"Block_T.skn",
		"Block_X.skn",
		"Roomset_Base.skn",
		"Roomset_Sealing.skn",
		"Roomset_wall_Closed.skn",
		"Roomset_wall_Open.skn",
	};
	float fRotationList[] = {
		0.f,
		0.f,
		0.f,
		0.f,
		0.f,
		0.f,
	};
	*/


	ObjectStruct Struct;
	for( int z=0; z<m_DungeonSize.nZ; z++ ) {
		Struct.m_pObjectArray = new EtObjectHandle **[m_DungeonSize.nX];
		Struct.m_pArrayCount = new int *[m_DungeonSize.nX];
		for( int i=0; i<m_DungeonSize.nX; i++ ) {
			Struct.m_pObjectArray[i] = new EtObjectHandle *[m_DungeonSize.nY];
			Struct.m_pArrayCount[i] = new int [m_DungeonSize.nY];
			for( int j=0; j<m_DungeonSize.nY; j++ ) {
				Struct.m_pObjectArray[i][j] = NULL;
				Struct.m_pArrayCount[i][j] = 0;
				int nAttr = GetDungeonAt( i, j, z );
				if( nAttr & BA_PASSAGE ) {
					Struct.m_pArrayCount[i][j] = 1;
					Struct.m_pObjectArray[i][j] = new EtObjectHandle[1];

					CrossVec.Identity();
					CrossVec.m_vPosition.x = ( -( m_DungeonSize.nX * m_fBlockSize ) / 2.f ) + ( i * m_fBlockSize );
					CrossVec.m_vPosition.y = m_fBlockSize * z;
					CrossVec.m_vPosition.z = ( ( m_DungeonSize.nY * m_fBlockSize ) / 2.f ) - ( j * m_fBlockSize );
					CrossVec.m_vPosition += m_vOffset;

					// '+' 1가지
					if( (nAttr & BA_EAST) && (nAttr & BA_WEST) && (nAttr & BA_NORTH) && (nAttr & BA_SOUTH) ) {
						CrossVec.RotateYaw( fRotationList[4] );
						Struct.m_pObjectArray[i][j][0] = EternityEngine::CreateStaticObject( szFolder + szFileNameList[4] );
					}
					// 'I' 자 경우 2가지
					if( (nAttr & BA_EAST) && (nAttr & BA_WEST) && !(nAttr & BA_NORTH) && !(nAttr & BA_SOUTH) ) {
						CrossVec.RotateYaw( fRotationList[0] );
						Struct.m_pObjectArray[i][j][0] = EternityEngine::CreateStaticObject( szFolder + szFileNameList[0] );
					}
					if( !(nAttr & BA_EAST) && !(nAttr & BA_WEST) && (nAttr & BA_NORTH) && (nAttr & BA_SOUTH) ) {
						CrossVec.RotateYaw( 90.f + fRotationList[0] );
						Struct.m_pObjectArray[i][j][0] = EternityEngine::CreateStaticObject( szFolder + szFileNameList[0] );
					}
					// 막힌길 4 가지
					if( (nAttr & BA_EAST) && !(nAttr & BA_WEST) && !(nAttr & BA_NORTH) && !(nAttr & BA_SOUTH) ) {
						CrossVec.RotateYaw( 90.f + fRotationList[1] );
						Struct.m_pObjectArray[i][j][0] = EternityEngine::CreateStaticObject( szFolder + szFileNameList[1] );
					}
					if( !(nAttr & BA_EAST) && (nAttr & BA_WEST) && !(nAttr & BA_NORTH) && !(nAttr & BA_SOUTH) ) {
						CrossVec.RotateYaw( -90.f + fRotationList[1] );
						Struct.m_pObjectArray[i][j][0] = EternityEngine::CreateStaticObject( szFolder + szFileNameList[1] );
					}
					if( !(nAttr & BA_EAST) && !(nAttr & BA_WEST) && (nAttr & BA_NORTH) && !(nAttr & BA_SOUTH) ) {
						CrossVec.RotateYaw( 180.f + fRotationList[1] );
						Struct.m_pObjectArray[i][j][0] = EternityEngine::CreateStaticObject( szFolder + szFileNameList[1] );
					}
					if( !(nAttr & BA_EAST) && !(nAttr & BA_WEST) && !(nAttr & BA_NORTH) && (nAttr & BA_SOUTH) ) {
						CrossVec.RotateYaw( fRotationList[1] );
						Struct.m_pObjectArray[i][j][0] = EternityEngine::CreateStaticObject( szFolder + szFileNameList[1] );
					}

					// 'L' 자 경우 4가지
					if( (nAttr & BA_EAST) && !(nAttr & BA_WEST) && !(nAttr & BA_NORTH) && (nAttr & BA_SOUTH) ) { //동남
						CrossVec.RotateYaw( -270.f + fRotationList[2] );
						Struct.m_pObjectArray[i][j][0] = EternityEngine::CreateStaticObject( szFolder + szFileNameList[2] );
					}
					if( (nAttr & BA_EAST) && !(nAttr & BA_WEST) && (nAttr & BA_NORTH) && !(nAttr & BA_SOUTH) ) { //동북
						CrossVec.RotateYaw( -180.f + fRotationList[2] );
						Struct.m_pObjectArray[i][j][0] = EternityEngine::CreateStaticObject( szFolder + szFileNameList[2] );
					}
					if( !(nAttr & BA_EAST) && (nAttr & BA_WEST) && (nAttr & BA_NORTH) && !(nAttr & BA_SOUTH) ) { //서북
						CrossVec.RotateYaw( -90.f + fRotationList[2] );
						Struct.m_pObjectArray[i][j][0] = EternityEngine::CreateStaticObject( szFolder + szFileNameList[2] );
					}
					if( !(nAttr & BA_EAST) && (nAttr & BA_WEST) && !(nAttr & BA_NORTH) && (nAttr & BA_SOUTH) ) { //서남
						CrossVec.RotateYaw( fRotationList[2] );
						Struct.m_pObjectArray[i][j][0] = EternityEngine::CreateStaticObject( szFolder + szFileNameList[2] );
					}
					// 'T' 자 경우 4가지
					if( (nAttr & BA_EAST) && !(nAttr & BA_WEST) && (nAttr & BA_NORTH) && (nAttr & BA_SOUTH) ) { //동남북
						CrossVec.RotateYaw( -270.f + fRotationList[3] );
						Struct.m_pObjectArray[i][j][0] = EternityEngine::CreateStaticObject( szFolder + szFileNameList[3] );
					}
					if( (nAttr & BA_EAST) && (nAttr & BA_WEST) && (nAttr & BA_NORTH) && !(nAttr & BA_SOUTH) ) { //동서북
						CrossVec.RotateYaw( -180.f + fRotationList[3] );
						Struct.m_pObjectArray[i][j][0] = EternityEngine::CreateStaticObject( szFolder + szFileNameList[3] );
					}
					if( !(nAttr & BA_EAST) && (nAttr & BA_WEST) && (nAttr & BA_NORTH) && (nAttr & BA_SOUTH) ) { //서남북
						CrossVec.RotateYaw( -90.f + fRotationList[3] );
						Struct.m_pObjectArray[i][j][0] = EternityEngine::CreateStaticObject( szFolder + szFileNameList[3] );
					}
					if( (nAttr & BA_EAST) && (nAttr & BA_WEST) && !(nAttr & BA_NORTH) && (nAttr & BA_SOUTH) ) { //동서남
						CrossVec.RotateYaw( fRotationList[3] );
						Struct.m_pObjectArray[i][j][0] = EternityEngine::CreateStaticObject( szFolder + szFileNameList[3] );
					}

					if( Struct.m_pObjectArray[i][j][0] ) {
						Struct.m_pObjectArray[i][j][0]->Update( CrossVec );
					}
				}
			}
		}
		for( int i=0; i<GetRoomCount(); i++ ) {
			CEtRandomDungeonRoom *pRoom = GetRoom(i);
			if( pRoom->GetPosition()->nZ != z ) continue;
			for( int j=0; j<pRoom->GetWidth(); j++ ) {
				for( int k=0; k<pRoom->GetHeight(); k++ ) {
					int nX = pRoom->GetPosition()->nX+j;
					int nY = pRoom->GetPosition()->nY+k;
					int nAttr = GetDungeonAt( nX, nY, z );

					CrossVec.Identity();
					CrossVec.m_vPosition.x = ( -( m_DungeonSize.nX * m_fBlockSize ) / 2.f ) + ( nX * m_fBlockSize );
					CrossVec.m_vPosition.y = m_fBlockSize * z;
					CrossVec.m_vPosition.z = ( ( m_DungeonSize.nY * m_fBlockSize ) / 2.f ) - ( nY * m_fBlockSize );
					CrossVec.m_vPosition += m_vOffset;

					if( j == 0 && k == 0 ) {
						Struct.m_pArrayCount[nX][nY] = 4;
						Struct.m_pObjectArray[nX][nY] = new EtObjectHandle[4];

						if( nAttr & BA_NORTH )
							Struct.m_pObjectArray[nX][nY][2] = EternityEngine::CreateStaticObject( szFolder + szFileNameList[8] );
						else Struct.m_pObjectArray[nX][nY][2] = EternityEngine::CreateStaticObject( szFolder + szFileNameList[7] );
						float fRotation = fRotationList[5];
						CrossVec.RotateYaw( fRotation );
						Struct.m_pObjectArray[nX][nY][2]->Update( CrossVec );
						CrossVec.RotateYaw( -fRotation );

						if( nAttr & BA_WEST )
							Struct.m_pObjectArray[nX][nY][3] = EternityEngine::CreateStaticObject( szFolder + szFileNameList[8] );
						else Struct.m_pObjectArray[nX][nY][3] = EternityEngine::CreateStaticObject( szFolder + szFileNameList[7] );
						CrossVec.RotateYaw( 90.f + fRotationList[5] );
						Struct.m_pObjectArray[nX][nY][3]->Update( CrossVec );
					}
					else if( j == 0 && k == pRoom->GetHeight() - 1 ) {
						Struct.m_pArrayCount[nX][nY] = 4;
						Struct.m_pObjectArray[nX][nY] = new EtObjectHandle[4];

						if( nAttr & BA_SOUTH )
							Struct.m_pObjectArray[nX][nY][2] = EternityEngine::CreateStaticObject( szFolder + szFileNameList[8] );
						else Struct.m_pObjectArray[nX][nY][2] = EternityEngine::CreateStaticObject( szFolder + szFileNameList[7] );
						float fRotation = 180.f + fRotationList[5];
						CrossVec.RotateYaw( fRotation );
						Struct.m_pObjectArray[nX][nY][2]->Update( CrossVec );
						CrossVec.RotateYaw( -fRotation );

						if( nAttr & BA_WEST )
							Struct.m_pObjectArray[nX][nY][3] = EternityEngine::CreateStaticObject( szFolder + szFileNameList[8] );
						else Struct.m_pObjectArray[nX][nY][3] = EternityEngine::CreateStaticObject( szFolder + szFileNameList[7] );
						CrossVec.RotateYaw( 90.f + fRotationList[5] );
						Struct.m_pObjectArray[nX][nY][3]->Update( CrossVec );

					}
					else if( j == pRoom->GetWidth() - 1 && k == 0 ) {
						Struct.m_pArrayCount[nX][nY] = 4;
						Struct.m_pObjectArray[nX][nY] = new EtObjectHandle[4];

						if( nAttr & BA_NORTH )
							Struct.m_pObjectArray[nX][nY][2] = EternityEngine::CreateStaticObject( szFolder + szFileNameList[8] );
						else Struct.m_pObjectArray[nX][nY][2] = EternityEngine::CreateStaticObject( szFolder + szFileNameList[7] );
						float fRotation = fRotationList[5];
						CrossVec.RotateYaw( fRotation );
						Struct.m_pObjectArray[nX][nY][2]->Update( CrossVec );
						CrossVec.RotateYaw( -fRotation );

						if( nAttr & BA_EAST )
							Struct.m_pObjectArray[nX][nY][3] = EternityEngine::CreateStaticObject( szFolder + szFileNameList[8] );
						else Struct.m_pObjectArray[nX][nY][3] = EternityEngine::CreateStaticObject( szFolder + szFileNameList[7] );
						CrossVec.RotateYaw( -90.f + fRotationList[5] );
						Struct.m_pObjectArray[nX][nY][3]->Update( CrossVec );

					}
					else if( j == pRoom->GetWidth() - 1 && k == pRoom->GetHeight() - 1 ) {
						Struct.m_pArrayCount[nX][nY] = 4;
						Struct.m_pObjectArray[nX][nY] = new EtObjectHandle[4];

						if( nAttr & BA_SOUTH )
							Struct.m_pObjectArray[nX][nY][2] = EternityEngine::CreateStaticObject( szFolder + szFileNameList[8] );
						else Struct.m_pObjectArray[nX][nY][2] = EternityEngine::CreateStaticObject( szFolder + szFileNameList[7] );
						float fRotation = 180.f + fRotationList[5];
						CrossVec.RotateYaw( fRotation );
						Struct.m_pObjectArray[nX][nY][2]->Update( CrossVec );
						CrossVec.RotateYaw( -fRotation );

						if( nAttr & BA_EAST )
							Struct.m_pObjectArray[nX][nY][3] = EternityEngine::CreateStaticObject( szFolder + szFileNameList[8] );
						else Struct.m_pObjectArray[nX][nY][3] = EternityEngine::CreateStaticObject( szFolder + szFileNameList[7] );
						CrossVec.RotateYaw( -90.f + fRotationList[5] );
						Struct.m_pObjectArray[nX][nY][3]->Update( CrossVec );
					}
					else if( j == 0 && k != 0 && k != pRoom->GetHeight() - 1 ) {
						Struct.m_pArrayCount[nX][nY] = 3;
						Struct.m_pObjectArray[nX][nY] = new EtObjectHandle[3];

						if( nAttr & BA_WEST )
							Struct.m_pObjectArray[nX][nY][2] = EternityEngine::CreateStaticObject( szFolder + szFileNameList[8] );
						else Struct.m_pObjectArray[nX][nY][2] = EternityEngine::CreateStaticObject( szFolder + szFileNameList[7] );
						CrossVec.RotateYaw( 90.f + fRotationList[5] );
						Struct.m_pObjectArray[nX][nY][2]->Update( CrossVec );
					}
					else if( j == pRoom->GetWidth() - 1 && k != 0 && k != pRoom->GetHeight() - 1 ) {
						Struct.m_pArrayCount[nX][nY] = 3;
						Struct.m_pObjectArray[nX][nY] = new EtObjectHandle[3];

						if( nAttr & BA_EAST )
							Struct.m_pObjectArray[nX][nY][2] = EternityEngine::CreateStaticObject( szFolder + szFileNameList[8] );
						else Struct.m_pObjectArray[nX][nY][2] = EternityEngine::CreateStaticObject( szFolder + szFileNameList[7] );
						CrossVec.RotateYaw( -90.f + fRotationList[5] );
						Struct.m_pObjectArray[nX][nY][2]->Update( CrossVec );
					}
					else if( k == 0 && j != 0 && j != pRoom->GetWidth() - 1 ) {
						Struct.m_pArrayCount[nX][nY] = 3;
						Struct.m_pObjectArray[nX][nY] = new EtObjectHandle[3];

						if( nAttr & BA_NORTH )
							Struct.m_pObjectArray[nX][nY][2] = EternityEngine::CreateStaticObject( szFolder + szFileNameList[8] );
						else Struct.m_pObjectArray[nX][nY][2] = EternityEngine::CreateStaticObject( szFolder + szFileNameList[7] );
						CrossVec.RotateYaw( fRotationList[5] );
						Struct.m_pObjectArray[nX][nY][2]->Update( CrossVec );

					}
					else if( k == pRoom->GetHeight() - 1 && j != 0 && j != pRoom->GetWidth() - 1 ) {
						Struct.m_pArrayCount[nX][nY] = 3;
						Struct.m_pObjectArray[nX][nY] = new EtObjectHandle[3];

						if( nAttr & BA_SOUTH )
							Struct.m_pObjectArray[nX][nY][2] = EternityEngine::CreateStaticObject( szFolder + szFileNameList[8] );
						else Struct.m_pObjectArray[nX][nY][2] = EternityEngine::CreateStaticObject( szFolder + szFileNameList[7] );
						CrossVec.RotateYaw( 180.f + fRotationList[5] );
						Struct.m_pObjectArray[nX][nY][2]->Update( CrossVec );

					}

					else {
						Struct.m_pArrayCount[nX][nY] = 2;
						Struct.m_pObjectArray[nX][nY] = new EtObjectHandle[2];
					}

					CrossVec.Identity();
					CrossVec.m_vPosition.x = ( -( m_DungeonSize.nX * m_fBlockSize ) / 2.f ) + ( nX * m_fBlockSize );
					CrossVec.m_vPosition.y = m_fBlockSize * z;
					CrossVec.m_vPosition.z = ( ( m_DungeonSize.nY * m_fBlockSize ) / 2.f ) - ( nY * m_fBlockSize );
					CrossVec.m_vPosition += m_vOffset;

					Struct.m_pObjectArray[nX][nY][0] = EternityEngine::CreateStaticObject( szFolder + szFileNameList[5] );
					Struct.m_pObjectArray[nX][nY][1] = EternityEngine::CreateStaticObject( szFolder + szFileNameList[6] );
					Struct.m_pObjectArray[nX][nY][0]->Update( CrossVec );
					Struct.m_pObjectArray[nX][nY][1]->Update( CrossVec );
				}
			}
		}
		m_VecRenderList.push_back( Struct );
	}

	return true;
}

void CTEtRandomDungeon::RenderDungeon()
{
}