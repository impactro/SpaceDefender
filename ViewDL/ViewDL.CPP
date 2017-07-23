//============================================================================
//
//  IMPACTRO Informática
//  Fábio Ferreira de Souza - 05/12/2002
//
//  Visualizador de arquivos DL1
//
//============================================================================

#include "..\Lib1.7\dLib.h"
#include "ViewDL.h"
#include "resource.h"

WORD		MouseX=0;
WORD		MouseY=0;

// ---------------------------------------------------------
// WinMain -> Inicio do programa
// (nenhum paramentro de linha de comando é tratado diretamente)
//

int nObjeto;

int APIENTRY WinMain( HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR pCmdLine, int nCmdShow )
{
	HWND hWnd;
	MSG	msg;
	char cPath[200];
	char cLoad[200];
	char cName[20];
	char cViewDLINI[200];
	int n, nLoad;
	
    if( FAILED( WinInit( hInst, nCmdShow, &hWnd ) ) )
        return FALSE;


	// Inicializa a biblioteca gráfica
	dStart( hWnd );

	// Obtem o Diretório do executável
	GetModuleFileName( hInst, cPath, 200 );
	n=strlen(cPath);
	do{
		n--;
	}while( n>0 && cPath[n]!='\\' );
	cPath[n+1]=0;

	dConfigure( cPath, 640, 480, false );

	// Define o nome do arquivo .INI
	sprintf( cViewDLINI, "%s%s", cPath, "ViewDL.INI" );
	nLoad=1;

	// Carrega os objetos padrões
	do{
		sprintf( cName, "LOAD%d", nLoad );
		GetPrivateProfileString( "ViewDL", cName, "", cLoad, 200, cViewDLINI );
		n=strlen( cLoad );
		if( n>0 )
			dLoadDL( cLoad );
		nLoad++;
	}while(n>0);

	n=GetPrivateProfileInt( "ViewDL", "BASIC", 0, cViewDLINI );
	dSetBasic( n );

	// Carrega o objeto passado por linha de comando
	n=strlen(pCmdLine);
	if( n>0 )
		nObjeto=dLoadDL( pCmdLine );

	// Inicio do MainLOOP
	while( TRUE ){
		dRedrall();

        if( PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE ) )
        {
            if( 0 == GetMessage(&msg, NULL, 0, 0 ) )
            {
                // WM_QUIT was posted, so exit
				return (int)msg.wParam;
            }
            else 
			{
                TranslateMessage( &msg ); 
                DispatchMessage( &msg );
            }

        }
    }

	//Finaliza Módulos

	dEnd();

	return S_OK;
}


//-----------------------------------------------------------------------------
// Name: WinInit()
// Desc: Init the window
//-----------------------------------------------------------------------------
HRESULT WinInit( HINSTANCE hInst, int nCmdShow, HWND* phWnd )
{
    WNDCLASSEX	wc;
	HWND		hWnd;

    // Register the window class
    wc.cbSize        = sizeof(wc);
    wc.lpszClassName = PROGRAMNAME;
    wc.lpfnWndProc   = MainWndProc;
    wc.style         = CS_VREDRAW | CS_HREDRAW;
    wc.hInstance     = hInst;
    wc.hIcon         = LoadIcon( hInst, MAKEINTRESOURCE(IDI_MAIN) );
    wc.hIconSm       = LoadIcon( hInst, MAKEINTRESOURCE(IDI_MAIN) );
    wc.hCursor       = LoadCursor( NULL, IDC_ARROW );
    wc.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
    wc.lpszMenuName  = NULL;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;

    if( RegisterClassEx( &wc ) == 0 )
        return E_FAIL;

    // Create and show the main window
    DWORD dwStyle = WS_OVERLAPPED		| \
                    WS_CAPTION			| \
                    WS_SYSMENU			| \
                    WS_SIZEBOX			| \
                    WS_MINIMIZEBOX		| \
					WS_MAXIMIZEBOX		;
    hWnd = CreateWindowEx( 0, PROGRAMNAME, PROGRAMNAME,
                           dwStyle, CW_USEDEFAULT, CW_USEDEFAULT,
  	                       NULL, NULL, NULL, NULL, hInst, NULL );
    if( hWnd == NULL )
    	return E_FAIL;

    UpdateWindow( hWnd );
	ShowWindow( hWnd, nCmdShow );

	*phWnd   = hWnd;

    return S_OK;

}

// ----------------------------------------------
//
// User Interface
//
LRESULT CALLBACK MainWndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    switch (msg)
    {
		case WM_MOUSEMOVE:
			MouseX=(WORD) ( 0xFFFF & lParam );
			MouseY=(WORD) ( 0xFFFF & ( lParam>>16) );
			break;

		case WM_KEYDOWN:
			switch( wParam ){
			case VK_ESCAPE:		// Sai do programa
				PostMessage( hWnd, WM_CLOSE, 0, 0 );
				break;

			}
			break;

		case WM_MOVE:
			dUpdate();
			break;

		case WM_SETFOCUS:
			dUpdate();
			break;

		case WM_NCPAINT:
			dUpdate();
			break;

        case WM_DESTROY:
            PostQuitMessage( 0 );
            return 0L;
			break;

	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}
