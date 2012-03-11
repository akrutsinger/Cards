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
CGame Scorpion;

void Initialize() // must be called only once
{
	//do some initialization and create the regions in here

	SDL_WM_SetCaption("Scorpion", NULL); // Set the window title
	font1 = initFont("font/data/font1");
	font2 = initFont("font/data/font2");

	InitDeck(screen);
	Scorpion.Initialize(screen);

	//index 0: The Stock
 	Scorpion.CreateRegion(CRD_STOCK,
						CRD_VISIBLE|CRD_3D,
						0,
						0,
						CRD_OSYMBOL,
						(SCREEN_WIDTH / 2) - (CARDWIDTH / 2), 400,
						2, 2);

	//index 1-7: The tableau
	for(int i=1; i <= 7; i++){
		Scorpion.CreateRegion(CRD_TABLEAU,
							CRD_VISIBLE|CRD_FACEUP|CRD_DODRAG|CRD_DODROP,
							CRD_DOLOWER|CRD_DOKING,
							CRD_DRAGCARDS,
							CRD_HSYMBOL,
							(CARDWIDTH * (i - 1)) + (i * 35), 10,
							0, 25);
	}
}

void NewGame()
{
	//Empty the card regions from the previous game
	Scorpion.EmptyStacks();

 	//Create the deck
	Scorpion[0].NewDeck();
	//Shuffle the deck
	Scorpion[0].Shuffle();

	//Deal
	for(int i = 1; i <= 7; i++)
		Scorpion[i].Push(Scorpion[0].Pop(7));

    //Initialize all card coordinates
    Scorpion.InitAllCoords();

	//Set the first 4 cards of the first 3 columns face down
	for(int i = 1; i <= 4; i++){
		for(int j = 0; j < 3; j++){
			Scorpion[i].SetCardFaceUp(false, j);
		}
	}
}

void HandleKeyDownEvent(SDL_Event &event)
{
	if(event.key.keysym.sym == SDLK_n)		{ NewGame(); Scorpion.DrawStaticScene(); }
	if(event.key.keysym.sym == SDLK_a)		{ AnimateCards(); }; // Test animation
	if(event.key.keysym.sym == SDLK_r)		{ Scorpion.DrawStaticScene(); }; // Refresh
}

bool startdrag = false;
void HandleMouseDownEvent(SDL_Event &event)
{
	CCardRegion *srcReg;
	if(event.button.button == SDL_BUTTON_LEFT){
		srcReg = Scorpion.OnMouseDown(event.button.x, event.button.y);
		if(srcReg == NULL) return;
				//clicked on the top of the foundations
		if((srcReg->Id == CRD_TABLEAU) && srcReg->PtOnTop(event.button.x, event.button.y)){
			srcReg->SetCardFaceUp(true, srcReg->Size() - 1);
		}
		//clicked on the tableau or piles for dragging
		if(((srcReg->Id == CRD_TABLEAU) || (srcReg->Id == CRD_FOUNDATION)) && Scorpion.InitDrag(event.button.x, event.button.y)){
			startdrag = true;
			SDL_WM_GrabInput(SDL_GRAB_ON);
		}
		//clicked on the stock pile
		if(srcReg->Id == CRD_STOCK){
			//printf("clicked on stock pile\n");
			CCardStack *cs = new CCardStack;
			for(int i = 0; i < 4; i++){
				*cs = Scorpion[0].Pop(1);
				cs->SetCardsFaceUp(true);
				Scorpion.InitDrag(cs, -1, -1);
				Scorpion.DoDrop(&Scorpion[i]);
			}
		}
	}

	//substitute right-click for double-click event
	if(event.button.button == SDL_BUTTON_RIGHT){
		srcReg = Scorpion.OnMouseDown(event.button.x, event.button.y);
		if(srcReg == NULL) return;
		CCardRegion *cr;
		CCard card =  srcReg->GetCard(srcReg->Size()-1);

		//clicked on the top of the foundations
		if(((srcReg->Id == CRD_FOUNDATION) || (srcReg->Id == CRD_STOCK)) && card.FaceUp() && srcReg->PtOnTop(event.button.x, event.button.y)){
			if((cr = Scorpion.FindDropRegion(CRD_FOUNDATION, card))){
				CCardStack *cs = new CCardStack;
				*cs = srcReg->Pop(1);
				Scorpion.InitDrag(cs, -1 , -1);
				Scorpion.DoDrop(cr);
			}
		}
	}
}

void HandleMouseMoveEvent(SDL_Event &event)
{
	if(event.motion.state == SDL_BUTTON(1) && startdrag){
		Scorpion.DoDrag(event.motion.x, event.motion.y);
	}
}

void HandleMouseUpEvent(SDL_Event &event)
{
	if(startdrag){
		startdrag = false;
		Scorpion.DoDrop();
		SDL_WM_GrabInput(SDL_GRAB_OFF);
	}
	if(Scorpion[0].Empty()){
		Scorpion[0].SetSymbol(1);
		Scorpion.DrawStaticScene();
	}
	//Victory: When we have 4 stacks from King to Ace of the same suit
	int ordered_stacks = 0;
	for(int i = 1; i < 8; i++){
		if(Scorpion[i].SuitBuilt()){
			ordered_stacks++;
		}
		if(ordered_stacks == 4){
			AnimateCards();
			NewGame();
			Scorpion.DrawStaticScene();
		}
	}
}


int main(int argc, char *argv[])
{
	if( SDL_Init(SDL_INIT_AUDIO|SDL_INIT_VIDEO) < 0 ){
		printf("Unable to initialize SDL: %s\n", SDL_GetError());
		exit(1);
	}
	atexit(SDL_Quit);

	screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 32, SDL_HWSURFACE|SDL_HWPALETTE|SDL_DOUBLEBUF);
//	screen = SDL_SetVideoMode(800, 600, 32, SDL_SWSURFACE|SDL_HWPALETTE); //Slow


	if(screen == NULL)
	{
		printf("Unable to set 0x0 video: %s\n", SDL_GetError());
		exit(1);
	}

	Initialize();
	NewGame();
	Scorpion.DrawStaticScene();

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
