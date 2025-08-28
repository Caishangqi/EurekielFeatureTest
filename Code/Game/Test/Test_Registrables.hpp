#pragma once
#include "Engine/Register/IRegistrable.hpp"
#include <string>
#include <vector>



// Test Block class - represents a simple game block
class TestBlock : public enigma::core::IRegistrable
{
public:
    TestBlock(const std::string& name, int hardness, bool transparent = false)
        : m_name(name), m_hardness(hardness), m_transparent(transparent)
    {
    }

    const std::string& GetRegistryName() const override { return m_name; }

    int  GetHardness() const { return m_hardness; }
    bool IsTransparent() const { return m_transparent; }

    void SetHardness(int hardness) { m_hardness = hardness; }
    void SetTransparent(bool transparent) { m_transparent = transparent; }

private:
    std::string m_name;
    int         m_hardness;
    bool        m_transparent;
};

// Test Item class - represents a game item
class TestItem : public enigma::core::IRegistrable
{
public:
    TestItem(const std::string& name, int stackSize = 64, bool consumable = false)
        : m_name(name), m_stackSize(stackSize), m_consumable(consumable)
    {
    }

    const std::string& GetRegistryName() const override { return m_name; }

    int  GetStackSize() const { return m_stackSize; }
    bool IsConsumable() const { return m_consumable; }

    void SetStackSize(int stackSize) { m_stackSize = stackSize; }
    void SetConsumable(bool consumable) { m_consumable = consumable; }

private:
    std::string m_name;
    int         m_stackSize;
    bool        m_consumable;
};

// Test Entity class - represents a game entity/mob
class TestEntity : public enigma::core::IRegistrable
{
public:
    TestEntity(const std::string& name, float health, float speed)
        : m_name(name), m_health(health), m_speed(speed)
    {
    }

    const std::string& GetRegistryName() const override { return m_name; }
    const std::string& GetNamespace() const override { return m_namespace; }

    float GetHealth() const { return m_health; }
    float GetSpeed() const { return m_speed; }

    void SetHealth(float health) { m_health = health; }
    void SetSpeed(float speed) { m_speed = speed; }
    void SetNamespace(const std::string& ns) { m_namespace = ns; }

private:
    std::string m_name;
    std::string m_namespace = "game";
    float       m_health;
    float       m_speed;
};

// Test Recipe class - represents a crafting recipe
class TestRecipe : public enigma::core::IRegistrable
{
public:
    struct Ingredient
    {
        std::string itemName;
        int         count;

        Ingredient(const std::string& name, int c) : itemName(name), count(c)
        {
        }
    };

    TestRecipe(const std::string& name, const std::string& result, int resultCount = 1)
        : m_name(name), m_result(result), m_resultCount(resultCount)
    {
    }

    const std::string& GetRegistryName() const override { return m_name; }

    void AddIngredient(const std::string& itemName, int count)
    {
        m_ingredients.emplace_back(itemName, count);
    }

    const std::vector<Ingredient>& GetIngredients() const { return m_ingredients; }
    const std::string&             GetResult() const { return m_result; }
    int                            GetResultCount() const { return m_resultCount; }

private:
    std::string             m_name;
    std::string             m_result;
    int                     m_resultCount;
    std::vector<Ingredient> m_ingredients;
};

void RunTest_Registrables();
