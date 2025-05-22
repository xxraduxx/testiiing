#pragma once

void WriteCString( CString *pStr, FILE *fp );
void ReadCString( CString *pStr, FILE *fp );

bool IsInMouseRect( CWnd *pWnd );

void ParseFileList( CString &szFileBuffer, char *szImportExt, std::vector<CString> &szVecList );

bool MakeResourceToFile( LPCSTR lpType, int nResourceID, LPCSTR lpTargetFile );

void CalcGaussianTable( int nBlurValue, float fEpsilon, float **ppfTable );