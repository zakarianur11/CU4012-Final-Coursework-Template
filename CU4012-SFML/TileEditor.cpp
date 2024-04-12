#include "TileEditor.h"

TileEditor::TileEditor(sf::RenderWindow* hwnd, Input* in, GameState* game, sf::View* v, World* w, TileManager* tm)
{
	window = hwnd;
	input = in;
	gameState = game;
	view = v;
	world = w;
	tileManager = tm;
	if (!font.loadFromFile("font/arial.ttf")) {
		std::cout << "error loading font" << std::endl;
	};

	TileEditorText.setFont(font);
	TileEditorText.setCharacterSize(20);
	TileEditorText.setOutlineThickness(0.5);
	TileEditorText.setFillColor(sf::Color::Black);

	// Set up tile manager
	tileManager->setCollectableTexture("gfx/Collectable.png");
	tileManager->setPlatformTexture("gfx/Platform.png");
	tileManager->setWallTexture("gfx/Wall.png");
	tileManager->setInput(input);
	tileManager->setWindow(window);
	tileManager->setWorld(world);
	tileManager->setView(view);
	tileManager->ShowDebugCollisionBox(true);

	if (!tileManager->loadTiles())
	{
		std::cout << "Tiles not found\n";
	}
	else
	{
		std::cout << "Tiles loaded\n";
	}

	hudView = window->getDefaultView();

	originalViewSize = v->getSize(); // Store the original size of the view
}

TileEditor::~TileEditor()
{
}

void TileEditor::handleInput(float dt)
{
	if (input->isKeyDown(sf::Keyboard::Tab))
	{
		input->setKeyUp(sf::Keyboard::Tab);
		std::cout << "Exiting edit mode. Saving tiles...\n";
		tileManager->saveTiles(tileManager->getTiles(), tileManager->getFilePath());
		gameState->setCurrentState(State::LEVEL);

		// Reset the view to the original size
		view->setSize(originalViewSize);
		view->setCenter(window->getSize().x / 2, window->getSize().y / 2);
	}
}

void TileEditor::update(float dt)
{
	sf::Vector2f viewSize = sf::Vector2f(window->getSize().x, window->getSize().y);

	TileEditorText.setString(
		"EDIT MODE\n"
		"-Place: LMB\n-Move: Arrow Keys\n"
		"-Scale: IJKL\n-Wall: B\n-Collectable: C\n"
		"-Platform: P\n-Zoom: Q/E\n-Duplicate: Ctrl+D\n"
		"-Save/Exit: E"
	);
	tileManager->handleInput(dt);
	tileManager->update(dt);
	moveView(dt);

	window->setView(hudView);
	// Recalculate the viewSize considering the new zoom level
	sf::Vector2f newViewSize = window->mapPixelToCoords(sf::Vector2i(window->getSize()), hudView) - window->mapPixelToCoords(sf::Vector2i(0, 0), hudView);
	// Adjust the text position to be at the top left of the new viewSize
	TileEditorText.setPosition(hudView.getCenter() - newViewSize * 0.5f);
	window->setView(*view);
}

void TileEditor::render()
{
	beginDraw();
	window->setView(*view);
	tileManager->render(true);

	//ALL the HUD should be drawn after this line
	window->setView(hudView);
	window->draw(TileEditorText);
	endDraw();
}

void TileEditor::beginDraw()
{
	window->clear(sf::Color(100, 149, 237));
}

void TileEditor::endDraw()
{
	window->display();
}

void TileEditor::moveView(float dt)
{
	float speed = 200.0f;
	if (input->isKeyDown(sf::Keyboard::W))
	{
		view->move(0, -speed * dt);
	}
	if (input->isKeyDown(sf::Keyboard::S))
	{
		view->move(0, speed * dt);
	}
	if (input->isKeyDown(sf::Keyboard::A))
	{
		view->move(-speed * dt, 0);
	}
	if (input->isKeyDown(sf::Keyboard::D))
	{
		view->move(speed * dt, 0);
	}
	if (input->isKeyDown(sf::Keyboard::Q))
	{
		view->zoom(1.0005f);
	}
	if (input->isKeyDown(sf::Keyboard::E))
	{
		view->zoom(0.9995f);
	}
	window->setView(*view);
}