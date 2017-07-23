//+---------------------------------------------------------------------------
//
//  IMPACTRO Informática
//  Fábio Ferreira de Souza - 25/11/2002
//
//  Create Dinamic File Libary 1
//  Versão 1.7
//
//----------------------------------------------------------------------------

#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>

#include "resource.h"
#include "CreateDL.h"
#include "..\Lib1.7\DL1.H"

HANDLE	DL1=NULL;
char *	cFileINI;
char	cPath[200];
char	cFileDefault[50];

// -------------------------------------------------------------

int APIENTRY WinMain( HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR pCmdLine, int nCmdShow )
{

	// Inicia e Registra tarefa no Windows
	HWND hWnd;
	WinInit( hInst, nCmdShow, &hWnd );

	// Arquivo de saida
	char		cFileOut[200];
	char		cFile[50];

	DWORD		dwByte;
	WORD		wLoad;
	char 		cCopyrights[]="DL1.7 - www.impactro.com.br (Arquivos de dados dinamico)";
	BYTE		Bt;
	BYTE		nBMPs=0,bCols,bRows,bImgs;

	// Tipos
	char		cTipo[10];
	int 		iTipo;
	dlArray		dlTipo[]= { { "STRING"		,DLTP_STRING },		// 0
							{ "VARIAVEL"	,DLTP_VARIAVEL },	// 1
							{ "TELA"		,DLTP_TELA },		// 2
							{ "IMAGEM"		,DLTP_IMAGEM },		// 3
							{ "ANIMACAO"	,DLTP_ANIMACAO },	// 4
							{ "QUADRANTE"	,DLTP_QUADRANTE },	// 5
							{ "SPRITE"		,DLTP_SPRITE },		// 6
							{ "TEXT"		,DLTP_TEXT },		// 7
							{ "PLAYERDEF"	,DLTP_PLAYERDEF },	// 8
							{ "IA"			,DLTP_IA },			// 9
							{ "TIRO"		,DLTP_TIRO },		// 10
							{ "BOMBA"		,DLTP_BOMBA },		// 11
							{ "BOTAO"		,DLTP_BOTAO },		// 12
							{ "INPUT"		,DLTP_INPUT },		// 13
							{ "SELECT"		,DLTP_SELECT },		// 14
							{ "CHECK"		,DLTP_CHECK },		// 15
							{ "RADIO"		,DLTP_RADIO },		// 16
							{ "MENU"		,DLTP_MENU } };		// 17

	int i,n,l;
	char cTexto[100];
	n=strlen(pCmdLine);
	
	if( n==0 ){
		MessageBox( hWnd, "Informe o Nome do arquivo ?.INI\n(DragDrop)", "Create DL", MB_ICONERROR | MB_OK );
		return -1;
		}

	cFileINI=pCmdLine;

	GetPrivateProfileString("DL1", "TIPO", "?", cTipo, 10, cFileINI );

	if( cTipo[0]=='?' ){
		MessageBox( hWnd, "Não foi encontrado o TIPO DE DESCRITOR", "Create DL", MB_ICONSTOP | MB_OK );
		return -1;
		}

	// Monta o nome do arquivo de saida
	for(i=0;i<n;i++){
		cPath[i]=*( pCmdLine+i );
		if( cPath[i]=='\\' )
			l=i;
		}

	cPath[l+1]=0;
	
	// Especifica o arquivo Default de saida e entrada com o mesno nome do INI
	for( i=0; *( pCmdLine+i+l ) && *( pCmdLine+i+l )!='.'; i++ ){
		cFileDefault[i]=*( pCmdLine+i+l );
		}
	cFileDefault[i]=0;

	cFileOut[0]=0;
	strcat( cFileOut, cPath );

	GetPrivateProfileString("DL1", "OUTPUT", "?", cFile, 50, cFileINI );
	if( cFile[0]=='?' ){
		sprintf( cFile, "%s.DL1", cFileDefault );
		}
	strcat( cFileOut, cFile );

	strcat( cFileDefault, "%d.bmp" );

	// Apaga e cria um novo arquivo
	DeleteFile( cFileOut );
	DL1 = CreateFile( cFileOut, GENERIC_WRITE, 0, NULL, CREATE_NEW, 0, NULL );

	// Obtem o Tipo
	_strupr( cTipo );
	iTipo=dlGetTipo( dlTipo, cTipo, sizeof(dlTipo) );
	
	// Grava o Identificador e CopyRights do arquivo .DL1
	WriteFile( DL1, cCopyrights, sizeof(cCopyrights), &dwByte, NULL );
	
	Bt=26;		// EOF para texto
	WriteFile( DL1, &Bt, 1, &dwByte, NULL );

	Bt=iTipo;	// Grava o tipo de arquivo
	WriteFile( DL1, &Bt, 1, &dwByte, NULL );

	Bt=0xFF;	// ESCAPE - Inicio de dados
	WriteFile( DL1, &Bt, 1, &dwByte, NULL );

	// a Nave do jogador é um sprote 'burro', sem trajetória, disparo e IA
	if( iTipo==DLTP_PLAYERDEF )
		iTipo=DLTP_SPRITE;

	if( iTipo>=DLTP_TELA && iTipo<=DLTP_SPRITE ){

		// Plano
		Bt=(BYTE) GetPrivateProfileInt("DL1", "Plano", 0, cFileINI );
		WriteFile( DL1, &Bt, sizeof(Bt), &dwByte, NULL );

		if( iTipo!=DLTP_TELA ){

			// Posição
			wLoad=GetPrivateProfileInt("DL1", "PosX", 0, cFileINI );
			WriteFile( DL1, &wLoad, sizeof(wLoad), &dwByte, NULL );

			wLoad=GetPrivateProfileInt("DL1", "PosY", 0, cFileINI );
			WriteFile( DL1, &wLoad, sizeof(wLoad), &dwByte, NULL );
		}

		if( iTipo==DLTP_ANIMACAO || iTipo==DLTP_QUADRANTE || iTipo==DLTP_SPRITE ){

			// Lê o número de Linhas e Colunas
			bCols=(BYTE) GetPrivateProfileInt("DL1", "Cols", 0, cFileINI );
			bRows=(BYTE) GetPrivateProfileInt("DL1", "Rows", 0, cFileINI );
			bImgs=(BYTE) GetPrivateProfileInt("DL1", "Imgs", 0, cFileINI );

			if( bImgs>0 && bRows==0 && bCols==0 ){
				bCols=1;
				bRows=bImgs;
				nBMPs=bImgs;
				}

			WriteFile( DL1, &bCols, sizeof(bCols), &dwByte, NULL );
			WriteFile( DL1, &bRows, sizeof(bRows), &dwByte, NULL );

			// Frame Rate
			wLoad=GetPrivateProfileInt("DL1", "TickFrame", 1000, cFileINI );
			WriteFile( DL1, &wLoad, sizeof(wLoad), &dwByte, NULL );

		}

		if( iTipo==DLTP_ANIMACAO ){
			// Variável que define o CurrentFrame
			Bt=(BYTE) GetPrivateProfileInt("DL1", "VAR", 0, cFileINI );
			WriteFile( DL1, &Bt, sizeof(Bt), &dwByte, NULL );
		}

		if( iTipo==DLTP_QUADRANTE ){
			nBMPs=bRows*bCols;

			// Incremento X e Y
			Bt=(BYTE) GetPrivateProfileInt("DL1", "IncX", 0, cFileINI );
			WriteFile( DL1, &Bt, sizeof(Bt), &dwByte, NULL );

			Bt=(BYTE) GetPrivateProfileInt("DL1", "IncY", 0, cFileINI );
			WriteFile( DL1, &Bt, sizeof(Bt), &dwByte, NULL );

			// Define se é uma tela que ao terinar volta ao começo!!! (BOOL)
			Bt=(BYTE) GetPrivateProfileInt("DL1", "LOOP", 0, cFileINI );
			WriteFile( DL1, &Bt, sizeof(Bt), &dwByte, NULL );

		}

		if( iTipo==DLTP_SPRITE ){

			wLoad=(BYTE) GetPrivateProfileInt("DL1", "Pontos", 5, cFileINI );
			WriteFile( DL1, &wLoad, sizeof(wLoad), &dwByte, NULL );

			wLoad=(BYTE) GetPrivateProfileInt("DL1", "Power", 3, cFileINI );
			WriteFile( DL1, &wLoad, sizeof(wLoad), &dwByte, NULL );

			wLoad=(BYTE) GetPrivateProfileInt("DL1", "Energia", 5, cFileINI );
			WriteFile( DL1, &wLoad, sizeof(wLoad), &dwByte, NULL );

			LoadInfoSegmento();
			LoadInfoTrajetoria();
			LoadInfoPosicoes();
			LoadInfoBomba();
		}

		LoadInfoBMP(nBMPs);
	}
	else if( iTipo==DLTP_TEXT ){

		// Plano
		Bt=(BYTE) GetPrivateProfileInt("DL1", "Plano", 0, cFileINI );
		WriteFile( DL1, &Bt, sizeof(Bt), &dwByte, NULL );

		// Posição
		wLoad=GetPrivateProfileInt("DL1", "PosX", 0, cFileINI );
		WriteFile( DL1, &wLoad, sizeof(wLoad), &dwByte, NULL );

		wLoad=GetPrivateProfileInt("DL1", "PosY", 0, cFileINI );
		WriteFile( DL1, &wLoad, sizeof(wLoad), &dwByte, NULL );

		LoadInfoTEXT();

	}
	else if( iTipo==DLTP_TIRO || iTipo==DLTP_BOMBA ){
		// ID
		Bt=(BYTE) GetPrivateProfileInt("DL1", "ID", 0, cFileINI );
		WriteFile( DL1, &Bt, sizeof(Bt), &dwByte, NULL );

		// Lê o número de Linhas e Colunas
		bCols=(BYTE) GetPrivateProfileInt("DL1", "Cols", 0, cFileINI );
		WriteFile( DL1, &bCols, sizeof(bCols), &dwByte, NULL );

		bRows=(BYTE) GetPrivateProfileInt("DL1", "Rows", 0, cFileINI );
		WriteFile( DL1, &bRows, sizeof(bRows), &dwByte, NULL );

		// Frame Rate
		wLoad=GetPrivateProfileInt("DL1", "TickFrame", 50, cFileINI );
		WriteFile( DL1, &wLoad, sizeof(wLoad), &dwByte, NULL );

		// Power
		Bt=GetPrivateProfileInt("DL1", "Power", 1, cFileINI );
		WriteFile( DL1, &Bt, sizeof(Bt), &dwByte, NULL );

		// Delay entre um disparo e outro
		wLoad=GetPrivateProfileInt("DL1", "Delay", 200, cFileINI );
		WriteFile( DL1, &wLoad, sizeof(wLoad), &dwByte, NULL );

		// Velocidades X e Y (Velocidades Máximas)
		Bt=GetPrivateProfileInt("DL1", "VX", 0, cFileINI );
		WriteFile( DL1, &Bt, sizeof(Bt), &dwByte, NULL );

		Bt=GetPrivateProfileInt("DL1", "VY", 0, cFileINI );
		WriteFile( DL1, &Bt, sizeof(Bt), &dwByte, NULL );

		if( iTipo==DLTP_TIRO ){

			// Posição do Dispado X e Y
			wLoad=GetPrivateProfileInt("DL1", "DisparoX", 10, cFileINI );
			WriteFile( DL1, &wLoad, sizeof(wLoad), &dwByte, NULL );

			wLoad=GetPrivateProfileInt("DL1", "DisparoY", 10, cFileINI );
			WriteFile( DL1, &wLoad, sizeof(wLoad), &dwByte, NULL );

		}

		LoadInfoBMP(0);

	}

	// Fecha o arquivo de saida
	CloseHandle( DL1 );
	
	// Se o tipo for invalido
	if( iTipo==0 ){
		sprintf( cTexto, "DESCRITOR INVALIDO: '%s'", cTipo );
		MessageBox( hWnd, cTexto, "Create DL", MB_ICONERROR | MB_OK );
		return -1;
	}

	return 0;
}

