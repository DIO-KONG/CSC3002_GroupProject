#pragma once
#include <unordered_map>
#include <variant>
#include <string>
#include <memory>
#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class ResourceLoader
{
    using ResourceValue = std::variant<std::monostate, bool, int, float, std::string>;
    using ResourceDict = std::unordered_map<std::string, ResourceValue>;

    public:
        ResourceLoader(const std::string& jsonFilePath);
        ~ResourceLoader();
        // 根据键获取资源值
        ResourceValue getResource(const std::string& key) const;
        // 获取标记为对象容器的Key的列表
        std::vector<std::string> getObjKeys() const;
        // 设定标记为对象容器的Key
        void addObjKey(const std::string& keyName);
        // 获取容器内对象个数
        int getObjCount(const std::string& keyName) const;
        // 获取容器内对象数据
        ResourceValue getObjResources(const int index, const std::string& objKey, const std::string& valueKey) const;
        // 获取容器内对象的所有数据
        ResourceDict getAllObjResources(const int index, const std::string& objKey) const;

    
    private:
        // 存储容器Key的列表
        std::vector<std::string> objKeyList;
        // json对象
        json levelJson;
};