//+---------------------------------------------------------------------------
//
//  IMPACTRO Informática
//  Fábio Ferreira de Souza - 5/12/2002
//
//  dLib.cpp - Controle dos recursos do Direct Draw (DirectX8)
//  Versão 1.7
//
//----------------------------------------------------------------------------

#include "dLib.h"

HWND		hWind;

DWORD		ScreenWidth=SCREEN_WIDTH;
DWORD		ScreenHeight=SCREEN_HEIGHT;
BOOL		lFullScreen=false;		// Indica se é tela cheia ou Janela
BOOL		lAtivo=false;			// Indica se as Rotinas do DirectDraw estão ativas
BOOL		lProcess=true;			// Habilita o processamento das rotinas de animação

DWORD		dLastTick=0;
DWORD		dMinWaitTick=1000;
DWORD		dDiffTick=1000;
DWORD		nIDTick=123;
DWORD		dwCont=0;

BOOL		lBasic=false;

// Váriáveis para controle das 'Surfaces'
CDisplay*		oDisplay=NULL;
CSurface*		oSurface[MAX_SURFACE];
WORD			wLastSurface=0;

// Telas
dBackObj		dTela[MAX_PLANO+1];	// Telas não possuem contador da ultima tela...
									// Cada plano teoricamente é a sequencia das telas!
									// A ultima tela extra é a tela que ficará por cima de todas
									// enquanto a fase está sendo carregada.

// Objetos
dObjSurface		dObject[MAX_SURFACE];
WORD			wLastdObject=0;

dObjTiro		dTiro[MAX_TIRO];			// Tiros pertencentes a nave (Player)
dObjBomba		dBomba[MAX_BOMBA];			// Bombas pertencentes as naves inimigas

// Diretórtio padrão para os arquivos DL1
TCHAR *			cDefaultPath="";

// Variáveis para depuração do View (_TESTE)
#ifdef _TESTE
	DWORD		nColorFundo=0;
#endif

// Contadores de Objetos para carga
DWORD	nObjLoad=0;
DWORD	nObjTotalLoad=1;
DWORD	nObjPerLoad=0;
DWORD	PlayerKill=0;
DWORD	PlayerWait=0;
DWORD	PlayerPontos=0;
DWORD	PlayerPontLife=500;	// 1 Vida a cada 500 pontos
DWORD	PlayerEnergia=8;
DWORD	PlayerVidas=3;


// Definições para o Plyer 1
WORD ExplosaoObj=0xff;
WORD EnergiaObj=0xff;

WORD PlayerObj=0xff;
int * PlayerPosX;
int * PlayerPosY;
WORD * PlayerSeg;

void CALLBACK TimerProc( HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime ); 

void CALLBACK TimerProc( HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime ){
	dwCont++;
}

UINT  nIDEvent=1;

// ------------------------------------------------
void dEnd(){
	int i;

	for( i=0; i<MAX_SURFACE; i++ )
		SAFE_DELETE( oSurface[i] );

    SAFE_DELETE( oDisplay );

	lAtivo=false;

	dEndDL();

}



// ------------------------------------------------
void dConfigure( TCHAR *cDefPath, WORD Width, WORD Height, BOOL lFull ){
	ScreenWidth=Width;
	ScreenHeight=Height;
	lFullScreen=lFull;
	cDefaultPath=cDefPath;
}



// ------------------------------------------------
HRESULT dStart( HWND hWindow ){
	HRESULT hr;
	int i;
	hWind=hWindow;

	srand( GetTickCount() );

	dStartDL();

	SetTimer( hWindow, nIDEvent, 1, TimerProc );

	for( i=0; i<MAX_SURFACE; i++ )
		dObject[i].Ativo=false;

	for( i=0; i<MAX_PLANO; i++ )
		dTela[i].Ativo=false;

	for( i=0; i<MAX_TIRO; i++ )
		dTiro[i].Load=false;

	for( i=0; i<MAX_BOMBA; i++ )
		dBomba[i].Load=false;

	oDisplay = new CDisplay();

	if( lFullScreen )
		hr=oDisplay->CreateFullScreenDisplay(hWind, ScreenWidth, ScreenHeight, SCREEN_BPP);
	else
		hr=oDisplay->CreateWindowedDisplay(hWind, ScreenWidth, ScreenHeight);

    if( FAILED( hr ) )
		return hr;

	lAtivo=true;

	// Variáveis padrão 
	
	// TICK
	dLastTick=timeGetTime();
	dSetIntVarPublic( IVAR_LASTTICK, &dLastTick );
	dSetIntVarPublic( IVAR_MINTICK, &dMinWaitTick );
	dSetIntVarPublic( IVAR_DIFFTICK, &dDiffTick );
	dSetCharVarPublic( CVAR_TICK, "(TICK) Last: %06d Mim: %06d Diff: %06d ",
					   IVAR_LASTTICK, IVAR_MINTICK, IVAR_DIFFTICK );

	// WIDTH
	dSetIntVarPublic( IVAR_SCREENWIDTH, &ScreenWidth );
	dSetIntVarPublic( IVAR_SCREENHEIGHT, &ScreenHeight );
	dSetCharVarPublic( CVAR_SCREEN, "(SCREEN) %03d x %03d", IVAR_SCREENWIDTH, IVAR_SCREENHEIGHT, 0 );

	// Variáveis de carga de objetos
	dSetIntVarPublic( IVAR_OBJLOAD, &nObjLoad );
	dSetIntVarPublic( IVAR_OBJMAXLOAD, &nObjTotalLoad );
	dSetIntVarPublic( IVAR_OBJPERLOAD, &nObjPerLoad );
	dSetCharVarPublic( CVAR_LOADING, "%d de %d (%s)", IVAR_OBJLOAD, IVAR_OBJMAXLOAD, IVAR_OBJPERLOAD );

	dSetIntVarPublic( IVAR_PONTOS, &PlayerPontos);
	dSetIntVarPublic( IVAR_KILL, &PlayerKill);
	dSetIntVarPublic( IVAR_ENERGIA, &PlayerEnergia );
	dSetIntVarPublic( IVAR_VIDAS, &PlayerVidas );

	dSetIntVarPublic( 20, &dwCont );

	return S_OK;

}



