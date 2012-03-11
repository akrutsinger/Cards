#include <stdio.h>
#include <stdlib.h>

#include "../DRAC/engine/includes/CGame.h"
#include "../DRAC/font/font.h"

#define 	CRD_STOCK			0
#define 	CRD_TABLEAU			1
#define 	CRD_FOUNDATION 	    2

#define		SCREEN_WIDTH		800
#define		SCREEN_HEIGHT		600

SDL_Surface *screen;
SDLFont *font1;        // 2 fonts
SDLFont *font2;
CGame Golf;

void Initialize() // must be called only once
{
	//do some initialization and create the regions in here

	SDL_WM_SetCaption("Golf", NULL); // Set the window title
	font1 = initFont("font/data/font1");
	font2 = initFont("font/data/font2");

	InitDeck(screen);
	Golf.Initialize(screen);

	//index 0: The Stock
 	Golf.CreateRegion(CRD_STOCK,
						CRD_VISIBLE|CRD_3D,
						0,
						0,
						CRD_OSYMBOL,
						(SCREEN_WIDTH / 2) - (CARDWIDTH / 2), 400,
						2, 2);

	//index 1-7: The tableau
	for(int i=1; i <= 7; i++)
		Golf.CreateRegion(CRD_TABLEAU,
							CRD_VISIBLE|CRD_FACEUP|CRD_DODRAG|CRD_DODROP,
							CRD_DOOPCOLOR|CRD_DOLOWERBY1|CRD_DOKING,
							CRD_DRAGOPPOSITECOLORS|CRD_DRAGLOWERBY1,
							CRD_HSYMBOL,
							(CARDWIDTH * (i - 1)) + (i * 35), 10,
							0, 25);

	//index 8: The Foundation
	Golf.CreateRegion(CRD_FOUNDATION,
						CRD_VISIBLE|CRD_FACEUP|CRD_DODROP,
						CRD_DOLOWERBY1|CRD_DOHIGHERBY1|CRD_DOWRAP,
						CRD_DRAGTOP,
						CRD_NSYMBOL,
						10, 250,
						15, 0);
}

void NewGame()
{
	//Empty the card regions from the previous game
	Golf.EmptyStacks();

 	//Create the deck
	Golf[0].NewDeck();
	//Shuffle the deck
	Golf[0].Shuffle();

	//Deal
	for(int i=1; i <= 7; i++){
		Golf[i].Push(Golf[0].Pop(5));
	}

	//Put first card onto the foundation
	Golf[8].Push(Golf[0].Pop(1));

    //Initialize all card coordinates
    Golf.InitAllCoords();
}

void HandleKeyDownEvent(SDL_Event &event)
{
	if(event.key.keysym.sym == SDLK_n)		{ NewGame(); Golf.DrawStaticScene(); }
	if(event.key.keysym.sym == SDLK_a)		{ AnimateCards(); }; // Test animation
	if(event.key.keysym.sym == SDLK_r)		{ Golf.DrawStaticScene(); }; // Refresh
}

bool startdrag = false;
void HandleMouseDownEvent(SDL_Event &event)
{
	CCardRegion *srcReg;
	if(event.button.button == SDL_BUTTON_LEFT){
		srcReg = Golf.OnMouseDown(event.button.x, event.button.y);
		if(srcReg == NULL) return;
		//clicked on the tableau or piles for dragging
		if(((srcReg->Id == CRD_TABLEAU) || (srcReg->Id == CRD_FOUNDATION)) && Golf.InitDrag(event.button.x, event.button.y)){
			startdrag = true;
			SDL_WM_GrabInput(SDL_GRAB_ON);
		}
		//clicked on the pile
		if(srcReg->Id == CRD_STOCK){
			printf("clicked on stock pile\n");

			CCardStack *cs = new CCardStack;
			*cs = Golf[0].Pop(1);
			cs->SetCardsFaceUp(true);
			Golf.InitDrag(cs, -1, -1);
			Golf.DoDrop(&Golf[8]);
		}
	}

	//substitute right-click for double-click event
	if(event.button.button == SDL_BUTTON_RIGHT)
	{
		srcReg = Golf.OnMouseDown(event.button.x, event.button.y);
		if(srcReg == NULL) return;
		CCardRegion *cr;
		CCard card =  srcReg->GetCard(srcReg->Size()-1);

		//clicked on the top of the foundations
		if(((srcReg->Id == CRD_FOUNDATION) || (srcReg->Id == CRD_STOCK)) && card.FaceUp() && srcReg->PtOnTop(event.button.x, event.button.y))
		{
			if((cr = Golf.FindDropRegion(CRD_FOUNDATION, card)))
			{
				CCardStack *cs = new CCardStack;
				*cs = srcReg->Pop(1);
				Golf.InitDrag(cs, -1 , -1);
				Golf.DoDrop(cr);
			}
		}
	}
}

void HandleMouseMoveEvent(SDL_Event &event)
{
	if(event.motion.state == SDL_BUTTON(1) && startdrag)
		Golf.DoDrag(event.motion.x, event.motion.y);
}

void HandleMouseUpEvent(SDL_Event &event)
{
	if(startdrag)
	{
		startdrag = false;
		Golf.DoDrop();
		SDL_WM_GrabInput(SDL_GRAB_OFF);
	}
	if(Golf[0].Empty() && Golf[8].Empty())
	{
		Golf[0].SetSymbol(1);
		Golf.DrawStaticScene();
	}
	//victory
	if(Golf[1].Empty() && Golf[2].Empty() && Golf[3].Empty() && Golf[4].Empty() && Golf[5].Empty() && Golf[6].Empty() && Golf[7].Empty())
	{
		AnimateCards();
		NewGame();
		Golf.DrawStaticScene();
	}
}


int main(int argc, char *argv[])
{
	if( SDL_Init(SDL_INIT_AUDIO|SDL_INIT_VIDEO) < 0 )
	{
		printf("Unable to initialize SDL: %s\n", SDL_GetError());
		exit(1);
	}
	atexit(SDL_Quit);

	screen=SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 32, SDL_HWSURFACE|SDL_HWPALETTE|SDL_DOUBLEBUF);
//	screen=SDL_SetVideoMode(800, 600, 32, SDL_SWSURFACE|SDL_HWPALETTE); //Slow


	if(screen == NULL)
	{
		printf("Unable to set 0x0 video: %s\n", SDL_GetError());
		exit(1);
	}

	Initialize();
	NewGame();
	Golf.DrawStaticScene();

	SDL_Event event;
	int done = 0;

	while(done == 0)
	{
		while(SDL_PollEvent(&event))
		{
			switch(event.type)
			{
				case SDL_QUIT:
     				return 0;

				case SDL_KEYDOWN:
     				if(event.key.keysym.sym == SDLK_ESCAPE) { done = 1; }
					HandleKeyDownEvent(event);
         			break;

				case SDL_MOUSEBUTTONDOWN:
					HandleMouseDownEvent(event);
  					break;

				case SDL_MOUSEMOTION:
					HandleMouseMoveEvent(event);
					break;

				case SDL_MOUSEBUTTONUP:
					HandleMouseUpEvent(event);
 					break;
			}
		}
	}

// perform cleaning up in here

	freeFont(font1);
	freeFont(font2);
	return 0;
}
