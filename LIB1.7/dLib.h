//+---------------------------------------------------------------------------
//
//  IMPACTRO Informática
//  Fábio Ferreira de Souza - 25/11/2002
//
//  dLib.h - Controle dos recursos do Direct Draw (DirectX8)
//  Versão 1.5
//
//----------------------------------------------------------------------------

#include <windows.h>
#include <mmsystem.h>
#include <ddraw.h>
#include <ddutil.h>
#include <stdio.h>
#include "dl1.h"

// Limites e valores padrões

#define MAX_SURFACE			200
#define	MAX_PLANO			7
#define MAX_TIRO			10
#define MAX_BOMBA			10
#define	MAX_TIROSDISPARO	10
#define	MAX_BOMBADISPARO	50

#define SCREEN_WIDTH		640
#define SCREEN_HEIGHT		480
#define SCREEN_BPP			32

#define	TP_OBJETO			0
#define	TP_PLAYER			1
#define	TP_INIMIGO			2

// Macros

#define SAFE_DELETE(p)  { if(p) { delete (p);     (p)=NULL; } }
#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=NULL; } }

/* Definições das funções */

// ===================
// Estrutura de Objetos nas Surfaces

struct dObjCopyQ{
	WORD		Sprite;
	BYTE		Quadrante;
	BYTE		Quantidade;
	WORD		Tick;
	WORD		TickWait;
	WORD		PosX;
	WORD		PosY;
};

// Telas - Statica ou Quadrante
struct	dBackObj{
	WORD		Srf;
//	BYTE		Plano;	- Sem utilidade: posição 0 -> Plano 0, posição 1 é plano 1....
	BOOL		Ativo;
	WORD		TamX;
	WORD		TamY;
	BYTE		Cols;
	BYTE		Rows;
	WORD		PosX;
	WORD		PosY;
	int			IncX;
	int			IncY;
	BOOL		Loop;
	DWORD		FrameTick;
	DWORD		FrameTickWait;
	BYTE		oObjMax;
	dObjCopyQ	oObj[50];
};

// Sprites
struct	dSegFrame{
	char		Name[20];
	BYTE		FrameIni;
	BYTE		FrameFim;
};

struct	dTiroON{
	BOOL		Ativo;
	int			PosX;
	int			PosY;
	BYTE		FrameCurrent;
	DWORD		FrameTick;
};

struct	dBombaON{
	BOOL		Ativo;
	int			PosX;
	int			PosY;
	int			dx;
	int			dy;
	int			dis;
	int			erx;
	int			ery;
	int			vx;
	int			vy;
	BYTE		FrameCurrent;
	DWORD		FrameTick;
};

struct	dObjBomba{	// o ID representa fisicamente a posição de tiro no vetor
	WORD		Srf;
	BOOL		Load;
	WORD		Delay;
	BYTE		Cols;
	BYTE		Rows;
	WORD		TamX;
	WORD		TamY;
	DWORD		FrameTickWait;
	BYTE		Power;
	BYTE		vMaxX;
	BYTE		vMaxY;
	dBombaON	Bomba[MAX_BOMBADISPARO];
};

struct	dObjTiro{	// o ID representa fisicamente a posição de tiro no vetor
	WORD		Srf;
	BOOL		Load;
	WORD		DisparoX;
	WORD		DisparoY;
	WORD		Delay;
	WORD		DelayCur;
	BYTE		Cols;
	BYTE		Rows;
	WORD		TamX;
	WORD		TamY;
	DWORD		FrameTickWait;
	BYTE		Power;
	char		vx;
	char		vy;
	dTiroON		ON[MAX_TIROSDISPARO];
};

struct	dObjSurface{
	WORD		Srf;
	BOOL		Ativo;
	BYTE		Plano;
	int			PosX;
	int			PosY;
	BYTE		Cols;
	BYTE		Rows;
	WORD		FrameCurrent;
	DWORD		FrameTick;
	DWORD		FrameTickWait;
	WORD		TamX;
	WORD		TamY;
	BYTE		Segmentos;
	BYTE		SegCur;
	dSegFrame	Seg[20];
	BYTE		TrajPosT;
	BYTE		TrajPosCur;
	TrajPos		Trajetoria[30];
	BYTE		TextID;
	HFONT		hFont;
	DWORD		rgbFore;
	DWORD		rgbBack;
	BYTE		Tipo;
	int			Energia;
	WORD		Power;
	WORD		Pontos;
	BYTE		iVar;
	BYTE		Visible;
	BYTE		VisibleCount;
	BYTE		BombaID;
	WORD		DisparoX;
	WORD		DisparoY;
	BYTE		Erro;
	WORD		DelayCur;
};

// Controle
void dEnd();
void dConfigure( TCHAR *cDefPath, WORD Width, WORD Height, BOOL lFull );
HRESULT dStart( HWND hWindow );
void dRedrall();
void dUpdate();
void dPlay();
void dStop();
void dSetBasic( BOOL lSetBasic );
void dResetLoadObj( DWORD nObj );
void ClearPlan( WORD nPaln );

// Ajustes e exibição
void dGetObjRectFrame( int iObject, RECT * Rect, DWORD nDiffTick );
void dAdjustRect( int * PosX, int * PosY, RECT * Rect, WORD nTamX, WORD nTamY );
void dDisplayQuadrante( BYTE nPl, DWORD nDiffTick );
void dLoadQuadrantObject( BYTE nPlan, BYTE Quad );
void dRunObject( WORD wObj );
void dTiroNew();
void dRunTiro( BYTE bTiroCurrent, BYTE bTiro, RECT * Rect );
void dRunBomba( BYTE bBombaCurrent, BYTE bbomba, RECT * Rect );

// Carrega Arquivo DL1
int dLoadDL( char * cFileDL1 );
int dLoadImage( HANDLE dlFile, WORD * wTamXout, WORD * wTamYout, BYTE nBmpMax, WORD nQuadros );
void dLoadSegmento( HANDLE dlFile, int iObj );
void dLoadTrajetoria( HANDLE dlFile, int iObj );
void dLoadPosicoes( HANDLE dlFile, int iObj );
void dLoadBomba( HANDLE dlFile, int iObj );
void dLoadText( HANDLE dlFile, int iObj );

// Rotinas úteis
void dFillSurface( WORD nSrf, DWORD rgbColor );
void dGetObject( WORD iObj, dObjSurface *oObj );
void dSetPlayer( WORD wObj, int * wPosX, int * wPosY, WORD * nSeg, WORD wObjEsplo );
void dCheckColisao( WORD wObj );
void dExplodeObj( WORD iObj );
void ValidaPlayer();

// Rotina que está no Main (SpaceDefender.CPP)
void GameOver();