// ------------------------------------------------
void dRedrall(){
	if( !lAtivo )
		return;

	DWORD dCurTime=timeGetTime();
	dDiffTick=dCurTime-dLastTick;

	if( dMinWaitTick>dDiffTick )
		return;

	dLastTick=dCurTime;

#ifdef _TESTE
	oDisplay->Clear(nColorFundo++);
	nObjLoad++;
	nObjPerLoad=(100*nObjLoad)/nObjTotalLoad;
#else
	oDisplay->Clear(0);
#endif

	BYTE iPlan=0,iPlanMax=MAX_PLANO;
	BYTE bTiroCurrent, bBombaCurrent;
	WORD i;
	RECT rc;
	char cVar[200];

	int iPosX, iPosY;

	if( lBasic ){
		iPlan=MAX_PLANO;
		iPlanMax=MAX_PLANO+1;
	}
	else{
		iPlan=0;
		iPlanMax=MAX_PLANO;
		
		if( PlayerObj>0 ){
#ifdef _TESTE
			dTiroNew();
#else
			if( dObject[PlayerObj].Ativo ){
				dTiroNew();
				dObject[PlayerObj].PosX=*PlayerPosX;
				dObject[PlayerObj].PosY=*PlayerPosY;
				dObject[PlayerObj].SegCur=(BYTE)*PlayerSeg;
			}
			ValidaPlayer();

#endif
		}

	}

	for(; iPlan<iPlanMax; iPlan++ ){
		if( dTela[iPlan].Ativo ){

			// Coloca o plano defundo Statico/Quadrante
			if( dTela[iPlan].Cols==1 && dTela[iPlan].Rows==1 )
				oDisplay->Blt( 0, 0, oSurface[ dTela[iPlan].Srf ] );
			else
				dDisplayQuadrante( iPlan, dDiffTick );

			// Coloca os Tiros do Player na tela
			for( bTiroCurrent=1; bTiroCurrent<MAX_TIRO; bTiroCurrent++ ){
				if( dTiro[bTiroCurrent].Load ){
					for( i=0; i<MAX_TIROSDISPARO; i++ ){
						dRunTiro( bTiroCurrent, (BYTE) i, &rc );
						if( dTiro[bTiroCurrent].ON[i].Ativo ){
							iPosX=dTiro[bTiroCurrent].ON[i].PosX;
							iPosY=dTiro[bTiroCurrent].ON[i].PosY;

							// Corta a imagem se necessário
							dAdjustRect( &iPosX, &iPosY, &rc, dTiro[bTiroCurrent].TamX, dTiro[bTiroCurrent].TamY );
							oDisplay->Blt( iPosX, iPosY, oSurface[ dTiro[bTiroCurrent].Srf ], &rc );

						}
					}
				}
			}

			// Coloca os Objetos da tela
			for( i=0; i<wLastdObject; i++ ){
				if( dObject[i].Ativo && dObject[i].Plano==iPlan ){

					dRunObject(i);
#ifndef _TESTE
					dCheckColisao(i);
#endif
					WORD lDisplay=true;

					if( dObject[i].Visible ){
						lDisplay=false;
						if( dObject[i].Visible<(++dObject[i].VisibleCount) ){
							dObject[i].VisibleCount=0;
							lDisplay=true;
							}
						}

					if( lDisplay ){
						dGetObjRectFrame( i, &rc, dDiffTick );

						if( dObject[i].TextID>0 ){
							dCompileCharVar( cVar, dObject[i].TextID );
							oSurface[ dObject[i].Srf ]->DrawText( dObject[i].hFont, cVar, 0, 0, dObject[i].rgbBack, dObject[i].rgbFore );
						}

						iPosX=dObject[i].PosX;
						iPosY=dObject[i].PosY;

						dAdjustRect( &iPosX, &iPosY, &rc, dObject[i].TamX, dObject[i].TamY );
						oDisplay->Blt( iPosX, iPosY, oSurface[ dObject[i].Srf ], &rc );
					}
					
				}
			}

			// Coloca as Bombas inimigas na tela
			for( bBombaCurrent=1; bBombaCurrent<MAX_BOMBA; bBombaCurrent++ ){
				if( dBomba[bBombaCurrent].Load ){
					for( i=0; i<MAX_BOMBADISPARO; i++ ){
						dRunBomba( bBombaCurrent, (BYTE) i, &rc );
						if( dBomba[bBombaCurrent].Bomba[i].Ativo ){
							iPosX=dBomba[bBombaCurrent].Bomba[i].PosX;
							iPosY=dBomba[bBombaCurrent].Bomba[i].PosY;

							// Corta a imagem se necessário
							dAdjustRect( &iPosX, &iPosY, &rc, dBomba[bBombaCurrent].TamX, dBomba[bBombaCurrent].TamY );
							oDisplay->Blt( iPosX, iPosY, oSurface[ dBomba[bBombaCurrent].Srf ], &rc );

						}
					}
				}
			}

			// Fim do plano

		}
	}


    oDisplay->Present();

	dLastTick=timeGetTime();
 
}


// ------------------------------------------------
void dUpdate(){
	if( lAtivo ){
		oDisplay->UpdateBounds();
	}
}


// ------------------------------------------------
void dPlay(){
	lProcess = true;
}



// ------------------------------------------------
void dStop(){
	lProcess = false;
}



// ------------------------------------------------
void dSetBasic( BOOL lSetBasic ){
	lBasic = lSetBasic;
}



// ------------------------------------------------
void dResetLoadObj( DWORD nObj ){
	nObjLoad=0;
	nObjTotalLoad=nObj;
	nObjPerLoad=0;
}

void ClearPlan( WORD nPaln ){
	WORD i;
	for( i=0; i<wLastdObject; i++ )
		dObject[i].Ativo=false;
}

// ------------------------------------------------

void ValidaPlayer(){
	
	// Conta pontos para ganhar vida!
	if(PlayerPontos>PlayerPontLife ){
		PlayerPontLife+=PlayerPontLife;
		PlayerVidas++;
	}

	if( dObject[PlayerObj].Ativo ){
		// Transparencia (invencibilidade)
		if( dObject[PlayerObj].Visible ){
			PlayerWait+=dDiffTick;
			if( PlayerWait>1500 )
				dObject[PlayerObj].Visible=0;
		}
		// Verifica se acabou a energia
		else if( PlayerEnergia==0 || PlayerEnergia>8 ){
			PlayerVidas--;
			WORD iNew=wLastdObject;
			dObject[iNew]=dObject[PlayerObj];
			dObject[PlayerObj].Ativo=false;
			dExplodeObj( iNew );
			wLastdObject++;
			PlayerWait=0;
		}
	}
	else {
		PlayerWait+=dDiffTick;
		if( PlayerWait>1500 ){
#ifndef _TESTE
			if( PlayerVidas>999 )
				GameOver();
			else {
				dObject[PlayerObj].Ativo=true;
				dObject[PlayerObj].Visible=2;
				PlayerEnergia=8;
				PlayerWait=0;
			}
#endif
		}
	}
}

// ------------------------------------------------
void dRunObject( WORD wObj ){
	BYTE nTraj,i;
	if( dObject[wObj].Tipo!=TP_INIMIGO )
		return;

	BYTE BombaID=dObject[wObj].BombaID;

	if( dBomba[BombaID].Load ){
		dObject[wObj].DelayCur+=(WORD) dDiffTick;

		// Verifica se é hora de dar tiro
		if( dObject[wObj].DelayCur>dBomba[BombaID].Delay ){
			dObject[wObj].DelayCur=0;
			for( i=0; i<MAX_BOMBADISPARO; i++ ){
				if( !dBomba[BombaID].Bomba[i].Ativo ){

					// Atira uma bomba
					dBomba[BombaID].Bomba[i].Ativo=true;
					dBomba[BombaID].Bomba[i].FrameCurrent=0;
					dBomba[BombaID].Bomba[i].FrameTick=0;
					dBomba[BombaID].Bomba[i].PosX=dObject[wObj].PosX+dObject[wObj].DisparoX;
					dBomba[BombaID].Bomba[i].PosY=dObject[wObj].PosY+dObject[wObj].DisparoY;

					dObjSurface Player=dObject[PlayerObj];

					// Calcula Direção para traçar a trgetória linear

					if( dBomba[BombaID].Bomba[i].PosX>Player.PosX )
						dBomba[BombaID].Bomba[i].vx=-dBomba[BombaID].vMaxX;
					else
						dBomba[BombaID].Bomba[i].vx=dBomba[BombaID].vMaxX;

					if( dBomba[BombaID].Bomba[i].PosY>Player.PosY )
						dBomba[BombaID].Bomba[i].vy=-dBomba[BombaID].vMaxY;
					else
						dBomba[BombaID].Bomba[i].vy=dBomba[BombaID].vMaxY;
					
					dBomba[BombaID].Bomba[i].dx=abs(dBomba[BombaID].Bomba[i].PosX-Player.PosX);
					dBomba[BombaID].Bomba[i].dy=abs(dBomba[BombaID].Bomba[i].PosY-Player.PosY);

					if(dBomba[BombaID].Bomba[i].dx>dBomba[BombaID].Bomba[i].dy)
						dBomba[BombaID].Bomba[i].dis=dBomba[BombaID].Bomba[i].dx;
					else 
						dBomba[BombaID].Bomba[i].dis=dBomba[BombaID].Bomba[i].dy;

					dBomba[BombaID].Bomba[i].erx=0;
					dBomba[BombaID].Bomba[i].ery=0;

					// OK - Tiro disparado

					break;
				}
			}
		}
	}

	// Movimenta o objeto Inimigo
	nTraj=dObject[wObj].TrajPosT;
	dObject[wObj].PosX+=dObject[wObj].Trajetoria[nTraj].incX;
	dObject[wObj].PosY+=dObject[wObj].Trajetoria[nTraj].incY;
	if( dObject[wObj].Trajetoria[nTraj].Repete ){
		if( (++dObject[wObj].TrajPosCur)>dObject[wObj].Trajetoria[nTraj].Repete ){
			dObject[wObj].TrajPosCur=0;
			dObject[wObj].TrajPosT++;
			if( dObject[wObj].Trajetoria[nTraj+1].Repete==0 )
				dObject[wObj].Ativo=false;
		}
	}

	// Movimenta
}