// -------------------------------------------------------------

HRESULT WinInit( HINSTANCE hInst, int nCmdShow, HWND* phWnd )
{
    WNDCLASS wc;
    HWND     hWnd;

    // Register the Window Class
    wc.lpszClassName = PROGRAMNAME;
    wc.lpfnWndProc   = MainWndProc;
    wc.style         = CS_VREDRAW | CS_HREDRAW;
    wc.hInstance     = hInst;
    wc.hIcon         = LoadIcon( hInst, MAKEINTRESOURCE(IDI_MAIN) );
    wc.hCursor       = LoadCursor( NULL, IDC_ARROW );
    wc.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
    wc.lpszMenuName  = NULL;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;

    if( RegisterClass( &wc ) == 0 )
        return E_FAIL;

    hWnd = CreateWindowEx( 0, PROGRAMNAME, PROGRAMNAME,
                           0, 0, 0, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, hInst, 0);
    if( hWnd == NULL )
    	return E_FAIL;

    UpdateWindow( hWnd );

    *phWnd   = hWnd;
    
    return S_OK;
}

// -------------------------------------------------------------

LRESULT CALLBACK MainWndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    switch (msg)
    {

	case WM_DESTROY:
		PostQuitMessage( 0 );
        return 0L;
    }

    return DefWindowProc(hWnd, msg, wParam, lParam);
}

