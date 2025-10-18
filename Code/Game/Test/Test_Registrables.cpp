#include "Test_Registrables.hpp"

#include "Engine/Core/Logger/LoggerAPI.hpp"
#include "Engine/Registry/Core/RegisterSubsystem.hpp"

// Implementation file for TestRegistrables
// Currently all methods are inline in the header, but this file is here
// for future expansion and to maintain consistent project structure
void RunTest_Registrables()
{
    using namespace enigma::core;

    LogInfo("App", "=== RegisterSubsystem Test Starting ===");

    auto* registerSubsystem = GEngine->GetSubsystem<RegisterSubsystem>();
    if (!registerSubsystem)
    {
        LogError("App", "RegisterSubsystem not found!");
        return;
    }

    LogInfo("App", "RegisterSubsystem found and initialized");
    LogInfo("App", "Total registrations before test: %zu", registerSubsystem->GetTotalRegistrations());

    // Test 1: Register some blocks
    LogInfo("App", "--- Test 1: Registering TestBlock items ---");

    auto stone = std::make_shared<TestBlock>("stone", 3, false);
    auto glass = std::make_shared<TestBlock>("glass", 1, true);
    auto dirt  = std::make_shared<TestBlock>("dirt", 2, false);

    registerSubsystem->RegisterItem<TestBlock>("stone", stone);
    registerSubsystem->RegisterItem<TestBlock>("game", "glass", glass);
    registerSubsystem->RegisterItem<TestBlock>("engine", "dirt", dirt);

    LogInfo("App", "Registered 3 blocks: stone, game:glass, engine:dirt");
    LogInfo("App", "Total registrations after blocks: %zu", registerSubsystem->GetTotalRegistrations());

    // Test 2: Register some items  
    LogInfo("App", "--- Test 2: Registering TestItem items ---");

    auto sword  = std::make_shared<TestItem>("iron_sword", 1, false);
    auto potion = std::make_shared<TestItem>("health_potion", 16, true);
    auto apple  = std::make_shared<TestItem>("apple", 64, true);

    registerSubsystem->RegisterItem<TestItem>("game", "iron_sword", sword);
    registerSubsystem->RegisterItem<TestItem>("potion", potion);
    registerSubsystem->RegisterItem<TestItem>("engine", "apple", apple);

    LogInfo("App", "Registered 3 items: game:iron_sword, potion, engine:apple");
    LogInfo("App", "Total registrations after items: %zu", registerSubsystem->GetTotalRegistrations());

    // Test 3: Register some entities
    LogInfo("App", "--- Test 3: Registering TestEntity items ---");

    auto player = std::make_shared<TestEntity>("player", 100.0f, 5.0f);
    player->SetNamespace("game");
    auto zombie = std::make_shared<TestEntity>("zombie", 20.0f, 2.0f);
    zombie->SetNamespace("game");
    auto spider = std::make_shared<TestEntity>("spider", 16.0f, 3.0f);
    spider->SetNamespace("game");

    registerSubsystem->RegisterItem<TestEntity>("game", "player", player);
    registerSubsystem->RegisterItem<TestEntity>("game", "zombie", zombie);
    registerSubsystem->RegisterItem<TestEntity>("game", "spider", spider);

    LogInfo("App", "Registered 3 entities: game:player, game:zombie, game:spider");
    LogInfo("App", "Total registrations after entities: %zu", registerSubsystem->GetTotalRegistrations());

    // Test 4: Retrieve and verify items
    LogInfo("App", "--- Test 4: Retrieving and verifying items ---");

    auto retrievedStone = registerSubsystem->GetItem<TestBlock>("stone");
    if (retrievedStone)
    {
        LogInfo("App", "Retrieved stone: hardness=%d, transparent=%s",
                retrievedStone->GetHardness(),
                retrievedStone->IsTransparent() ? "true" : "false");
    }

    auto retrievedGlass = registerSubsystem->GetItem<TestBlock>("game", "glass");
    if (retrievedGlass)
    {
        LogInfo("App", "Retrieved game:glass: hardness=%d, transparent=%s",
                retrievedGlass->GetHardness(),
                retrievedGlass->IsTransparent() ? "true" : "false");
    }

    auto retrievedSword = registerSubsystem->GetItem<TestItem>("game", "iron_sword");
    if (retrievedSword)
    {
        LogInfo("App", "Retrieved game:iron_sword: stack_size=%d, consumable=%s",
                retrievedSword->GetStackSize(),
                retrievedSword->IsConsumable() ? "true" : "false");
    }

    auto retrievedPlayer = registerSubsystem->GetItem<TestEntity>("game", "player");
    if (retrievedPlayer)
    {
        LogInfo("App", "Retrieved game:player: health=%.1f, speed=%.1f, namespace=%s",
                retrievedPlayer->GetHealth(),
                retrievedPlayer->GetSpeed(),
                retrievedPlayer->GetNamespace().c_str());
    }

    // Test 5: List all registries and their contents
    LogInfo("App", "--- Test 5: Listing all registries ---");

    auto allRegistryTypes = registerSubsystem->GetAllRegistryTypes();
    LogInfo("App", "Found %zu registry types:", allRegistryTypes.size());

    for (const auto& typeName : allRegistryTypes)
    {
        auto* registry = registerSubsystem->GetRegistry(typeName);
        if (registry)
        {
            LogInfo("App", "  - %s: %zu items", typeName.c_str(), registry->GetRegistrationCount());

            auto allKeys = registry->GetAllKeys();
            for (const auto& key : allKeys)
            {
                LogInfo("App", "    * %s", key.ToString().c_str());
            }
        }
    }

    // Test 6: Test registry access by type
    LogInfo("App", "--- Test 6: Testing registry access by type ---");

    auto* blockRegistry = registerSubsystem->GetRegistry<TestBlock>();
    if (blockRegistry)
    {
        LogInfo("App", "Block registry has %zu items", blockRegistry->GetRegistrationCount());
        auto allBlocks = blockRegistry->GetAll();
        for (const auto& block : allBlocks)
        {
            LogInfo("App", "  Block: %s (hardness=%d, transparent=%s)",
                    block->GetRegistryName().c_str(),
                    block->GetHardness(),
                    block->IsTransparent() ? "true" : "false");
        }
    }

    LogInfo("App", "=== RegisterSubsystem Test Complete ===");
}
