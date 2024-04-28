#include "TileManager.h"
#include "Collision.h"
#include "imgui.h"
#include "imgui-SFML.h"
#include "Utilities.h"

TileManager::TileManager()
{
    filePath = "TilesData.txt";
    textureManager.loadTexturesFromDirectory("gfx/TileTextures");
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
            // Clicked on empty space
            if (!(input->isKeyDown(sf::Keyboard::LControl) || input->isKeyDown(sf::Keyboard::RControl))) {
                if (!selectedTileIndices.empty()) {
                    // Clear all current selections
                    for (auto index : selectedTileIndices) {
                        tiles[index]->setEditing(false); // Set all currently selected tiles to not editing
                    }
                    selectedTileIndices.clear();
                    // Set a flag or remember this state to know a clearing has just occurred
                    //recentlyCleared = true;
                }
                else {
                    // Create a new tile only if it was recently cleared and now clicking again on empty space
                    auto newTile = std::make_unique<Tiles>();
                    newTile->setPosition(worldPos.x, worldPos.y);
                    world->AddGameObject(*newTile);
                    tiles.push_back(std::move(newTile));
                    int newIndex = tiles.size() - 1; // Get the index of the newly added tile
                    selectedTileIndices.insert(newIndex); // Select the newly added tile
                    tiles[newIndex]->setEditing(true);
                    recentlyCleared = false; // Reset the flag
                }
            }
            else {
                // If Ctrl is held, only clear selection without adding a new tile
                for (auto index : selectedTileIndices) {
                    tiles[index]->setEditing(false); // Set all currently selected tiles to not editing
                }
                selectedTileIndices.clear();
                //recentlyCleared = false; // Ensure the flag is reset even if Ctrl was held
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
            if(!tilePtr->getTexture()==NULL) window->draw(*tilePtr); // Draw the tile
        }
    }
}


//
//void TileManager::saveTiles(const std::vector<std::unique_ptr<Tiles>>& tiles, const std::string& filePath)
//{
//    std::cout << "Saving tiles to file: " << filePath << std::endl;
//    std::ofstream file(filePath);
//    for (const auto& tile : tiles) {
//        file << tile->getTag() << ","
//
//            << tile->getPosition().x << ","
//            << tile->getPosition().y << ","
//            << tile->getSize().x << ","
//            << tile->getSize().y << ","
//            <<tile->getTrigger() << ","
//            << tile->getStatic() << ","
//            << tile->getMassless() << ","
//            << tile->getTile() << ",""\n";
//    }
//}
void TileManager::saveTiles(const std::vector<std::unique_ptr<Tiles>>& tiles, const std::string& filePath)
{
    std::ofstream file(filePath);
    if (!file.is_open()) {
        std::cout << "Failed to open file for saving tiles." << std::endl;
        return;
    }

    for (const auto& tile : tiles) {
        file << tile->getTag() << ","
            << tile->getPosition().x << ","
            << tile->getPosition().y << ","
            << tile->getSize().x << ","
            << tile->getSize().y << ","
            << tile->getTrigger() << ","
            << tile->getStatic() << ","
            << tile->getMassless() << ","
            << tile->getTile() << ","
            << tile->getTextureName() << "\n";  // Ensure the texture name is always written, even if it's empty
    }
}

bool TileManager::loadTiles()
{
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cout << "Failed to open file for loading tiles." << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::stringstream linestream(line);
        std::string segment;
        std::vector<std::string> seglist;

        while (std::getline(linestream, segment, ',')) {
            seglist.push_back(segment);
        }

        if (seglist.size() >= 9) {
            auto newTile = std::make_unique<Tiles>();
            newTile->setTag(seglist[0]);
            newTile->setPosition(sf::Vector2f(std::stof(seglist[1]), std::stof(seglist[2])));
            newTile->setSize(sf::Vector2f(std::stof(seglist[3]), std::stof(seglist[4])));
            newTile->setTrigger(std::stoi(seglist[5]));
            newTile->setStatic(std::stoi(seglist[6]));
            newTile->setMassless(std::stoi(seglist[7]));
            newTile->setTile(std::stoi(seglist[8]));

            // Check if a texture name exists and is valid
            if (seglist.size() > 9 && !seglist[9].empty()) {
                newTile->setTextureName(seglist[9]);
                sf::Texture* texture = textureManager.getTexture(seglist[9]);
                if (texture) {
                    newTile->setTexture(texture);
                }
            }

            world->AddGameObject(*newTile);
            tiles.push_back(std::move(newTile));
        }
    }

    return true;
}