// -------------------------------------------------------------

void LoadInfoBMP( BYTE nBmpMax ){
	BYTE				nBmp=1;
	char				cBMP[50];
	char				cVarBMP[50];
	char				cFileBMP[200];
	HANDLE				Bmp;
    BITMAPFILEHEADER	bFile;
    BITMAPINFOHEADER	bInfo;
	DWORD				dwBytes;
	DWORD				dwRepete;
	dlRGB				p1,p2,p3;
	WORD				wTamX,wTamY;

	GetPrivateProfileString( "DL1", "BMP", cFileDefault, cVarBMP, 50, cFileINI );

	do{

		sprintf( cBMP, cVarBMP, nBmp );

		cFileBMP[0]=0;
		strcat( cFileBMP, cPath );
		strcat( cFileBMP, cBMP );

		Bmp = CreateFile( cFileBMP, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL );

		ReadFile( Bmp, &bFile, sizeof(bFile), &dwBytes, NULL );
		if( dwBytes != sizeof(bFile) ){
			MessageBox( NULL, "Arquivo BMP não encontrado ou corrompido", "Create DL", MB_ICONERROR | MB_OK );
			CloseHandle( Bmp );
			return;
		}

		ReadFile( Bmp, &bInfo, sizeof(bInfo), &dwBytes, NULL );
		if( dwBytes != sizeof(bInfo) ){
			CloseHandle( Bmp );
			return;
		}

		if( !( bInfo.biBitCount==24 && bInfo.biCompression==0 ) )
			return;

		if( nBmp==1 ){
			// Grava Dimenções reais apenas para a primeira imagem
			// Nos Quadrantes todas as imagens tem o mesmo tamanho
		
			wTamX=(WORD) bInfo.biWidth;
			wTamY=(WORD) bInfo.biHeight;

			WriteFile( DL1, &wTamX, sizeof(wTamX), &dwBytes, NULL );
			WriteFile( DL1, &wTamY, sizeof(wTamY), &dwBytes, NULL );

			// Configura fundo e transparencia em p3
			p3.R=GetPrivateProfileInt( "DL1", "FundoR", 0, cFileINI );
			p3.G=GetPrivateProfileInt( "DL1", "FundoG", 0, cFileINI );
			p3.B=GetPrivateProfileInt( "DL1", "FundoB", 0, cFileINI );

			WriteFile( DL1, &p3, sizeof(p3), &dwBytes, NULL );
			
			p2.R=p3.R;
			p2.G=p3.G;
			p2.B=p3.B;

		}

		dwRepete=0;

		for( WORD iY=0; iY<wTamY; iY++ ){
			for( WORD iX=0; iX<wTamX; iX++ ){
				ReadFile( Bmp, &p1, sizeof(p1), &dwBytes, NULL );
				if( p1.R==p3.R && p1.G==p3.G && p1.B==p3.B ){
					dwRepete++;
				}
				else{
					if( dwRepete ){
						WriteFile( DL1, &p3, sizeof(p3), &dwBytes, NULL );
						WriteFile( DL1, &dwRepete, sizeof(dwRepete), &dwBytes, NULL );
						dwRepete=0;
					}
					WriteFile( DL1, &p1, sizeof(p1), &dwBytes, NULL );
				}
			}
		}

		if( dwRepete ){
			WriteFile( DL1, &p3, sizeof(p3), &dwBytes, NULL );
			WriteFile( DL1, &dwRepete, sizeof(dwRepete), &dwBytes, NULL );
			dwRepete=0;
		}

		CloseHandle( Bmp );

		nBmp++;

	}while( nBmp<=nBmpMax );
}

