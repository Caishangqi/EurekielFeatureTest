#include "Game/App.hpp"

#include "Engine/Window/Window.hpp"
#include "Game.hpp"
#include "Engine/Audio/AudioSubsystem.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Engine.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Engine/Renderer/Renderer.hpp"

// Resource system integration
#include "Engine/Resource/ResourceSubsystem.hpp"
#include "Engine/Resource/ResourceCommon.hpp"

// Logger system integration
#include "Engine/Core/Logger/Logger.hpp"

// Console system integration
#include "Engine/Core/Console/ConsoleSubsystem.hpp"

// MessageLog system integration
#include "Engine/Core/MessageLog/MessageLogSubsystem.hpp"

// ImGui system integration
#include "Engine/Core/ImGui/ImGuiSubsystem.hpp"

// Register system integration
#include "Engine/Registry/Core/RegisterSubsystem.hpp"
#include "Test/Test_Registrables.hpp"

// Atlas system integration
#include "Test/Test_AtlasSystem.hpp"
#include "Engine/Resource/Atlas/ImageLoader.hpp"

Window*                g_theWindow   = nullptr;
IRenderer*             g_theRenderer = nullptr;
App*                   g_theApp      = nullptr;
RandomNumberGenerator* g_rng         = nullptr;
InputSystem*           g_theInput    = nullptr;
AudioSubsystem*        g_theAudio    = nullptr;
Game*                  g_theGame     = nullptr;

App::App()
{
    // Create Engine instance
    enigma::core::Engine::CreateInstance();
}

App::~App()
{
}

