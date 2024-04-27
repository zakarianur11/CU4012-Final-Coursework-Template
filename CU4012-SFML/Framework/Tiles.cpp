#include "Tiles.h"

Tiles::Tiles()
{
	setSize(sf::Vector2f(50, 50));


	setStatic(true);
	setTile(true);
	//setMass(50.f);
	editing = true;
}

void Tiles::update(float dt)
{
	// Set the collision box to be the same as the sprite
	updateCollisionBox(dt);
}

void Tiles::handleInput(float dt)
{

	if (editing)
	{
		// Move the tile
		float moveSpeed = 100.0f; // Speed of movement

		if (input->isKeyDown(sf::Keyboard::Left)) {
			setPosition(getPosition().x - moveSpeed * dt, getPosition().y);
		}
		if (input->isKeyDown(sf::Keyboard::Right)) {
			setPosition(getPosition().x + moveSpeed * dt, getPosition().y);
		}
		if (input->isKeyDown(sf::Keyboard::Up)) {
			setPosition(getPosition().x, getPosition().y - moveSpeed * dt);
		}
		if (input->isKeyDown(sf::Keyboard::Down)) {
			setPosition(getPosition().x, getPosition().y + moveSpeed * dt);
		}
	}
}
