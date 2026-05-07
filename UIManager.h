// UIManager.h - Enhanced with Multiplayer Battle UI

#pragma once

#include "main.h"
#include "renderer.h"
#include "Box.h"
#include "GeometricTextRenderer.h"
#include <vector>
#include <string>
#include "TextureAtlas.h"
#include "MapEditor.h"

// UI element types
enum UI_ELEMENT_TYPE {
    UI_TEXT,           // Pure text
    UI_ICON_TEXT,      // Icon+text
    UI_PANEL,          // Panel
    UI_BUTTON,         // Button
    UI_PROGRESS_BAR,   // Progress bar (new for battle UI)
    UI_MINIMAP,        // Minimap display (new for battle UI)
    UI_PLAYER_INFO     // Player info panel (new for battle UI)
};

// UI element display positions
enum UI_POSITION {
    UI_POS_TOP_LEFT,
    UI_POS_TOP_CENTER,
    UI_POS_TOP_RIGHT,
    UI_POS_MID_LEFT,
    UI_POS_MID_CENTER,
    UI_POS_MID_RIGHT,
    UI_POS_BOTTOM_LEFT,
    UI_POS_BOTTOM_CENTER,
    UI_POS_BOTTOM_RIGHT,
    UI_POS_CUSTOM      // Custom position
};

// UI screen types
enum UI_SCREEN_TYPE {
    UI_SCREEN_GAME,           // Main game screen
    UI_SCREEN_EDIT,           // Edit mode screen
    UI_SCREEN_PAUSE,          // Pause screen
    UI_SCREEN_CONTROLS,       // Controls help screen
    UI_SCREEN_HELP,           // Help screen
    UI_SCREEN_MAINMENU,       // 主菜单屏幕
    UI_SCREEN_GAMEOVER,       // 游戏结束屏幕
    UI_SCREEN_SHOP,           // 商店屏幕
    UI_SCREEN_CHARACTER,      // 角色选择屏幕
    UI_SCREEN_BATTLE_LOBBY,   // 战斗大厅屏幕
    UI_SCREEN_MULTIPLAYER_BATTLE,  // 多人战斗屏幕 (NEW)
    UI_SCREEN_BATTLE_PAUSE,   // 战斗暂停屏幕 (NEW)
    UI_SCREEN_BATTLE_RESULT   // 战斗结果屏幕 (NEW)
};

// 按钮回调事件ID
enum UI_BUTTON_CALLBACK {
    UI_CALLBACK_NONE = 0,
    UI_CALLBACK_START_GAME,     // 开始游戏
    UI_CALLBACK_RESUME_GAME,    // 恢复游戏
    UI_CALLBACK_RETURN_MAINMENU,// 返回主菜单
    UI_CALLBACK_OPTIONS,        // 选项菜单
    UI_CALLBACK_MAP_EDITOR,     // 地图编辑器
    UI_CALLBACK_EXIT,           // 退出游戏
    UI_CALLBACK_RESTART,        // 重新开始
    UI_CALLBACK_SAVE_MAP,       // 保存地图
    UI_CALLBACK_LOAD_MAP,       // 加载地图
    UI_CALLBACK_SHOP,           // 商店
    UI_CALLBACK_CHARACTER,      // 角色选择
    UI_CALLBACK_CHARACTER_SELECT, // 选择角色
    UI_CALLBACK_BATTLE_HALL,    // 战斗大厅回调

    // 多人战斗相关回调 (NEW)
    UI_CALLBACK_MULTIPLAYER_BATTLE,   // 进入多人战斗
    UI_CALLBACK_START_BATTLE,         // 开始战斗
    UI_CALLBACK_END_BATTLE,           // 结束战斗
    UI_CALLBACK_SURRENDER,            // 投降
    UI_CALLBACK_TOGGLE_MINIMAP,       // 切换小地图
    UI_CALLBACK_BATTLE_SETTINGS,      // 战斗设置
    UI_CALLBACK_VIEW_STATS,           // 查看统计
    UI_CALLBACK_NEXT_ROUND,           // 下一轮
    UI_CALLBACK_RETURN_TO_LOBBY,      // 返回大厅

    UI_CALLBACK_COUNT
};

// 多人战斗UI状态 (NEW)
enum BATTLE_UI_STATE {
    BATTLE_UI_WAITING,     // 等待开始
    BATTLE_UI_ACTIVE,      // 战斗中
    BATTLE_UI_PAUSED,      // 暂停
    BATTLE_UI_FINISHED     // 战斗结束
};

// 玩家UI信息结构 (NEW)
struct PlayerUIInfo {
    int playerID;
    std::wstring playerName;
    XMFLOAT4 playerColor;
    int health;
    int maxHealth;
    int score;
    bool isAlive;
    bool isReady;
    XMFLOAT2 screenPosition;  // 在UI中的显示位置