void App::Startup(char*)
{
    using namespace enigma::resource;

    // Load Game Config
    LoadGameConfig(".enigma/config/GameConfig.xml");
    m_consoleSpace.m_mins = Vec2::ZERO;
    m_consoleSpace.m_maxs = Vec2(g_gameConfigBlackboard.GetValue("screenSizeX", 1600.f), g_gameConfigBlackboard.GetValue("screenSizeY", 800.f));

    EventSystemConfig eventSystemConfig;
    g_theEventSystem = new EventSystem(eventSystemConfig);
    g_theEventSystem->SubscribeEventCallbackFunction("WindowCloseEvent", WindowCloseEvent); // Subscribe the WindowCloseEvent
    g_theEventSystem->SubscribeEventCallbackFunction("Event.Console.Startup", Event_ConsoleStartup);

    // Create All Engine Subsystems
    InputSystemConfig inputConfig;
    g_theInput = new InputSystem(inputConfig);

    WindowConfig windowConfig;
    windowConfig.m_aspectRatio = 2.f;
    windowConfig.m_inputSystem = g_theInput;
    windowConfig.m_windowTitle = "Protogame3D DirectX12";
    g_theWindow                = new Window(windowConfig);

    RenderConfig renderConfig;
    renderConfig.m_window        = g_theWindow;
    renderConfig.m_defaultShader = "Default2D";
    renderConfig.m_backend       = RendererBackend::DirectX12;
    g_theRenderer                = IRenderer::CreateRenderer(renderConfig); // Create render


    DebugRenderConfig debugRenderConfig;
    debugRenderConfig.m_renderer = g_theRenderer;

    DevConsoleConfig consoleConfig;
    consoleConfig.renderer         = g_theRenderer;
    consoleConfig.m_camera         = new Camera();
    consoleConfig.m_camera->m_mode = eMode_Orthographic;
    consoleConfig.m_camera->SetOrthographicView(m_consoleSpace.m_mins, m_consoleSpace.m_maxs);
    g_theDevConsole = new DevConsole(consoleConfig);

    // Register Engine subsystems
    using namespace enigma::core;
    using namespace enigma::resource;

    // Create and register LoggerSubsystem first (highest priority)
    auto logger = std::make_unique<LoggerSubsystem>();
    GEngine->RegisterSubsystem(std::move(logger));

    // Create and register ConsoleSubsystem (high priority, after logger)
    auto consoleSubsystem = std::make_unique<ConsoleSubsystem>();
    GEngine->RegisterSubsystem(std::move(consoleSubsystem));

    // Create and register MessageLogSubsystem (after console, before imgui)
    auto messageLogSubsystem = std::make_unique<MessageLogSubsystem>();
    GEngine->RegisterSubsystem(std::move(messageLogSubsystem));

    // Create and register ImGuiSubsystem (after messagelog)
    ImGuiSubsystemConfig imguiConfig;
    imguiConfig.renderer     = g_theRenderer;
    imguiConfig.targetWindow = g_theWindow;
    auto imguiSubsystem      = std::make_unique<ImGuiSubsystem>(imguiConfig);
    GEngine->RegisterSubsystem(std::move(imguiSubsystem));

    // Create and register RegisterSubsystem (after imgui, before resource)
    RegisterConfig registerConfig;
    registerConfig.enableEvents     = true;
    registerConfig.threadSafe       = true;
    registerConfig.enableNamespaces = true;
    auto registerSubsystem          = std::make_unique<RegisterSubsystem>(registerConfig);
    GEngine->RegisterSubsystem(std::move(registerSubsystem));

    // Create ResourceSubsystem with configuration
    ResourceConfig resourceConfig;
    resourceConfig.baseAssetPath    = ".enigma/assets";
    resourceConfig.enableHotReload  = true;
    resourceConfig.logResourceLoads = true;
    resourceConfig.printScanResults = true;
    resourceConfig.AddNamespace("game", ""); // Add custom namespaces - will resolve to .enigma/assets/game
    resourceConfig.AddNamespace("test", ""); // Add custom namespaces - will resolve to .enigma/assets/test
    resourceConfig.AddNamespace("featuretest", ""); // Add featuretest namespace for Atlas testing - will resolve to .enigma/assets/featuretest
    resourceConfig.EnableNamespacePreload("engine", {"sounds/*"}); // Enable preloading for all sounds

    auto resourceSubsystem = std::make_unique<ResourceSubsystem>(resourceConfig);

    // Register ImageLoader before starting up ResourceSubsystem
    auto imageLoader = std::make_shared<ImageLoader>();
    resourceSubsystem->RegisterLoader(imageLoader);

    GEngine->RegisterSubsystem(std::move(resourceSubsystem));

    // Create AudioSubsystem with configuration
    AudioSystemConfig audioConfig;
    audioConfig.enableResourceIntegration = true;
    audioConfig.resourceSubsystem         = resourceSubsystem.get();
    auto audioSubsystem                   = std::make_unique<AudioSubsystem>(audioConfig);
    GEngine->RegisterSubsystem(std::move(audioSubsystem));

    // Set up global pointers for legacy compatibility
    g_theResource = GEngine->GetSubsystem<ResourceSubsystem>();
    g_theAudio    = GEngine->GetSubsystem<AudioSubsystem>();

    g_theEventSystem->Startup();

    // Startup legacy systems first (so renderer device is ready before ImGui initializes)
    g_theInput->Startup();
    g_theWindow->Startup();
    g_theRenderer->Startup(); // Must startup before GEngine to ensure device is initialized
    DebugRenderSystemStartup(debugRenderConfig);

    // Now start Engine subsystems (ImGuiSubsystem::Initialize() will succeed)
    GEngine->Startup();

    // Start legacy console last
    g_theDevConsole->Startup();

    g_theImGui = GEngine->GetSubsystem<ImGuiSubsystem>();

    // Test Logger subsystem after automatic configuration
    using namespace enigma::core;
    LoggerSubsystem* loggerSubsystem = GEngine->GetLogger();
    if (loggerSubsystem)
    {
        // LoggerSubsystem now automatically creates appenders based on configuration
        // No need to manually add appenders - they're created in CreateDefaultAppenders()

        // Log levels are also set automatically from configuration, but we can override if needed
        // loggerSubsystem->SetGlobalLogLevel(LogLevel::DEBUG);
        // loggerSubsystem->SetCategoryLogLevel("Engine", LogLevel::INFO);

        // Test basic logging functionality using new elegant API
        LogInfo("App", "Logger system initialized and configured automatically");
        LogDebug("App", "Debug logging is enabled");
        LogWarn("App", "This is a warning message");
        LogError("App", "This is an error message (for testing)");

        // Test formatted logging with same function name
        LogInfo("App", "Screen resolution: %dx%d",
                static_cast<int>(g_gameConfigBlackboard.GetValue("screenSizeX", 1600.f)),
                static_cast<int>(g_gameConfigBlackboard.GetValue("screenSizeY", 800.f)));

        // Test category-specific convenience functions
        LogEngineInfo("Engine subsystem started successfully");
        LogGameInfo("Game initialization starting...");
    }

    // Test Console subsystem
    auto* console = GEngine->GetSubsystem<enigma::core::ConsoleSubsystem>();
    if (console)
    {
        LogInfo("App", "ConsoleSubsystem found, initialized: %s", console->IsInitialized() ? "true" : "false");

        if (console->IsInitialized())
        {
            LogInfo("App", "Console is external active: %s", console->IsExternalConsoleActive() ? "true" : "false");
            LogInfo("App", "Console is visible: %s", console->IsVisible() ? "true" : "false");

            // Force show console and test output
            LogInfo("App", "Calling SetVisible(true)...");
            console->SetVisible(true);

            LogInfo("App", "After SetVisible - Console is external active: %s", console->IsExternalConsoleActive() ? "true" : "false");
            LogInfo("App", "After SetVisible - Console is visible: %s", console->IsVisible() ? "true" : "false");

            LogInfo("App", "Writing test messages to console...");

            // Test intelligent output routing
            console->WriteLine("=== Console Output Mode Test ===", enigma::core::LogLevel::INFO);
#ifdef _DEBUG
            console->WriteLine("DEBUG BUILD: In debug mode with debugger attached, this should appear in IDE Console", enigma::core::LogLevel::INFO);
            console->WriteLineColored("DEBUG BUILD: Colored text test (may not show colors in IDE)", Rgba8::GREEN);
#else
            console->WriteLine("RELEASE BUILD: This should appear in External Console window", enigma::core::LogLevel::INFO);
            console->WriteLineColored("RELEASE BUILD: Colored text test", Rgba8::GREEN);
#endif
            console->WriteLineColored("Press \\ to show External Console window", Rgba8::YELLOW);
            console->WriteLine("Press ~ to toggle DevConsole (rendered in-game)", enigma::core::LogLevel::INFO);
            console->WriteLine("Type commands in External Console - they forward to DevConsole", enigma::core::LogLevel::INFO);
            console->WriteLine("=== End Test Messages ===", enigma::core::LogLevel::INFO);
        }
    }
    else
    {
        LogError("App", "ConsoleSubsystem not found!");
    }

    // Test RegisterSubsystem
    RunTest_Registrables();

    // Test AtlasSystem
    RunTest_AtlasSystem();

    g_theGame = new Game();
    g_rng     = new RandomNumberGenerator();
}

