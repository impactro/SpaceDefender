// =============================================================================
// SpaceDefender.CPP
// Versão 1.0 - 13/12/2002
// Programa: Fábio Ferreira de Souza
// Desenhos: Marcio Batista
// =============================================================================

#include "..\LIB1.7\dLib.H"
#include "Resource.H"
#include "SpaceDefender.H"

// Diretório dos arquivos DL1
char	cDL1[200];

// Controle das Fases;
BYTE	bFase;
BYTE	bObj;

char *	objFase[]={ "PlanoQ0.DL1",
					"PlanoQ1.DL1",
					"Placar.DL1",
					"Fire1.DL1",
					"Fire2.DL1",
					"Fire3.DL1",
					"Bomba1.DL1",
					"Pontos.DL1",
					"Energia.DL1",
					"Vidas.DL1",
					"Inimigo1.DL1",
					"Inimigo2.DL1" };

dObjSurface	oNave;

BOOL	Loading;

// INFORMAÇÕES DO PLAYER
int		wPosX=100;
int		wPosY=100;
WORD	nSeg=0;

DWORD	bINCYUP=0;
DWORD	bINCYDN=0;
DWORD	bINCXRV=0;
DWORD	bINCXFF=0;

// ===============================================================================
int APIENTRY WinMain( HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR pCmdLine, int nCmdShow )
{
	HWND	hWnd;
	MSG		msg;

    if( FAILED( WinInit( hInst, nCmdShow, &hWnd ) ) )
        return FALSE;

	// Configura Biblioteca (dDraw)
	StartConfig( hInst );

	// Inicializa DirectDraw
	dStart( hWnd );

	// Prepara para carregar a fase
	dSetBasic( true );
	dLoadDL( "Fase1.DL1" );
	dLoadDL( "Loading.DL1" );
	bFase=0;
	bObj=0;
	Loading=true;

	// Inicio do MainLOOP
	while( TRUE ){
		dRedrall();
		if( Loading ){
			LoadFase();
		}

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

	//Finaliza Módulos dDraw

	dEnd();

	return S_OK;
}



// ===============================================================================
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

    ShowWindow( hWnd, nCmdShow );
    UpdateWindow( hWnd );

    // Save the window size/pos for switching modes

	*phWnd   = hWnd;

    return S_OK;

}



// ===============================================================================
void StartConfig(HINSTANCE hInst){

	// Obtem o Diretórito da executável
	char	cPath[200];
	char	cINI[200];
	BOOL	lFullScreen;
	BYTE	n;

	GetModuleFileName( hInst, cPath, 200 );
	n=strlen(cPath);
	do{
		n--;
	}while( n>0 && cPath[n]!='\\' );
	cPath[n+1]=0;

	sprintf( cINI, "%s%s", cPath, "SpaceDefender.INI" );
	lFullScreen=GetPrivateProfileInt( "SpaceDefender", "FULLSCREEN", 0, cINI );
	GetPrivateProfileString( "SpaceDefender", "DL1", cPath, cDL1, 200, cINI );

	//Configura e Inicializa Modulos
	dConfigure( cDL1, 640, 480, lFullScreen );
	dResetLoadObj( 40 );
	dSetIntVarPublic( 50, &bINCXRV );
	dSetIntVarPublic( 51, &bINCXFF );

}



// ===============================================================================
LRESULT CALLBACK MainWndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam ){
    switch (msg)
    {

		case WM_KEYDOWN:
			switch( wParam ){
			case VK_ESCAPE:
				PostMessage( hWnd, WM_CLOSE, 0, 0 );
				break;
			case VK_UP:
/*
				if(bINCYUP<15)bINCYUP++;
				if(bINCYDN)bINCYDN--;
				wPosY-=(WORD) bINCYUP;
*/
				if(wPosX>300)nSeg=1;
				else nSeg=0;

				wPosY-=15;
				if(wPosY<20)wPosY=20;

				break;
			case VK_DOWN:
/*
				if(bINCYDN<15)bINCYDN++;
				if(bINCYUP)bINCYUP--;
				wPosY+=(WORD) bINCYDN;
*/
				if(wPosX>300)nSeg=3;
				else nSeg=2;

				wPosY+=15;
				if(wPosY>400)wPosY=400;
				break;
			case VK_LEFT:
/*
				if(bINCXRV<15)bINCXRV++;
				if(bINCXFF)bINCXFF--;
				wPosX-=(WORD)bINCXRV;
*/
				if(wPosY>400)nSeg=2;
				else nSeg=0;

				wPosX-=15;
				if(wPosX<20)wPosX=20;

				break;
			case VK_RIGHT:
/*
				if(bINCXFF<15)bINCXFF++;
				if(bINCXRV)bINCXRV--;
				wPosX+=(WORD)bINCXFF;
*/				
				if(wPosY>400)nSeg=3;
				else nSeg=1;

				wPosX+=15;
				if(wPosX>550)wPosX=550;

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



// ===============================================================================

void LoadFase(){
	WORD wObj,wExplo;
	dLoadDL( objFase[bObj] );
	bObj++;
	int nMax=(sizeof(objFase)/4)-1;
	if( bObj>nMax ){
		wObj=dLoadDL( "Nave.DL1" );
		wExplo=dLoadDL( "Explosao.DL1" );
		dSetPlayer( wObj, &wPosX, &wPosY, &nSeg, wExplo );
		Loading=false;
		dSetBasic( false );
	}
	
}

void GameOver(){
	ClearPlan( 7 );
	dSetBasic( true );
	dLoadDL( "GameOver.DL1" );
}