void dCheckColisao( WORD wObj ){


	if( !dObject[PlayerObj].Ativo || dObject[PlayerObj].Visible )
		return;

	if( dObject[wObj].Tipo!=TP_INIMIGO )
		return;

	dObjSurface Player=dObject[PlayerObj];
	dObjSurface Objeto=dObject[wObj];

	if( dObject[wObj].Energia<=0 ){
		dExplodeObj( wObj );
		return;
	}

	// Ponto médio e raio da nave (PLAYER)
	WORD prX=Player.TamX/2;
	WORD prY=Player.TamY/2;
	WORD pX=Player.PosX+prX;
	WORD pY=Player.PosY+prY;

	// Ponto médio e raio do inimigo
	WORD irX=Objeto.TamX/2;
	WORD irY=Objeto.TamY/2;
	WORD iX=Objeto.PosX+irX;
	WORD iY=Objeto.PosY+irY;

	WORD rX=(prX+irY)-10;
	WORD rY=(prY+irY)-10;

	if( ( pX-rX<iX && iX<pX+rX ) &&
		( pY-rY<iY && iY<pY+rY ) ){
		dExplodeObj( wObj );
		PlayerEnergia-=dObject[wObj].Power;
		dObject[wObj].Energia-=Player.Power;
		return;
	}
}


void dExplodeObj( WORD iObj ){
	dObject[iObj].PosX+=dObject[iObj].TamX/2-50;
	dObject[iObj].PosY+=dObject[iObj].TamY/2-56;
	dObject[iObj].Srf=dObject[ExplosaoObj].Srf;
	dObject[iObj].Cols=dObject[ExplosaoObj].Cols;
	dObject[iObj].Rows=dObject[ExplosaoObj].Rows;
	dObject[iObj].TamX=dObject[ExplosaoObj].TamX;
	dObject[iObj].TamY=dObject[ExplosaoObj].TamY;
	dObject[iObj].FrameTick=dObject[ExplosaoObj].FrameTick;
	dObject[iObj].FrameTickWait=0;
	dObject[iObj].FrameCurrent=0;
	dObject[iObj].TrajPosCur=0;
	dObject[iObj].TrajPosT=0;
	dObject[iObj].TrajPosCur=0;
	dObject[iObj].Segmentos=0;
	dObject[iObj].Trajetoria[0].incY=0;
	dObject[iObj].Trajetoria[0].incY=0;
	dObject[iObj].Trajetoria[1].Repete=0;
	dObject[iObj].Tipo=TP_OBJETO;
	PlayerKill++;
	PlayerPontos+=dObject[iObj].Pontos;
}



// ------------------------------------------------
void dTiroNew(){
	int i;
	BYTE bTiroCurrent;
	for( bTiroCurrent=1; bTiroCurrent<MAX_TIRO; bTiroCurrent++ ){
		if( dTiro[bTiroCurrent].Load ){
			dTiro[bTiroCurrent].DelayCur+=(WORD) dDiffTick;
			if( dTiro[bTiroCurrent].DelayCur>dTiro[bTiroCurrent].Delay ){
				dTiro[bTiroCurrent].DelayCur=0;
				for( i=0; i<MAX_TIROSDISPARO; i++ ){
					if( !dTiro[bTiroCurrent].ON[i].Ativo ){
						dTiro[bTiroCurrent].ON[i].Ativo=true;
						dTiro[bTiroCurrent].ON[i].PosX=dObject[PlayerObj].PosX+dTiro[bTiroCurrent].DisparoX;
						dTiro[bTiroCurrent].ON[i].PosY=dObject[PlayerObj].PosY+dTiro[bTiroCurrent].DisparoY;
						dTiro[bTiroCurrent].ON[i].FrameCurrent=0;
						dTiro[bTiroCurrent].ON[i].FrameTick=0;
						break;
					}
				}
			}
		}
	}
}


void dRunTiro( BYTE bTiroCurrent, BYTE bTiro, RECT * Rect ){
	if( !dTiro[bTiroCurrent].ON[bTiro].Ativo )
		return;

	// Incrementa
	dTiro[bTiroCurrent].ON[bTiro].PosX+=dTiro[bTiroCurrent].vx;
	dTiro[bTiroCurrent].ON[bTiro].PosY+=dTiro[bTiroCurrent].vy;

	// Verifica se saiu da tela, e desaloca o tiro
	if( dTiro[bTiroCurrent].ON[bTiro].PosY>(int)ScreenHeight || dTiro[bTiroCurrent].ON[bTiro].PosX>(int)ScreenWidth ){
		dTiro[bTiroCurrent].ON[bTiro].Ativo=false;
		return;
	}

	if( dTiro[bTiroCurrent].ON[bTiro].PosY<0 || dTiro[bTiroCurrent].ON[bTiro].PosX<0 ){
		dTiro[bTiroCurrent].ON[bTiro].Ativo=false;
		return;
	}

	// Verifica se atingiu algum objeto - se sim, desabilita-o
	WORD tX=dTiro[bTiroCurrent].ON[bTiro].PosX+dTiro[bTiroCurrent].TamX/2;
	WORD tY=dTiro[bTiroCurrent].ON[bTiro].PosY+dTiro[bTiroCurrent].TamY/2;

	for( WORD i=0; i<wLastdObject; i++ ){
		if( dObject[i].Tipo==TP_INIMIGO && dObject[i].Ativo ){
			if( (dObject[i].PosX<tX && tX<dObject[i].PosX+dObject[i].TamX ) &&
				(dObject[i].PosY<tY && tY<dObject[i].PosY+dObject[i].TamY ) ){
				dTiro[bTiroCurrent].ON[bTiro].Ativo=false;
				dObject[i].Energia-=dTiro[bTiroCurrent].Power;
			}
		}
	}

	// Roda o Frame da Animação
	dTiro[bTiroCurrent].ON[bTiro].FrameTick+=dDiffTick;
	if( dTiro[bTiroCurrent].ON[bTiro].FrameTick>=dTiro[bTiroCurrent].FrameTickWait ){
		dTiro[bTiroCurrent].ON[bTiro].FrameTick=0;
		dTiro[bTiroCurrent].ON[bTiro].FrameCurrent++;
		if( dTiro[bTiroCurrent].ON[bTiro].FrameCurrent>=(dTiro[bTiroCurrent].Cols*dTiro[bTiroCurrent].Rows) )
			dTiro[bTiroCurrent].ON[bTiro].FrameCurrent=0;

	}

	WORD wCol = dTiro[bTiroCurrent].ON[bTiro].FrameCurrent % dTiro[bTiroCurrent].Cols;
	WORD wRow = dTiro[bTiroCurrent].ON[bTiro].FrameCurrent / dTiro[bTiroCurrent].Cols;

	Rect[0].top=wRow * dTiro[bTiroCurrent].TamY;
	Rect[0].left=wCol * dTiro[bTiroCurrent].TamX;
	Rect[0].bottom=Rect[0].top + dTiro[bTiroCurrent].TamY;
	Rect[0].right=Rect[0].left + dTiro[bTiroCurrent].TamX;

}