void App::Shutdown()
{
    /*
     *  All Destroy and ShutDown process should be reverse order of the StartUp
     */

    // Destroy the game
    delete g_theGame;
    g_theGame = nullptr;

    // Shutdown Engine subsystems (handles ResourceSubsystem and AudioSubsystem)
    GEngine->Shutdown();

    // Clear global pointers (Engine manages the objects now)
    g_theResource = nullptr;
    g_theAudio    = nullptr;

    g_theDevConsole->Shutdown();
    DebugRenderSystemShutdown();
    g_theRenderer->Shutdown();
    g_theWindow->Shutdown();
    g_theInput->Shutdown();
    g_theEventSystem->Shutdown();

    // Destroy remaining Engine Subsystems (not managed by Engine yet)
    delete g_theDevConsole;
    g_theDevConsole = nullptr;

    delete g_theRenderer;
    g_theRenderer = nullptr;

    delete g_theWindow;
    g_theWindow = nullptr;

    delete g_theInput;
    g_theInput = nullptr;

    delete g_theEventSystem;
    g_theEventSystem = nullptr;

    // Destroy Engine instance
    enigma::core::Engine::DestroyInstance();
}

void App::RunFrame()
{
    BeginFrame(); //Engine pre-frame stuff
    Update(); // Game updates / moves / spawns / hurts
    Render(); // Game draws current state of things
    EndFrame(); // Engine post-frame
}

bool App::IsQuitting() const
{
    return m_isQuitting;
}

void App::HandleQuitRequested()
{
    m_isQuitting = true;
}

void App::HandleKeyBoardEvent()
{
    // ESC compatibility controller
    if (g_theInput->GetController(0).WasButtonJustPressed(XBOX_BUTTON_B))
    {
        if (g_theGame->m_isInMainMenu)
        {
            m_isQuitting = true;
        }
    }

    // ESC
    if (g_theInput->WasKeyJustPressed(KEYCODE_ESC))
    {
        if (g_theGame->m_isInMainMenu)
        {
            m_isQuitting = true;
        }
    }

    // F2 - Toggle debug mode (moved from F1)
    if (g_theInput->WasKeyJustPressed(0x71)) // VK_F2 = 0x71
    {
        m_isDebug = !m_isDebug;
    }

    // F8 - Restart game
    if (g_theInput->WasKeyJustPressed(0x77)) // VK_F8 = 0x77
    {
        m_isPendingRestart = true;
    }

    // O - Step single frame
    if (g_theInput->WasKeyJustPressed('O'))
    {
        m_isPaused = true;
        g_theGame->m_clock->StepSingleFrame();
    }
}

bool App::Event_ConsoleStartup(EventArgs& args)
{
    UNUSED(args)
    g_theDevConsole->AddLine(DevConsole::COLOR_INPUT_NORMAL, Stringf("Protogame3D Key Mappings:"));
    g_theDevConsole->AddLine(Rgba8(95, 95, 95),

                             "Mouse   - Aim\n"
                             "W/A     - Move\n"
                             "S/D     - Strafe\n"
                             "Q/E     - Roll\n"
                             "Z/C     - Elevate\n"
                             "Shift   - Sprint\n"
                             "1       - Spawn Line\n"
                             "2       - Spawn Point\n"
                             "3       - Spawn Wireframe Sphere\n"
                             "4       - Spawn Basis\n"
                             "5       - Spawn Billboarded Text\n"
                             "6       - Spawn Wireframe Cylinder\n"
                             "7       - Add Message\n"
                             "H       - Reset position and orientation to zero\n"
                             "N       - Start the Game (In Menu)\n"
                             "ESC     - Quit\n"
                             "P       - Pause the Game\n"
                             "O       - Step single frame\n"
                             "T       - Toggle time scale between 0.1 and 1.0\n"
                             "~       - Toggle Develop Console");
    return true;
}