    PlayerUIInfo() :
        playerID(-1),
        playerName(L"Unknown"),
        playerColor(1.0f, 1.0f, 1.0f, 1.0f),
        health(100),
        maxHealth(100),
        score(0),
        isAlive(true),
        isReady(false),
        screenPosition(0.0f, 0.0f) {
    }
};

// UI element structure
struct UIElement {
    UI_ELEMENT_TYPE type;
    UI_POSITION position;
    std::wstring text;
    XMFLOAT4 color;
    XMFLOAT4 backgroundColor;
    float scale;
    bool visible;
    bool selected;
    XMFLOAT2 customPosition;
    XMFLOAT2 size;
    float lifeTime;
    ID3D11ShaderResourceView* icon;
    bool usePixelFont;
    UI_SCREEN_TYPE screenType;
    int callbackId;          // 按钮回调ID

    // 添加纹理支持所需的成员
    bool useTexture;                      // 是否使用纹理而非程序化文本
    ID3D11ShaderResourceView* texture;    // 按钮/UI元素的纹理
    XMFLOAT4 hoverColor;                  // 悬停时的颜色
    XMFLOAT4 pressedColor;                // 按下时的颜色

    // 纹理图集支持
    TextureAtlas* atlas;                  // 使用的图集
    std::string spriteNameKey;            // 图集中的精灵名称

    // UV坐标（用于自定义UV或图集精灵）
    XMFLOAT2 uvMin;
    XMFLOAT2 uvMax;

    // 多人战斗UI扩展属性 (NEW)
    int linkedPlayerID;                   // 关联的玩家ID（用于玩家信息面板）
    float animationTimer;                 // 动画计时器
    float maxValue;                       // 最大值（用于进度条）
    float currentValue;                   // 当前值（用于进度条）
    bool isBlinking;                      // 是否闪烁

    // 构造函数，初始化所有成员
    UIElement() :
        type(UI_TEXT),
        position(UI_POS_TOP_LEFT),
        color(1.0f, 1.0f, 1.0f, 1.0f),
        backgroundColor(0.2f, 0.2f, 0.2f, 0.0f),
        scale(1.0f),
        visible(true),
        selected(false),
        customPosition(0.0f, 0.0f),
        size(0.0f, 0.0f),
        lifeTime(-1.0f),
        icon(nullptr),
        usePixelFont(true),
        screenType(UI_SCREEN_GAME),
        callbackId(0),
        useTexture(false),
        texture(nullptr),
        hoverColor(1.2f, 1.2f, 1.2f, 1.0f),
        pressedColor(0.8f, 0.8f, 0.8f, 1.0f),
        atlas(nullptr),
        uvMin(0.0f, 0.0f),
        uvMax(1.0f, 1.0f),
        linkedPlayerID(-1),
        animationTimer(0.0f),
        maxValue(100.0f),
        currentValue(100.0f),
        isBlinking(false)
    {
    }
};

// UI manager class
class UIManager {
private:
    static std::vector<UIElement> m_elements;           // All UI elements
    static std::vector<UIElement> m_temporaryElements;  // Temporarily displayed UI elements
    static UI_SCREEN_TYPE m_currentScreen;              // Currently displayed screen type
    static bool m_initialized;                          // Whether initialized
    static float m_fadeTime;                            // Fade in/out time
    static bool m_isFading;                             // Whether fading in/out
    static float m_fadeAlpha;                           // Current fade in/out alpha value
    static float m_uiScale;                             // UI scale ratio (default is 1.5f)

    // 状态变量
    static int m_lives;                                // 玩家生命值
    static int m_score;                                // 玩家分数
    static int m_coins;                                // 玩家硬币数

    // UI元素ID
    static int m_scoreElementId;                       // 分数显示UI元素ID
    static int m_livesElementId;                       // 生命值显示UI元素ID
    static int m_shopScreenElementId;                  // 商店屏幕显示UI元素ID
    static int m_characterScreenElementId;             // 角色屏幕显示UI元素ID

    // 多人战斗UI状态 (NEW)
    static BATTLE_UI_STATE m_battleUIState;            // 战斗UI状态
    static PlayerUIInfo m_playerInfos[4];              // 玩家UI信息
    static bool m_showMinimap;                         // 是否显示小地图
    static float m_battleTimer;                        // 战斗计时器
    static int m_battleRound;                          // 当前回合
    static std::vector<int> m_battleUIElementIds;      // 战斗UI元素ID列表

    // Calculate screen coordinates based on position
    static XMFLOAT2 CalculatePosition(UI_POSITION position, XMFLOAT2 size);

    // 处理按钮点击
    static void HandleButtonClick(int callbackId);

    // 多人战斗UI私有方法 (NEW)
    static void UpdateBattleAnimations(float deltaTime);
    static void HandleKeyboardInput(float deltaTime);
    static void DrawPlayerInfoPanel(const PlayerUIInfo& playerInfo, XMFLOAT2 position);
    static void DrawProgressBar(XMFLOAT2 position, XMFLOAT2 size, float value, float maxValue, XMFLOAT4 color);
    static void DrawBattleMinimap();
    static void UpdatePlayerHealth(int playerID, int health, int maxHealth);

public:
    // Initialize UI system
    static void Init();

