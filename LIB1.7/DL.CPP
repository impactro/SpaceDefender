//============================================================================
//
//  IMPACTRO Informática
//  Fábio Ferreira de Souza - 05/12/2002
//
//  Dinamic Libary 1.7
//
//============================================================================


#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>

#include "DL1.H"

// Estruturas de memórias
DWORD *		iVarPublic[255];
StringMask	cVarPublic[100];
char *		cVarStrings[100];

BYTE		bLastString=0;
BYTE		bLastCVar=CVAR_USERINIT;



// -----------------------------------------------------------------------
void dStartDL(){
	int i;
	for(i=0;i<100;i++)
		cVarStrings[i]=0;
}



// -----------------------------------------------------------------------
void dEndDL(){
	int i;
	for(i=0;i<100;i++){
		if( cVarStrings[i] )
			free( cVarStrings[i] );
	}
}



// -----------------------------------------------------------------------
int dlGetTipo( dlArray * aVetor, char * cTipo, int nTam ){
	int i,c,l;
	char cBt;
	for( i=0; i<nTam; i++ ){
		l=true;
		for( c=0; l; c++ ){
			cBt=*(aVetor[i].Conteudo + c );
			if( *(cTipo+c )!=cBt )
				l=false;
			else if( cBt== 0)
				return aVetor[i].Item;
		}
			
	}
	return 0;
}



// ------------------------------------------------
void dSetIntVarPublic( int nVar, DWORD * iVar ){
	iVarPublic[nVar]=iVar;
}



// ------------------------------------------------
void dSetCharVarPublic( int nVar, char * cMaskVar, int iVar1, int iVar2, int iVar3 ){
	cVarPublic[nVar].cMask=cMaskVar;
	cVarPublic[nVar].iVar1=iVar1;
	cVarPublic[nVar].iVar2=iVar2;
	cVarPublic[nVar].iVar3=iVar3;
}



// ------------------------------------------------
void dCompileCharVar( char * cTextOut, int nVar ){
	if( nVar==0 )
		return;

	int iVar1=cVarPublic[nVar].iVar1;
	int iVar2=cVarPublic[nVar].iVar2;
	int iVar3=cVarPublic[nVar].iVar3;

	if( iVar1 )
		iVar1=*iVarPublic[iVar1];

	if( iVar2 )
		iVar2=*iVarPublic[iVar2];

	if( iVar3 )
		iVar3=*iVarPublic[iVar3];

	sprintf( cTextOut, cVarPublic[nVar].cMask, iVar1, iVar2, iVar3 );
}



// ------------------------------------------------
BYTE dCreateCVarPublic( char * cText, BYTE v1, BYTE v2, BYTE v3 ){
	WORD nTam;
	BYTE bVar;

	// Obtem o tamanho e aloca espaço na memória
	nTam=strlen( cText );
	cVarStrings[bLastString]=(char*)malloc(nTam+1);
	*cVarStrings[bLastString]=0;

	// Copia o texto em uma área publica de meória
	strcat( cVarStrings[bLastString], cText);

	bVar=bLastCVar;
	dSetCharVarPublic( bVar, cVarStrings[bLastString], v1, v2, v3 );

	bLastCVar++;	// Prepara para a proxima alocação
	bLastString++;

	return bVar;
}

DWORD GetiVar( BYTE iVar ){
	return *iVarPublic[iVar ];
}
