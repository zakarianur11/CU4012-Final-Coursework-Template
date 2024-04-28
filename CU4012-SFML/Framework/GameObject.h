// Game Object Class
// A class representing a single renderable game object.
// Game Object extend sf::RectangleShape, and added functionailty for collision, sprite states (alive/dead), velocity for movement, input component
// Most elements of your game will extend game object to create larger more complex elements such as player, enemies, bullets etc.

#pragma once
#include <string>
#include <iostream>
#include "SFML\Graphics.hpp"
#include "Input.h"
#include "AudioManager.h"

class GameObject : public sf::RectangleShape
{
public:
	GameObject();
	~GameObject();

	// Virtual update function. Base function does nothing (for static objects). Inheritted version will most likely override this function.
	virtual void handleInput(float dt);
	virtual void update(float dt);

	// Control sprite speed and direction
	void setVelocity(sf::Vector2f vel);
	void setVelocity(float vx, float vy);
	void applyImpulse(sf::Vector2f impulse);

	sf::Vector2f getVelocity();

	// Sprite state
	bool isAlive() { return alive; };
	void setAlive(bool b) { alive = b; };

	// For sprite collision, set collider box, get collider box, and dedicated virtual function for collision response
	bool isColliding() { return Colliding; };
	void setColliding(bool b) { Colliding = b; };
	sf::FloatRect getCollisionBox();
	sf::Vector2f getHalfSize() { return sf::Vector2f(getSize().x / 2, getSize().y / 2); }

	sf::RectangleShape getDebugCollisionBox() { return collisionBoxDebug; }

	std::string getTag() { return tag; }
	bool CollisionWithTag(const std::string& otherTag) { return collidingTag == otherTag; }

	// Set the input component
	void setInput(Input* in) { input = in; };
	void setAudio(AudioManager* aud) { audio = aud; };
	void setWindow(sf::RenderWindow* win) { window = win; };

	std::string getCollisionDirection();


	//Called Every Frame in world class 
	bool checkCollision(GameObject* other);
	void collisionResponse(GameObject* collider);
	void clearCollision() { collidingTag = ""; }
	void UpdatePhysics(sf::Vector2f* gravity, float deltaTime);

	//Collision Types 
	void setTrigger(bool t) { isTrigger = t; }
	bool getTrigger() { return isTrigger; }

	void setTile(bool t) { isTile = t; }
	bool getTile() { return isTile; }

	void setStatic(bool s) { isStatic = s; }
	bool getStatic() { return isStatic; }

	void setMassless(bool m) { isMassless = m; }
	bool getMassless() { return isMassless; }
	float getMass() const
	{
		return isStatic ? std::numeric_limits<float>::infinity() : mass;
	}

	float getInverseMass() const
	{
		// If the object is static, return 0, as the inverse of infinity is 0
		// This represents an immovable object in the physics simulation
		return isStatic ? 0.0f : inverseMass;
	}
	void setColor(sf::Color c) { collisionBoxDebug.setOutlineColor(c); }
	void setTag(const std::string& t) { tag = t; }


	void setTextureName(const std::string& name) { textureName = name; }
	std::string getTextureName() const { return textureName; }

protected:
	// Collision functions
	void setCollisionBox(float x, float y, float width, float height)
	{
		collisionBox = sf::FloatRect(x, y, width, height);
		setDebugCollisionBox(x, y, width, height);
	};
	void setCollisionBox(sf::Vector2f pos, sf::Vector2f size)
	{
		collisionBox = sf::FloatRect(pos.x, pos.y, size.x, size.y);
		setDebugCollisionBox(pos.x, pos.y, size.x, size.y);

	}
	void setCollisionBox(sf::FloatRect fr)
	{
		collisionBox = fr;
		setDebugCollisionBox(fr.left, fr.top, fr.width, fr.height);
	};

	void updateCollisionBox(float dt);
	float restitution = 1;

	void setMass(float m)
	{
		mass = m;
		if (mass == 0) // If mass is 0, the object is static
		{
			isStatic = true;
			inverseMass = 0;
		}
		else //else object is not static 
		{
			isStatic = false;
			inverseMass = 1 / mass;
		}
	}


	void setInertia(float i)
	{
		inertia = i;
		if (i == 0) inverseInertia = 0;
		else inverseInertia = 1 / inertia;
	}
	float getInertia() const { return inertia; }
	float getInverseInertia() const { return inverseInertia; }

	void setAngularVelocity(float av) { angularVelocity = av; }
	float getAngularVelocity() const { return angularVelocity; }

	void setDebugCollisionBox(float x, float y, float w, float h);

	void Jump(float jumpHeight);

	// Sprite properties
	sf::Vector2f velocity;
	bool alive;
	bool canJump;

	// Framework component
	Input* input;
	AudioManager* audio;
	sf::RenderWindow* window;
private:
	bool isStatic;
	bool isTrigger;
	bool isTile;
	bool isMassless;


	//Movement variables
	sf::Vector2f Direction;

	// Physics properties
	sf::Vector2f force;
	sf::Vector2f acceleration;

	float angularVelocity;
	float torque;
	float mass = 1;
	float inverseMass = 1;
	float inertia = 500.f;
	float inverseInertia = 0.002f;

	// Collision vars
	sf::FloatRect collisionBox;
	sf::RectangleShape collisionBoxDebug;
	bool Colliding;

	//Textures
	std::string textureName;

	std::string tag;
	std::string collidingTag;
};