// ------------------------------------------------
void dRunBomba( BYTE bBombaCurrent, BYTE bBomba, RECT * Rect ){
	if( !dBomba[bBombaCurrent].Bomba[bBomba].Ativo )
		return;

	// Incrementa - Trajetoria Retilinea

   dBomba[bBombaCurrent].Bomba[bBomba].erx+=dBomba[bBombaCurrent].Bomba[bBomba].dx;
   dBomba[bBombaCurrent].Bomba[bBomba].ery+=dBomba[bBombaCurrent].Bomba[bBomba].dy;
   if(dBomba[bBombaCurrent].Bomba[bBomba].erx>dBomba[bBombaCurrent].Bomba[bBomba].dis){
      dBomba[bBombaCurrent].Bomba[bBomba].erx-=dBomba[bBombaCurrent].Bomba[bBomba].dis;
      dBomba[bBombaCurrent].Bomba[bBomba].PosX+=dBomba[bBombaCurrent].Bomba[bBomba].vx;
      }
   if(dBomba[bBombaCurrent].Bomba[bBomba].ery>dBomba[bBombaCurrent].Bomba[bBomba].dis){
      dBomba[bBombaCurrent].Bomba[bBomba].ery-=dBomba[bBombaCurrent].Bomba[bBomba].dis;
      dBomba[bBombaCurrent].Bomba[bBomba].PosY+=dBomba[bBombaCurrent].Bomba[bBomba].vy;
      }


	// Verifica se saiu da tela, e desaloca o tiro
	if( dBomba[bBombaCurrent].Bomba[bBomba].PosY>(int)ScreenHeight || dBomba[bBombaCurrent].Bomba[bBomba].PosX>(int)ScreenWidth ){
		dBomba[bBombaCurrent].Bomba[bBomba].Ativo=false;
		return;
	}

	if( dBomba[bBombaCurrent].Bomba[bBomba].PosY<0 || dBomba[bBombaCurrent].Bomba[bBomba].PosX<0 ){
		dBomba[bBombaCurrent].Bomba[bBomba].Ativo=false;
		return;
	}

	// Verifica se atingiu a Jogados Player
	dObjSurface Player=dObject[PlayerObj];

	// Ponto médio e raio da nave (PLAYER)
	WORD prX=Player.TamX/2;
	WORD prY=Player.TamY/2;
	WORD pX=Player.PosX+prX;
	WORD pY=Player.PosY+prY;

	// Ponto médio e raio da Bomba Inimigo
	WORD irX=dBomba[bBombaCurrent].TamX/2;
	WORD irY=dBomba[bBombaCurrent].TamY/2;
	WORD iX=dBomba[bBombaCurrent].Bomba[bBomba].PosX+irX;
	WORD iY=dBomba[bBombaCurrent].Bomba[bBomba].PosY+irY;

	WORD rX=(prX+irY)-10;
	WORD rY=(prY+irY)-10;

	if( ( pX-rX<iX && iX<pX+rX ) &&
		( pY-rY<iY && iY<pY+rY ) ){
		PlayerEnergia-=dBomba[bBombaCurrent].Power;
		dBomba[bBombaCurrent].Bomba[bBomba].Ativo=false;
	}

	// Roda o Frame da Animação
	dBomba[bBombaCurrent].Bomba[bBomba].FrameTick+=dDiffTick;
	if( dBomba[bBombaCurrent].Bomba[bBomba].FrameTick>=dBomba[bBombaCurrent].FrameTickWait ){
		dBomba[bBombaCurrent].Bomba[bBomba].FrameTick=0;
		dBomba[bBombaCurrent].Bomba[bBomba].FrameCurrent++;
		if( dBomba[bBombaCurrent].Bomba[bBomba].FrameCurrent>=(dBomba[bBombaCurrent].Cols*dBomba[bBombaCurrent].Rows) )
			dBomba[bBombaCurrent].Bomba[bBomba].FrameCurrent=0;
	}

	WORD wCol = dBomba[bBombaCurrent].Bomba[bBomba].FrameCurrent % dBomba[bBombaCurrent].Cols;
	WORD wRow = dBomba[bBombaCurrent].Bomba[bBomba].FrameCurrent / dBomba[bBombaCurrent].Cols;

	Rect[0].top=wRow * dBomba[bBombaCurrent].TamY;
	Rect[0].left=wCol * dBomba[bBombaCurrent].TamX;
	Rect[0].bottom=Rect[0].top + dBomba[bBombaCurrent].TamY;
	Rect[0].right=Rect[0].left + dBomba[bBombaCurrent].TamX;

}


// ------------------------------------------------
void dGetObjRectFrame( int i, RECT * Rect, DWORD nDiffTick ){
	WORD wRow, wCol;

	if( dObject[i].FrameTick>0 || dObject[i].iVar>0 ){


		if( dObject[i].iVar>0 ){
			dObject[i].FrameCurrent=(BYTE) GetiVar( dObject[i].iVar );
		}
		else {

			dObject[i].FrameTickWait+=nDiffTick;
			if( dObject[i].FrameTickWait>=dObject[i].FrameTick ){
				dObject[i].FrameTickWait=0;

				// Posiciona o objeto

				dObject[i].FrameCurrent++;
				if( dObject[i].Segmentos>0 ){
					if( dObject[i].FrameCurrent>dObject[i].Seg[dObject[i].SegCur].FrameFim )
						dObject[i].FrameCurrent=dObject[i].Seg[dObject[i].SegCur].FrameIni;
				}
				else{
					if( dObject[i].FrameCurrent>=(dObject[i].Cols*dObject[i].Rows) ){
						dObject[i].FrameCurrent=0;
#ifndef _TESTE
						if( dObject[i].Srf==dObject[ExplosaoObj].Srf ){
							dObject[i].Ativo=false;
						}
#endif
					}		
				}
			}
		}
		
		wCol = dObject[i].FrameCurrent % dObject[i].Cols;
		wRow = dObject[i].FrameCurrent / dObject[i].Cols;

		Rect[0].top=wRow * dObject[i].TamY;
		Rect[0].left=wCol * dObject[i].TamX;
		Rect[0].bottom=Rect[0].top + dObject[i].TamY;
		Rect[0].right=Rect[0].left + dObject[i].TamX;
	}
	else {
		Rect[0].top=0;
		Rect[0].left=0;
		Rect[0].bottom=dObject[i].TamY;
		Rect[0].right=dObject[i].TamX;
	}

}
			


// ------------------------------------------------
void dAdjustRect( int * PosX, int * PosY, RECT * Rect, WORD nTamX, WORD nTamY ){
	int nRecX, nRecY;

	// XXXXXXXXXXXXXXXXXXXXXX
	if( *PosX+nTamX>0 ){
		if( *PosX<0 ){
			nRecX=-*PosX;
			Rect[0].left+=nRecX;
			*PosX=0;
			}
		}

	if( (*PosX+nTamX)>(int)ScreenWidth ){
		nRecX=ScreenWidth-*PosX;
		if( nRecX>0 ){
			Rect[0].right=Rect[0].left+nRecX;
			}
		}

	// YYYYYYYYYYYYYYYYYYYYYYY
	if( *PosY+nTamY>0 ){
		if( *PosY<0 ){
			nRecY=-*PosY;
			Rect[0].top+=nRecY;
			*PosY=0;
			}
		}

	if( (*PosY+nTamY)>(int)ScreenHeight ){
		nRecY=ScreenHeight-*PosY;
		if( nRecY>0 ){
			Rect[0].bottom=Rect[0].top+nRecY;
			}
		}
}



