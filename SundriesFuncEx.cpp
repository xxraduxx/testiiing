#include "StdAfx.h"
#include "SundriesFuncEx.h"

void WriteCString( CString *pStr, FILE *fp )
{
	static char szStr[4096];
	sprintf_s( szStr, pStr->GetBuffer() );

	int nLength = pStr->GetLength() + 1;
	fwrite( &nLength, sizeof(int), 1, fp );
	fwrite( szStr, nLength, 1, fp );
}

void ReadCString( CString *pStr, FILE *fp )
{
	static char szStr[4096];
	int nLength;

	fread( &nLength, sizeof(int), 1, fp );
	fread( szStr, nLength, 1, fp );

	*pStr = szStr;
}

bool IsInMouseRect( CWnd *pWnd )
{
	CPoint p;
	CRect rcRect;
	GetCursorPos( &p );

	pWnd->GetWindowRect( &rcRect );
	if( p.x < rcRect.left || p.x > rcRect.right || p.y < rcRect.top || p.y > rcRect.bottom ) return false;
	return true;
}

void ParseFileList( CString &szFileBuffer, char *szImportExt, std::vector<CString> &szVecList )
{
	char szBuffer[1024], szExt[256];
	CString szFolder;
	std::vector<CString> szVecAniList;


	int nBeginPos = 0;
	int nEndPos = 0;
	char cPrevChar, cCurChar;
	bool bFirstFlag = false;
	bool bSecondFlag = false;

	for( DWORD i=1;; i++ ) {
		nEndPos = i;
		cCurChar = szFileBuffer.GetBuffer()[i];
		cPrevChar = szFileBuffer.GetBuffer()[i-1];
		if( cCurChar == 0 && cPrevChar != 0 ) {
			strncpy_s( szBuffer, szFileBuffer.GetBuffer() + nBeginPos, nEndPos - nBeginPos );
			nBeginPos = i+1;

			if( bFirstFlag == false ) {	// 첫번째건 폴더
				bFirstFlag = true;
				szFolder = szBuffer;
			}
			else { // 나머진 파일리스트
				bSecondFlag = true;
				_GetExt( szExt, _countof(szExt), szBuffer );
				if( _stricmp( szExt, szImportExt ) == NULL || szImportExt == NULL ) {
					szVecList.push_back( szFolder + "\\" + szBuffer );
				}
			}
		}
		else if( cCurChar == 0 && cPrevChar == 0 ) break;
	}

	if( bSecondFlag == false ) {
		_GetExt( szExt, _countof(szExt), szFolder );
		if( _stricmp( szExt, szImportExt ) == NULL || szImportExt == NULL ) {
			szVecList.push_back( szFolder );
		}
	}
}

bool MakeResourceToFile( LPCSTR lpType, int nResourceID, LPCSTR lpTargetFile )
{
	FILE *fp = NULL;
	HRSRC hrc = FindResource( NULL, MAKEINTRESOURCE( nResourceID ), lpType );
	if( !hrc ) return false;
	HGLOBAL hGlobal = LoadResource( NULL, hrc );
	if( !hGlobal ) return false;

	long nLength = SizeofResource( NULL, hrc );
	void *pPtr = LockResource( hGlobal );

	fopen_s( &fp, lpTargetFile, "wb" );
	if( fp == NULL ) return false;

	fwrite( pPtr, nLength, 1, fp );
	fclose( fp );

	UnlockResource( hGlobal );

	return true;
}
void CalcGaussianTable( int nBlurValue, float fEpsilon, float **ppfTable )
{
	int nTableSize = ( nBlurValue * 2 ) + 1;

	float fTemp = fEpsilon * fEpsilon;
	for( int j=-nBlurValue; j<=nBlurValue; j++ ) {
		for( int i=-nBlurValue; i<=nBlurValue; i++ ) {
			ppfTable[i+nBlurValue][j+nBlurValue] = ( 1.f / ( 2.f * D3DX_PI * fTemp ) ) * exp( -( ( (i*i) + (j*j) ) / ( 2.f * fTemp ) ) );
		}
	}
}

