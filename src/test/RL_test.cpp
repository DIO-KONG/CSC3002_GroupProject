#include "ResourceLoader.hpp"
#include <iostream>
#include <variant>

int main()
{
    // Create ResourceLoader object and load menu.json
    ResourceLoader loader("config/menu.json");
    
    // Test basic property reading
    std::cout << "=== Test Basic Property Reading ===" << std::endl;
    
    auto name = loader.getResource("name");
    if (std::holds_alternative<std::string>(name)) {
        std::cout << "name: " << std::get<std::string>(name) << std::endl;
    }
    
    auto music = loader.getResource("music");
    if (std::holds_alternative<std::string>(music)) {
        std::cout << "music: " << std::get<std::string>(music) << std::endl;
    }
    
    auto gravityX = loader.getResource("gravityX");
    if (std::holds_alternative<float>(gravityX)) {
        std::cout << "gravityX: " << std::get<float>(gravityX) << std::endl;
    }
    
    auto gravityY = loader.getResource("gravityY");
    if (std::holds_alternative<float>(gravityY)) {
        std::cout << "gravityY: " << std::get<float>(gravityY) << std::endl;
    }
    
    // Test getting object key list
    std::cout << "\n=== Test Getting Object Key List ===" << std::endl;
    auto objKeys = loader.getObjKeys();
    std::cout << "objKeys count: " << objKeys.size() << std::endl;
    for (const auto& key : objKeys) {
        std::cout << "  - " << key << std::endl;
        // Add each key to objKeyList so getObjCount() can find it
        loader.addObjKey(key);
    }
    
    // Test getting GraphicObj object count
    std::cout << "\n=== Test Getting Object Count ===" << std::endl;
    int objCount = loader.getObjCount("GraphicObj");
    std::cout << "GraphicObj object count: " << objCount << std::endl;
    
    // Test reading GraphicObj array objects
    std::cout << "\n=== Test Reading GraphicObj Objects ===" << std::endl;
    if (objCount > 0) {
        for (int i = 0; i < objCount; i++) {
            std::cout << "\n--- GraphicObj[" << i << "] ---" << std::endl;
            
            // Get all properties
            auto allResources = loader.getAllObjResources(i, "GraphicObj");
            for (const auto& [key, value] : allResources) {
                std::cout << key << ": ";
                if (std::holds_alternative<std::string>(value)) {
                    std::cout << std::get<std::string>(value);
                } else if (std::holds_alternative<int>(value)) {
                    std::cout << std::get<int>(value);
                } else if (std::holds_alternative<float>(value)) {
                    std::cout << std::get<float>(value);
                } else if (std::holds_alternative<bool>(value)) {
                    std::cout << (std::get<bool>(value) ? "true" : "false");
                }
                std::cout << std::endl;
            }
        }
    }
    
    // Test getting specific property individually
    std::cout << "\n=== Test Getting Specific Property ===" << std::endl;
    auto buttonType = loader.getObjResources(0, "GraphicObj", "type");
    if (std::holds_alternative<std::string>(buttonType)) {
        std::cout << "First object type: " << std::get<std::string>(buttonType) << std::endl;
    }
    
    auto buttonX = loader.getObjResources(0, "GraphicObj", "x");
    if (std::holds_alternative<int>(buttonX)) {
        std::cout << "First object x: " << std::get<int>(buttonX) << std::endl;
    }
    
    return 0;
}
