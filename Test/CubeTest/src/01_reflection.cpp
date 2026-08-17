#include <iostream>

#include "Cube/Reflection/Serializer.h"
#include "Cube/Reflection/ClassBuilder.h"

using namespace Cube;

class Player {
public:
    std::string name;
    int health;

    Player() {
        std::cout << "Player constructed" << std::endl;
        name = "Alice";
        health = 100;
    }

    void hello() {
        std::cout << "Hello, I am " << name << std::endl;
    }

    void takeDamage(int damage) {
        health -= damage;
        std::cout << name << " took " << damage << " damage, health is now " << health << std::endl;
    }
};

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

void testFunction() {
    ClassBuilder<Player>("Player")
        .property("name", &Player::name)
        .property("health", &Player::health)
        .method("hello", &Player::hello)
        .method("takeDamage", &Player::takeDamage)
        .serializer();

    Class* playerClass = ClassRegistry::get().getClass<Player>();
    Any player = playerClass->createInstance();
    playerClass->getMethod("hello")->invoke(player.getData(), {});
    std::vector<Any> args;
    args.emplace_back(20);
    playerClass->getMethod("takeDamage")->invoke(player.getData(), args);
    playerClass->getProperty("name")->setValue(player.getData(), Any(std::string("Bob")));
    playerClass->getProperty("health")->setValue(player.getData(), Any(200));
    playerClass->getMethod("hello")->invoke(player.getData(), {});
    std::vector<Any> args2;
    args2.emplace_back(50);
    playerClass->getMethod("takeDamage")->invoke(player.getData(), args2);
}

int main() {
    testSerializer();
    testFunction();
    return 0;
}
