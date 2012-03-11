#include <stdio.h>
#include <stdlib.h>

#include "../DRAC/engine/includes/CGame.h"
#include "../DRAC/font/font.h"

#define 	CRD_PILE			0
#define 	CRD_FOUNDATION		1
#define 	CRD_RESERVE 	    2
#define		CRD_WASTE   		3

#define		SCREEN_WIDTH		800
#define		SCREEN_HEIGHT		600

SDL_Surface *screen;
SDLFont *font1;        // 2 fonts
SDLFont *font2;
CGame BeleagueredCastle;

void Initialize() // must be called only once
{
	//do some initialization and create the regions in here

	SDL_WM_SetCaption("Beleaguered Castle", NULL); // Set the window title
	font1 = initFont("font/data/font1");
	font2 = initFont("font/data/font2");

	InitDeck(screen);
	BeleagueredCastle.Initialize(screen);

	//index 0: Dealer's hand
	BeleagueredCastle.CreateRegion(CRD_RESERVE, CRD_FACEUP, NULL, NULL, CRD_NSYMBOL, 0, 0, 0, 0);

	//index 1-4: Foundations
	for(int i = 0; i < 4; i++)
		BeleagueredCastle.CreateRegion(CRD_FOUNDATION, CRD_VISIBLE|CRD_FACEUP|CRD_DODROP, CRD_DOSINGLE|CRD_DOHIGHER|CRD_DOHIGHERBY1|CRD_DOSUIT, NULL, CRD_NSYMBOL, (SCREEN_WIDTH / 2) - (CARDWIDTH / 2), ((CARDHEIGHT + 15) * i) + 15, 0, 0);

	//index 5-8: Left column of card piles
	for(int i = 0; i < 4; i++)
		BeleagueredCastle.CreateRegion(CRD_PILE, CRD_VISIBLE|CRD_FACEUP|CRD_DODRAG|CRD_DODROP, CRD_DOLOWER|CRD_DOLOWERBY1, CRD_DRAGTOP, CRD_HSYMBOL, (SCREEN_WIDTH / 2) - CARDWIDTH * 2, ((CARDHEIGHT + 15) * i) + 15, -25, 0);
		///*DEBUGGING*/BeleagueredCastle.CreateRegion(CRD_PILE, CRD_VISIBLE|CRD_FACEUP|CRD_DODRAG|CRD_DODROP, CRD_DOALL, CRD_DRAGTOP, CRD_HSYMBOL, (SCREEN_WIDTH / 2) - CARDWIDTH * 2, ((CARDHEIGHT + 15) * i) + 15, -25, 0);

	//index 9-12: Right column of card piles
	for(int i = 0; i < 4; i++)
		BeleagueredCastle.CreateRegion(CRD_PILE, CRD_VISIBLE|CRD_FACEUP|CRD_DODRAG|CRD_DODROP, CRD_DOLOWER|CRD_DOLOWERBY1, CRD_DRAGTOP, CRD_HSYMBOL, (SCREEN_WIDTH / 2) + CARDWIDTH, ((CARDHEIGHT + 15) * i) + 15, 25, 0);
		///*DEBUGGING*/BeleagueredCastle.CreateRegion(CRD_PILE, CRD_VISIBLE|CRD_FACEUP|CRD_DODRAG|CRD_DODROP, CRD_DOALL, CRD_DRAGTOP, CRD_HSYMBOL, (SCREEN_WIDTH / 2) + CARDWIDTH, ((CARDHEIGHT + 15) * i) + 15, 25, 0);
}

void NewGame()
{
	//Empty the card regions from the previous game
	BeleagueredCastle.EmptyStacks();

 	//Create the deck
	BeleagueredCastle[0].NewDeck();

	//Place Aces on the table
	for(int i = 0; i < 4; i++)
		BeleagueredCastle[i+1].Push(BeleagueredCastle[0].RemoveCard(i * 12));

	//Shuffle the deck
	BeleagueredCastle[0].Shuffle();

	//Deal
	for(int i=5; i <= 12; i++)
		BeleagueredCastle[i].Push(BeleagueredCastle[0].Pop(6));

    //initialize all card coordinates
    BeleagueredCastle.InitAllCoords();
}

void HandleKeyDownEvent(SDL_Event &event)
{
	if(event.key.keysym.sym == SDLK_n)		{ NewGame(); BeleagueredCastle.DrawStaticScene(); }
	if(event.key.keysym.sym == SDLK_a)		{ AnimateCards(); }; // Test animation
	if(event.key.keysym.sym == SDLK_r)		{ BeleagueredCastle.DrawStaticScene(); }; // Refresh
}

bool startdrag = false;
void HandleMouseDownEvent(SDL_Event &event)
{
	CCardRegion *srcReg;
	if(event.button.button == SDL_BUTTON_LEFT)
	{
		srcReg = BeleagueredCastle.OnMouseDown(event.button.x, event.button.y);
		if(srcReg == NULL) return;
		//clicked on the foundations or piles for dragging
		if(((srcReg->Id == CRD_FOUNDATION) || (srcReg->Id == CRD_PILE)) && BeleagueredCastle.InitDrag(event.button.x, event.button.y))
		{
			startdrag = true;
			SDL_WM_GrabInput(SDL_GRAB_ON);
		}
	}

	//substitute right-click for double-click event
	if(event.button.button == SDL_BUTTON_RIGHT)
	{
		srcReg = BeleagueredCastle.OnMouseDown(event.button.x, event.button.y);
		if(srcReg == NULL) return;
		CCardRegion *cr;
		CCard card =  srcReg->GetCard(srcReg->Size()-1);

		//clicked on the top of the foundations
		if(((srcReg->Id == CRD_FOUNDATION) || (srcReg->Id == CRD_RESERVE)) && card.FaceUp() && srcReg->PtOnTop(event.button.x, event.button.y))
		{
			if(cr = BeleagueredCastle.FindDropRegion(CRD_FOUNDATION, card))
			{
				CCardStack *cs = new CCardStack;
				*cs = srcReg->Pop(1);
				BeleagueredCastle.InitDrag(cs, -1 , -1);
				BeleagueredCastle.DoDrop(cr);
			}
		}
	}
}

void HandleMouseMoveEvent(SDL_Event &event)
{
	if(event.motion.state == SDL_BUTTON(1) && startdrag)
		BeleagueredCastle.DoDrag(event.motion.x, event.motion.y);
}

void HandleMouseUpEvent(SDL_Event &event)
{
	if(startdrag)
	{
		startdrag = false;
		BeleagueredCastle.DoDrop();
		SDL_WM_GrabInput(SDL_GRAB_OFF);
	}
	if(BeleagueredCastle[0].Empty() && BeleagueredCastle[8].Empty())
	{
		BeleagueredCastle[0].SetSymbol(1);
		BeleagueredCastle.DrawStaticScene();
	}
	//victory
	if((BeleagueredCastle[9].Size() == 13) && (BeleagueredCastle[10].Size() == 13) && (BeleagueredCastle[11].Size() == 13) && (BeleagueredCastle[12].Size() == 13))
	{
		AnimateCards();
		NewGame();
		BeleagueredCastle.DrawStaticScene();
	}
}


int main(int argc, char *argv[])
{
	if( SDL_Init(SDL_INIT_AUDIO|SDL_INIT_VIDEO) < 0 ){
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
	BeleagueredCastle.DrawStaticScene();

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
