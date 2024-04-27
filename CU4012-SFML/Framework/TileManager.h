#pragma once
#include "GameObject.h"
#include "World.h"
#include "Tiles.h"
#include "TextureManager.h"
#include <fstream>
#include <vector>
#include <string>
#include <sstream> // This is required for std::stringstream
#include <set>     // For selecting multiple tiles

class TileManager : public GameObject
{
    std::set<int> selectedTileIndices; // Set to keep track of selected tile indices
    bool recentlyCleared = false; // Used to prevent multiple tiles from being selected at once
    int activeTileIndex = -1; // -1 indicates no tile is actively being edited
    bool tilesLoaded = false;
    //std::vector<Tiles> tiles;

    std::vector<std::unique_ptr<Tiles>> tiles;
    
    TextureManager textureManager;

    std::string filePath; // File to store tile data

    World* world;
    sf::View* view;

    bool showDebugCollisionBox;

    //ImGui variables
    bool stuff;
    float imguiWidth;
    float imguiHeight;
    bool inputTextActive;

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

    void RemoveCollectable();

    void ShowDebugCollisionBox(bool b) { showDebugCollisionBox = b; }

    void DrawImGui();

    void displayTilePositions();
    void displayTileScales();

    bool isInputTextActive() { return inputTextActive; }

    void displayTextureSelection(TextureManager& textureManager);

    bool allTilesHaveSameTag();
    void displayTileProperties(Tiles& tile);
    void displayCheckBox(const char* label, bool& value);
    void addNewTile();
    void deleteSelectedTiles();
};