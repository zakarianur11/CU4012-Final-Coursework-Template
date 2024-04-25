#pragma once

#include <SFML/Graphics.hpp>
#include "Input.h"
#include "GameState.h"
#include "TileManager.h"
#include "AudioManager.h"
#include "World.h"
#include <string>
#include <iostream>

class BaseLevel
{
public:
	BaseLevel();
	~BaseLevel();

	virtual void handleInput(float dt) = 0;
	virtual void update(float dt) = 0;
	virtual void render() = 0;

protected:
	// Default variables for level class.
	sf::RenderWindow* window;
	Input* input;
	GameState* gameState;
	World* world;
	sf::View* view;
	TileManager* tileManager;
	AudioManager* audioManager;
};