// -------------------------------------------------------------

void LoadInfoTEXT(){
	char	cText[200];
	BYTE	n;
	dlRGB	bCor;
	BYTE	iVar;
	DWORD	dwBytes;
	WORD	wTamX,wTamY;
	BYTE	bSize,bBold;
	char	FontName[50];

	// Obtem o Texto e grava
	GetPrivateProfileString( "DL1", "TEXTO", "?", cText, 200, cFileINI );
	n=strlen( cText );
	WriteFile( DL1, &n, sizeof(n), &dwBytes, NULL );
	WriteFile( DL1, cText, n, &dwBytes, NULL );

	// Configura fundo e transparencia ou cor de fundo se diferente de 0,0,0
	bCor.R=GetPrivateProfileInt( "DL1", "FundoR", 0, cFileINI );
	bCor.G=GetPrivateProfileInt( "DL1", "FundoG", 0, cFileINI );
	bCor.B=GetPrivateProfileInt( "DL1", "FundoB", 0, cFileINI );
	WriteFile( DL1, &bCor, sizeof(bCor), &dwBytes, NULL );

	// Configura a cor das letras
	bCor.R=GetPrivateProfileInt( "DL1", "CorR", 0, cFileINI );
	bCor.G=GetPrivateProfileInt( "DL1", "CorG", 0, cFileINI );
	bCor.B=GetPrivateProfileInt( "DL1", "CorB", 0, cFileINI );
	WriteFile( DL1, &bCor, sizeof(bCor), &dwBytes, NULL );

	// Obtem e grava os tamnho máximo do boloco de exibição do texto
	wTamX=GetPrivateProfileInt( "DL1", "TamX", 300, cFileINI );
	wTamY=GetPrivateProfileInt( "DL1", "TamY", 20, cFileINI );
	WriteFile( DL1, &wTamX, sizeof(wTamX), &dwBytes, NULL );
	WriteFile( DL1, &wTamY, sizeof(wTamY), &dwBytes, NULL );

	// Grava as variáveis usadas - // 1
	iVar=GetPrivateProfileInt( "DL1", "Var1", 0, cFileINI );
	WriteFile( DL1, &iVar, sizeof(iVar), &dwBytes, NULL );
			// 2
	iVar=GetPrivateProfileInt( "DL1", "Var2", 0, cFileINI );
	WriteFile( DL1, &iVar, sizeof(iVar), &dwBytes, NULL );
			//3
	iVar=GetPrivateProfileInt( "DL1", "Var3", 0, cFileINI );
	WriteFile( DL1, &iVar, sizeof(iVar), &dwBytes, NULL );

	// Grava o tamnho da fonte
	bSize=GetPrivateProfileInt( "DL1", "Size", 10, cFileINI );
	WriteFile( DL1, &bSize, sizeof(bSize), &dwBytes, NULL );

	bBold=GetPrivateProfileInt( "DL1", "Bold", 0, cFileINI );
	WriteFile( DL1, &bBold, sizeof(bBold), &dwBytes, NULL );

	GetPrivateProfileString( "DL1", "Font", "Arial", FontName, 50, cFileINI );
	n=strlen( FontName );
	WriteFile( DL1, &n, sizeof(n), &dwBytes, NULL );
	WriteFile( DL1, FontName, n, &dwBytes, NULL );
}

