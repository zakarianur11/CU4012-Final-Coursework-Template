#pragma once
#include "Framework/GameObject.h"
class Player :
    public GameObject
{

	int health;
	float speed;
	sf::Texture textureLeft;
	sf::Texture textureRight;
	int numberOfCollectables;

public:
	Player();

	void handleInput(float dt);

	void addCollectable(int c) { numberOfCollectables += c; }
	int getCollectablesCount() { return numberOfCollectables; }
};

