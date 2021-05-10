/*
//
//	Platformer Game in SDL
//
*/
#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_timer.h>
#include <stdbool.h>

#define WINDOW_WIDTH  (640)
#define WINDOW_HEIGHT (480)
#define MAX_FIREBALLS (30)

float jumpDownward(float y, int windowHeight, SDL_Rect* ground);
void groundTranslate(SDL_Rect* ground, SDL_Rect* spritePos, SDL_Renderer* renderer, SDL_Texture* tex);

int main(int argc, char*argv[])
{
	// attempt to initialize graphics
	if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER) != 0)
	{
		fprintf(stderr, "error intitializing SDL: %s\n", SDL_GetError());
		return (1);
	}


	SDL_Window* window = SDL_CreateWindow( "Garbage game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL);

	if (!window)
	{
		fprintf(stderr, "error creating window: %s\n", SDL_GetError());
		SDL_Quit();
		return (2);
	}

	// Creating a renderer, sets up graphics
	Uint32 render_flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC;
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, render_flags);

	if(!renderer)
	{
		fprintf(stderr, "error creating renderer: %s\n", SDL_GetError());
		SDL_DestroyWindow(window);
		SDL_Quit();

		return (3);
	}

	/////////////////////////////////////////////////////////////////
	// Load the image into memory using SDL_image libray function

	SDL_Surface* surface     	 = IMG_Load("res/images/ground_grass_1.png");
	SDL_Surface* spriteSurf 	 = IMG_Load("res/images/hulking_knight.png");
	SDL_Surface* fireballSurf    = IMG_Load("res/images/fireball.png");


	if (!surface || !spriteSurf || !fireballSurf)
	{
		fprintf(stderr, "error loading image on surface\n");
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
		SDL_Quit();

		return (2);
	}

	///////////////////////////////////////////////////////
	// load the image in a texture

	SDL_Texture* tex      = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_Texture* sprite   = SDL_CreateTextureFromSurface(renderer, spriteSurf);
	SDL_Texture* fireball = SDL_CreateTextureFromSurface(renderer, fireballSurf);

	SDL_FreeSurface(surface);
	SDL_FreeSurface(spriteSurf);
	SDL_FreeSurface(fireballSurf);

	if (!tex || !spriteSurf)
	{
		fprintf(stderr, "error creating texture: %s\n ", SDL_GetError());
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
		SDL_Quit();

		return(3);
	}

	// Keystates

	const Uint8* keystate = SDL_GetKeyboardState(NULL);

	////////////////////////////////////////////////////////////////
	// struct to hold the position and size of the ground
	SDL_Rect ground;

	// get the dimensions of texture
	SDL_QueryTexture(tex, NULL, NULL, &ground.w, &ground.h);
	ground.w += 2100;
	ground.h *= 10;

	/////////////////////////////////////////////////////
	// Struct to hold the position and size of the sprite
	SDL_Rect spritePos;
	SDL_Rect 	offset;

	offset.x = 0;
	offset.y = 0;
	offset.w = 65;
	offset.h = 50;

	SDL_QueryTexture(sprite, NULL, NULL, &spritePos.w, &spritePos.h);
	spritePos.w = 64;
	spritePos.h = 64;

	////////////////////////////////////////////////////////////
	// Stuff for fireball

	SDL_Rect* fpos  = NULL;

	fpos 			= (SDL_Rect *) malloc (sizeof (SDL_Rect) * MAX_FIREBALLS);
	for ( int index = 0; index < MAX_FIREBALLS; index++)
	{
		(fpos + index) -> x = -1;
		(fpos + index) -> y = -1;
		(fpos + index) -> w = 32;
		(fpos + index) -> h = 16;
		SDL_QueryTexture(fireball, NULL, NULL, &(fpos + index) -> x, &(fpos + index) -> y);
	}
	(fpos + 0) -> x = 0;
	(fpos + 0) -> y = 0;



	////////////////////////////////////////////////////////
	// Start sprite in center of screen

	float sprite_xpos = (WINDOW_WIDTH / 2) - 40;
	float sprite_ypos = (WINDOW_HEIGHT - ground.h) / 2 + 135;

	//////////////////////////////////////////////////
	// start ground in center of screen

	float x_pos = (WINDOW_WIDTH - ground.w)  / 2;
	float y_pos = (WINDOW_HEIGHT - ground.h) / 2;

	//////////////////////////////////////////////////////
	// Fireball Stuff

	int   numfb	= 0;

	(fpos + numfb) -> x = 0;					// So it starts slightly infront of sprite
	(fpos + numfb) -> y = 0;					// Middle ish of sprite (by hands hopefully)

	//////////////////////////////////////////////////////////
	//
	//	Game LooP stuff
	//
	//////////////////////////////////////////////////////////


	int fdelay    = 0;
	bool running  = true;
	SDL_Event event;


	// game loop
	while(running)
	{
		//clear window
		SDL_RenderClear(renderer);



		if((fpos + numfb) -> x > 2 && (fpos + numfb) -> x < 1000)
		{
			fprintf(stdout, "fireball Pew Pew\n");
			SDL_RenderCopy(renderer, fireball, NULL, (fpos + numfb));
			(fpos + numfb) -> x += 10;
		}

		// actually making the ground where it is based on the float position (i think it makes its smoother)
		ground.y    = (int) y_pos + 200;
		ground.x    = (int) x_pos;

		spritePos.x = (int) sprite_xpos;
		spritePos.y = (int) sprite_ypos;




		// Processing events
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_KEYDOWN)
			{
				if (event.key.keysym.sym == SDLK_ESCAPE)
				{
					running = false;
			    }
			    if ((event.key.keysym.sym == SDLK_z) && (numfb < MAX_FIREBALLS))
			    {
			    	fprintf(stdout, "Z has been pressed\n");
			    	numfb++;
			    	(fpos + numfb) -> x = sprite_xpos + 40;
					(fpos + numfb) -> y = sprite_ypos * 1.1;


			    }
			}

			if (event.type == SDL_KEYUP)
			{
				if (event.key.keysym.sym == SDLK_SPACE)
				{
					sprite_ypos = jumpDownward(sprite_ypos, WINDOW_HEIGHT, &ground);
				}
			}
		}


		// Movement Checking where things are
		if (keystate[SDL_SCANCODE_SPACE])
		{
			while (sprite_ypos > (ground.y - 150))
			{
				sprite_ypos = sprite_ypos - 10;
			}

		}


		if (keystate[SDL_SCANCODE_DOWN])
		{
			// Move up as long as its not the top of the ground
			if (sprite_ypos <= (WINDOW_HEIGHT-ground.h) + 50)
			{
				sprite_ypos = sprite_ypos + 10;
			}
		}

		if (keystate[SDL_SCANCODE_RIGHT])
		{

			// Ground movement
			x_pos = x_pos - 4;

			// Calculating the sprite "Animation"
			fdelay++;
			if (fdelay == 10)
			{
				offset.x = offset.x + 65;
				if ( offset.x > 260)
				{
					offset.x = 0;
					fdelay   = 0;
				}

				fdelay   = 0;
			}
		}

		if (keystate[SDL_SCANCODE_LEFT])
		{
			// Ground movement

			if (x_pos != spritePos.w)
			{
				x_pos = x_pos + 4;
			}

			// Calculating the sprite animation
			fdelay++;
			if (fdelay == 10)
			{
				offset.x = offset.x - 65;
				if (offset.x < 65)
				{
					offset.x = 260;
					fdelay   = 0;
				}
				fdelay	 = 0;
			}
		}

		// draw image to window
		SDL_RenderCopy(renderer, tex, NULL, &ground);
		SDL_RenderCopy(renderer, sprite, &offset, &spritePos);




		// If sprite makes it to a certain place then reprint sprite
		groundTranslate(&ground, &spritePos, renderer, tex);

		SDL_RenderPresent(renderer);

	}

	// Clean up resources before exiting
	SDL_DestroyTexture(fireball);
	SDL_DestroyTexture(tex);
	SDL_DestroyTexture(sprite);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return (0);
}

float jumpDownward(float y, int windowHeight, SDL_Rect* ground)
{
	int delay;

	while (y < (windowHeight - ground -> y) + 10)
	{
		delay++;
		if (delay == 10)
		{
			y += 1.1;

			delay = 0;
		}
	}

	return (y);
}

void groundTranslate(SDL_Rect* ground, SDL_Rect* spritePos, SDL_Renderer* renderer, SDL_Texture* tex)
{
	if (spritePos -> x > (ground -> x + ground -> w))
	{
		ground->x += ground->w;

		SDL_RenderCopy(renderer, tex, NULL, ground);
	}
	else
	{
		return;
	}
}
