#include "ResourceLoader.hpp"

ResourceLoader::ResourceLoader(const std::string& jsonFilePath)
{
    // 构造函数逻辑
    std::ifstream file(jsonFilePath);
    if (!file.is_open())
    {
        // 处理文件打开失败的情况
        // 向Error类发出报错请求（待实现）
        return;
    }
    file >> levelJson;
    file.close();
}

ResourceLoader::~ResourceLoader()
{
    // 析构函数逻辑
}

ResourceLoader::ResourceValue ResourceLoader::getResource(const std::string& key) const
{
    // ResourceLoader类根据键获取资源值的实现
    if (levelJson.contains(key))
    {
        const auto& jsonValue = levelJson[key];
        if (jsonValue.is_boolean())
        {
            return jsonValue.get<bool>();
        }
        else if (jsonValue.is_number_float())
        {
            return jsonValue.get<float>();
        }
        else if (jsonValue.is_number_integer())
        {
            return jsonValue.get<int>();
        }
        else if (jsonValue.is_string())
        {
            return jsonValue.get<std::string>();
        }
    }
    return std::monostate{};
}

std::vector<std::string> ResourceLoader::getObjKeys() const
{
    // 获取对象容器Key列表的实现
    std::vector<std::string> objKeyList;
    if (levelJson.contains("objKeys") && levelJson["objKeys"].is_array())
    {
        for (const auto& key : levelJson["objKeys"])
        {
            if (key.is_string())
            {
                objKeyList.push_back(key.get<std::string>());
            }
        }
    }
    return objKeyList;
}

void ResourceLoader::addObjKey(const std::string& keyName)
{
    // 添加对象容器Key的实现
    objKeyList.push_back(keyName);
}

int ResourceLoader::getObjCount(const std::string& keyName) const
{
    // 获取容器内对象个数的实现
    // key不是在objKeyList中则返回-1
    if (std::find(objKeyList.begin(), objKeyList.end(), keyName) == objKeyList.end())
    {
        return -1;
    }
    if (levelJson.contains(keyName) && levelJson[keyName].is_array())
    {
        return static_cast<int>(levelJson[keyName].size());
    }
    return 0;
}

ResourceLoader::ResourceValue ResourceLoader::getObjResources(
    const int index, 
    const std::string& objKey, 
    const std::string& valueKey) const
{
    // 获取容器内对象数据的实现
    if (levelJson.contains(objKey) && levelJson[objKey].is_array())
    {
        const auto& objArray = levelJson[objKey];
        if (index >= 0 && index < static_cast<int>(objArray.size()))
        {
            const auto& obj = objArray[index];
            if (obj.contains(valueKey))
            {
                const auto& jsonValue = obj[valueKey];
                if (jsonValue.is_boolean())
                {
                    return jsonValue.get<bool>();
                }
                else if (jsonValue.is_number_float())
                {
                    return jsonValue.get<float>();
                }
                else if (jsonValue.is_number_integer())
                {
                    return jsonValue.get<int>();
                }
                else if (jsonValue.is_string())
                {
                    return jsonValue.get<std::string>();
                }
            }
        }
    }
    return std::monostate{};
}

ResourceLoader::ResourceDict ResourceLoader::getAllObjResources(
    const int index, 
    const std::string& objKey) const
{
    // 获取容器内对象的所有数据的实现
    ResourceDict resources;
    if (levelJson.contains(objKey) && levelJson[objKey].is_array())
    {
        const auto& objArray = levelJson[objKey];
        if (index >= 0 && index < static_cast<int>(objArray.size()))
        {
            const auto& obj = objArray[index];
            for (auto it = obj.begin(); it != obj.end(); ++it)
            {
                const std::string& key = it.key();
                const auto& jsonValue = it.value();
                if (jsonValue.is_boolean())
                {
                    resources[key] = jsonValue.get<bool>();
                }
                else if (jsonValue.is_number_float())
                {
                    resources[key] = jsonValue.get<float>();
                }
                else if (jsonValue.is_number_integer())
                {
                    resources[key] = jsonValue.get<int>();
                }
                else if (jsonValue.is_string())
                {
                    resources[key] = jsonValue.get<std::string>();
                }
            }
        }
    }
    return resources;
}