#pragma once
#include <SFML/Graphics.hpp>
#include "Framework/BaseLevel.h"
#include "Framework/Input.h"
#include "Framework/GameState.h"
#include "Framework/World.h"
#include "Framework/TileManager.h"
#include <string>
#include <iostream>

class TileEditor
{
public:
	TileEditor(sf::RenderWindow* hwnd, Input* in, GameState* game, sf::View* view, World* w, TileManager* tm);
	~TileEditor();

	void handleInput(float dt);
	void update(float dt);
	void render();
	void moveView(float dt);
	void beginDraw();
	void endDraw();

private:
	// Default variables for level class.
	sf::RenderWindow* window;
	sf::View* view;
	Input* input;
	GameState* gameState;
	World* world;

	sf::View hudView;
	TileManager* tileManager;
	sf::Text TileEditorText;
	sf::Font font;
	
	sf::Vector2f originalViewSize;
};