void App::AdjustForPauseAndTimeDistortion()
{
    m_isSlowMo = g_theInput->IsKeyDown('T');
    if (m_isSlowMo)
    {
        g_theGame->m_clock->SetTimeScale(0.1f);
    }
    else
    {
        g_theGame->m_clock->SetTimeScale(1.0f);
    }


    if (g_theInput->WasKeyJustPressed('P'))
    {
        m_isPaused = !m_isPaused;
    }

    if (m_isPaused)
    {
        g_theGame->m_clock->Pause();
    }
    else
    {
        g_theGame->m_clock->Unpause();
    }
}


void App::BeginFrame()
{
    Clock::TickSystemClock();

    // Begin Engine subsystems frame (includes ImGuiSubsystem::BeginFrame())
    GEngine->BeginFrame();

    g_theInput->BeginFrame();
    g_theWindow->BeginFrame();
    g_theRenderer->BeginFrame();
    DebugRenderBeginFrame();
    g_theAudio->BeginFrame();
    g_theEventSystem->BeginFrame();
    g_theDevConsole->BeginFrame();
}

void App::UpdateCameras()
{
}

void App::Update()
{
    // Calculate delta time
    static float lastTime    = 0.0f;
    float        currentTime = Clock::GetSystemClock().GetTotalSeconds();
    float        deltaTime   = currentTime - lastTime;
    lastTime                 = currentTime;

    // Update Engine subsystems (including ConsoleSubsystem)
    GEngine->Update(deltaTime);

    // Update resource system for hot reload
    g_theResource->Update();

    /// Cursor
    auto windowHandle   = static_cast<HWND>(g_theWindow->GetWindowHandle());
    bool windowHasFocus = (GetActiveWindow() == windowHandle);

    bool devConsoleOpen = (g_theDevConsole->GetMode() != DevConsoleMode::HIDDEN);
    if (!windowHasFocus || devConsoleOpen)
    {
        g_theInput->SetCursorMode(CursorMode::POINTER);
    }
    else
    {
        g_theInput->SetCursorMode(CursorMode::FPS);
    }

    /// 

    HandleKeyBoardEvent();
    AdjustForPauseAndTimeDistortion();
    g_theGame->Update();
}

// If this methods is const, the methods inn the method should also promise
// const
void App::Render() const
{
    g_theRenderer->ClearScreen(Rgba8(m_backgroundColor));
    g_theGame->Render();
    g_theDevConsole->Render(m_consoleSpace);

    // Render ImGui (after all other rendering)
    g_theImGui->Render();
}

void App::EndFrame()
{
    g_theWindow->EndFrame();
    g_theRenderer->EndFrame();
    DebugRenderEndFrame();
    g_theInput->EndFrame();
    g_theAudio->EndFrame();
    g_theEventSystem->EndFrame();
    g_theDevConsole->EndFrame();

    // End Engine subsystems frame (includes ImGuiSubsystem::EndFrame())
    GEngine->EndFrame();

    if (m_isPendingRestart)
    {
        delete g_theGame;
        g_theGame = nullptr;
        g_theGame = new Game();
        // Restore state
        m_isPendingRestart = false;
        m_isPaused         = false;
    }
}

void App::LoadGameConfig(const char* filename)
{
    XmlDocument gameConfigXml;
    XmlResult   result = gameConfigXml.LoadFile(filename);
    if (result == XmlResult::XML_SUCCESS)
    {
        XmlElement* rootElement = gameConfigXml.RootElement();
        if (rootElement)
        {
            g_gameConfigBlackboard.PopulateFromXmlElementAttributes(*rootElement);
            DebuggerPrintf("[SYSTEM]    Game config from file \"%s\" was loaded\n", filename);
        }
        else
        {
            DebuggerPrintf("[SYSTEM]    Game config from file \"%s\"was invalid (missing root element)\n", filename);
        }
    }
    else
    {
        DebuggerPrintf("[SYSTEM]    Failed to load game config from file \"%s\"\n", filename);
    }
}

bool App::WindowCloseEvent(EventArgs& args)
{
    UNUSED(args)
    g_theApp->HandleQuitRequested();
    return false;
}
