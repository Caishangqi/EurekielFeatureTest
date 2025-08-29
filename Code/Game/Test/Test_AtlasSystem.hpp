#pragma once
#include "Engine/Resource/Atlas/AtlasManager.hpp"
#include "Engine/Resource/ResourceSubsystem.hpp"
#include <string>
#include <vector>

// Forward declarations
namespace enigma::resource 
{
    class ResourceSubsystem;
    class AtlasManager;
    class ImageResource;
    class TextureAtlas;
    struct AtlasSprite;
}

// Test Atlas Configuration class - represents different atlas test scenarios
class TestAtlasConfig
{
public:
    TestAtlasConfig(const std::string& name, const std::string& textureDirectory, int resolution = 16)
        : m_name(name), m_textureDirectory(textureDirectory), m_resolution(resolution)
    {
    }

    const std::string& GetName() const { return m_name; }
    const std::string& GetTextureDirectory() const { return m_textureDirectory; }
    int GetResolution() const { return m_resolution; }

    void SetResolution(int resolution) { m_resolution = resolution; }

private:
    std::string m_name;
    std::string m_textureDirectory;
    int m_resolution;
};

// Test Atlas Results class - stores test results for verification
class TestAtlasResults
{
public:
    struct AtlasResult
    {
        std::string name;
        int spriteCount = 0;
        int atlasWidth = 0;
        int atlasHeight = 0;
        float packingEfficiency = 0.0f;
        bool exportSuccess = false;
        std::vector<std::string> foundSprites;

        AtlasResult(const std::string& atlasName) : name(atlasName) {}
    };

    void AddAtlasResult(const AtlasResult& result) { m_atlasResults.push_back(result); }
    const std::vector<AtlasResult>& GetResults() const { return m_atlasResults; }
    
    AtlasResult* FindResult(const std::string& atlasName)
    {
        for (auto& result : m_atlasResults)
        {
            if (result.name == atlasName)
                return &result;
        }
        return nullptr;
    }

    void Clear() { m_atlasResults.clear(); }
    size_t GetTotalSpriteCount() const
    {
        size_t total = 0;
        for (const auto& result : m_atlasResults)
            total += result.spriteCount;
        return total;
    }

private:
    std::vector<AtlasResult> m_atlasResults;
};

// Test Sprite Verification class - helper for sprite lookup tests
class TestSpriteVerifier
{
public:
    struct ExpectedSprite
    {
        std::string namespaceName;
        std::string path;
        std::string expectedAtlas;
        bool shouldExist;

        ExpectedSprite(const std::string& ns, const std::string& p, const std::string& atlas, bool exists = true)
            : namespaceName(ns), path(p), expectedAtlas(atlas), shouldExist(exists) {}
    };

    void AddExpectedSprite(const ExpectedSprite& sprite) { m_expectedSprites.push_back(sprite); }
    const std::vector<ExpectedSprite>& GetExpectedSprites() const { return m_expectedSprites; }
    void Clear() { m_expectedSprites.clear(); }

    bool VerifySprite(const enigma::resource::AtlasManager* manager, const ExpectedSprite& expected) const;
    int RunAllVerifications(const enigma::resource::AtlasManager* manager) const;

private:
    std::vector<ExpectedSprite> m_expectedSprites;
};

// Main test function following the established naming convention
void RunTest_AtlasSystem();