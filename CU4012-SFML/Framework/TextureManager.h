#include <SFML/Graphics.hpp>
#include <filesystem>
#include <vector>
#include <unordered_map>
#include <iostream>

namespace fs = std::filesystem;

class TextureManager {
    std::unordered_map<std::string, sf::Texture> textures;
    std::vector<std::string> names;

public:
    void loadTexturesFromDirectory(const std::string& path) {
        fs::path dir_path(path);
        
        if (!fs::exists(dir_path) || !fs::is_directory(dir_path)) {
            std::cerr << "Provided path is not a directory or does not exist." << std::endl;
            return;
        }

        for (const auto& entry : fs::directory_iterator(dir_path)) {
            if (fs::is_regular_file(entry) && hasSupportedExtension(entry.path().extension().string())) {
                sf::Texture texture;
                if (texture.loadFromFile(entry.path().string())) {
                    std::string filename = entry.path().filename().string();
                    textures[filename] = std::move(texture);
                    names.push_back(filename);
                    std::cout << "Loaded texture: " << filename << std::endl;
                }
                else {
					std::cerr << "Failed to load texture: " << entry.path().filename().string() << std::endl;
				}
            }
        }
    }

    bool hasSupportedExtension(const std::string& ext) const {
        return ext == ".png" || ext == ".jpg" || ext == ".jpeg";
    }

    const std::vector<std::string>& getTextureNames() const {
        return names;
    }

    sf::Texture* getTexture(const std::string& name) {
        auto it = textures.find(name);
        if (it != textures.end()) {
            return &it->second;
        }
        return nullptr;
    }
};