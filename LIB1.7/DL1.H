//============================================================================
//
//  IMPACTRO Informática
//  Fábio Ferreira de Souza - 05/12/2002
//
//  Dinamic Libary 1.7
//
//============================================================================
// STRUTURAS / CLASSES / UNIONS / DEFINIÇÕES // FUNÇÕES
//============================================================================


// ======================================================
// Definições

// Tipos de arquivos DL
			// String & Variáveis
#define		DLTP_STRING			10
#define		DLTP_VARIAVEL		11

			// Imagens
#define		DLTP_TELA			20	// Tela de fundo Statica.
#define		DLTP_IMAGEM			21	// Imagem Statica.
#define		DLTP_ANIMACAO		22	// Imagem Multipla (Row*Col) 1 arquvo fonte.
#define		DLTP_QUADRANTE		23	// " multiplos arquvos fonte.
#define		DLTP_SPRITE			24	// " Segmentada por tipo de ação.

			// Texto
#define		DLTP_TEXT			25	// Texto Dinámico ou Statico contindo na variáveis públicas

			// Objetos lógicos		
#define		DLTP_PLAYERDEF		30	// Um objeto carregado utiliza um Sprite
#define		DLTP_IA				31	// carregado na memória, ou o carrega na hora.
#define		DLTP_TIRO			32	// Tiro do jogador
#define		DLTP_BOMBA			33	// bombas/Tiros dispados contra o Jogador

			// Comandos				-- Reservado para futuras implementações
#define		DLTP_BOTAO			40
#define		DLTP_INPUT			41
#define		DLTP_SELECT			42
#define		DLTP_CHECK			43
#define		DLTP_RADIO			44
#define		DLTP_MENU			45

			// Comandos interno de compactação de BitMap
#define		DLCMD_CANCEL		0
#define		DLCMD_FILL			1
#define		DLCMD_REPETE		2
#define		DLCMD_SKIP			3

// Variáveis publicas
			// Variáveis inteiras (PADRÂO)
#define		IVAR_LASTTICK		1	// Timers
#define		IVAR_MINTICK		2
#define		IVAR_DIFFTICK		3
#define		IVAR_SCREENWIDTH	4	// Tamanho da Tela
#define		IVAR_SCREENHEIGHT	5
#define		IVAR_MOUSEX			6	// Posição do Mouse
#define		IVAR_MOUSEY			7
#define		IVAR_TARGETX		8	// Posição do Plyer 1
#define		IVAR_TARGETY		9
#define		IVAR_OBJLOAD		10	// Numero de Objetos Carregados
#define		IVAR_OBJMAXLOAD		11	// Total de Objetos a serem Carregados
#define		IVAR_OBJPERLOAD		12	// Percentual de Objetos a serem Carregados
#define		IVAR_PONTOS			13	// Pontos
#define		IVAR_KILL			14	// Inimigos eliminados
#define		IVAR_ENERGIA		15	// Energia
#define		IVAR_VIDAS			16	// Vidas

			// Variáveis String (PADRÂO)
#define		CVAR_TICK			1
#define		CVAR_SCREEN			2
#define		CVAR_MOUSE			3
#define		CVAR_LOADING		4
#define		CVAR_USERINIT		10

// ======================================================
// Estruturas

struct	dlRGB{
	BYTE B;
	BYTE G;
	BYTE R;
};

struct	dlArray{
	char *Conteudo;
	int Item;
};

union dlWordByte{
	WORD i;
	CHAR c[2];
};

struct StringMask{
	char * cMask;
	int iVar1;
	int iVar2;
	int iVar3;
};

struct TrajPos{
	WORD Repete;
	char incX;
	char incY;
};

struct QuadStart{
	BYTE Quadrante;
	BYTE Quantidade;
	WORD TickNext;
};

struct QuadStartPos{
	BYTE Quadrante;
	WORD PosX;
	WORD PosY;
};


// ======================================================
// Funções Genéricas
int dlGetTipo( dlArray * aVetor, char * cTipo, int nTam );

//Inicialização e termino
void dStartDL();
void dEndDL();

// Controle de variáveis Publicas
void dSetIntVarPublic( int nVar, DWORD * iVar );
void dSetCharVarPublic( int nVar, char * cMaskVar, int iVar1, int iVar2, int iVar3 );
void dCompileCharVar( char * cTextOut, int nVar );
BYTE dCreateCVarPublic( char * cText, BYTE v1, BYTE v2, BYTE v3 );
DWORD GetiVar( BYTE iVar );
