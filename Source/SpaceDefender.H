/*
SpaceDefender.H
*/

#define	PROGRAMNAME	"SpaceDefender"

HRESULT WinInit( HINSTANCE hInst, int nCmdShow, HWND* phWnd );
LRESULT CALLBACK MainWndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
void StartConfig(HINSTANCE hInst);
void LoadFase();