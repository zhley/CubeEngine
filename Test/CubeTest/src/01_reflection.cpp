#include <iostream>

#include "Cube/Reflection/Serializer.h"

using namespace Cube;

void testSerializer() {
    registerBasicSerializers();
    registerSerializer<std::unordered_map<std::string, std::string>>();

    std::unordered_map<std::string, std::string> myMap = {
        {"key1", "value1"},
        {"key2", "value2"}
    };
    nlohmann::json j = Serializer::get().serialize(getTypeID<std::unordered_map<std::string, std::string>>(), Any(myMap));
    std::cout << j.dump(4) << std::endl;
}

int main() {
    testSerializer();
    return 0;
}