    // Clean up UI system
    static void Uninit();

    // Update UI system
    static void Update(float deltaTime);

    // Draw UI interface
    static void Draw();

    // Switch to specified UI screen
    static void SwitchToScreen(UI_SCREEN_TYPE screenType, bool fadeEffect = true);

    // Add UI element
    static int AddElement(UI_ELEMENT_TYPE type, UI_POSITION position,
        const wchar_t* text, XMFLOAT4 color,
        float scale = 1.5f, bool visible = true,
        UI_SCREEN_TYPE screenType = UI_SCREEN_GAME,
        bool usePixelFont = true);

    // 新增：添加带回调的按钮
    static int AddButton(UI_POSITION position, const wchar_t* text,
        XMFLOAT4 color, UI_BUTTON_CALLBACK callbackId,
        float scale = 1.5f, UI_SCREEN_TYPE screenType = UI_SCREEN_GAME);

    // Add temporary UI element (with lifetime)
    static int AddTemporaryElement(UI_ELEMENT_TYPE type, UI_POSITION position,
        const wchar_t* text, XMFLOAT4 color,
        float lifeTime, float scale = 1.5f,
        bool usePixelFont = true);

    // 添加使用图集的按钮
    static int AddAtlasButton(
        UI_POSITION position,
        const std::string& atlasName,
        const std::string& spriteName,
        UI_BUTTON_CALLBACK callbackId,
        XMFLOAT4 color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
        float scale = 1.0f,
        UI_SCREEN_TYPE screenType = UI_SCREEN_GAME);

    // Set UI element visibility
    static void SetElementVisible(int elementId, bool visible);

    // Remove UI element
    static void RemoveElement(int elementId);

    // Show controls hint (default at bottom of screen)
    static void ShowControlsHint(const wchar_t* hint, float duration = 3.0f);

    // Show status information (default at top right corner)
    static void ShowStatusInfo(const wchar_t* info, XMFLOAT4 color, float duration = 3.0f);

    // Initialize various preset screens
    static void InitGameScreen();
    static void InitEditScreen();
    static void InitPauseScreen();
    static void InitControlsScreen();
    static void InitHelpScreen();
    static void InitMainMenuScreen();   // 初始化主菜单屏幕
    static void InitGameOverScreen();   // 初始化游戏结束屏幕
    static void InitShopScreen();       // 商店屏幕
    static void InitCharacterScreen();  // 角色选择屏幕
    static void InitBattleLobbyScreen();

    // 多人战斗UI初始化方法 (NEW)
    static void InitMultiplayerBattleScreen();  // 初始化多人战斗屏幕
    static void InitBattlePauseScreen();        // 初始化战斗暂停屏幕
    static void InitBattleResultScreen();       // 初始化战斗结果屏幕

    // Set UI scale
    static void SetUIScale(float scale);

    // Get current UI screen type
    static UI_SCREEN_TYPE GetCurrentScreen();

    // 更新玩家状态显示
    static void UpdateCoinDisplay(int coins);
    static void UpdateScoreDisplay(int score);
    static void UpdateLivesDisplay(int lives);

    // 处理UI事件
    static void HandleMouseEvents();  // 处理鼠标事件（点击等）

    // 角色场景专用UI方法
    static void UpdateElementText(int elementId, const wchar_t* newText);
    static void UpdateElementColor(int elementId, XMFLOAT4 newColor);
    static void SetElementCustomPosition(int elementId, XMFLOAT2 position);
    static void CreateCharacterSelectionUI();

    // 多人战斗UI公共方法 (NEW)
    static void InitializeBattleUI();               // 初始化战斗UI
    static void UpdateBattleUI(float deltaTime);    // 更新战斗UI
    static void UpdatePlayerUIElements(int playerID);
    static void SetBattleUIState(BATTLE_UI_STATE state);
    static void HandleButtonHoverEffects();
    // 设置战斗UI状态
    static void UpdatePlayerInfo(int playerID, const wchar_t* name, XMFLOAT4 color);  // 更新玩家信息
    static void UpdatePlayerStats(int playerID, int health, int maxHealth, int score);  // 更新玩家统计
    static void SetPlayerAlive(int playerID, bool alive);  // 设置玩家存活状态
    static void ShowBattleMessage(const wchar_t* message, XMFLOAT4 color, float duration = 3.0f);  // 显示战斗消息
    static void UpdateBattleTimer(float timeRemaining);    // 更新战斗计时器
    static void SetBattleRound(int round);                 // 设置战斗回合
    static void ToggleMinimap();                           // 切换小地图显示
    static void ShowBattleResults(int winnerID, const wchar_t* results);  // 显示战斗结果

    // 战斗UI状态查询
    static BATTLE_UI_STATE GetBattleUIState();
    static bool IsMinimapVisible();
    static int GetBattleRound();
};