// ------------------------------------------------
void dDisplayQuadrante( BYTE nPl, DWORD nDiffTick ){

	dTela[nPl].FrameTickWait+=nDiffTick;
	if( dTela[nPl].FrameTickWait>=dTela[nPl].FrameTick){
		dTela[nPl].FrameTickWait=0;
	}
	else
		return;

	WORD	nColPos,nRowPos;
	WORD	Wx,Wy,adjX,adjY;
	WORD	x0,x1,x2,x3;
	WORD	y0,y1,y2,y3;
	RECT	rp;
	BYTE	q;
	char	rqX=1,rqY=1;
	WORD	wSrf,wMax;

	// Incrementa (move o fundo)
	dTela[nPl].PosX+=dTela[nPl].IncX;
	dTela[nPl].PosY+=dTela[nPl].IncY;

	if( dTela[nPl].Loop ){
		// Verificas os maximos!!! em X e Y
		wMax=dTela[nPl].TamX*dTela[nPl].Cols;		// X MAX
		if( dTela[nPl].PosX>wMax ){
			dTela[nPl].PosX-=wMax;
		}
		wMax=dTela[nPl].TamY*dTela[nPl].Rows;		// Y MAX
		if( dTela[nPl].PosY>wMax ){
			dTela[nPl].PosY-=wMax;
		}
	}
	else {
		// Verificas os maximos!!! em X e Y
		wMax=dTela[nPl].TamX*(dTela[nPl].Cols-1);		// X MAX
		if( dTela[nPl].PosX>wMax ){
			dTela[nPl].PosX=wMax;
			dTela[nPl].IncX=0;
		}
		wMax=dTela[nPl].TamY*(dTela[nPl].Rows-1);		// Y MAX
		if( dTela[nPl].PosY>wMax ){
			dTela[nPl].PosY=wMax;
			dTela[nPl].IncY=0;
		}
	}
	
	nColPos=dTela[nPl].PosX/dTela[nPl].TamX;
	nRowPos=dTela[nPl].PosY/dTela[nPl].TamY;

	q=(nRowPos*dTela[nPl].Cols)+nColPos;
	if( nColPos==(dTela[nPl].Cols-1) )
		rqX=1-dTela[nPl].Cols;

	// Calculos dos QUADRANTES

	Wx=dTela[nPl].PosX;
	Wy=dTela[nPl].PosY;

	x0=0;
	y0=0;

	x1=dTela[nPl].TamX;
	y1=0;

	x2=0;
	y2=dTela[nPl].TamY;

	x3=dTela[nPl].TamX;
	y3=dTela[nPl].TamY;

	adjX=nColPos*x3;
	adjY=nRowPos*y3;

	Wx-=adjX;
	Wy-=adjY;

	// QUADRANTE 0

	rp.left=Wx;
	rp.top=Wy;
	rp.right=x3;
	rp.bottom=y3;
	wSrf=dTela[nPl].Srf+q;
	oDisplay->Blt( x0, y0, oSurface[wSrf], &rp );

	// QUADRANTE 1

	if( dTela[nPl].Cols>1 ){
		rp.left=0;
		rp.top=Wy;
		rp.right=ScreenWidth-(x3-Wx);
		rp.bottom=y3;
		wSrf=dTela[nPl].Srf+q+rqX;
		oDisplay->Blt( x1-Wx, y1, oSurface[wSrf], &rp );
		
		dLoadQuadrantObject(nPl, q+rqX);

	}

	// QUADRANTE 2 - Proxima Linha

	if( dTela[nPl].Rows>1 ){
		if( nRowPos==(dTela[nPl].Rows-1) )
			rqY=dTela[nPl].Cols-1;
		else
			rqY=q+dTela[nPl].Cols;

		rp.left=Wx;
		rp.top=0;
		rp.right=x3;
		rp.bottom=ScreenHeight-(y3-Wy);
		wSrf=dTela[nPl].Srf+rqY;
		oDisplay->Blt( x2, y2-Wy, oSurface[wSrf], &rp );

		dLoadQuadrantObject(nPl, rqY);

		// QUADRANTE 3

		if( dTela[nPl].Cols>1 ){
			rp.left=0;
			rp.top=0;
			rp.right=ScreenWidth-(x3-Wx);
			rp.bottom=ScreenHeight-(y3-Wy);
			wSrf=dTela[nPl].Srf+rqY+rqX;
			oDisplay->Blt( x3-Wx, y3-Wy, oSurface[wSrf], &rp );

			dLoadQuadrantObject(nPl, rqY+rqX);

		}

	}
}

// ------------------------------------------------------

void dLoadQuadrantObject( BYTE nPlan, BYTE Quad ){
	int			i, nMax;
	WORD		iNew,iSpr;
	dObjCopyQ	oObjQ;
	nMax=dTela[nPlan].oObjMax;
	for( i=0; i<nMax; i++ ){
		oObjQ=dTela[nPlan].oObj[i];

		if( oObjQ.Quadrante==Quad && oObjQ.Quantidade ){
			oObjQ.Tick+=(WORD) dDiffTick;
			if( oObjQ.Tick>oObjQ.TickWait ){
				oObjQ.Tick=0;
				oObjQ.Quantidade--;

				//Obtem a origem e o destino
				iSpr=dTela[nPlan].oObj[i].Sprite;
				iNew=wLastdObject;

				// Copia e Ativa o Objeto
				dObject[iNew]=dObject[iSpr];
				dObject[iNew].Ativo=true;

				if( oObjQ.PosX!=0 ){
					dObject[iNew].PosX=dTela[nPlan].PosX+oObjQ.PosX;
					dObject[iNew].PosY=dTela[nPlan].PosY+oObjQ.PosY;
				}

				wLastdObject++;

			}
		}
		dTela[nPlan].oObj[i]=oObjQ;
	}
}

//********************************