std::vector<std::unique_ptr<Tiles>>& TileManager::getTiles() {
    return tiles;
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

        if (ImGui::CollapsingHeader("Best Checkbox Combinations"))
        {
            if (ImGui::CollapsingHeader("Checkpoint:"))
            {
                ImGui::Text("Should be set as a Trigger, Static, and Tile.");
                if (ImGui::CollapsingHeader("Explanation:"))
                    ImGui::Text("Checkpoints need to be interacted with but should not\nimpede player movement or be affected by physics.");
            }


            if (ImGui::CollapsingHeader("Coin:"))
            {
                ImGui::Text("Should be set as a Trigger, Massless, and Tile.");
                if (ImGui::CollapsingHeader("Explanation:"))
                    ImGui::Text("Coins should be collectable without affecting the physics\nof the player or game environment.");
            }


            if (ImGui::CollapsingHeader("Moving Platform:"))
            {

                ImGui::Text("Should not be a Trigger or Massless, but Static if not moving vertically.");
                if (ImGui::CollapsingHeader("Explanation:"))
                    ImGui::Text("Platforms that carry the player or objects should interact with \nphysics correctly and not trigger events.");
            }
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

                if (!selectedTileIndices.empty()) {

                    // Buttons for setting properties to common types
                    if (ImGui::Button("Convert to Collectable")) {
                        for (int idx : selectedTileIndices) {
                            auto& tile = *tiles[idx];
                            tile.setMassless(true);
                            tile.setTrigger(true);
                            tile.setTile(true);
                            tile.setStatic(false);
                            tile.setTag("Collectable");
                        }
                    }
                    ImGui::SameLine();
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("Use these settings to convert the selected tile(s) to a Collectable.");
                    }

                    if (ImGui::Button("Convert to Platform")) {
                        for (int idx : selectedTileIndices) {
                            auto& tile = *tiles[idx];
                            tile.setStatic(true);
                            tile.setTile(true);
                            tile.setTrigger(false);
                            tile.setMassless(false);
                            tile.setTag("Platform");
                        }
                    }
                    ImGui::SameLine();
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("Use these settings to convert the selected tile(s) to a Platform.");
                    }

                    if (ImGui::Button("Convert to Checkpoint")) {
                        for (int idx : selectedTileIndices) {
                            auto& tile = *tiles[idx];
                            tile.setStatic(true);
                            tile.setTrigger(true);
                            tile.setTile(true);
                            tile.setMassless(false);
                            tile.setTag("Checkpoint");
                        }
                    }
                    ImGui::SameLine();
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("Use these settings to convert the selected tile(s) to a Checkpoint.");
                    }

                    ImGui::Text("Selected Tiles: %d", (int)selectedTileIndices.size());
                    displayTilePositions();  // Edit positions
                    displayTileScales();     // Edit scales

                    // Display properties if tiles have the same tag or only one is selected
                    if (selectedTileIndices.size() == 1 || allTilesHaveSameTag()) {
                        auto& firstTile = *tiles[*selectedTileIndices.begin()];
                        displayTileProperties(firstTile);
                    }

                    displayTextureSelection(textureManager);
                }
                
                if (ImGui::Button("Add New Tile")) {
                    addNewTile();
                }
                ImGui::SameLine();
                if (ImGui::Button("Delete Selected Tiles")) {
                    deleteSelectedTiles();
                }


                if (ImGui::Button("Save")) {
                    saveTiles(tiles, filePath);
                }


                ImGui::EndTabItem();

                
            }

            if (ImGui::BeginTabItem("Tile Map")) {
                ImGui::Text("Comming Soon........");
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }

        

        ImGui::End();
    }
}


void TileManager::displayTextureSelection(TextureManager& textureManager) {
    if (selectedTileIndices.empty()) return;

    // Assume first selected tile's texture as the default for simplicity
    std::string currentTextureName = tiles[*selectedTileIndices.begin()]->getTextureName();
    const std::vector<std::string>& textureNames = textureManager.getTextureNames();

    // Find the current index based on the texture name
    int current_item = 0; // Default to the first texture if not found
    for (int i = 0; i < textureNames.size(); i++) {
        if (textureNames[i] == currentTextureName) {
            current_item = i;
            break;
        }
    }

    // Start the ImGui combo box
    if (ImGui::BeginCombo("Texture", currentTextureName.c_str())) {
        for (int n = 0; n < textureNames.size(); n++) {
            bool is_selected = (current_item == n);
            if (ImGui::Selectable(textureNames[n].c_str(), is_selected)) {
                // Set the new current item
                current_item = n;
                // Update the texture on all selected tiles
                sf::Texture* selectedTexture = textureManager.getTexture(textureNames[n]);
                for (auto idx : selectedTileIndices) 
                {
                    tiles[idx]->setTexture(selectedTexture,true);
                    tiles[idx]->setTextureName(textureNames[n]);  // Save the texture name
                }
                
            }
            if (is_selected) {
                ImGui::SetItemDefaultFocus();  // Automatically scroll to the selected item
            }
        }
        ImGui::EndCombo();
    }
}



