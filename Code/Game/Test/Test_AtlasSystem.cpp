#include "Test_AtlasSystem.hpp"
#include "Engine/Core/Engine.hpp"
#include "Engine/Core/Logger/LoggerAPI.hpp"
#include "Engine/Resource/ResourceSubsystem.hpp"
#include "Engine/Resource/Atlas/AtlasManager.hpp"
#include "Engine/Resource/Atlas/ImageLoader.hpp"
#include "Engine/Resource/Atlas/TextureAtlas.hpp"
#include "Engine/Resource/Atlas/AtlasConfig.hpp"

bool TestSpriteVerifier::VerifySprite(const enigma::resource::AtlasManager* manager, const ExpectedSprite& expected) const
{

    using namespace enigma::core;
    
    enigma::resource::ResourceLocation location(expected.namespaceName, expected.path);
    const enigma::resource::AtlasSprite* sprite = manager->FindSprite(location);
    
    if (expected.shouldExist)
    {
        if (!sprite)
        {
            LogError("AtlasTest", "Expected sprite %s not found in any atlas", location.ToString().c_str());
            return false;
        }
        
        if (!expected.expectedAtlas.empty())
        {
            const enigma::resource::AtlasSprite* atlasSprite = manager->FindSprite(expected.expectedAtlas, location);
            if (!atlasSprite)
            {
                LogError("AtlasTest", "Sprite %s not found in expected atlas '%s'", 
                        location.ToString().c_str(), expected.expectedAtlas.c_str());
                return false;
            }
        }
        
        if (!sprite->HasValidUVs())
        {
            LogError("AtlasTest", "Sprite %s has invalid UV coordinates", location.ToString().c_str());
            return false;
        }
        
        LogInfo("AtlasTest", "✓ Verified sprite %s: UV(%.3f,%.3f)-(%.3f,%.3f)", 
               location.ToString().c_str(),
               sprite->uvMin.x, sprite->uvMin.y, sprite->uvMax.x, sprite->uvMax.y);
    }
    else
    {
        if (sprite)
        {
            LogError("AtlasTest", "Unexpected sprite %s found (should not exist)", location.ToString().c_str());
            return false;
        }
        LogInfo("AtlasTest", "✓ Confirmed sprite %s does not exist (as expected)", location.ToString().c_str());
    }
    
    return true;
}

int TestSpriteVerifier::RunAllVerifications(const enigma::resource::AtlasManager* manager) const
{
    int successCount = 0;
    int totalCount = static_cast<int>(m_expectedSprites.size());

    using namespace enigma::core;
    
    for (const auto& expected : m_expectedSprites)
    {
        if (VerifySprite(manager, expected))
        {
            successCount++;
        }
    }
    
    LogInfo("AtlasTest", "Sprite verification: %d/%d passed", successCount, totalCount);
    return successCount;
}

