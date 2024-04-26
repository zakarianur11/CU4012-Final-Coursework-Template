#include "TileManager.h"
#include "Collision.h"
#include "imgui.h"
#include "imgui-SFML.h"
#include "Utilities.h"

TileManager::TileManager()
{
    filePath = "TilesData.txt";

    // Set up ImGui variables
    imguiWidth = SCREEN_WIDTH / 4;
    imguiHeight = SCREEN_HEIGHT;
}

void TileManager::handleInput(float dt)
{
    sf::Vector2i pixelPos = sf::Vector2i(input->getMouseX(), input->getMouseY());
    sf::Vector2f worldPos = window->mapPixelToCoords(pixelPos, *view);

    if (input->isLeftMouseDown()) {
        bool tileClicked = false;
        int clickedTileIndex = -1;

        // Check if any tile is clicked
        for (int i = 0; i < tiles.size(); ++i) {
            if (Collision::checkBoundingBox(tiles[i]->getCollisionBox(), sf::Vector2i(worldPos))) {
                tileClicked = true;
                clickedTileIndex = i;
                break;
            }
        }

        if (tileClicked) {
            if (input->isKeyDown(sf::Keyboard::LControl) || input->isKeyDown(sf::Keyboard::RControl)) {
                // Ctrl is held, toggle the selection state of the tile
                if (selectedTileIndices.find(clickedTileIndex) == selectedTileIndices.end()) {
                    selectedTileIndices.insert(clickedTileIndex); // Add to selection
                    tiles[clickedTileIndex]->setEditing(true);
                }
                else {
                    selectedTileIndices.erase(clickedTileIndex); // Remove from selection
                    tiles[clickedTileIndex]->setEditing(false);
                }
            }
            else {
                // No Ctrl key, clear existing selections and select the new tile only
                for (auto index : selectedTileIndices) {
                    tiles[index]->setEditing(false); // Set all currently selected tiles to not editing
                }
                selectedTileIndices.clear();
                selectedTileIndices.insert(clickedTileIndex);
                tiles[clickedTileIndex]->setEditing(true);
            }
        }
        else {
            // Clicked on empty space, clear selection if Ctrl is not held
            if (!(input->isKeyDown(sf::Keyboard::LControl) || input->isKeyDown(sf::Keyboard::RControl))) {
                for (auto index : selectedTileIndices) {
                    tiles[index]->setEditing(false); // Set all currently selected tiles to not editing
                }
                selectedTileIndices.clear();
            }
        }
        input->setLeftMouse(Input::MouseState::UP); // Mark the mouse click as handled
    }

    // Handle input for the active tiles
    for (int index : selectedTileIndices) {
        tiles[index]->setInput(input);
        tiles[index]->handleInput(dt);
    }

    // Update the color of the tiles based on selection and tag
    for (int i = 0; i < tiles.size(); ++i) {
        if (selectedTileIndices.find(i) != selectedTileIndices.end()) {
            tiles[i]->setColor(sf::Color::Green); // Highlight selected tiles
        }
        else if (tiles[i]->getTag() == "Wall") {
            tiles[i]->setColor(sf::Color::Blue);
        }
        else {
            tiles[i]->setColor(sf::Color::Red);
        }
    }

    // Additional functionality like duplication and deletion...

    // Duplication
    if (input->isKeyDown(sf::Keyboard::LControl) || input->isKeyDown(sf::Keyboard::RControl)) {
        if (input->isKeyDown(sf::Keyboard::D)) {
            // Duplicate all selected tiles
            std::vector<std::unique_ptr<Tiles>> newTiles; // Temporarily store new tiles to avoid modifying the collection while iterating
            for (int index : selectedTileIndices) {
                auto& tile = tiles[index];
                auto duplicatedTile = std::make_unique<Tiles>();
                duplicatedTile->setPosition(tile->getPosition());
                duplicatedTile->setSize(tile->getSize());
                duplicatedTile->setTag(tile->getTag());
                duplicatedTile->setTexture(tile->getTexture()); // Ensure this method exists and works correctly
                duplicatedTile->setTrigger(tile->getTrigger());
                duplicatedTile->setStatic(tile->getStatic());
                duplicatedTile->setMassless(tile->getMassless());
                newTiles.push_back(std::move(duplicatedTile));
            }

            // Add new tiles to the main collection and select them
            for (auto& newTile : newTiles) {
                world->AddGameObject(*newTile);
                int newIndex = tiles.size();
                tiles.push_back(std::move(newTile));
                selectedTileIndices.insert(newIndex); // Select new tiles
            }

            input->setKeyUp(sf::Keyboard::D); // Prevent continuous duplication while the key is held down
        }
    }

    //Deletion
    if (input->isKeyDown(sf::Keyboard::Delete)) {
        // Sort selected indices in descending order to safely delete multiple tiles without invalidating indices
        std::vector<int> sortedIndices(selectedTileIndices.begin(), selectedTileIndices.end());
        std::sort(sortedIndices.rbegin(), sortedIndices.rend()); // Reverse sort

        for (int index : sortedIndices) {
            if (index >= 0 && index < tiles.size()) {
                world->RemoveGameObject(*tiles[index]);
                tiles.erase(tiles.begin() + index);
            }
        }
        selectedTileIndices.clear(); // Clear selection after deletion
        input->setKeyUp(sf::Keyboard::Delete); // Prevent continuous deletion while the key is held down
    }
}
void TileManager::update(float dt)
{
    for (auto& tilePtr : tiles) {
        if (tilePtr) { // Check if the pointer is not null
            tilePtr->update(dt); // Dereference the pointer to get the Tiles object
        }
    }
}