int dLoadDL( char * cFileDL1 ){
	HANDLE		DL1;
	BYTE		cBt,bTipo;
	DWORD		dwBytesRead;
	char		cFileOpen[200];
	int			i;
	BYTE		nPlano;
	WORD		wPosX,wPosY,wTamX,wTamY;
	BYTE		nBmps=0,bRows,bCols;
	WORD		wTickFrame=0, wBt;
	int			wSrf;
	WORD		nQuadros=0;

	if( strlen(cFileDL1)>30 )
		sprintf( cFileOpen, "%s", cFileDL1 );
	else
		sprintf( cFileOpen, "%s%s", cDefaultPath, cFileDL1 );
	
	DL1 = CreateFile( cFileOpen, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL );

	i=0;
	do{
		ReadFile( DL1, &cBt, sizeof(cBt), &dwBytesRead, NULL );
		i++;
	}while( cBt!=26 && i<100 && dwBytesRead>0 );

	ReadFile( DL1, &bTipo, sizeof(bTipo), &dwBytesRead, NULL );

	ReadFile( DL1, &cBt, sizeof(cBt), &dwBytesRead, NULL );	

	if( cBt!=0xFF )
		return -1;	// Arquivo inválido

	if( bTipo==DLTP_PLAYERDEF ){
		bTipo=DLTP_SPRITE;
		PlayerObj=wLastdObject;
		}

	if( bTipo>=DLTP_TELA && bTipo<=DLTP_SPRITE ){

		ReadFile( DL1, &nPlano, sizeof(nPlano), &dwBytesRead, NULL );

		if( bTipo!=DLTP_TELA ){
			ReadFile( DL1, &wPosX, sizeof(wPosX), &dwBytesRead, NULL );
			ReadFile( DL1, &wPosY, sizeof(wPosY), &dwBytesRead, NULL );
			bRows=1;
			bCols=1;
			wTickFrame=0;
		}

		bRows=1;					// Define o padrão de objeto unico
		bCols=1;

		// Obtem a posição correta já para configurar o SPRITE.
		i=wLastdObject;
		if( PlayerObj==i )
			dObject[i].Tipo=TP_PLAYER;
		else
			dObject[i].Tipo=0;

		dObject[i].Ativo=true;		// Somente desabilita se for multiplo sprite
		dObject[i].Segmentos=0;		// Se não for SPRITE já zera os segmentos
		dObject[i].FrameCurrent=0;	// Se for Sprite irá posicionar no Primeiro frame do primeiro segmento
		dObject[i].TextID=0;		// Nâo é texto
		dObject[i].Plano=nPlano;
		dObject[i].iVar=0;
		dObject[i].Visible=0;
		dObject[i].VisibleCount=0;
		
		// Trajetoria padrão (STATICA)
		dObject[i].TrajPosT=0;
		dObject[i].TrajPosCur=0;
		dObject[i].Trajetoria[0].Repete=0;
		dObject[i].Trajetoria[0].incY=0;
		dObject[i].Trajetoria[0].incY=0;
		// Se for tela o objeto é descartado

		if( bTipo==DLTP_ANIMACAO || bTipo==DLTP_QUADRANTE || bTipo==DLTP_SPRITE ){
			ReadFile( DL1, &bCols, sizeof(bCols), &dwBytesRead, NULL );
			ReadFile( DL1, &bRows, sizeof(bRows), &dwBytesRead, NULL );
			ReadFile( DL1, &wTickFrame, sizeof(wTickFrame), &dwBytesRead, NULL );
			if(bCols==1 && bRows>1 ){
				nBmps=bRows;
				nQuadros=bRows;
				}
		}

		if( bTipo==DLTP_ANIMACAO ){
			ReadFile( DL1, &cBt, sizeof(cBt), &dwBytesRead, NULL );
			dObject[i].iVar=cBt;
		}

		if( bTipo==DLTP_QUADRANTE ){
			
			nBmps=bCols*bRows;		// Obtem o número de arquivos anexos
			dTela[nPlano].FrameTickWait=wTickFrame;
			dTela[nPlano].FrameTick=0;

			// Lê Incremento X e Y da movimentação do quadrante
			ReadFile( DL1, &cBt, sizeof(cBt), &dwBytesRead, NULL );
			dTela[nPlano].IncX=cBt;
			ReadFile( DL1, &cBt, sizeof(cBt), &dwBytesRead, NULL );
			dTela[nPlano].IncY=cBt;

			// Lê se o quadrante efetua LOOP (tipo lógico: BOOL)
			ReadFile( DL1, &cBt, sizeof(cBt), &dwBytesRead, NULL );
			dTela[nPlano].Loop=cBt;
			dTela[nPlano].oObjMax=0;

		}

		if( bTipo==DLTP_SPRITE ){
			if( !dObject[i].Tipo ){
				dObject[i].Tipo=TP_INIMIGO;
			}

			ReadFile( DL1, &wBt, sizeof(wBt), &dwBytesRead, NULL );
			dObject[i].Pontos=wBt;

			ReadFile( DL1, &wBt, sizeof(wBt), &dwBytesRead, NULL );
			dObject[i].Power=wBt;

			ReadFile( DL1, &wBt, sizeof(wBt), &dwBytesRead, NULL );
			dObject[i].Energia=wBt;

			dLoadSegmento( DL1, i );
			dLoadTrajetoria( DL1, i );
			dLoadPosicoes( DL1, i );
			dLoadBomba( DL1, i );
		}

		wSrf=dLoadImage( DL1, &wTamX, &wTamY, nBmps, nQuadros );
		if( nQuadros>0 )
			wTamY*=nQuadros;
		
		if( wSrf<0 )
			return wSrf;	// Já é negativa
		else {
			if( bTipo==DLTP_TELA || bTipo==DLTP_QUADRANTE ){
				// Carrega a tela...
				dTela[nPlano].Ativo=true;
				if( bTipo==DLTP_QUADRANTE )
					dTela[nPlano].Srf=(1+wSrf)-nBmps;
				else
					dTela[nPlano].Srf=wSrf;

				dTela[nPlano].TamX=wTamX;
				dTela[nPlano].TamY=wTamY;
				dTela[nPlano].Cols=bCols;
				dTela[nPlano].Rows=bRows;
				dTela[nPlano].PosX=0;
				dTela[nPlano].PosY=0;
			}
			else {
				// Carega o Objeto
				dObject[i].TextID=0;
				dObject[i].Srf=wSrf;
				dObject[i].PosX=wPosX;
				dObject[i].PosY=wPosY;
				dObject[i].Cols=bCols;
				dObject[i].Rows=bRows;
				dObject[i].TamX=wTamX/bCols;
				dObject[i].TamY=wTamY/bRows;
				dObject[i].FrameTick=wTickFrame;
				dObject[i].FrameTickWait=0;
				wLastdObject++;

			}
		}
	}
	else if( bTipo==DLTP_TEXT ){
		ReadFile( DL1, &nPlano, sizeof(nPlano), &dwBytesRead, NULL );
		ReadFile( DL1, &wPosX, sizeof(wPosX), &dwBytesRead, NULL );
		ReadFile( DL1, &wPosY, sizeof(wPosY), &dwBytesRead, NULL );

		// Aloca úma posição
		i=wLastdObject;
		dObject[i].Plano=nPlano;
		dObject[i].PosX=wPosX;
		dObject[i].PosY=wPosY;

		dLoadText( DL1, i );

		wLastdObject++;
	}
	else if( bTipo==DLTP_TIRO || bTipo==DLTP_BOMBA ){
		WORD DisparoX, DisparoY, Delay;
		BYTE ID,Power;
		char vx,vy;

		// Lê o ID
		ReadFile( DL1, &ID, sizeof(ID), &dwBytesRead, NULL );

		// Lê as informações ref a animação do tiro Row/Col/Frams
		ReadFile( DL1, &bCols, sizeof(bCols), &dwBytesRead, NULL );
		ReadFile( DL1, &bRows, sizeof(bRows), &dwBytesRead, NULL );
		ReadFile( DL1, &wTickFrame, sizeof(wTickFrame), &dwBytesRead, NULL );

		//Poder de fogo / Delay Mínimo entre um desparo e outro
		ReadFile( DL1, &Power, sizeof(Power), &dwBytesRead, NULL );
		ReadFile( DL1, &Delay, sizeof(Delay), &dwBytesRead, NULL );

		//Velocidades maximas
		ReadFile( DL1, &vx, sizeof(vx), &dwBytesRead, NULL );
		ReadFile( DL1, &vy, sizeof(vy), &dwBytesRead, NULL );

		if( bTipo==DLTP_TIRO ){
			// Lê as posições de disparo
			ReadFile( DL1, &DisparoX, sizeof(DisparoX), &dwBytesRead, NULL );
			ReadFile( DL1, &DisparoY, sizeof(DisparoY), &dwBytesRead, NULL );
		}
		
		wSrf=dLoadImage( DL1, &wTamX, &wTamY, 0, 0 );
		
		if( wSrf<0 )
			return wSrf;	// Já é negativa

		if( bTipo==DLTP_TIRO ){
			dTiro[ID].Load=true;
			dTiro[ID].Srf=wSrf;
			dTiro[ID].Rows=bRows;
			dTiro[ID].Cols=bCols;
			dTiro[ID].DisparoX=DisparoX;
			dTiro[ID].DisparoY=DisparoY;
			dTiro[ID].Delay=Delay;
			dTiro[ID].DelayCur=0;
			dTiro[ID].Power=Power;
			dTiro[ID].vx=vx;
			dTiro[ID].vy=vy;
			dTiro[ID].FrameTickWait=wTickFrame;
			dTiro[ID].TamX=wTamX/bCols;
			dTiro[ID].TamY=wTamY/bRows;

			// Inicializa as posições dos tiros disparados
			for( BYTE n=0; n<MAX_TIROSDISPARO; n++ ){
				dTiro[ID].ON[i].Ativo=false;
			}
		}
		else{
			dBomba[ID].Load=true;
			dBomba[ID].Srf=wSrf;
			dBomba[ID].Rows=bRows;
			dBomba[ID].Cols=bCols;
			dBomba[ID].Delay=Delay;
			dBomba[ID].Power=Power;
			dBomba[ID].vMaxX=vx;
			dBomba[ID].vMaxY=vy;
			dBomba[ID].FrameTickWait=wTickFrame;
			dBomba[ID].TamX=wTamX/bCols;
			dBomba[ID].TamY=wTamY/bRows;
			for( BYTE n=0; n<MAX_BOMBADISPARO; n++ )
				dBomba[ID].Bomba[n].Ativo=false;
		}

	}
	else
		return -3;

	if( wTickFrame && dMinWaitTick>wTickFrame )
		dMinWaitTick=wTickFrame;

	CloseHandle(DL1);

	return i;
}

