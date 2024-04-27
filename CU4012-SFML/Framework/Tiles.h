#pragma once
#include "GameObject.h"
class Tiles :
    public GameObject
{
    bool editing; // To track editing mode
public:
    Tiles();

    void handleInput(float dt) override;
    void update(float dt) override;
    void setEditing(bool edit) {
		editing = edit;
	}
    bool isEditing() {
		return editing;
	}
};

