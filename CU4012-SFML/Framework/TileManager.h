#pragma once
#include "GameObject.h"
#include "World.h"
#include "Tiles.h"
#include <fstream>
#include <vector>
#include <string>
#include <sstream> // This is required for std::stringstream


class TileManager :
    public GameObject
{
    int activeTileIndex = -1; // -1 indicates no tile is actively being edited
    bool tilesLoaded = false;
    //std::vector<Tiles> tiles;

    std::vector<std::unique_ptr<Tiles>> tiles;

    std::string filePath; // File to store tile data

    World* world;
    sf::View* view;

    sf::Texture collectableTexture;
    sf::Texture platformTexture;
    sf::Texture wallTexture;

    bool showDebugCollisionBox;

public:
    TileManager();

    void update(float dt) override;
    void handleInput(float dt) override;
    void render(bool editMode);

    void saveTiles(const std::vector<std::unique_ptr<Tiles>>& tiles, const std::string& filePath);
    bool loadTiles();

    std::vector<std::unique_ptr<Tiles>>& getTiles();

    void setWorld(World* world) { this->world = world; }
    void setView(sf::View* view) { this->view = view; }

    std::string getFilePath() { return filePath; }

    void setCollectableTexture(std::string path);

    void setPlatformTexture(std::string path);

    void setWallTexture(std::string path);

    void RemoveCollectable();

    void ShowDebugCollisionBox(bool b) { showDebugCollisionBox = b; }
};