int dLoadImage( HANDLE dlFile, WORD * wTamXout, WORD * wTamYout, BYTE nBmpMax, WORD nQuadros ){
	BYTE				nBmp=0;
	DWORD				dwBytesRead;
	WORD				nSrf, iX, iY;
	WORD				wTamX, wTamY;
	dlRGB				rgbFundo, rgbPixel;
	BYTE				bRed, bBlue, bGreen;
	DWORD				dwCor,dwColorKey;
	DWORD				dwRepete;
    HRESULT				hr;

	// Obtem o Tamnho em X e Y
	ReadFile( dlFile, &wTamX, sizeof(wTamX), &dwBytesRead, NULL );
	ReadFile( dlFile, &wTamY, sizeof(wTamY), &dwBytesRead, NULL );

	if( nQuadros>0 ){
		nSrf=wLastSurface;
		wLastSurface++;

		// Atualiza a contagem de objetos carregados
		nObjLoad++;

		hr=oDisplay->CreateSurface( &oSurface[nSrf], wTamX, wTamY*nQuadros );
		if( FAILED( hr ) )
			return -(99);
	}

	*wTamXout=wTamX;
	*wTamYout=wTamY;

	// Cria a imagem na Surface
	LPDIRECTDRAWSURFACE7	pDDS;
	DDSURFACEDESC2			ddsd;
	DWORD *					pDDSColor;

	do{

		// Atualiza tela enquanto carrega imagens...
		dRedrall();

		if( nQuadros==0 ){
			nSrf=wLastSurface;
			wLastSurface++;

			SAFE_DELETE( oSurface[nSrf] );

			// Atualiza a contagem de objetos carregados
			nObjLoad++;
			nObjPerLoad=100*nObjLoad/nObjTotalLoad;

			hr=oDisplay->CreateSurface( &oSurface[nSrf], wTamX, wTamY );
		}

		if( FAILED( hr ) )
			return -(10+nBmp);

		if( nBmp==0 ){
			// Obtem a cor de Fundo (Tranparencia)
			ReadFile( dlFile, &rgbFundo, sizeof(rgbFundo), &dwBytesRead, NULL );

			// Inversão !!! ???
			bRed=(BYTE) rgbFundo.B;
			bGreen=(BYTE) rgbFundo.G;
			bBlue=(BYTE) rgbFundo.R;

			dwColorKey=(bRed<<16) | (bGreen<<8) | bBlue;
		}

		oSurface[nSrf]->SetColorKey( dwColorKey );

		// Carrega a imagem
		pDDS=oSurface[nSrf]->GetDDrawSurface();

		ZeroMemory( &ddsd, sizeof(ddsd) );
		ddsd.dwSize = sizeof(ddsd);

		if( FAILED( hr = pDDS->Lock( NULL, &ddsd, DDLOCK_WAIT, NULL ) ) )
			return -(100+nBmp);

		pDDSColor = (DWORD*) ddsd.lpSurface;
		dwRepete=0;
		
		for( iY = 0; iY<wTamY; iY++ ){

			if( nQuadros>0 )
				pDDSColor = (DWORD*) ( (BYTE*) ddsd.lpSurface + ( (wTamY*nBmp) + (wTamY-iY-1) ) * ddsd.lPitch );
			else
				pDDSColor = (DWORD*) ( (BYTE*) ddsd.lpSurface + (wTamY-iY-1) * ddsd.lPitch );

			for( iX = 0; iX < wTamX; iX++ ){

				// Verifica compactação (repetição do pixel de fundo)

				if( dwRepete )
					dwRepete--;
				else{
					// Lê pixel
					ReadFile( dlFile, &rgbPixel, sizeof(rgbPixel), &dwBytesRead, NULL );

					if( rgbFundo.R==rgbPixel.R && rgbFundo.G==rgbPixel.G && rgbFundo.B==rgbPixel.B ){
						ReadFile( dlFile, &dwRepete, sizeof(dwRepete), &dwBytesRead, NULL );
						dwRepete--;
					}

					bRed=(BYTE) rgbPixel.R;
					bGreen=(BYTE) rgbPixel.G;
					bBlue=rgbPixel.B;

					dwCor=(bRed<<16) | (bGreen<<8) | bBlue;

				}

				*pDDSColor=dwCor;

				pDDSColor++;
			}
		}

		pDDS->Unlock(NULL); 

		nBmp++;

	}while( nBmp<nBmpMax );

	return nSrf;

}

void dLoadSegmento( HANDLE dlFile, int iObj ){
	BYTE	i, nSeg, n;
	BYTE	bFrameIni, bFrameFim;
	DWORD	dwBytesRead;

	// Obtem o número de segmentos
	ReadFile( dlFile, &nSeg, sizeof(nSeg), &dwBytesRead, NULL );
	dObject[iObj].Segmentos=nSeg-1;
	dObject[iObj].SegCur=0;

	for( i=0; i<nSeg; i++ ){

		// Obtem o tamanho do nome do segmento
		ReadFile( dlFile, &n, sizeof(n), &dwBytesRead, NULL );
		ReadFile( dlFile, dObject[iObj].Seg[i].Name, n, &dwBytesRead, NULL );
		dObject[iObj].Seg[i].Name[n]=0;

		// Obtem o inifio e o fim do segmento
		ReadFile( dlFile, &bFrameIni, sizeof(bFrameIni), &dwBytesRead, NULL );
		dObject[iObj].Seg[i].FrameIni=bFrameIni;
		ReadFile( dlFile, &bFrameFim, sizeof(bFrameFim), &dwBytesRead, NULL );
		dObject[iObj].Seg[i].FrameFim=bFrameFim;
	}

	//Inicializa o Frame Inicial
	dObject[iObj].FrameCurrent=dObject[iObj].Seg[dObject[iObj].SegCur].FrameIni;
}

void dLoadTrajetoria( HANDLE dlFile, int iObj ){
	TrajPos	Pos;
	DWORD	dwBytesRead;
	int		i=0;

	do{
		ReadFile( dlFile, &Pos, sizeof(Pos), &dwBytesRead, NULL );
		dObject[iObj].Trajetoria[i].Repete=Pos.Repete;
		dObject[iObj].Trajetoria[i].incX=Pos.incX;
		dObject[iObj].Trajetoria[i].incY=Pos.incY;
		i++;
	}while( Pos.Repete>0 && i<30 );

}

