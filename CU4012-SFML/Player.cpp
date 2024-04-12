#include "Player.h"
Player::Player()
{
	health = 100;
	speed = 200;


	if (!textureLeft.loadFromFile("gfx/mario-left.png"))
	{
		std::cout << "File not found\n";
	}
	if (!textureRight.loadFromFile("gfx/mario-right.png"))
	{
		std::cout << "File not found\n";
	}
	setTexture(&textureRight);

	setSize(sf::Vector2f(100, 100));
	setMass(100.f);
	setTag("Player");
}

void Player::handleInput(float dt)
{
	velocity.x = 0.f;

	setTextureRect(sf::IntRect(0, 0, abs(getTextureRect().width), getTextureRect().height));
	
	// Update velocity based on input
	if (input->isKeyDown(sf::Keyboard::A))
	{
		// Update only the horizontal component, preserving vertical velocity
		velocity.x = -speed;
		setTexture(&textureLeft);
	}
	if (input->isKeyDown(sf::Keyboard::D))
	{
		// Update only the horizontal component, preserving vertical velocity
		velocity.x = speed;
		setTexture(&textureRight);
	}

	if (input->isKeyDown(sf::Keyboard::Space) && canJump)
	{
		Jump(200.f);
	}
}
