//
//
//  HEADER - Create DL1 
//  Estruturas bases para a geração da DL1.H
//
//

#define	PROGRAMNAME	"CresteDL1"

// Rotinas básicas
HRESULT WinInit( HINSTANCE hInst, int nCmdShow, HWND* phWnd );
LRESULT CALLBACK MainWndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );

// DL1
void LoadInfoBMP( BYTE nBmp );
void LoadInfoSegmento();
void LoadInfoTrajetoria();
void LoadInfoPosicoes();
void LoadInfoBomba();
void LoadInfoTEXT();
void LoadIntString( char * cStr, int * iVar1, int * iVar2, int * iVar3 );