void TileManager::render(bool editMode) {
    for (auto& tilePtr : tiles) {
        if (tilePtr) { // Check if the pointer is not null
            if (editMode) {
                sf::RectangleShape rect = tilePtr->getDebugCollisionBox();
                rect.setOutlineThickness(5);
                int tileIndex = &tilePtr - &tiles[0]; // Get index of the tile

                // Highlight selected tiles
                if (selectedTileIndices.find(tileIndex) != selectedTileIndices.end()) {
                    rect.setOutlineColor(sf::Color::Green);
                } else {
                    rect.setOutlineColor(sf::Color::Red);
                }
                
                window->draw(rect);
            }
            window->draw(*tilePtr); // Draw the tile
        }
    }
}



void TileManager::saveTiles(const std::vector<std::unique_ptr<Tiles>>& tiles, const std::string& filePath)
{
    std::cout << "Saving tiles to file: " << filePath << std::endl;
    std::ofstream file(filePath);
    for (const auto& tile : tiles) {
        file << tile->getTag() << ","
            << tile->getPosition().x << ","
            << tile->getPosition().y << ","
            << tile->getSize().x << ","
            << tile->getSize().y << "\n";
    }
}

bool TileManager::loadTiles()
{
    if (tiles.empty())
    {
        std::ifstream file(filePath);

        if (!file.is_open()) {
            return false;
        }
        std::string line;

        while (std::getline(file, line)) {
            std::stringstream linestream(line);
            std::string segment;
            std::vector<std::string> seglist;

            while (std::getline(linestream, segment, ',')) {
                //std::cout << segment << std::endl;
                seglist.push_back(segment);
            }

            if (seglist.size() >= 5) {
                // Assuming segment order is type, posX, posY, sizeX, sizeY
                auto newTile = std::make_unique<Tiles>();
                //tile.setTag(std::stoi(seglist[0])); // Ensure this matches your type representation
                newTile->setTag(seglist[0]);
                newTile->setPosition(sf::Vector2f(std::stof(seglist[1]), std::stof(seglist[2])));
                newTile->setSize(sf::Vector2f(std::stof(seglist[3]), std::stof(seglist[4])));
                if (newTile->getTag() == "Collectable")
                {
                    newTile->setTexture(&collectableTexture);
                    newTile->setTrigger(true);
                    newTile->setStatic(false);
                    newTile->setMassless(true);
                }
                if (newTile->getTag() == "Platform")
                {
					newTile->setTexture(&platformTexture);
				}
                if (newTile->getTag() == "Wall")
                {
                    setTexture(&wallTexture);
                }
                world->AddGameObject(*newTile);
                tiles.push_back(std::move(newTile));
                //std::cout<<"Tiles: "<<tiles.size()<<std::endl;  
            }

        }

        return true;
    }

    return false;
}