// -------------------------------------------------------------

void LoadInfoSegmento(){
	BYTE	i, nSeg, n;
	BYTE	bFrameIni, bFrameFim;
	DWORD	dwBytes;
	char cName[20], cSeg[10];

	nSeg=GetPrivateProfileInt( "DL1", "Segmentos", 0, cFileINI );
	WriteFile( DL1, &nSeg, sizeof(nSeg), &dwBytes, NULL );

	for( i=1; i<=nSeg; i++ ){

		//Obtem as informações do segmento
		sprintf( cSeg, "SEG%d", i );
		GetPrivateProfileString(cSeg, "NAME", "X", cName, 20, cFileINI );
		bFrameIni=(BYTE) GetPrivateProfileInt( cSeg, "FrameIni", 0, cFileINI );
		bFrameFim=(BYTE) GetPrivateProfileInt( cSeg, "FrameFim", 0, cFileINI );
		n=strlen(cName);

		// Grava as informações do segmento
		WriteFile( DL1, &n, sizeof(n), &dwBytes, NULL );
		WriteFile( DL1, cName, n, &dwBytes, NULL );
		WriteFile( DL1, &bFrameIni, sizeof(bFrameIni), &dwBytes, NULL );
		WriteFile( DL1, &bFrameFim, sizeof(bFrameFim), &dwBytes, NULL );
	}

}

// -------------------------------------------------------------