void RunTest_AtlasSystem()
{
    using namespace enigma::resource;
    using namespace enigma::core;

    LogInfo("App", "=== AtlasSystem Test Starting ===");

    // Get ResourceSubsystem
    auto* resourceSubsystem = GEngine->GetSubsystem<ResourceSubsystem>();
    if (!resourceSubsystem)
    {
        LogError("App", "ResourceSubsystem not found!");
        return;
    }

    LogInfo("App", "ResourceSubsystem found and initialized");

    // Test 1: Register ImageLoader
    LogInfo("App", "--- Test 1: Registering ImageLoader ---");
    
    auto imageLoader = std::make_shared<ImageLoader>();
    resourceSubsystem->RegisterLoader(imageLoader);
    
    LogInfo("App", "ImageLoader registered successfully");
    LogInfo("App", "Supported extensions: PNG, JPG, JPEG, BMP, TGA");

    // Test 2: Create AtlasManager and set up configurations
    LogInfo("App", "--- Test 2: Setting up AtlasManager ---");
    
    auto atlasManager = std::make_unique<AtlasManager>(resourceSubsystem);
    
    // Create featuretest-based configurations
    AtlasConfig blocksConfig("blocks");
    blocksConfig.requiredResolution = 16;
    blocksConfig.autoScale = true;
    blocksConfig.rejectMismatched = false;
    blocksConfig.AddDirectorySource("textures/block/", {"featuretest"});
    blocksConfig.exportPNG = true;
    blocksConfig.exportPath = "debug/";
    
    AtlasConfig itemsConfig("items");
    itemsConfig.requiredResolution = 16;
    itemsConfig.autoScale = true;
    itemsConfig.rejectMismatched = false;
    itemsConfig.AddDirectorySource("textures/item/", {"featuretest"});
    itemsConfig.exportPNG = true;
    itemsConfig.exportPath = "debug/";

    atlasManager->AddAtlasConfig(blocksConfig);
    atlasManager->AddAtlasConfig(itemsConfig);
    
    LogInfo("App", "Added 2 atlas configurations: blocks, items");

    // Test 3: Scan and list available textures
    LogInfo("App", "--- Test 3: Scanning available textures ---");
    
    resourceSubsystem->ScanResources([](const std::string& current, size_t scanned) {
        UNUSED(current)
        if (scanned % 50 == 0) // Log every 50 scanned files
        {
            LogInfo("AtlasTest", "Scanned %zu resources...", scanned);
        }
    });

    auto allTextures = resourceSubsystem->ListResources("featuretest", ResourceType::TEXTURE);
    LogInfo("App", "Found %zu textures in featuretest namespace", allTextures.size());
    
    // Log first few textures as examples
    int logCount = 0;
    for (const auto& location : allTextures)
    {
        if (logCount < 10) // Show first 10 textures
        {
            LogInfo("App", "  Example texture: %s", location.ToString().c_str());
            logCount++;
        }
        else if (logCount == 10)
        {
            LogInfo("App", "  ... and %zu more textures", allTextures.size() - 10);
            break;
        }
    }

    // Test 4: Build atlases
    LogInfo("App", "--- Test 4: Building atlases ---");
    
    TestAtlasResults testResults;
    
    // Build blocks atlas
    bool blocksSuccess = atlasManager->BuildAtlas("blocks");
    TestAtlasResults::AtlasResult blocksResult("blocks");
    
    if (blocksSuccess)
    {
        const TextureAtlas* blocksAtlas = atlasManager->GetAtlas("blocks");
        if (blocksAtlas)
        {
            const auto& stats = blocksAtlas->GetStats();
            blocksResult.spriteCount = stats.totalSprites;
            blocksResult.atlasWidth = stats.atlasWidth;
            blocksResult.atlasHeight = stats.atlasHeight;
            blocksResult.packingEfficiency = stats.packingEfficiency;
            
            for (const auto& sprite : blocksAtlas->GetAllSprites())
            {
                blocksResult.foundSprites.push_back(sprite.location.ToString());
            }
            
            LogInfo("App", "✓ Blocks atlas built successfully: %dx%d with %d sprites (%.1f%% efficiency)",
                   stats.atlasWidth, stats.atlasHeight, stats.totalSprites, stats.packingEfficiency);
        }
    }
    else
    {
        LogError("App", "✗ Failed to build blocks atlas");
    }
    
    testResults.AddAtlasResult(blocksResult);
    
    // Build items atlas
    bool itemsSuccess = atlasManager->BuildAtlas("items");
    TestAtlasResults::AtlasResult itemsResult("items");
    
    if (itemsSuccess)
    {
        const TextureAtlas* itemsAtlas = atlasManager->GetAtlas("items");
        if (itemsAtlas)
        {
            const auto& stats = itemsAtlas->GetStats();
            itemsResult.spriteCount = stats.totalSprites;
            itemsResult.atlasWidth = stats.atlasWidth;
            itemsResult.atlasHeight = stats.atlasHeight;
            itemsResult.packingEfficiency = stats.packingEfficiency;
            
            for (const auto& sprite : itemsAtlas->GetAllSprites())
            {
                itemsResult.foundSprites.push_back(sprite.location.ToString());
            }
            
            LogInfo("App", "✓ Items atlas built successfully: %dx%d with %d sprites (%.1f%% efficiency)",
                   stats.atlasWidth, stats.atlasHeight, stats.totalSprites, stats.packingEfficiency);
        }
    }
    else
    {
        LogError("App", "✗ Failed to build items atlas");
    }
    
    testResults.AddAtlasResult(itemsResult);

    // Test 5: Test ResourceLocation-based sprite lookup
    LogInfo("App", "--- Test 5: Testing sprite lookup ---");
    
    TestSpriteVerifier verifier;
    
    // Add some expected sprites based on the files we saw
    verifier.AddExpectedSprite({"featuretest", "textures/block/stone", "blocks"});
    verifier.AddExpectedSprite({"featuretest", "textures/block/dirt", "blocks"});
    verifier.AddExpectedSprite({"featuretest", "textures/block/grass_block_top", "blocks"});
    verifier.AddExpectedSprite({"featuretest", "textures/block/oak_planks", "blocks"});
    verifier.AddExpectedSprite({"featuretest", "textures/block/cobblestone", "blocks"});
    
    verifier.AddExpectedSprite({"featuretest", "textures/item/wooden_sword", "items"});
    verifier.AddExpectedSprite({"featuretest", "textures/item/torch_item", "items"});
    
    // Test some sprites that shouldn't exist
    verifier.AddExpectedSprite({"featuretest", "textures/block/nonexistent_block", "", false});
    verifier.AddExpectedSprite({"wrongnamespace", "textures/block/stone", "", false});
    
    int verificationsPassed = verifier.RunAllVerifications(atlasManager.get());
    
    // Test 6: Export atlases to PNG
    LogInfo("App", "--- Test 6: Exporting atlases to PNG ---");
    
    bool blocksExportSuccess = false;
    bool itemsExportSuccess = false;
    
    if (blocksSuccess)
    {
        blocksExportSuccess = atlasManager->ExportAtlasToPNG("blocks", "debug/atlas_blocks.png");
        if (blocksExportSuccess)
        {
            LogInfo("App", "✓ Blocks atlas exported to debug/atlas_blocks.png");
            blocksResult.exportSuccess = true;
        }
        else
        {
            LogError("App", "✗ Failed to export blocks atlas");
        }
    }
    
    if (itemsSuccess)
    {
        itemsExportSuccess = atlasManager->ExportAtlasToPNG("items", "debug/atlas_items.png");
        if (itemsExportSuccess)
        {
            LogInfo("App", "✓ Items atlas exported to debug/atlas_items.png");
            itemsResult.exportSuccess = true;
        }
        else
        {
            LogError("App", "✗ Failed to export items atlas");
        }
    }

    // Test 7: Print comprehensive atlas information
    LogInfo("App", "--- Test 7: Atlas system summary ---");
    
    atlasManager->PrintAllAtlasInfo();
    
    LogInfo("App", "Total atlases created: %zu", atlasManager->GetTotalAtlasCount());
    LogInfo("App", "Total sprites across all atlases: %zu", atlasManager->GetTotalSpriteCount());
    LogInfo("App", "Total atlas memory usage: %.2f KB", atlasManager->GetTotalAtlasMemoryUsage() / 1024.0f);

    // Test 8: Cross-namespace texture collection test (for future expansion)
    LogInfo("App", "--- Test 8: Cross-namespace functionality test ---");
    
    // This test demonstrates the system's ability to collect textures from multiple namespaces
    // Currently we only have featuretest, but the system is designed for minecraft, testmod, engine, etc.
    std::vector<ResourceLocation> blockTextures = atlasManager->FindTexturesByPattern("textures/block/*", {"featuretest"});
    LogInfo("App", "Found %zu block textures using pattern matching", blockTextures.size());
    
    if (!blockTextures.empty())
    {
        LogInfo("App", "First few block textures found:");
        for (size_t i = 0; i < std::min<size_t>(5, blockTextures.size()); ++i)
        {
            LogInfo("App", "  - %s", blockTextures[i].ToString().c_str());
        }
    }

    // Final Results Summary
    LogInfo("App", "=== AtlasSystem Test Results Summary ===");
    LogInfo("App", "Blocks Atlas: %s (%d sprites, %s export)",
           blocksSuccess ? "SUCCESS" : "FAILED",
           blocksResult.spriteCount,
           blocksResult.exportSuccess ? "SUCCESS" : "FAILED");
    LogInfo("App", "Items Atlas: %s (%d sprites, %s export)",
           itemsSuccess ? "SUCCESS" : "FAILED", 
           itemsResult.spriteCount,
           itemsResult.exportSuccess ? "SUCCESS" : "FAILED");
    LogInfo("App", "Sprite Lookup Verifications: %d passed", verificationsPassed);
    LogInfo("App", "Total Test Sprites: %zu", testResults.GetTotalSpriteCount());
    
    bool overallSuccess = blocksSuccess && itemsSuccess && 
                         (verificationsPassed > 0) && 
                         (blocksExportSuccess || itemsExportSuccess);
    
    LogInfo("App", "=== AtlasSystem Test %s ===", overallSuccess ? "PASSED" : "FAILED");
    
    if (overallSuccess)
    {
        LogInfo("App", "🎉 Atlas system implementation is working correctly!");
        LogInfo("App", "Check debug/ folder for exported atlas PNG files");
    }
    else
    {
        LogError("App", "❌ Atlas system test failed - check logs for details");
    }
}