std::vector<std::unique_ptr<Tiles>>& TileManager::getTiles() {
    return tiles;
}

void TileManager::setCollectableTexture(std::string path)
{
    if (!collectableTexture.loadFromFile(path))
    {
        std::cout << "Tile Manager file not found\n";
    }
}

void TileManager::setPlatformTexture(std::string path)
{
    if (!platformTexture.loadFromFile(path))
    {
        std::cout << "Tile Manager file not found\n";
    }
}

void TileManager::setWallTexture(std::string path)
{
    if (!wallTexture.loadFromFile(path))
    {
		std::cout << "Tile Manager file not found\n";
	}
}

void TileManager::RemoveCollectable()
{
    auto newEnd = std::remove_if(tiles.begin(), tiles.end(),
        [this](const std::unique_ptr<Tiles>& tilePtr) -> bool
        {
            if (tilePtr->CollisionWithTag("Player") && tilePtr->getTag()== "Collectable")
            {
                world->RemoveGameObject(*tilePtr);
                return true; // Mark for removal
            }
            return false; // Keep in the vector
        });

    tiles.erase(newEnd, tiles.end());
}

void TileManager::DrawImGui() {
    ImVec2 imguiSize(imguiWidth, imguiHeight);
    ImVec2 imguiPos(SCREEN_WIDTH - imguiWidth, 0); // Positioned on the right-hand side

    // Set the window size
    ImGui::SetNextWindowSize(imguiSize);

    // Set the window position
    ImGui::SetNextWindowPos(imguiPos);

    // Window flags
    ImGuiWindowFlags window_flags = 0;
    window_flags |= ImGuiWindowFlags_NoMove;          // The window will not be movable
    window_flags |= ImGuiWindowFlags_NoResize;        // Disable resizing
    window_flags |= ImGuiWindowFlags_NoCollapse;      // Disable collapsing
    //window_flags |= ImGuiWindowFlags_NoTitleBar;      // Disable the title bar
    //window_flags |= ImGuiWindowFlags_NoScrollbar;     // Disable the scrollbar

    if (ImGui::Begin("Tile Editor", nullptr, window_flags)) 
    {
        if (ImGui::CollapsingHeader("Help"))
        {
            ImGui::Text("Left Click: Place Tile");
            ImGui::Text("Right Click and Drag: Move Camera");
            ImGui::Text("Delete: Delete Tile");
            ImGui::Text("Ctrl+D: Duplicate Tile");
            ImGui::Text("Tab: Save and Exit");
        }

        if (ImGui::BeginTabBar("Tile Editor Tabs")) {
            if (ImGui::BeginTabItem("Tiles")) {
                // Tiles List
                if (ImGui::BeginListBox("Tile List")) {
                    for (int i = 0; i < tiles.size(); i++) {
                        std::string item_label = tiles[i]->getTag().empty() ? "Tile" + std::to_string(i) : tiles[i]->getTag();
                        item_label += "##" + std::to_string(i);

                        bool isSelected = selectedTileIndices.find(i) != selectedTileIndices.end();
                        if (ImGui::Selectable(item_label.c_str(), isSelected)) {
                            if (ImGui::GetIO().KeyCtrl) {
                                // Toggle selection with Ctrl pressed
                                if (isSelected) {
                                    selectedTileIndices.erase(i);
                                }
                                else {
                                    selectedTileIndices.insert(i);
                                }
                            }
                            else {
                                // Single selection
                                selectedTileIndices.clear();
                                selectedTileIndices.insert(i);
                            }
                        }
                    }
                    ImGui::EndListBox();
                }

                // Display properties if tiles have the same tag or only one is selected
                if (!selectedTileIndices.empty() && (selectedTileIndices.size() == 1 || allTilesHaveSameTag())) {
                    auto& firstTile = *tiles[*selectedTileIndices.begin()];

                    ImGui::Text("Selected Tiles: %d", (int)selectedTileIndices.size());
                    if (selectedTileIndices.size() == 1 || allTilesHaveSameTag()) { 
                        displayTileProperties(firstTile);
                    }
                }

                if (ImGui::Button("Add New Tile")) {
                    addNewTile();
                }
                ImGui::SameLine();
                if (ImGui::Button("Delete Selected Tiles")) {
                    deleteSelectedTiles();
                }

                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Settings")) {
                ImGui::Text("General settings for the tile editor.");
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }

        if (ImGui::Button("Save")) {
            saveTiles(tiles, filePath);
        }

        ImGui::End();
    }
}

bool TileManager::allTilesHaveSameTag() {
    std::string firstTag = tiles[*selectedTileIndices.begin()]->getTag();
    for (auto idx : selectedTileIndices) {
        if (tiles[idx]->getTag() != firstTag) return false;
    }
    return true;
}

void TileManager::displayTileProperties(Tiles& tile) {

    if (ImGui::Button("Collectable##CollectableButton")) {
        tile.setTexture(&collectableTexture);
        tile.setTrigger(true);
        tile.setStatic(false);
        tile.setMassless(true);
        tile.setTag("Collectable");
    }
    // Assuming Tile properties like position, size, tag are standard across all selected tiles if they have the same tag
    sf::Vector2f pos = tile.getPosition();
    if (ImGui::DragFloat2("Position", &pos.x)) {
        for (int idx : selectedTileIndices) {
            tiles[idx]->setPosition(pos);
        }
    }

    sf::Vector2f size = tile.getSize();
    if (ImGui::DragFloat2("Size", &size.x)) {
        for (int idx : selectedTileIndices) {
            tiles[idx]->setSize(size);
        }
    }

    // Tag, Trigger, Static, Massless, Tile settings
    char buffer[256];
    strcpy_s(buffer, tile.getTag().c_str());
    if (ImGui::InputText("Tag", buffer, sizeof(buffer))) {
        for (int idx : selectedTileIndices) {
            tiles[idx]->setTag(std::string(buffer));
        }
    }
    bool isTrigger = tile.getTrigger();
    bool isStatic = tile.getStatic();
    bool isMassless = tile.getMassless();
    bool isTile = tile.getTile();


    displayCheckBox("Trigger", isTrigger);
    displayCheckBox("Static", isStatic);
    displayCheckBox("Massless", isMassless);
    displayCheckBox("Tile", isTile);
}

void TileManager::displayCheckBox(const char* label, bool& value) {
    if (ImGui::Checkbox(label, &value)) {
        for (int idx : selectedTileIndices) {
            // Assuming these methods exist and are appropriately set
            if (label == std::string("Trigger")) tiles[idx]->setTrigger(value);
            if (label == std::string("Static")) tiles[idx]->setStatic(value);
            if (label == std::string("Massless")) tiles[idx]->setMassless(value);
            if (label == std::string("Tile")) tiles[idx]->setTile(value);
        }
    }
}

void TileManager::addNewTile() {
    auto newTile = std::make_unique<Tiles>();
    newTile->setPosition(0, 0);  // Default position
    world->AddGameObject(*newTile);
    tiles.push_back(std::move(newTile));
    selectedTileIndices.clear();
    selectedTileIndices.insert(tiles.size() - 1);
}

void TileManager::deleteSelectedTiles() {
    std::vector<int> sortedIndices(selectedTileIndices.begin(), selectedTileIndices.end());
    std::sort(sortedIndices.rbegin(), sortedIndices.rend());
    for (int idx : sortedIndices) {
        world->RemoveGameObject(*tiles[idx]);
        tiles.erase(tiles.begin() + idx);
    }
    selectedTileIndices.clear();
}