void LoadInfoTrajetoria(){
	DWORD	dwBytes;
	int		i,v1,v2,v3;
	char	cVar[5],cValor[20];
	TrajPos	Pos;

	for( i=1; i<30; i++ ){	// maximo 30 posições

		//Obtem as informações do ponto
		sprintf( cVar, "P%d", i );
		GetPrivateProfileString( "TRAJETORIA", cVar, "-,-,-", cValor, 20, cFileINI );
		if( cValor[0]=='-' ){
			Pos.Repete=0;
			Pos.incX=0;
			Pos.incY=0;
			break;
			}

		LoadIntString( cValor, &v1, &v2, &v3 );
		Pos.Repete=v1;
		Pos.incX=v2;
		Pos.incY=v3;

		if( Pos.Repete )
			WriteFile( DL1, &Pos, sizeof(Pos), &dwBytes, NULL );
	}

	WriteFile( DL1, &Pos, sizeof(Pos), &dwBytes, NULL );
}


// -------------------------------------------------------------

void LoadInfoPosicoes(){
	DWORD			dwBytes;
	int				i,v1,v2,v3;
	char			cVar[5],cValor[20];
	QuadStart		Start;
	QuadStartPos	StartPos;

	// Posições iniciais repitidas
	for( i=1; i<255; i++ ){

		//Obtem as informações do segmento
		sprintf( cVar, "Q%d", i );
		GetPrivateProfileString( "START", cVar, "-,-", cValor, 20, cFileINI );
		if( cValor[0]!='-' ){

			LoadIntString( cValor, &v1, &v2, &v3 );
			Start.Quadrante=i;
			Start.Quantidade=v1;
			Start.TickNext=v2;

			WriteFile( DL1, &Start, sizeof(Start), &dwBytes, NULL );

		}
	}

	Start.Quadrante=0xFF;
	WriteFile( DL1, &Start, sizeof(Start), &dwBytes, NULL );

	//Posições iniciais específicas
	for( i=1; i<30; i++ ){

		//Obtem as informações do segmento
		sprintf( cVar, "P%d", i );
		GetPrivateProfileString( "START", cVar, "-,-", cValor, 20, cFileINI );
		if( cValor[0]=='-' )
			break;

		LoadIntString( cValor, &v1, &v2, &v3 );
		StartPos.Quadrante=v1;
		StartPos.PosX=v2;
		StartPos.PosY=v3;

		WriteFile( DL1, &StartPos, sizeof(StartPos), &dwBytes, NULL );

	}

	StartPos.Quadrante=0xFF;
	WriteFile( DL1, &StartPos, sizeof(StartPos), &dwBytes, NULL );

}

void LoadInfoBomba(){
	DWORD	dwBytes;
	BYTE	ID;
	WORD	DisparoX,DisparoY;
	BYTE	Erro;

	// ID
	ID=(BYTE) GetPrivateProfileInt( "BOMBA", "ID", 1, cFileINI );
	WriteFile( DL1, &ID, sizeof(ID), &dwBytes, NULL );

	// DisparoX, DisparoY
	ID=(BYTE) GetPrivateProfileInt( "BOMBA", "DisparoX", 1, cFileINI );
	WriteFile( DL1, &DisparoX, sizeof(DisparoX), &dwBytes, NULL );
	ID=(BYTE) GetPrivateProfileInt( "BOMBA", "DisparoY", 1, cFileINI );
	WriteFile( DL1, &DisparoY, sizeof(DisparoY), &dwBytes, NULL );

	// %Erro
	ID=(BYTE) GetPrivateProfileInt( "BOMBA", "ERRO", 30, cFileINI );
	WriteFile( DL1, &Erro, sizeof(Erro), &dwBytes, NULL );

}

// -------------------------------------------------------------

void LoadIntString( char * cStr, int * iVar1, int * iVar2, int * iVar3 ){
	int i=0, n10=1,n,dig,nVar;
	char cBt,cBt2;
	int iVar[3];
	nVar=0;
	iVar[0]=0;
	iVar[1]=0;
	iVar[2]=0;
	do{
		cBt=*(cStr+i);
		if( cBt==',' || cBt==0 ){
			n=1;
			n10=1;
			do{
				if( i-n<0 )
					break;
				cBt2=*(cStr+i-n);
				if( cBt2=='-' ){
					iVar[nVar]*=-1;
					break;
				}
				else if( cBt2==',' )
					break;
				dig=cBt2-'0';
				dig*=n10;
				n10*=10;
				iVar[nVar]+=dig;
				n++;
			}while( n<6 );		// 5 digitos no máximo
			nVar++;
		}
		i++;
	}while( cBt && nVar<3 );

	*iVar1=iVar[0];
	*iVar2=iVar[1];
	*iVar3=iVar[2];
}