void dLoadBomba( HANDLE dlFile, int iObj ){
	DWORD	dwBytesRead;
	BYTE	ID, Erro;
	WORD	DisparoX,DisparoY;

	// ID
	ReadFile( dlFile, &ID, sizeof(ID), &dwBytesRead, NULL );

	// DisparoX DisparoY
	ReadFile( dlFile, &DisparoX, sizeof(DisparoX), &dwBytesRead, NULL );
	ReadFile( dlFile, &DisparoY, sizeof(DisparoY), &dwBytesRead, NULL );

	// Erro
	ReadFile( dlFile, &Erro, sizeof(Erro), &dwBytesRead, NULL );

	dObject[iObj].BombaID=ID;
	dObject[iObj].DisparoX=DisparoX;
	dObject[iObj].DisparoY=DisparoY;
	dObject[iObj].Erro=Erro;
	dObject[iObj].DelayCur=0;

}

void dLoadPosicoes( HANDLE dlFile, int iObj ){
	QuadStart		Start;
	QuadStartPos	StartPos;
	DWORD			dwBytesRead;
	int				i=0,n;
	BYTE			nPlan;
	
	nPlan=dObject[iObj].Plano;
	n=dTela[nPlan].oObjMax;

	// Lê posições de gatilho (repetidas)
	do{
		ReadFile( dlFile, &Start, sizeof(Start), &dwBytesRead, NULL );
		if( Start.Quadrante==0xFF )
			break;
		dTela[nPlan].oObj[n].Quadrante=Start.Quadrante;
		dTela[nPlan].oObj[n].Quantidade=Start.Quantidade;
		dTela[nPlan].oObj[n].TickWait=Start.TickNext;
		dTela[nPlan].oObj[n].Tick=0;
		dTela[nPlan].oObj[n].Sprite=iObj;
		dTela[nPlan].oObj[n].PosX=0;
		dTela[nPlan].oObj[n].PosY=0;
		n++;
		i++;
	}while( Start.Quadrante!=0xFF && i<50 );

	// Lê posições específicas
	do{
		ReadFile( dlFile, &StartPos, sizeof(StartPos), &dwBytesRead, NULL );
		if( StartPos.Quadrante==0xFF )
			break;
		dTela[nPlan].oObj[n].Quadrante=StartPos.Quadrante;
		dTela[nPlan].oObj[n].Quantidade=1;
		dTela[nPlan].oObj[n].TickWait=0;
		dTela[nPlan].oObj[n].Tick=0;
		dTela[nPlan].oObj[n].Sprite=iObj;
		dTela[nPlan].oObj[n].PosX=StartPos.PosX;
		dTela[nPlan].oObj[n].PosY=StartPos.PosY;
		n++;
		i++;
	}while( StartPos.Quadrante!=0xFF && i<50 );

	
	dTela[nPlan].oObjMax=n;

	if( i )
		dObject[iObj].Ativo=false;

}

void dLoadText( HANDLE dlFile, int iObj ){
	BYTE	n;
	DWORD	dwBytesRead;
	HRESULT	hr;
	char	cVar[200];
	char	FontName[50];
	dlRGB	Cor;
	DWORD	rgbFore, rgbBack;
	BYTE	Size,Bold;
	BYTE	nVar1,nVar2,nVar3;
	WORD	wSrf;
	WORD	wTamX,wTamY;

	nObjLoad++;
	nObjPerLoad=100*nObjLoad/nObjTotalLoad;

	// Le a Variável (string)
	ReadFile( dlFile, &n, sizeof(n), &dwBytesRead, NULL );
	ReadFile( dlFile, cVar, n, &dwBytesRead, NULL );
	cVar[n]=0;

	// Lé a cor de Fundo
	ReadFile( dlFile, &Cor, sizeof(Cor), &dwBytesRead, NULL );
	rgbBack=(Cor.R<<16) | (Cor.G<<8) | Cor.B;
	
	// Lé a cor da Letra
	ReadFile( dlFile, &Cor, sizeof(Cor), &dwBytesRead, NULL );
	rgbFore=(Cor.R<<16) | (Cor.G<<8) | Cor.B;

	// Lê os tamnhos máximos da caixa de texto
	ReadFile( dlFile, &wTamX, sizeof(wTamX), &dwBytesRead, NULL );
	ReadFile( dlFile, &wTamY, sizeof(wTamY), &dwBytesRead, NULL );

	// Obtem as 3 variáveis de parameto
	ReadFile( dlFile, &nVar1, sizeof(nVar1), &dwBytesRead, NULL );
	ReadFile( dlFile, &nVar2, sizeof(nVar2), &dwBytesRead, NULL );
	ReadFile( dlFile, &nVar3, sizeof(nVar3), &dwBytesRead, NULL );

	// Le o Tamanho e o nome da Fonte (string)
	ReadFile( dlFile, &Size, sizeof(Size), &dwBytesRead, NULL );
	ReadFile( dlFile, &Bold, sizeof(Bold), &dwBytesRead, NULL );
	ReadFile( dlFile, &n, sizeof(n), &dwBytesRead, NULL );
	ReadFile( dlFile, FontName, n, &dwBytesRead, NULL );
	FontName[n]=0;

	LOGFONT lf;								// Handle da Fonte
	memset(&lf, 0, sizeof(LOGFONT));		// Limpa a estrutura da fonte
	lf.lfHeight = Size;						// Define o tamanho
	if( Bold )
		lf.lfWeight=FW_BOLD;				// BOLD
	strcpy(lf.lfFaceName, FontName);		// Requisita a fonte
	dObject[iObj].hFont=CreateFontIndirect(&lf);

	dObject[iObj].TextID=dCreateCVarPublic( cVar, nVar1, nVar2, nVar3 );
	dObject[iObj].rgbBack=rgbBack;
	dObject[iObj].rgbFore=rgbFore;

	// Cria um objeto de Surface para o texto
	wSrf=wLastSurface;
	wLastSurface++;

	SAFE_DELETE( oSurface[wSrf] );

	hr=oDisplay->CreateSurface( &oSurface[wSrf], wTamX, wTamY );
	if( FAILED( hr ) )
		return;
	
	dFillSurface( wSrf, rgbBack );

	if( !rgbBack )
		oSurface[wSrf]->SetColorKey( 0 );

	dObject[iObj].Ativo=true;
	dObject[iObj].Srf=wSrf;
	dObject[iObj].TamX=wTamX;
	dObject[iObj].TamY=wTamY;
	dObject[iObj].Cols=1;
	dObject[iObj].Rows=1;
	dObject[iObj].FrameTickWait=0;

}

void dFillSurface( WORD nSrf, DWORD rgbColor ){
    HRESULT					hr;
	LPDIRECTDRAWSURFACE7	pDDS;
	DDSURFACEDESC2			ddsd;
	DWORD *					pDDSColor;
	WORD					wY,wX,wTamX,wTamY;

	pDDS=oSurface[nSrf]->GetDDrawSurface();

	ZeroMemory( &ddsd, sizeof(ddsd) );
	ddsd.dwSize = sizeof(ddsd);

	if( FAILED( hr = pDDS->Lock( NULL, &ddsd, DDLOCK_WAIT, NULL ) ) )
			return;

	pDDSColor = (DWORD*) ddsd.lpSurface;
	
	wTamX=(WORD) ddsd.dwWidth;
	wTamY=(WORD) ddsd.dwHeight;

	for( wY = 0; wY<wTamY; wY++ ){
		pDDSColor = (DWORD*) ( (BYTE*) ddsd.lpSurface + wY * ddsd.lPitch );
			for( wX = 0; wX < wTamX; wX++ ){

				*pDDSColor=rgbColor;

				pDDSColor++;
			}
		}

	pDDS->Unlock(NULL); 

}

void dGetObject( WORD iObj, dObjSurface * oObj ){
	oObj[0]=dObject[iObj];
}

void dSetPlayer( WORD wObj, int * wPosX, int * wPosY, WORD * nSeg, WORD wObjEsplo ){
	PlayerObj=wObj;
	PlayerPosX=wPosX;
	PlayerPosY=wPosY;
	PlayerSeg=nSeg;
	ExplosaoObj=wObjEsplo;
}