void TileManager::displayTilePositions() {
    if (selectedTileIndices.empty()) return;

    // Compute an average position to start with for simplicity
    sf::Vector2f averagePos(0, 0);
    for (int idx : selectedTileIndices) {
        averagePos += tiles[idx]->getPosition();
    }
    averagePos.x /= selectedTileIndices.size();
    averagePos.y /= selectedTileIndices.size();

    sf::Vector2f newPos = averagePos;
    if (ImGui::DragFloat2("Position", &newPos.x, 0.5f, 0, 0, "%.3f")) {
        sf::Vector2f deltaPos = newPos - averagePos;
        for (int idx : selectedTileIndices) {
            sf::Vector2f currentPos = tiles[idx]->getPosition();
            tiles[idx]->setPosition(currentPos + deltaPos);
        }
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::SetTooltip("Drag or double-click to edit.");
    }
}

void TileManager::displayTileScales() {
    if (selectedTileIndices.empty()) return;

    // Compute an average scale to start with for simplicity
    sf::Vector2f averageScale(0, 0);
    for (int idx : selectedTileIndices) {
        averageScale += tiles[idx]->getSize();
    }
    averageScale.x /= selectedTileIndices.size();
    averageScale.y /= selectedTileIndices.size();

    sf::Vector2f newScale = averageScale;
    if (ImGui::DragFloat2("Scale", &newScale.x, 0.1f, 0.01f, 1000.0f, "%.3f")) {
        sf::Vector2f deltaScale = newScale - averageScale;
        for (int idx : selectedTileIndices) {
            sf::Vector2f currentScale = tiles[idx]->getSize();
            tiles[idx]->setSize(currentScale + deltaScale);
        }
    }
    if (ImGui::IsItemHovered())
    {
		ImGui::SetTooltip("Drag or double-click to edit.");
	}
}


void TileManager::displayTileProperties(Tiles& tile) 
{
    if (selectedTileIndices.empty() || !allTilesHaveSameTag()) return;

    auto& firstTile = *tiles[*selectedTileIndices.begin()];

    char buffer[256];
    strcpy_s(buffer, firstTile.getTag().c_str());
    if (ImGui::InputText("Tag", buffer, sizeof(buffer))) {
        for (int idx : selectedTileIndices) {
            tiles[idx]->setTag(std::string(buffer));
        }
    }
    if (ImGui::IsItemActive())
    {
        inputTextActive = true;
    }
    else
    {
        inputTextActive = false;
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::SetTooltip("Enter Tag, this can be used during collision detection");
    }

    bool istrigger = firstTile.getTrigger();
    bool isstatic = firstTile.getStatic();
    bool ismassless = firstTile.getMassless();
    bool istile = firstTile.getTile();

    displayCheckBox("Trigger", istrigger);
    displayCheckBox("Static", isstatic);
    displayCheckBox("Massless", ismassless);
    displayCheckBox("Tile", istile);
}


bool TileManager::allTilesHaveSameTag() {
    if (selectedTileIndices.size() < 2) return true;
    std::string firstTag = tiles[*selectedTileIndices.begin()]->getTag();
    for (auto idx : selectedTileIndices) {
        if (tiles[idx]->getTag() != firstTag) return false;
    }
    return true;
}

void TileManager::displayCheckBox(const char* label, bool& value) {
    bool currentValue = value;
    if (ImGui::Checkbox(label, &currentValue)) {
        for (int idx : selectedTileIndices) {
            if (strcmp(label, "Trigger") == 0) {
                tiles[idx]->setTrigger(currentValue);
            }
            else if (strcmp(label, "Static") == 0) {
                tiles[idx]->setStatic(currentValue);
            }
            else if (strcmp(label, "Massless") == 0) {
                tiles[idx]->setMassless(currentValue);
            }
            else if (strcmp(label, "Tile") == 0) {
                tiles[idx]->setTile(currentValue);
            }
        }
    }
    if (ImGui::IsItemHovered()) {
        if (strcmp(label, "Trigger") == 0) {
            ImGui::SetTooltip("Mark the tile as a trigger. Triggers do not impede player movement and are often used for items like checkpoints and collectables that execute actions on contact.");
        }
        else if (strcmp(label, "Static") == 0) {
            ImGui::SetTooltip("Set the tile to be static. Static tiles do not move and cannot be affected by physics, suitable for immovable objects like walls.");
        }
        else if (strcmp(label, "Massless") == 0) {
            ImGui::SetTooltip("Make the tile massless. Massless tiles are not affected by gravitational forces and are typically used for items that should not fall or weigh down. Collision is still detected");
        }
        else if (strcmp(label, "Tile") == 0) {
            ImGui::SetTooltip("Mark the object as a tile. Used for general tile properties in the game's level design. Tiles do not collide with other tiles.");
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




