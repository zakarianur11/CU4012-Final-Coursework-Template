#include "TileManager.h"
#include "Collision.h"

TileManager::TileManager()
{
    filePath = "TilesData.txt";
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
            // If we clicked on a tile while another one was being edited, stop editing the previous one
            if (activeTileIndex != -1 && activeTileIndex != clickedTileIndex) {
                tiles[activeTileIndex]->setEditing(false);
            }
            // Set the clicked tile as the new active tile
            activeTileIndex = clickedTileIndex;
            tiles[activeTileIndex]->setEditing(true);
        }
        else {
            // If we clicked on empty space and no tile is currently selected, create a new tile
            if (activeTileIndex == -1) {
                auto newTile = std::make_unique<Tiles>();
                newTile->setPosition(worldPos.x, worldPos.y);
                world->AddGameObject(*newTile);
                tiles.push_back(std::move(newTile));
                activeTileIndex = tiles.size() - 1; // Select the newly added tile
                tiles[activeTileIndex]->setEditing(true);
            }
            else {
                // If we clicked on empty space, stop editing the current tile
                tiles[activeTileIndex]->setEditing(false);
                activeTileIndex = -1;
            }
        }
        input->setLeftMouse(Input::MouseState::UP); // Mark the mouse click as handled
    }

    // Handle input for the active tile
    if (activeTileIndex != -1) {
        tiles[activeTileIndex]->setInput(input);
        tiles[activeTileIndex]->handleInput(dt);
        // Similar for "Platform"

        // If editing is finished (the tile itself should be responsible for setting this)
        if (!tiles[activeTileIndex]->isEditing()) {
            activeTileIndex = -1;
        }
    }

    // Update the color of the tiles based on selection and tag
    for (int i = 0; i < tiles.size(); ++i) {
        // Check if the tile is the active one and is in editing mode
        if (i == activeTileIndex && tiles[i]->isEditing()) {
            tiles[i]->setColor(sf::Color::Green);
        }
        // Check if the tile's tag is "Walls"
        else if (tiles[i]->getTag() == "Wall") {
            tiles[i]->setColor(sf::Color::Blue);
        }
        // Default color
        else {
            tiles[i]->setColor(sf::Color::Red);
        }
    }
    // Duplicate the selected tile
    if (input->isKeyDown(sf::Keyboard::LControl) || input->isKeyDown(sf::Keyboard::RControl))
    {
        if (input->isKeyDown(sf::Keyboard::D))
        {
            // Ensure we have an active tile to duplicate
            if (activeTileIndex != -1 && tiles.size() > activeTileIndex)
            {
                // Get the properties of the current tile
                sf::Vector2f position = tiles[activeTileIndex]->getPosition();
                sf::Vector2f size = tiles[activeTileIndex]->getSize();
                std::string tag = tiles[activeTileIndex]->getTag();

                // Create the new tile
                auto duplicatedTile = std::make_unique<Tiles>();
                duplicatedTile->setPosition(position);
                duplicatedTile->setSize(size);
                duplicatedTile->setTag(tag);

                // Depending on the tag, set the texture
                if (tag == "Collectable")
                {
                    duplicatedTile->setTexture(&collectableTexture);
                    duplicatedTile->setTrigger(true);
                    duplicatedTile->setStatic(false);
                    duplicatedTile->setMassless(true);
                }
                else if (tag == "Platform")
                {
                    duplicatedTile->setTexture(&platformTexture);
                }
                else if (tag == "Wall")
                {
                    duplicatedTile->setTexture(&wallTexture);
                }

                // Add the new tile to the world and select it
                world->AddGameObject(*duplicatedTile);
                tiles.push_back(std::move(duplicatedTile));
                activeTileIndex = tiles.size() - 1; // Select the newly added tile
                tiles[activeTileIndex]->setEditing(true);

                // Prevent continuous duplication while the key is held down
                input->setKeyUp(sf::Keyboard::D);
            }
        }
    }

    // Delete the selected tile
    if (input->isKeyDown(sf::Keyboard::Delete)) {
        input->setKeyUp(sf::Keyboard::Delete);
        if (activeTileIndex != -1) {
            world->RemoveGameObject(*tiles[activeTileIndex]);
            tiles.erase(tiles.begin() + activeTileIndex);
            activeTileIndex = -1;
        }
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


void TileManager::render(bool editMode)
{

    for (auto& tilePtr : tiles) {
        if (tilePtr) { // Check if the pointer is not null
            if (editMode)
            {
                sf::RectangleShape rect = tilePtr->getDebugCollisionBox();
                rect.setOutlineThickness(5);
                window->draw(rect);

                if (tilePtr->getTag() == "Collectable")
                {
                    window->draw(*tilePtr); // Dereference the pointer to get the Tiles object
                }
                if (tilePtr->getTag() == "Platform")
                {
                    window->draw(*tilePtr); // Dereference the pointer to get the Tiles object
                }
                if (tilePtr->getTag() == "Wall")
                {
                    window->draw(*tilePtr);
                }
            }
            else
            {
                if (tilePtr->getTag() == "Collectable")
                {
                    window->draw(*tilePtr); // Dereference the pointer to get the Tiles object
                }
                if (tilePtr->getTag() == "Platform")
                {
                    window->draw(*tilePtr); // Dereference the pointer to get the Tiles object
                }
                if (tilePtr->getTag() == "Wall")
                {
					window->draw(*tilePtr);
				}
            }
        }
    }
}



void TileManager::saveTiles(const std::vector<std::unique_ptr<Tiles>>& tiles, const std::string& filePath)
{
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

