#include <iostream>
#include <GBMUMenu/GBMUMenu.hpp>

GBMUMenu::GBMUMenu()
{
	// std::cout << "new GBMUMenu" << std::endl;

	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("Couldn't initialize SDL: %s\n", SDL_GetError());
		exit(1);
	}

	_window = SDL_CreateWindow(
			"GBMU",
			SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED,
			SCREEN_WIDTH,
			SCREEN_HEIGHT,
			0);

	if (!_window)
	{
		printf("Failed to open %d x %d window: %s\n", SCREEN_WIDTH, SCREEN_HEIGHT, SDL_GetError());
		exit(1);
	}

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");

	_renderer = SDL_CreateRenderer(_window, -1, SDL_RENDERER_ACCELERATED);
	if (!_renderer)
	{
		printf("Failed to create renderer: %s\n", SDL_GetError());
		exit(1);
	}
}

GBMUMenu::~GBMUMenu()
{
	// std::cout << "GBMUMenu deleted" << std::endl;

	SDL_Quit();
}

void GBMUMenu::doInput(void)
{
	SDL_Event event;

	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
		case SDL_QUIT:
			_running = false;
			break;

		default:
			break;
		}
	}
}

void GBMUMenu::prepareScene(void)
{
	SDL_SetRenderDrawColor(_renderer, 96, 128, 255, 255);
	SDL_RenderClear(_renderer);
}

void GBMUMenu::presentScene(void)
{
	SDL_RenderPresent(_renderer);
}

void GBMUMenu::run()
{
	_running = true;

	while (_running)
	{
		prepareScene();
		doInput();
		presentScene();
		SDL_Delay(16);
	}
}
