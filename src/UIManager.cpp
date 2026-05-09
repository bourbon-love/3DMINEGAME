// UIManager.cpp
// Specialized UI display system for game guides and control tips

#include "UIManager.h"
#include "sprite.h"
#include "keyboard.h"
#include "mouse.h"
#include <algorithm>
#include  "SceneManager.h"
#include  "CharactorScene.h"
#include  "BattleHall.h"
#include  "MultiplayerBattle.h"
#include "MapEditor.h"      
#include "MiniMap.h"        
#include "FrameWork/TextureManager.h"
// Static member variable initialization
std::vector<UIElement> UIManager::m_elements;
std::vector<UIElement> UIManager::m_temporaryElements;
UI_SCREEN_TYPE UIManager::m_currentScreen = UI_SCREEN_GAME;
bool UIManager::m_initialized = false;
float UIManager::m_fadeTime = 0.0f;
bool UIManager::m_isFading = false;
float UIManager::m_fadeAlpha = 1.0f;
float UIManager::m_uiScale = 1.5f;  // 默认全局UI缩放比例增加到1.5

extern GeometricTextRenderer g_geometricTextRenderer;
extern HWND g_hWnd ;



// 添加到静态成员变量初始化部分
int UIManager::m_scoreElementId = -1;
int UIManager::m_livesElementId = -1;
int UIManager::m_lives = 3;
int UIManager::m_score = 0;
int UIManager::m_coins = 0;
int UIManager::m_shopScreenElementId = -1;  
int UIManager::m_characterScreenElementId = -1;

void InitUIAtlas() {
    // 加载UI图集
    TextureAtlasManager* atlasManager = TextureAtlasManager::GetInstance();


    if (!atlasManager) {
        OutputDebugStringA("错误：无法创建图集管理器\n");
    }

    // 加载主UI图集
    if (atlasManager->LoadAtlas("MainUI", "asset\\texture\\mine_menu.png", nullptr)) {
        TextureAtlas* mainAtlas = atlasManager->GetAtlas("MainUI");

        // 手动定义图集中的精灵位置（实际项目中应从配置文件读取）
        mainAtlas->AddSprite("btn_start", 0, 0, 1024, 280);      // 开始按钮
        mainAtlas->AddSprite("btn_editor", 0, 256, 1024, 280);    // 编辑器按钮
        mainAtlas->AddSprite("btn_options", 0, 512, 1024, 280);  // 选项按钮
        mainAtlas->AddSprite("btn_exit", 0, 768, 1024, 280);     // 退出按钮


    }
     // 加载主UI按键图集
    if (atlasManager->LoadAtlas("ButtonUI", "asset\\texture\\ui_option.png", nullptr)) {
        TextureAtlas* KeyAtlas = atlasManager->GetAtlas("ButtonUI");

        // 手动定义图集中的精灵位置（实际项目中应从配置文件读取）
        KeyAtlas->AddSprite("btn_ren_return", 0, 0, 1024, 280);      // 开始按钮
        KeyAtlas->AddSprite("btn_quick", 0, 256, 1024, 280);    // 编辑器按钮
        KeyAtlas->AddSprite("btn_char", 0, 512, 1024, 280);  // 选项按钮
        KeyAtlas->AddSprite("btn_shop", 0, 768, 1024, 280);     // 退出按钮


    }
      // 加载主UI编辑界面图集
    if (atlasManager->LoadAtlas("EditUI", "asset\\texture\\UI_Editor.png", nullptr)) {
        TextureAtlas* EditAtlas = atlasManager->GetAtlas("EditUI");

        // 手动定义图集中的精灵位置（实际项目中应从配置文件读取）
        EditAtlas->AddSprite("btn_save", 0, 0, 1024, 280);      // 开始按钮
        EditAtlas->AddSprite("btn_load", 0, 256, 1024, 280);    // 编辑器按钮
        EditAtlas->AddSprite("btn_return", 0, 512, 1024, 280);  // 选项按钮
        EditAtlas->AddSprite("btn_help", 0, 768, 1024, 280);     // 退出按钮


    }

}
//多人战斗ui变量
BATTLE_UI_STATE UIManager::m_battleUIState = BATTLE_UI_WAITING;
PlayerUIInfo UIManager::m_playerInfos[4];
bool UIManager::m_showMinimap = true;
float UIManager::m_battleTimer = 0.0f;
int UIManager::m_battleRound = 1;
std::vector<int> UIManager::m_battleUIElementIds;

//====================================================

// Initialize UI system
void UIManager::Init() {
    if (m_initialized) return;

    m_elements.clear();
    m_temporaryElements.clear();
    m_currentScreen = UI_SCREEN_MAINMENU;
    m_fadeTime = 0.0f;
    m_isFading = false;
    m_fadeAlpha = 1.0f;
    m_uiScale = 2.5f;  // 默认全局UI缩放比例增加到1.5

    // 初始化多人战斗UI状态
    m_battleUIState = BATTLE_UI_WAITING;
    m_showMinimap = true;
    m_battleTimer = 0.0f;
    m_battleRound = 1;
    m_battleUIElementIds.clear();

    // 初始化玩家信息
    for (int i = 0; i < 4; i++) {
        m_playerInfos[i] = PlayerUIInfo();
        m_playerInfos[i].playerID = i;
        wchar_t playerName[32];
        swprintf_s(playerName, L"Player %d", i + 1);
        m_playerInfos[i].playerName = playerName;

        // 设置玩家颜色
        switch (i) {
        case 0: m_playerInfos[i].playerColor = XMFLOAT4(1.0f, 0.2f, 0.2f, 1.0f); break; // 红色
        case 1: m_playerInfos[i].playerColor = XMFLOAT4(0.2f, 0.2f, 1.0f, 1.0f); break; // 蓝色
        case 2: m_playerInfos[i].playerColor = XMFLOAT4(0.2f, 1.0f, 0.2f, 1.0f); break; // 绿色
        case 3: m_playerInfos[i].playerColor = XMFLOAT4(1.0f, 1.0f, 0.2f, 1.0f); break; // 黄色
        }
    }
    // Initialize each screen
    InitUIAtlas();
    InitGameScreen();
    InitEditScreen();
    InitControlsScreen();
    InitHelpScreen();
    InitPauseScreen();
    InitMainMenuScreen();   
    InitGameOverScreen();   
    InitShopScreen();       
    InitCharacterScreen();  
    InitBattleLobbyScreen();
    InitMultiplayerBattleScreen(); 
    InitBattlePauseScreen();        
    InitBattleResultScreen();
    // Initialize geometric text renderer
    GeometricTextRenderer::Init();
    m_initialized = true;



    OutputDebugStringA("UI system initialized\n");
}

// Initialize edit screen
void UIManager::InitEditScreen() {
    // Add edit mode UI elements

    // Edit mode title
   int top= AddElement(
        UI_PANEL,
        UI_POS_CUSTOM,
        L"MAP EDIT MODE",
        XMFLOAT4(0.3f, 0.8f, 1.0f, 1.0f),
        3.0f,  // 增加标题大小
        true,
        UI_SCREEN_EDIT,
        true
    );
    m_elements[top].customPosition = XMFLOAT2(650.0f, 50.0f);

    // 合并底部操作提示与其他指令
   int butom= AddElement(
        UI_TEXT,
        UI_POS_CUSTOM,
        L"[TAB] Game Mode  [O] Toggle Terrain/Obstacle  [F1] Help  [ESC] Menu",
        XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
        3.0f,
        true,
        UI_SCREEN_EDIT,
        true
    );
   m_elements[butom].customPosition = XMFLOAT2(50.0f,SCREEN_HEIGHT- 50.0f);

    // 添加保存/加载地图按钮到编辑屏幕
    int saveBtnId = AddAtlasButton(
        UI_POS_CUSTOM,
        "EditUI",
        "btn_save",
        UI_CALLBACK_SAVE_MAP,
        XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
        0.3f,
        UI_SCREEN_EDIT
    );
    m_elements[saveBtnId].customPosition = XMFLOAT2(SCREEN_WIDTH - 400.0f, 50.0f);

    int loadBtnId = AddAtlasButton(
        UI_POS_CUSTOM,
        "EditUI",
        "btn_load",
        UI_CALLBACK_LOAD_MAP,
        XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
        0.3f,
        UI_SCREEN_EDIT
    );
    m_elements[loadBtnId].customPosition = XMFLOAT2(SCREEN_WIDTH - 400.0f, 150.0f);

    int returnBtnId = AddAtlasButton(
        UI_POS_CUSTOM,
        "ButtonUI",
        "btn_ren_return",
        UI_CALLBACK_RETURN_MAINMENU,
        XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
        0.3f,  // 放大返回按钮
        UI_SCREEN_EDIT
    );
    m_elements[returnBtnId].customPosition = XMFLOAT2(SCREEN_WIDTH - 400.0f, SCREEN_HEIGHT - 150.0f);
}

// 添加初始化主菜单屏幕函数
void UIManager::InitMainMenuScreen() {
    // 主菜单标题
    AddElement(
        UI_PANEL,
        UI_POS_TOP_CENTER,
        L"3D MINE GAME",
        XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
        3.0f,  // 增加标题大小
        true,
        UI_SCREEN_MAINMENU,
        true
    );

   
    int startBtnId = AddAtlasButton(
        UI_POS_CUSTOM,
        "MainUI",               // 图集名称
        "btn_start",            // 精灵名称d
        /*UI_CALLBACK_START_GAME,*/
        UI_CALLBACK_MULTIPLAYER_BATTLE,
        XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
        0.4f,
        UI_SCREEN_MAINMENU
    );

        // 设置位置
        m_elements[startBtnId].customPosition = XMFLOAT2(100.0f, SCREEN_HEIGHT * 0.1f);

        // 添加其他按钮
        int editorBtnId = AddAtlasButton(
            UI_POS_CUSTOM,
            "MainUI",
            "btn_editor",
            UI_CALLBACK_MAP_EDITOR,
            XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
            0.4f,
            UI_SCREEN_MAINMENU
        );

        m_elements[editorBtnId].customPosition = XMFLOAT2(100, SCREEN_HEIGHT * 0.2f);

        // 添加其他按钮
        int optionsBtnId = AddAtlasButton(
            UI_POS_CUSTOM,
            "MainUI",
            "btn_options",
            UI_CALLBACK_OPTIONS,
            XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
            0.4f,
            UI_SCREEN_MAINMENU
        );

        m_elements[optionsBtnId].customPosition = XMFLOAT2(100, SCREEN_HEIGHT * 0.3f);
         // 添加其他按钮
        int charBtnId = AddAtlasButton(
            UI_POS_CUSTOM,
            "ButtonUI",
            "btn_char",
            UI_CALLBACK_CHARACTER,      // 角色选择
          
            XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
            0.4f,
            UI_SCREEN_MAINMENU
        );

        m_elements[charBtnId].customPosition = XMFLOAT2(100, SCREEN_HEIGHT * 0.4f);

        // 在角色按钮之后添加战斗大厅按钮
        int battleHallBtnId = AddAtlasButton(
            UI_POS_CUSTOM,
            "ButtonUI",
            "btn_quick",              // 使用现有的快速游戏按钮图片
            UI_CALLBACK_BATTLE_HALL,
            XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
            0.4f,
            UI_SCREEN_MAINMENU
        );

        m_elements[battleHallBtnId].customPosition = XMFLOAT2(100, SCREEN_HEIGHT * 0.5f);

        // 添加其他按钮
        int exitBtnId = AddAtlasButton(
            UI_POS_CUSTOM,
            "MainUI",
            "btn_exit",
            UI_CALLBACK_EXIT,
            XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
            0.4f,
            UI_SCREEN_MAINMENU
        );

        m_elements[exitBtnId].customPosition = XMFLOAT2(100, SCREEN_HEIGHT * 0.6f);


}

// 初始化游戏结束屏幕
void UIManager::InitGameOverScreen() {
    // 游戏结束标题
    AddElement(
        UI_PANEL,
        UI_POS_TOP_CENTER,
        L"GAME OVER",
        XMFLOAT4(1.0f, 0.3f, 0.3f, 1.0f),
        3.0f,  // 增加标题大小
        true,
        UI_SCREEN_GAMEOVER,
        true
    );

    // 分数显示
    int scoreId = AddElement(
        UI_PANEL,
        UI_POS_MID_CENTER,
        L"SCORE: 0",
        XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
        2.5f,
        true,
        UI_SCREEN_GAMEOVER,
        true
    );

    // 重新开始按钮
    int restartBtnId = AddButton(
        UI_POS_CUSTOM,
        L"RESTART",
        XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
        UI_CALLBACK_RESTART,
        2.0f,
        UI_SCREEN_GAMEOVER
    );
    m_elements[restartBtnId].customPosition = XMFLOAT2((SCREEN_WIDTH - 300.0f) * 0.5f, SCREEN_HEIGHT * 0.6f);
    m_elements[restartBtnId].size = XMFLOAT2(300.0f, 70.0f);
    m_elements[restartBtnId].backgroundColor = XMFLOAT4(0.2f, 0.6f, 0.4f, 0.7f);

    // 返回主菜单按钮
    int menuBtnId = AddButton(
        UI_POS_CUSTOM,
        L"RETURN MAIN MENU",
        XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
        UI_CALLBACK_RETURN_MAINMENU,
        2.0f,
        UI_SCREEN_GAMEOVER
    );
    m_elements[menuBtnId].customPosition = XMFLOAT2((SCREEN_WIDTH - 300.0f) * 0.5f, SCREEN_HEIGHT * 0.7f);
    m_elements[menuBtnId].size = XMFLOAT2(300.0f, 70.0f);
    m_elements[menuBtnId].backgroundColor = XMFLOAT4(0.6f, 0.4f, 0.2f, 0.7f);

}
void UIManager::InitShopScreen()
{
    // 商店标题
    AddElement(
        UI_PANEL,
        UI_POS_TOP_CENTER,
        L"SHOP",
        XMFLOAT4(0.3f, 0.6f, 1.0f, 1.0f),
        3.0f,
        true,
        UI_SCREEN_SHOP,
        true
    );

    // 商店说明
    int descId = AddElement(
        UI_PANEL,
        UI_POS_MID_CENTER,
        L"购买游戏道具和角色皮肤\n\n金币: 0",
        XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
        2.0f,
        true,
        UI_SCREEN_SHOP,
        true
    );
    m_elements[descId].size = XMFLOAT2(500.0f, 200.0f);
    m_shopScreenElementId = descId;

    // 商店物品列表（示例）
    int item1Id = AddElement(
        UI_PANEL,
        UI_POS_CUSTOM,
        L"额外生命 - 100金币",
        XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
        1.8f,
        true,
        UI_SCREEN_SHOP,
        true
    );
    m_elements[item1Id].customPosition = XMFLOAT2(200.0f, 300.0f);
    m_elements[item1Id].size = XMFLOAT2(400.0f, 60.0f);
    m_elements[item1Id].backgroundColor = XMFLOAT4(0.3f, 0.5f, 0.8f, 0.7f);

    // 返回按钮
    int returnBtnId = AddAtlasButton(
        UI_POS_CUSTOM,
        "ButtonUI",
        "btn_ren_return",
        UI_CALLBACK_RETURN_MAINMENU,
        XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
        0.3f,
        UI_SCREEN_SHOP
    );
    m_elements[returnBtnId].customPosition = XMFLOAT2(SCREEN_WIDTH - 350.0f, SCREEN_HEIGHT - 150.0f);
}
void UIManager::InitCharacterScreen() {
    // 角色选择标题 - 去除背景，加大文字
    AddElement(
        UI_TEXT,  // 去除背景
        UI_POS_TOP_CENTER,
        L"CHARACTER SELECT",
        XMFLOAT4(0.5f, 1.0f, 0.5f, 1.0f),
        3.0f,  // 大幅增加标题文字
        true,
        UI_SCREEN_CHARACTER,
        true
    );

    // 角色说明 - 去除背景，大幅放大文字
    int descId = AddElement(
        UI_TEXT,  
        UI_POS_CUSTOM,
        L"Choose Your Game Character",
        XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
        2.8f,  // 大幅增加说明文字
        true,
        UI_SCREEN_CHARACTER,
        true
    );
    // 设置说明文字的位置
    m_elements[descId].customPosition = XMFLOAT2(SCREEN_WIDTH * 0.3f, SCREEN_HEIGHT * 0.2f);
    m_characterScreenElementId = descId;

    //// 角色特性说明
    //AddElement(
    //    UI_TEXT,
    //    UI_POS_CUSTOM,
    //    L"Character Abilities:\n\n[1] Doctor - Can heal teammates\n[2] Soldier - Extra armor and weapons\n[3] Scout - Faster movement speed\n[4] Engineer - Build defensive structures\n[5] Mage - Cast powerful spells",
    //    XMFLOAT4(0.8f, 1.0f, 0.8f, 1.0f),
    //    2.2f,  // 增加特性说明文字
    //    true,
    //    UI_SCREEN_CHARACTER,
    //    true
    //);
    //// 设置特性说明位置
    //m_elements[m_elements.size() - 1].customPosition = XMFLOAT2(50, SCREEN_HEIGHT * 0.4f);

    // 解锁提示
    //AddElement(
    //    UI_TEXT,
    //    UI_POS_CUSTOM,
    //    L"How to unlock more characters:\n• Complete battles to earn XP\n• Collect special items\n• Win multiplayer matches\n• Explore hidden areas",
    //    XMFLOAT4(1.0f, 0.8f, 0.5f, 1.0f),
    //    2.0f,  // 增加解锁提示文字
    //    true,
    //    UI_SCREEN_CHARACTER,
    //    true
    //);
    //// 设置解锁提示位置
    //m_elements[m_elements.size() - 1].customPosition = XMFLOAT2(SCREEN_WIDTH * 0.6f, SCREEN_HEIGHT * 0.5f);

    // 返回按钮
    int returnBtnId = AddAtlasButton(
        UI_POS_CUSTOM,
        "ButtonUI",
        "btn_ren_return",
        UI_CALLBACK_RETURN_MAINMENU,
        XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
        0.3f,  // 放大返回按钮
        UI_SCREEN_CHARACTER
    );
    m_elements[returnBtnId].customPosition = XMFLOAT2(SCREEN_WIDTH- 350.0f, SCREEN_HEIGHT - 150.0f);

    // 底部控制提示
   int butom= AddElement(
        UI_TEXT,
        UI_POS_CUSTOM,
        L"[1-4] Select Character  [enter] into battleHall",
        XMFLOAT4(0.9f, 0.9f, 0.3f, 1.0f),
        2.0f,  // 加大控制提示
        true,
        UI_SCREEN_CHARACTER,
        true
    );
   m_elements[butom].customPosition = XMFLOAT2(350.0f, SCREEN_HEIGHT - 150.0f);

    OutputDebugStringA("✅ 角色选择UI屏幕初始化完成\n");
}
void UIManager::InitBattleLobbyScreen() {
   

   
}
void UIManager::InitPauseScreen() {
    // Add pause screen elements

    // Pause title
    AddElement(
        UI_PANEL,
        UI_POS_MID_CENTER,
        L"GAME PAUSED",
        XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
        2.5f,  // 增加标题大小
        true,
        UI_SCREEN_PAUSE,
        true
    );

    // Pause menu options
    int id1 = AddElement(
        UI_BUTTON,
        UI_POS_CUSTOM,
        L"Continue",
        XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
        2.0f,  // 增加按钮文字大小
        true,
        UI_SCREEN_PAUSE,
        true
    );
    m_elements[id1].customPosition = XMFLOAT2((SCREEN_WIDTH - 250.0f) * 0.5f, SCREEN_HEIGHT * 0.4f);
    m_elements[id1].size = XMFLOAT2(250.0f, 60.0f);  // 增加按钮大小

    int id2 = AddElement(
        UI_BUTTON,
        UI_POS_CUSTOM,
        L"Help",
        XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
        2.0f,  // 增加按钮文字大小
        true,
        UI_SCREEN_PAUSE,
        true
    );
    m_elements[id2].customPosition = XMFLOAT2((SCREEN_WIDTH - 250.0f) * 0.5f, SCREEN_HEIGHT * 0.5f);
    m_elements[id2].size = XMFLOAT2(250.0f, 60.0f);  // 增加按钮大小

    int id3 = AddElement(
        UI_BUTTON,
        UI_POS_CUSTOM,
        L"Exit",
        XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
        2.0f,  // 增加按钮文字大小
        true,
        UI_SCREEN_PAUSE,
        true
    );
    m_elements[id3].customPosition = XMFLOAT2((SCREEN_WIDTH - 250.0f) * 0.5f, SCREEN_HEIGHT * 0.6f);
    m_elements[id3].size = XMFLOAT2(250.0f, 60.0f);  // 增加按钮大小

    // Pause instructions
    AddElement(
        UI_TEXT,
        UI_POS_BOTTOM_CENTER,
        L"Press [ESC] to return to game",
        XMFLOAT4(0.9f, 0.9f, 0.3f, 1.0f),
        1.8f,  // 增加文字大小
        true,
        UI_SCREEN_PAUSE,
        true
    );
}

// Initialize controls screen
void UIManager::InitControlsScreen() {
    // Add controls screen elements

    // Controls title
    AddElement(
        UI_PANEL,
        UI_POS_TOP_CENTER,
        L"CONTROLS",
        XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
        2.0f,  // 增加标题大小
        true,
        UI_SCREEN_CONTROLS,
        true
    );

    // General controls
    int id1 = AddElement(
        UI_PANEL,
        UI_POS_CUSTOM,
        L"General Controls:\n\n[TAB] - Toggle Game/Edit Mode\n[ESC] - Pause Game\n[F1] - Show Help\n[F5] - Save Map\n[F8] - Load Map",
        XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
        1.5f,  // 增加文字大小
        true,
        UI_SCREEN_CONTROLS,
        true
    );
    m_elements[id1].customPosition = XMFLOAT2(50.0f, 100.0f);
    m_elements[id1].size = XMFLOAT2(400.0f, 250.0f);  // 增加面板大小

    // Game mode controls
    int id2 = AddElement(
        UI_PANEL,
        UI_POS_CUSTOM,
        L"Game Mode Controls:\n\n[W,A,S,D] - Move Camera\n[Left Mouse] - Rotate View\n[Right Mouse] - Pan View\n[Mouse Wheel] - Zoom View",
        XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
        1.5f,  // 增加文字大小
        true,
        UI_SCREEN_CONTROLS,
        true
    );
    m_elements[id2].customPosition = XMFLOAT2(SCREEN_WIDTH - 450.0f, 100.0f);
    m_elements[id2].size = XMFLOAT2(400.0f, 250.0f);  // 增加面板大小

    // Edit mode controls
    int id3 = AddElement(
        UI_PANEL,
        UI_POS_CUSTOM,
        L"Edit Mode Controls:\n\n[O] - Toggle Terrain/Obstacle\n[Left Mouse] - Select and Edit\n[Ctrl+S] - Save Map\n[Ctrl+L] - Load Map",
        XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
        1.5f,  // 增加文字大小
        true,
        UI_SCREEN_CONTROLS,
        true
    );
    m_elements[id3].customPosition = XMFLOAT2(50.0f, 370.0f);
    m_elements[id3].size = XMFLOAT2(400.0f, 250.0f);  // 增加面板大小

    // Return button
    int id4 = AddElement(
        UI_BUTTON,
        UI_POS_CUSTOM,
        L"Return",
        XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
        2.0f,  // 增加按钮文字大小
        true,
        UI_SCREEN_CONTROLS,
        true
    );
    m_elements[id4].customPosition = XMFLOAT2((SCREEN_WIDTH - 250.0f) * 0.5f, SCREEN_HEIGHT - 100.0f);
    m_elements[id4].size = XMFLOAT2(250.0f, 60.0f);  // 增加按钮大小
}

// Initialize help screen
void UIManager::InitHelpScreen() {
    // Add help screen elements

    // Help title
    AddElement(
        UI_PANEL,
        UI_POS_TOP_CENTER,
        L"GAME HELP",
        XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
        2.0f,  // 增加标题大小
        true,
        UI_SCREEN_HELP,
        true
    );

    // Game introduction
    int id1 = AddElement(
        UI_PANEL,
        UI_POS_CUSTOM,
        L"Map Editor - Game Introduction\n\nThis is a map editing tool for creating and editing game maps.\nYou can design different terrains and obstacles.\nUse TAB key to switch between Game Mode and Edit Mode.",
        XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
        1.5f,  // 增加文字大小
        true,
        UI_SCREEN_HELP,
        true
    );
    m_elements[id1].customPosition = XMFLOAT2(50.0f, 100.0f);
    m_elements[id1].size = XMFLOAT2(SCREEN_WIDTH - 100.0f, 200.0f);  // 增加面板大小

    // Terrain description
    int id2 = AddElement(
        UI_PANEL,
        UI_POS_CUSTOM,
        L"Terrain Types:\n\nNormal Ground - Standard Movement\nIce - Sliding Effect\nSand - Slow Movement\nGrass - Standard Terrain\nWater - Impassable",
        XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
        1.5f,  // 增加文字大小
        true,
        UI_SCREEN_HELP,
        true
    );
    m_elements[id2].customPosition = XMFLOAT2(50.0f, 320.0f);
    m_elements[id2].size = XMFLOAT2(400.0f, 250.0f);  // 增加面板大小

    // Obstacle description
    int id3 = AddElement(
        UI_PANEL,
        UI_POS_CUSTOM,
        L"Obstacle Types:\n\nWall - Impassable\nTree - Impassable\nStarting Point - Player Spawn\nNo Obstacle - Clear Obstacles",
        XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
        1.5f,  // 增加文字大小
        true,
        UI_SCREEN_HELP,
        true
    );
    m_elements[id3].customPosition = XMFLOAT2(SCREEN_WIDTH - 450.0f, 320.0f);
    m_elements[id3].size = XMFLOAT2(400.0f, 250.0f);  // 增加面板大小

    // Return button
    int id4 = AddElement(
        UI_BUTTON,
        UI_POS_CUSTOM,
        L"Return",
        XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
        2.0f,  // 增加按钮文字大小
        true,
        UI_SCREEN_HELP,
        true
    );
    m_elements[id4].customPosition = XMFLOAT2((SCREEN_WIDTH - 250.0f) * 0.5f, SCREEN_HEIGHT - 100.0f);
    m_elements[id4].size = XMFLOAT2(250.0f, 60.0f);  // 增加按钮大小
}

// Clean up UI system
void UIManager::Uninit() {
    m_elements.clear();
    m_temporaryElements.clear();

  

    m_initialized = false;

    // Clean up geometric text renderer
    GeometricTextRenderer::Uninit();

    OutputDebugStringA("UI system cleaned up\n");
}

// 初始化多人战斗屏幕
//void UIManager::InitMultiplayerBattleScreen() {
//    // 战斗状态标题
//    int titleId = AddElement(
//        UI_TEXT,
//        UI_POS_TOP_CENTER,
//        L"MULTIPLAYER BATTLE",
//        XMFLOAT4(1.0f, 0.5f, 0.0f, 1.0f),
//        2.5f,
//        true,
//        UI_SCREEN_MULTIPLAYER_BATTLE,
//        true
//    );
//    m_battleUIElementIds.push_back(titleId);
//
//    // 战斗计时器
//    int timerId = AddElement(
//        UI_TEXT,
//        UI_POS_CUSTOM,
//        L"TIME: 00:00",
//        XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
//        2.0f,
//        true,
//        UI_SCREEN_MULTIPLAYER_BATTLE,
//        true
//    );
//    m_elements[timerId].customPosition = XMFLOAT2(SCREEN_WIDTH / 2 - 100, 80);
//    m_battleUIElementIds.push_back(timerId);
//
//    // 回合显示
//    int roundId = AddElement(
//        UI_TEXT,
//        UI_POS_CUSTOM,
//        L"ROUND: 1",
//        XMFLOAT4(0.5f, 1.0f, 0.5f, 1.0f),
//        1.8f,
//        true,
//        UI_SCREEN_MULTIPLAYER_BATTLE,
//        true
//    );
//    m_elements[roundId].customPosition = XMFLOAT2(SCREEN_WIDTH / 2 - 80, 120);
//    m_battleUIElementIds.push_back(roundId);
//
//    // 玩家信息面板 - 4个玩家，分布在屏幕四角
//    XMFLOAT2 playerPanelPositions[4] = {
//        XMFLOAT2(20, 160),                           // 玩家1 - 左上
//        XMFLOAT2(SCREEN_WIDTH - 220, 160),          // 玩家2 - 右上
//        XMFLOAT2(20, SCREEN_HEIGHT - 120),          // 玩家3 - 左下
//        XMFLOAT2(SCREEN_WIDTH - 220, SCREEN_HEIGHT - 120)  // 玩家4 - 右下
//    };
//
//    for (int i = 0; i < 4; i++) {
//        // 玩家名称
//        int nameId = AddElement(
//            UI_TEXT,
//            UI_POS_CUSTOM,
//            L"Player 1",
//            m_playerInfos[i].playerColor,
//            1.3f,
//            true,
//            UI_SCREEN_MULTIPLAYER_BATTLE,
//            true
//        );
//        m_elements[nameId].customPosition = playerPanelPositions[i];
//        m_elements[nameId].linkedPlayerID = i;
//        m_battleUIElementIds.push_back(nameId);
//
//        // 玩家血量
//        int healthId = AddElement(
//            UI_PROGRESS_BAR,
//            UI_POS_CUSTOM,
//            L"HP: 100/100",
//            XMFLOAT4(0.2f, 1.0f, 0.2f, 1.0f),
//            1.1f,
//            true,
//            UI_SCREEN_MULTIPLAYER_BATTLE,
//            true
//        );
//        m_elements[healthId].customPosition = XMFLOAT2(playerPanelPositions[i].x, playerPanelPositions[i].y + 25);
//        m_elements[healthId].size = XMFLOAT2(180, 15);
//        m_elements[healthId].linkedPlayerID = i;
//        m_elements[healthId].maxValue = 100.0f;
//        m_elements[healthId].currentValue = 100.0f;
//        m_battleUIElementIds.push_back(healthId);
//
//        // 玩家分数
//        int scoreId = AddElement(
//            UI_TEXT,
//            UI_POS_CUSTOM,
//            L"Score: 0",
//            XMFLOAT4(1.0f, 1.0f, 0.2f, 1.0f),
//            1.0f,
//            true,
//            UI_SCREEN_MULTIPLAYER_BATTLE,
//            true
//        );
//        m_elements[scoreId].customPosition = XMFLOAT2(playerPanelPositions[i].x, playerPanelPositions[i].y + 45);
//        m_elements[scoreId].linkedPlayerID = i;
//        m_battleUIElementIds.push_back(scoreId);
//
//        // 存储玩家UI位置
//        m_playerInfos[i].screenPosition = playerPanelPositions[i];
//    }
//
//    // 小地图切换按钮
//    int minimapBtnId = AddButton(
//        UI_POS_CUSTOM,
//        L"MAP",
//        XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
//        UI_CALLBACK_TOGGLE_MINIMAP,
//        1.2f,
//        UI_SCREEN_MULTIPLAYER_BATTLE
//    );
//    m_elements[minimapBtnId].customPosition = XMFLOAT2(SCREEN_WIDTH - 80, 20);
//    m_elements[minimapBtnId].size = XMFLOAT2(60, 30);
//    m_elements[minimapBtnId].backgroundColor = XMFLOAT4(0.2f, 0.4f, 0.6f, 0.8f);
//    m_battleUIElementIds.push_back(minimapBtnId);
//
//    // 投降按钮
//    int surrenderBtnId = AddButton(
//        UI_POS_CUSTOM,
//        L"SURRENDER",
//        XMFLOAT4(1.0f, 0.3f, 0.3f, 1.0f),
//        UI_CALLBACK_SURRENDER,
//        1.1f,
//        UI_SCREEN_MULTIPLAYER_BATTLE
//    );
//    m_elements[surrenderBtnId].customPosition = XMFLOAT2(SCREEN_WIDTH - 120, SCREEN_HEIGHT - 40);
//    m_elements[surrenderBtnId].size = XMFLOAT2(100, 30);
//    m_elements[surrenderBtnId].backgroundColor = XMFLOAT4(0.6f, 0.2f, 0.2f, 0.8f);
//    m_battleUIElementIds.push_back(surrenderBtnId);
//
//    // 控制提示
//    int controlsId = AddElement(
//        UI_TEXT,
//        UI_POS_BOTTOM_CENTER,
//        L"[ESC] Pause  [M] Toggle Map  [TAB] Stats",
//        XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f),
//        1.0f,
//        true,
//        UI_SCREEN_MULTIPLAYER_BATTLE,
//        true
//    );
//    m_battleUIElementIds.push_back(controlsId);
//
//    OutputDebugStringA("多人战斗UI屏幕初始化完成\n");
//}
// 初始化战斗暂停屏幕
void UIManager::InitMultiplayerBattleScreen() {
    // 战斗状态标题 - 去除背景，加大文字
    int titleId = AddElement(
        UI_TEXT,  // 去除背景
        UI_POS_TOP_CENTER,
        L"MULTIPLAYER BATTLE",
        XMFLOAT4(1.0f, 0.5f, 0.0f, 1.0f),
        3.5f,  // 增大标题
        true,
        UI_SCREEN_MULTIPLAYER_BATTLE,
        true
    );
    m_battleUIElementIds.push_back(titleId);

    // 战斗计时器 - 加大文字
    int timerId = AddElement(
        UI_TEXT,
        UI_POS_CUSTOM,
        L"TIME: 00:00",
        XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
        2.5f,  // 增大文字
        true,
        UI_SCREEN_MULTIPLAYER_BATTLE,
        true
    );
    m_elements[timerId].customPosition = XMFLOAT2(SCREEN_WIDTH / 2 - 100, 80);
    m_battleUIElementIds.push_back(timerId);

    // 回合显示 - 加大文字
    int roundId = AddElement(
        UI_TEXT,
        UI_POS_CUSTOM,
        L"ROUND: 1",
        XMFLOAT4(0.5f, 1.0f, 0.5f, 1.0f),
        2.2f,  // 增大文字
        true,
        UI_SCREEN_MULTIPLAYER_BATTLE,
        true
    );
    m_elements[roundId].customPosition = XMFLOAT2(SCREEN_WIDTH / 2 - 80, 120);
    m_battleUIElementIds.push_back(roundId);

    // 玩家信息面板 - 保持原有的4个玩家面板，但加大文字
    XMFLOAT2 playerPanelPositions[4] = {
        XMFLOAT2(20, 160),                           // 玩家1 - 左上
        XMFLOAT2(SCREEN_WIDTH - 220, 160),          // 玩家2 - 右上
        XMFLOAT2(20, SCREEN_HEIGHT - 120),          // 玩家3 - 左下
        XMFLOAT2(SCREEN_WIDTH - 220, SCREEN_HEIGHT - 120)  // 玩家4 - 右下
    };

    for (int i = 0; i < 4; i++) {
        // 玩家名称 - 加大文字
        int nameId = AddElement(
            UI_TEXT,
            UI_POS_CUSTOM,
            L"Player 1",
            m_playerInfos[i].playerColor,
            1.8f,  // 增大玩家名称文字
            true,
            UI_SCREEN_MULTIPLAYER_BATTLE,
            true
        );
        m_elements[nameId].customPosition = playerPanelPositions[i];
        m_elements[nameId].linkedPlayerID = i;
        m_battleUIElementIds.push_back(nameId);

        // 玩家血量
        int healthId = AddElement(
            UI_PROGRESS_BAR,
            UI_POS_CUSTOM,
            L"HP: 100/100",
            XMFLOAT4(0.2f, 1.0f, 0.2f, 1.0f),
            1.5f,  // 增大血量文字
            true,
            UI_SCREEN_MULTIPLAYER_BATTLE,
            true
        );
        m_elements[healthId].customPosition = XMFLOAT2(playerPanelPositions[i].x, playerPanelPositions[i].y + 30);
        m_elements[healthId].size = XMFLOAT2(180, 20);  // 增大血量条
        m_elements[healthId].linkedPlayerID = i;
        m_elements[healthId].maxValue = 100.0f;
        m_elements[healthId].currentValue = 100.0f;
        m_battleUIElementIds.push_back(healthId);

        // 玩家分数 - 加大文字
        int scoreId = AddElement(
            UI_TEXT,
            UI_POS_CUSTOM,
            L"Score: 0",
            XMFLOAT4(1.0f, 1.0f, 0.2f, 1.0f),
            1.4f,  // 增大分数文字
            true,
            UI_SCREEN_MULTIPLAYER_BATTLE,
            true
        );
        m_elements[scoreId].customPosition = XMFLOAT2(playerPanelPositions[i].x, playerPanelPositions[i].y + 55);
        m_elements[scoreId].linkedPlayerID = i;
        m_battleUIElementIds.push_back(scoreId);

        m_playerInfos[i].screenPosition = playerPanelPositions[i];
    }

    // 🔧 **只在右侧添加返回大厅按钮**
    int returnBtnId = AddAtlasButton(
        UI_POS_CUSTOM,
        "ButtonUI",
        "btn_ren_return",
        UI_CALLBACK_RETURN_TO_LOBBY,
        XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
        0.4f,  // 适中大小的按钮
        UI_SCREEN_MULTIPLAYER_BATTLE
    );
    m_elements[returnBtnId].customPosition = XMFLOAT2(SCREEN_WIDTH - 150, SCREEN_HEIGHT - 100);
    m_battleUIElementIds.push_back(returnBtnId);

    // 小地图切换按钮
    int minimapBtnId = AddButton(
        UI_POS_CUSTOM,
        L"MAP",
        XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
        UI_CALLBACK_TOGGLE_MINIMAP,
        1.5f,  // 增大文字
        UI_SCREEN_MULTIPLAYER_BATTLE
    );
    m_elements[minimapBtnId].customPosition = XMFLOAT2(SCREEN_WIDTH - 80, 20);
    m_elements[minimapBtnId].size = XMFLOAT2(60, 30);
    m_elements[minimapBtnId].backgroundColor = XMFLOAT4(0.2f, 0.4f, 0.6f, 0.8f);
    m_battleUIElementIds.push_back(minimapBtnId);

    // 投降按钮
    int surrenderBtnId = AddButton(
        UI_POS_CUSTOM,
        L"SURRENDER",
        XMFLOAT4(1.0f, 0.3f, 0.3f, 1.0f),
        UI_CALLBACK_SURRENDER,
        1.3f,  // 增大文字
        UI_SCREEN_MULTIPLAYER_BATTLE
    );
    m_elements[surrenderBtnId].customPosition = XMFLOAT2(SCREEN_WIDTH - 120, SCREEN_HEIGHT - 60);
    m_elements[surrenderBtnId].size = XMFLOAT2(100, 30);
    m_elements[surrenderBtnId].backgroundColor = XMFLOAT4(0.6f, 0.2f, 0.2f, 0.8f);
    m_battleUIElementIds.push_back(surrenderBtnId);

    // 控制提示 - 加大文字
    int controlsId = AddElement(
        UI_TEXT,
        UI_POS_BOTTOM_CENTER,
        L"[ESC] Pause  [M] Toggle Map  [TAB] Stats",
        XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f),
        1.6f,  // 增大提示文字
        true,
        UI_SCREEN_MULTIPLAYER_BATTLE,
        true
    );
    m_battleUIElementIds.push_back(controlsId);

    OutputDebugStringA("✅ 多人战斗UI屏幕初始化完成\n");
}
void UIManager::InitBattlePauseScreen() {
    // 暂停标题
    AddElement(
        UI_PANEL,
        UI_POS_MID_CENTER,
        L"BATTLE PAUSED",
        XMFLOAT4(1.0f, 1.0f, 0.3f, 1.0f),
        2.5f,
        true,
        UI_SCREEN_BATTLE_PAUSE,
        true
    );

    // 继续战斗按钮
    int continueBtnId = AddButton(
        UI_POS_CUSTOM,
        L"CONTINUE BATTLE",
        XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
        UI_CALLBACK_RESUME_GAME,
        2.0f,
        UI_SCREEN_BATTLE_PAUSE
    );
    m_elements[continueBtnId].customPosition = XMFLOAT2((SCREEN_WIDTH - 300.0f) * 0.5f, SCREEN_HEIGHT * 0.4f);
    m_elements[continueBtnId].size = XMFLOAT2(300.0f, 60.0f);
    m_elements[continueBtnId].backgroundColor = XMFLOAT4(0.2f, 0.6f, 0.4f, 0.8f);

    // 战斗设置按钮
    int settingsBtnId = AddButton(
        UI_POS_CUSTOM,
        L"BATTLE SETTINGS",
        XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
        UI_CALLBACK_BATTLE_SETTINGS,
        2.0f,
        UI_SCREEN_BATTLE_PAUSE
    );
    m_elements[settingsBtnId].customPosition = XMFLOAT2((SCREEN_WIDTH - 300.0f) * 0.5f, SCREEN_HEIGHT * 0.5f);
    m_elements[settingsBtnId].size = XMFLOAT2(300.0f, 60.0f);
    m_elements[settingsBtnId].backgroundColor = XMFLOAT4(0.4f, 0.4f, 0.6f, 0.8f);

    // 查看统计按钮
    int statsBtnId = AddButton(
        UI_POS_CUSTOM,
        L"VIEW STATISTICS",
        XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
        UI_CALLBACK_VIEW_STATS,
        2.0f,
        UI_SCREEN_BATTLE_PAUSE
    );
    m_elements[statsBtnId].customPosition = XMFLOAT2((SCREEN_WIDTH - 300.0f) * 0.5f, SCREEN_HEIGHT * 0.6f);
    m_elements[statsBtnId].size = XMFLOAT2(300.0f, 60.0f);
    m_elements[statsBtnId].backgroundColor = XMFLOAT4(0.6f, 0.4f, 0.2f, 0.8f);

    // 返回大厅按钮
    int lobbyBtnId = AddButton(
        UI_POS_CUSTOM,
        L"RETURN TO LOBBY",
        XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
        UI_CALLBACK_RETURN_TO_LOBBY,
        2.0f,
        UI_SCREEN_BATTLE_PAUSE
    );
    m_elements[lobbyBtnId].customPosition = XMFLOAT2((SCREEN_WIDTH - 300.0f) * 0.5f, SCREEN_HEIGHT * 0.7f);
    m_elements[lobbyBtnId].size = XMFLOAT2(300.0f, 60.0f);
    m_elements[lobbyBtnId].backgroundColor = XMFLOAT4(0.6f, 0.3f, 0.3f, 0.8f);

    // 暂停说明
    AddElement(
        UI_TEXT,
        UI_POS_BOTTOM_CENTER,
        L"Press [ESC] to return to battle",
        XMFLOAT4(0.9f, 0.9f, 0.3f, 1.0f),
        1.5f,
        true,
        UI_SCREEN_BATTLE_PAUSE,
        true
    );

    OutputDebugStringA("战斗暂停UI屏幕初始化完成\n");
}

// 初始化战斗结果屏幕
void UIManager::InitBattleResultScreen() {
    // 战斗结果标题
    AddElement(
        UI_PANEL,
        UI_POS_TOP_CENTER,
        L"BATTLE RESULTS",
        XMFLOAT4(1.0f, 0.8f, 0.2f, 1.0f),
        3.0f,
        true,
        UI_SCREEN_BATTLE_RESULT,
        true
    );

    // 获胜者显示
    int winnerId = AddElement(
        UI_TEXT,
        UI_POS_CUSTOM,
        L"WINNER: Player 1",
        XMFLOAT4(1.0f, 1.0f, 0.2f, 1.0f),
        2.5f,
        true,
        UI_SCREEN_BATTLE_RESULT,
        true
    );
    m_elements[winnerId].customPosition = XMFLOAT2(SCREEN_WIDTH / 2 - 150, 150);

    // 详细统计面板
    int statsId = AddElement(
        UI_PANEL,
        UI_POS_CUSTOM,
        L"Battle Statistics:\n\nTotal Time: 05:32\nTotal Eliminations: 12\nMost Active Player: Player 2",
        XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
        1.5f,
        true,
        UI_SCREEN_BATTLE_RESULT,
        true
    );
    m_elements[statsId].customPosition = XMFLOAT2(50, 220);
    m_elements[statsId].size = XMFLOAT2(SCREEN_WIDTH - 100, 200);
    m_elements[statsId].backgroundColor = XMFLOAT4(0.1f, 0.1f, 0.3f, 0.8f);

    // 玩家排名表
    for (int i = 0; i < 4; i++) {
        int rankId = AddElement(
            UI_TEXT,
            UI_POS_CUSTOM,
            L"1. Player 1 - Score: 1250",
            m_playerInfos[i].playerColor,
            1.3f,
            true,
            UI_SCREEN_BATTLE_RESULT,
            true
        );
        m_elements[rankId].customPosition = XMFLOAT2(SCREEN_WIDTH / 2 + 50, 250 + i * 30);
        m_elements[rankId].linkedPlayerID = i;
    }

    // 下一轮按钮
    int nextRoundBtnId = AddButton(
        UI_POS_CUSTOM,
        L"NEXT ROUND",
        XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
        UI_CALLBACK_NEXT_ROUND,
        2.0f,
        UI_SCREEN_BATTLE_RESULT
    );
    m_elements[nextRoundBtnId].customPosition = XMFLOAT2((SCREEN_WIDTH - 400.0f) * 0.3f, SCREEN_HEIGHT * 0.8f);
    m_elements[nextRoundBtnId].size = XMFLOAT2(180.0f, 60.0f);
    m_elements[nextRoundBtnId].backgroundColor = XMFLOAT4(0.2f, 0.6f, 0.2f, 0.8f);

    // 返回大厅按钮
    int lobbyBtnId = AddButton(
        UI_POS_CUSTOM,
        L"RETURN TO LOBBY",
        XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
        UI_CALLBACK_RETURN_TO_LOBBY,
        2.0f,
        UI_SCREEN_BATTLE_RESULT
    );
    m_elements[lobbyBtnId].customPosition = XMFLOAT2((SCREEN_WIDTH - 400.0f) * 0.7f, SCREEN_HEIGHT * 0.8f);
    m_elements[lobbyBtnId].size = XMFLOAT2(180.0f, 60.0f);
    m_elements[lobbyBtnId].backgroundColor = XMFLOAT4(0.6f, 0.3f, 0.3f, 0.8f);

    OutputDebugStringA("战斗结果UI屏幕初始化完成\n");
}

void UIManager::Update(float deltaTime) {
    if (!m_initialized) return;

    // 添加最小帧时间限制，防止更新过于频繁
    static float accumulatedTime = 0.0f;
    accumulatedTime += deltaTime;

    // 对于编辑模式，限制更新频率为每秒15次
    if (m_currentScreen == UI_SCREEN_EDIT) {
        if (accumulatedTime < 1.0f / 15.0f) {
            return;
        }
        accumulatedTime = 0.0f;
    }
    else {
        // 其他模式正常更新
        accumulatedTime = 0.0f;
    }

    
   //  Mouse_SetVisible(true);
    // 限制deltaTime防止大时间步长
    deltaTime = std::min(deltaTime, 0.05f);

    // 更新淡入淡出效果
    if (m_isFading) {
        m_fadeTime -= deltaTime;
        if (m_fadeTime <= 0.0f) {
            m_isFading = false;
            m_fadeAlpha = 1.0f;
        }
        else {
            // 计算alpha值(简单线性插值)
            m_fadeAlpha = m_fadeTime / 0.5f; // 0.5秒过渡时间
            if (m_fadeAlpha > 1.0f) m_fadeAlpha = 2.0f - m_fadeAlpha; // 先淡出再淡入
        }
    }
    // 更新多人战斗UI
    if (m_currentScreen == UI_SCREEN_MULTIPLAYER_BATTLE) {
        UpdateBattleUI(deltaTime);
    }
    // 临时元素更新
    for (auto& element : m_temporaryElements) {
        if (element.lifeTime > 0) {
            // 减慢衰减速度，使文本保持更长时间
            element.lifeTime -= deltaTime * 0.5f;

            // 确保最小不透明度
            if (element.lifeTime < 1.0f) {
                // 至少保持30%不透明度
                element.color.w = 0.3f + element.lifeTime * 0.7f;
            }
        }
    }
    // 只有生命值小于等于0的元素才会被移除
    m_temporaryElements.erase(
        std::remove_if(m_temporaryElements.begin(), m_temporaryElements.end(),
            [](const UIElement& element) { return element.lifeTime <= 0; }),
        m_temporaryElements.end()
    );

   

    // 处理键盘输入
    HandleKeyboardInput(deltaTime);

    // 为按钮添加悬停效果
    HandleButtonHoverEffects();
}

void UIManager::SwitchToScreen(UI_SCREEN_TYPE screenType, bool fadeEffect) {
    if (screenType == m_currentScreen) return;

    // 保存旧屏幕类型以进行清理
    UI_SCREEN_TYPE oldScreen = m_currentScreen;

    m_currentScreen = screenType;

    if (fadeEffect) {
        m_isFading = true;
        m_fadeTime = 1.0f; // 0.5 seconds fade out + 0.5 seconds fade in
    }

   
    // 确保当前屏幕的元素可见，其他屏幕的元素不可见
    for (auto& element : m_elements) {
        // 如果元素属于当前屏幕，确保它可见
        if (element.screenType == screenType) {
            element.visible = true;
        }
        // 如果元素属于其他屏幕，确保它不可见
        else {
            element.visible = false;
        }
    }
}

int UIManager::AddElement(UI_ELEMENT_TYPE type, UI_POSITION position,
    const wchar_t* text, XMFLOAT4 color,
    float scale, bool visible,
    UI_SCREEN_TYPE screenType,
    bool usePixelFont) {

    UIElement element;
    element.type = type;
    element.position = position;
    element.text = text;
    element.color = color;
    element.backgroundColor = XMFLOAT4(0.2f, 0.2f, 0.2f, 0.0f); // Transparent background
    element.scale = scale;
    element.visible = visible;
    element.selected = false;
    element.customPosition = XMFLOAT2(0.0f, 0.0f);
    element.size = XMFLOAT2(0.0f, 0.0f);
    element.lifeTime = -1.0f; // Permanent display
    element.icon = nullptr;
    element.usePixelFont = usePixelFont;
    element.screenType = screenType;
    element.callbackId = 0;

    // 初始化纹理相关成员
    element.useTexture = false;
    element.texture = nullptr;
    element.hoverColor = XMFLOAT4(1.2f, 1.2f, 1.2f, 1.0f);
    element.pressedColor = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
    element.atlas = nullptr;
    element.spriteNameKey = "";
    element.uvMin = XMFLOAT2(0.0f, 0.0f);
    element.uvMax = XMFLOAT2(1.0f, 1.0f);

    // 设置默认大小基于元素类型
    switch (type) {
    case UI_PANEL:
        element.size.x = 300.0f * scale;
        element.size.y = 200.0f * scale;
        break;
    case UI_BUTTON:
        element.size.x = 150.0f * scale;
        element.size.y = 40.0f * scale;
        break;
    default:
        // For text, size will be calculated dynamically during drawing
        break;
    }

    m_elements.push_back(element);
    return (int)(m_elements.size() - 1);
}

// Add temporary UI element
int UIManager::AddTemporaryElement(UI_ELEMENT_TYPE type, UI_POSITION position,
    const wchar_t* text, XMFLOAT4 color,
    float lifeTime, float scale,
    bool usePixelFont) {
    UIElement element;
    element.type = type;
    element.position = position;
    element.text = text;
    element.color = color;
    element.backgroundColor = XMFLOAT4(0.2f, 0.2f, 0.2f, 0.0f); // Transparent background
    element.scale = scale;
    element.visible = true;
    element.customPosition = XMFLOAT2(0.0f, 0.0f);
    element.size = XMFLOAT2(0.0f, 0.0f);
    element.lifeTime = lifeTime;
    element.icon = nullptr;
    element.usePixelFont = usePixelFont;

    m_temporaryElements.push_back(element);
    return (int)(m_temporaryElements.size() - 1);
}

int UIManager::AddAtlasButton(
    UI_POSITION position,
    const std::string& atlasName,
    const std::string& spriteName,
    UI_BUTTON_CALLBACK callbackId,
    XMFLOAT4 color,
    float scale,
    UI_SCREEN_TYPE screenType) {

    // 获取图集
    TextureAtlas* atlas = TextureAtlasManager::GetInstance()->GetAtlas(atlasName);
    if (!atlas) {
        OutputDebugStringA("错误：未找到指定的图集\n");
        return -1;
    }

    // 获取精灵信息
    const AtlasSprite* sprite = atlas->GetSprite(spriteName);
    if (!sprite) {
        OutputDebugStringA("错误：未找到指定的精灵\n");
        return -1;
    }

    // 创建按钮元素
    int elementId = AddElement(
        UI_BUTTON,
        position,
        L"",  // 空文本
        color,
        scale,
        true,
        screenType,
        false
    );

    if (elementId >= 0 && elementId < (int)m_elements.size()) {
        UIElement& element = m_elements[elementId];

        // 设置使用图集
        element.useTexture = true;
        element.atlas = atlas;
        element.spriteNameKey = spriteName;
        element.texture = atlas->GetTexture();  // 设置图集纹理

        // 设置UV坐标
        element.uvMin = sprite->uvMin;
        element.uvMax = sprite->uvMax;

        // 设置回调
        element.callbackId = callbackId;

        // 计算按钮大小
        element.size.x = sprite->pixelSize.x * scale;
        element.size.y = sprite->pixelSize.y * scale;

        // 设置交互颜色
        element.hoverColor = XMFLOAT4(1.2f, 1.2f, 1.2f, 1.0f);
        element.pressedColor = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);

        char debug[256];
        sprintf_s(debug, "创建图集按钮: %s/%s, UV=(%.3f,%.3f)-(%.3f,%.3f)\n",
            atlasName.c_str(), spriteName.c_str(),
            element.uvMin.x, element.uvMin.y, element.uvMax.x, element.uvMax.y);
        OutputDebugStringA(debug);
    }

    return elementId;
}

// Set UI element visibility
void UIManager::SetElementVisible(int elementId, bool visible) {
    if (elementId >= 0 && elementId < (int)m_elements.size()) {
        m_elements[elementId].visible = visible;
    }
}

// Remove UI element
void UIManager::RemoveElement(int elementId) {
    if (elementId >= 0 && elementId < (int)m_elements.size()) {
        m_elements.erase(m_elements.begin() + elementId);
    }
}

// Show controls hint
void UIManager::ShowControlsHint(const wchar_t* hint, float duration) {
    // Display control hint at the bottom of the screen
    AddTemporaryElement(
        UI_TEXT,
        UI_POS_BOTTOM_CENTER,
        hint,
        XMFLOAT4(0.9f, 0.9f, 0.2f, 1.0f),
        duration,
        1.8f,  // 增加文字大小
        true
    );
}

// Show status information
void UIManager::ShowStatusInfo(const wchar_t* info, XMFLOAT4 color, float duration) {
    // Display status information in the top right corner
    AddTemporaryElement(
        UI_TEXT,
        UI_POS_TOP_RIGHT,
        info,
        color,
        duration,
        1.5f,  // 增加文字大小
        true
    );
}

// Set UI scale
void UIManager::SetUIScale(float scale) {
    m_uiScale = scale;
}

// Get current UI screen type
UI_SCREEN_TYPE UIManager::GetCurrentScreen() {
    return m_currentScreen;
}

// Calculate screen coordinates based on position
XMFLOAT2 UIManager::CalculatePosition(UI_POSITION position, XMFLOAT2 size) {
    XMFLOAT2 result = XMFLOAT2(0.0f, 0.0f);

    // Get screen dimensions
    float screenWidth = (float)SCREEN_WIDTH;
    float screenHeight = (float)SCREEN_HEIGHT;

    // Use percentage-based margins for better scaling across resolutions
    float marginX = screenWidth * 0.02f;  // 2% of screen width
    float marginY = screenHeight * 0.02f; // 2% of screen height

    // Ensure minimum size to prevent division by zero
    if (size.x <= 0.0f) size.x = 10.0f;
    if (size.y <= 0.0f) size.y = 10.0f;

    // Calculate coordinates based on position
    switch (position) {
    case UI_POS_TOP_LEFT:
        result.x = marginX;
        result.y = marginY;
        break;

    case UI_POS_TOP_CENTER:
        result.x = (screenWidth - size.x) * 0.5f;
        result.y = marginY;
        break;

    case UI_POS_TOP_RIGHT:
        result.x = screenWidth - size.x - marginX;
        result.y = marginY;
        break;

    case UI_POS_MID_LEFT:
        result.x = marginX;
        result.y = (screenHeight - size.y) * 0.5f;
        break;

    case UI_POS_MID_CENTER:
        result.x = (screenWidth - size.x) * 0.5f;
        result.y = (screenHeight - size.y) * 0.5f;
        break;

    case UI_POS_MID_RIGHT:
        result.x = screenWidth - size.x - marginX;
        result.y = (screenHeight - size.y) * 0.5f;
        break;

    case UI_POS_BOTTOM_LEFT:
        result.x = marginX;
        result.y = screenHeight - size.y - marginY;
        break;

    case UI_POS_BOTTOM_CENTER:
        result.x = (screenWidth - size.x) * 0.5f;
        result.y = screenHeight - size.y - marginY;
        break;

    case UI_POS_BOTTOM_RIGHT:
        result.x = screenWidth - size.x - marginX;
        result.y = screenHeight - size.y - marginY;
        break;

    default:
        // UI_POS_CUSTOM should not call this function
        OutputDebugStringA("Warning: CalculatePosition called with UI_POS_CUSTOM\n");
        break;
    }

    // Add safety bounds checking
    if (result.x < 0) result.x = 0;
    if (result.y < 0) result.y = 0;
    if (result.x > screenWidth - size.x) result.x = screenWidth - size.x;
    if (result.y > screenHeight - size.y) result.y = screenHeight - size.y;

    return result;
}
// Initialize game screen
void UIManager::InitGameScreen() {
    // Add game mode UI elements

    // Game title
    AddElement(
        UI_PANEL,
        UI_POS_TOP_CENTER,
        L"MAP EDITOR GAME MODE",
        XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
        2.0f,  // 增加标题大小
        true,
        UI_SCREEN_GAME,
        true
    );

    

    // Add common key hints (top right)
    AddElement(
        UI_PANEL,
        UI_POS_TOP_RIGHT,
        L"Help Tips\n[F1] Help\n[ESC] Pause\n[F5] Save Map\n[F8] Load Map",
        XMFLOAT4(0.9f, 0.9f, 0.3f, 1.0f),
        1.2f,  // 增加文字大小
        true,
        UI_SCREEN_GAME,
        true
    );
    // Add coin counter UI element (NEW)
    int coinUIId = AddElement(
        UI_ICON_TEXT,
        UI_POS_TOP_LEFT,
        L"Coins: 0",
        XMFLOAT4(1.0f, 0.84f, 0.0f, 1.0f),  // Gold color
        1.5f,  // 增加文字大小
        true,
        UI_SCREEN_GAME,
        true
    );

    // Set coin icon texture
    m_elements[coinUIId].icon = GET_TEXTURE(COIN);
    m_elements[coinUIId].customPosition = XMFLOAT2(20.0f, 20.0f);
    m_elements[coinUIId].size = XMFLOAT2(200.0f, 40.0f);
}



int UIManager::AddButton(UI_POSITION position, const wchar_t* text,
    XMFLOAT4 color, UI_BUTTON_CALLBACK callbackId,
    float scale, UI_SCREEN_TYPE screenType) {

    // 创建一个按钮类型的UI元素
    int elementId = AddElement(
        UI_BUTTON,
        position,
        text,
        color,
        scale,
        true,
        screenType,
        true
    );

    // 设置按钮特有的属性
    if (elementId >= 0 && elementId < (int)m_elements.size()) {
        // 设置回调ID
        m_elements[elementId].callbackId = callbackId;

        // 设置默认按钮大小
        m_elements[elementId].size = XMFLOAT2(250.0f, 60.0f);

        // 设置默认按钮背景颜色
        m_elements[elementId].backgroundColor = XMFLOAT4(0.3f, 0.3f, 0.7f, 0.8f);
    }

    return elementId;
}

// 实现分数、生命值和金币显示更新函数
void UIManager::UpdateScoreDisplay(int score) {
    m_score = score;

    // 查找分数元素并更新它
    for (size_t i = 0; i < m_elements.size(); i++) {
        auto& element = m_elements[i];
        if (element.position == UI_POS_TOP_LEFT &&
            element.type == UI_TEXT &&
            element.text.find(L"SCORE") != std::wstring::npos) {
            wchar_t scoreText[32];
            swprintf_s(scoreText, L"SCORE: %d", score);
            element.text = scoreText;

          
            return;
        }
    }

    // 如果没有找到，创建一个新的分数显示
    if (m_scoreElementId < 0) {
        m_scoreElementId = AddElement(
            UI_TEXT,
            UI_POS_TOP_LEFT,
            L"SCORE: 0",
            XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
            1.5f,
            true,
            UI_SCREEN_GAME,
            true
        );
    }
}

void UIManager::UpdateLivesDisplay(int lives) {
    m_lives = lives;

    // 查找生命值元素并更新它
    for (size_t i = 0; i < m_elements.size(); i++) {
        auto& element = m_elements[i];
        if (element.position == UI_POS_TOP_LEFT &&
            element.type == UI_TEXT &&
            element.text.find(L"LIFE") != std::wstring::npos) {
            wchar_t scoreText[32];
            swprintf_s(scoreText, L"LIFE: %d", lives);
            element.text = scoreText;


            return;
        }
    }

    // 如果没有找到，创建一个新的分数显示
    if (m_livesElementId < 0) {
        m_livesElementId = AddElement(
            UI_TEXT,
            UI_POS_TOP_LEFT,
            L"LIFE: 0",
            XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
            1.5f,
            true,
            UI_SCREEN_GAME,
            true
        );
    }
}

// 使用更简洁的方法更新金币显示
void UIManager::UpdateCoinDisplay(int coins) {
    m_coins = coins;

    // 查找并更新金币计数器
    bool found = false;
    for (auto& element : m_elements) {
        if (element.type == UI_ICON_TEXT &&
            element.text.find(L"GOLE") != std::wstring::npos) {

            // 更新计数
            wchar_t coinText[32];
            swprintf_s(coinText, L"GOLD: %d", coins);
            element.text = coinText;

            // 添加短暂的视觉反馈（放大效果）
            element.scale = 1.8f; // 暂时放大
            found = true;
            break;
        }
    }

    // 如果没有找到，创建新的金币显示
    if (!found) {
        int id = AddElement(
            UI_ICON_TEXT,
            UI_POS_CUSTOM,
            L"GOLD: 0",
            XMFLOAT4(1.0f, 0.84f, 0.0f, 1.0f),
            1.5f,
            true,
            UI_SCREEN_GAME,
            true
        );

        // 设置位置和图标
        m_elements[id].customPosition = XMFLOAT2(20.0f, 70.0f); // 放在分数下方
        m_elements[id].icon = GET_TEXTURE(COIN);
    }
}
// 改进的UIManager::HandleMouseEvents函数
void UIManager::HandleMouseEvents() {
    // 获取鼠标状态
    Mouse_State mouse;
    Mouse_GetState(&mouse);

    // 获取实际窗口尺寸
    RECT clientRect;
    GetClientRect(g_hWnd, &clientRect);
    float windowWidth = (float)(clientRect.right - clientRect.left);
    float windowHeight = (float)(clientRect.bottom - clientRect.top);

    // 将鼠标坐标转换为UI坐标系统
    float uiX = (mouse.x / windowWidth) * SCREEN_WIDTH;
    float uiY = (mouse.y / windowHeight) * SCREEN_HEIGHT;

  

    static bool wasLeftButtonDown = false;
    bool isLeftButtonDown = mouse.leftButton;

    if (!isLeftButtonDown && wasLeftButtonDown) {
        for (size_t i = 0; i < m_elements.size(); i++) {
            auto& element = m_elements[i];

            // 只检查当前屏幕上可见的按钮
            if (element.visible && element.type == UI_BUTTON &&
                element.screenType == m_currentScreen) {

                // 计算按钮位置
                XMFLOAT2 pos;
                if (element.position == UI_POS_CUSTOM) {
                    pos = element.customPosition;
                }
                else {
                    pos = CalculatePosition(element.position, element.size);
                }


                // 更宽松的点击区域
                const float CLICK_MARGIN = 20.0f;

                // 检查转换后的鼠标坐标是否在按钮区域内
                if (uiX >= pos.x - CLICK_MARGIN &&
                    uiX <= pos.x + element.size.x + CLICK_MARGIN &&
                    uiY >= pos.y - CLICK_MARGIN &&
                    uiY <= pos.y + element.size.y + CLICK_MARGIN) {

                  

                    // 添加视觉反馈
                    element.selected = true;

                    // 处理回调
                    HandleButtonClick(element.callbackId);

                    // 重置选择状态
                    element.selected = false;

                    break;
                }
            }
        }
    }

    wasLeftButtonDown = isLeftButtonDown;
}
// 根据回调ID处理按钮点击
// 改进的UIManager::HandleButtonClick函数
// 增强的HandleButtonClick函数，支持多人战斗回调
void UIManager::HandleButtonClick(int callbackId) {
    char debug[128];
    sprintf_s(debug, "处理按钮回调ID: %d\n", callbackId);
    OutputDebugStringA(debug);

    switch (callbackId) {
    case UI_CALLBACK_START_GAME:
        OutputDebugStringA("开始游戏按钮被点击！\n");
        SwitchToGameMode();
        break;

    case UI_CALLBACK_RESUME_GAME: {
        OutputDebugStringA("继续游戏按钮被点击！\n");
        AppMode currentMode = GetCurrentAppMode();
        if (currentMode == APP_MODE_MULTIPLAYER_BATTLE) {
            SwitchToScreen(UI_SCREEN_MULTIPLAYER_BATTLE);
        }
        else {
            SwitchToGameMode();
        }
        break;
    }
    case UI_CALLBACK_RETURN_MAINMENU:
        OutputDebugStringA("返回主菜单按钮被点击！\n");
        if (IsCharacterSceneActive()) {
            DeactivateCharacterScene();
        }
        if (BattleHallManager::IsFromBattleHall()) {
            DeactivateBattleHall();
        }
        // 🔧 如果在多人战斗中，先结束战斗
        if (GetCurrentAppMode() == APP_MODE_MULTIPLAYER_BATTLE) {
            MultiplayerBattleManager::EndBattle();
        }
        ReturnToMainMenu();
        break;

    case UI_CALLBACK_MAP_EDITOR:
        OutputDebugStringA("地图编辑器按钮被点击！\n");
        SwitchToMode(APP_MODE_EDIT);
        break;

    case UI_CALLBACK_OPTIONS:
        OutputDebugStringA("选项按钮被点击！\n");
        SwitchToScreen(UI_SCREEN_CONTROLS);
        break;

    case UI_CALLBACK_EXIT:
        OutputDebugStringA("退出按钮被点击！\n");
        PostQuitMessage(0);
        break;

    case UI_CALLBACK_RESTART:
        OutputDebugStringA("重新开始按钮被点击！\n");
        SetPlayerScore(0);
        SetPlayerLives(3);
        SwitchToGameMode();
        break;

    case UI_CALLBACK_SAVE_MAP:
        OutputDebugStringA("保存地图按钮被点击！\n");
        if (SaveCurrentMap()) {
            ShowStatusInfo(L"地图保存成功！", XMFLOAT4(0.2f, 1.0f, 0.2f, 1.0f), 3.0f);
        }
        else {
            ShowStatusInfo(L"地图保存失败", XMFLOAT4(1.0f, 0.2f, 0.2f, 1.0f), 3.0f);
        }
        break;

    case UI_CALLBACK_LOAD_MAP:
        OutputDebugStringA("加载地图按钮被点击！\n");
        if (LoadCurrentMap()) {
            ShowStatusInfo(L"地图加载成功！", XMFLOAT4(0.2f, 1.0f, 0.2f, 1.0f), 3.0f);
        }
        else {
            ShowStatusInfo(L"地图加载失败", XMFLOAT4(1.0f, 0.2f, 0.2f, 1.0f), 3.0f);
        }
        break;

    case UI_CALLBACK_SHOP:
        OutputDebugStringA("商店按钮被点击！\n");
        SwitchToScreen(UI_SCREEN_SHOP);
        break;

    case UI_CALLBACK_CHARACTER:
        OutputDebugStringA("角色按钮被点击！\n");
        SwitchToMode(APP_MODE_CHARACTER);
        ActivateCharacterScene();
        SwitchToScreen(UI_SCREEN_CHARACTER);
        break;

    case UI_CALLBACK_BATTLE_HALL:
        OutputDebugStringA("战斗大厅按钮被点击！\n");
        SwitchToBattleHallMode();
        break;

        // 🔧 新增多人战斗相关回调处理
    case UI_CALLBACK_MULTIPLAYER_BATTLE:
        OutputDebugStringA("多人战斗按钮被点击！\n");
        SwitchToMultiplayerBattleMode();
        break;

    case UI_CALLBACK_SURRENDER:
        OutputDebugStringA("投降按钮被点击！\n");
        ShowBattleMessage(L"You have surrendered!", XMFLOAT4(1.0f, 0.3f, 0.3f, 1.0f), 3.0f);
        // 这里可以添加投降逻辑
        break;

    case UI_CALLBACK_TOGGLE_MINIMAP:
        OutputDebugStringA("小地图切换按钮被点击！\n");
        ToggleMinimap();
        break;

    case UI_CALLBACK_BATTLE_SETTINGS:
        OutputDebugStringA("战斗设置按钮被点击！\n");
        ShowStatusInfo(L"Battle settings coming soon!", XMFLOAT4(0.8f, 0.8f, 0.2f, 1.0f), 2.0f);
        break;

    case UI_CALLBACK_VIEW_STATS:
        OutputDebugStringA("查看统计按钮被点击！\n");
        ShowBattleMessage(L"Battle Statistics - Check player panels for details",
            XMFLOAT4(0.5f, 1.0f, 1.0f, 1.0f), 5.0f);
        break;

    case UI_CALLBACK_NEXT_ROUND:
        OutputDebugStringA("下一轮按钮被点击！\n");
        SetBattleRound(m_battleRound + 1);
        SwitchToScreen(UI_SCREEN_MULTIPLAYER_BATTLE);
        ShowBattleMessage(L"Starting next round!", XMFLOAT4(0.2f, 1.0f, 0.2f, 1.0f), 3.0f);
        break;

    case UI_CALLBACK_RETURN_TO_LOBBY:
        OutputDebugStringA("返回大厅按钮被点击！\n");
        if (GetCurrentAppMode() == APP_MODE_MULTIPLAYER_BATTLE) {
            MultiplayerBattleManager::EndBattle();
        }
        SwitchToBattleHallMode();
        break;

    case UI_CALLBACK_END_BATTLE:
        OutputDebugStringA("结束战斗按钮被点击！\n");
        MultiplayerBattleManager::EndBattle();
        SwitchToScreen(UI_SCREEN_BATTLE_RESULT);
        break;

    default:
        sprintf_s(debug, "未知的按钮回调ID: %d\n", callbackId);
        OutputDebugStringA(debug);
        break;
    }
}


void UIManager::Draw() {
    if (!m_initialized) {
        OutputDebugStringA("Warning: UI system not initialized!\n");
        return;
    }
    // 设置2D渲染状态
    SetWorldViewProjection2D();
    SetDepthEnable(false);
    SetBlendState(true);
    SetCulingMode(D3D11_CULL_NONE);

    XMMATRIX identityMatrix = XMMatrixIdentity();
    SetWorldMatrix(identityMatrix);

    // 保存当前渲染状态
    bool oldDepthEnable = GetDepthEnable();
    bool oldBlendEnable = GetBlendState();
    D3D11_CULL_MODE oldCullMode = GetCullingMode();

    // 设置UI渲染状态
    SetDepthEnable(false);
    SetBlendState(true);
    SetCulingMode(D3D11_CULL_NONE);
    SetWorldViewProjection2D();

    // Alpha混合设置
    D3D11_BLEND_DESC blendDesc;
    ZeroMemory(&blendDesc, sizeof(blendDesc));
    blendDesc.RenderTarget[0].BlendEnable = TRUE;
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    ID3D11BlendState* pBlendState = nullptr;
    HRESULT hr = GetDevice()->CreateBlendState(&blendDesc, &pBlendState);
    if (SUCCEEDED(hr)) {
        float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
        GetDeviceContext()->OMSetBlendState(pBlendState, blendFactor, 0xffffffff);
        pBlendState->Release();
    }

    // 绘制所有可见的UI元素
    for (size_t i = 0; i < m_elements.size(); i++) {
        const UIElement& element = m_elements[i];

     /*   char debugVisibility[256];
        sprintf_s(debugVisibility, "UI元素 %zu: 可见=%d, 当前屏幕=%d, 元素屏幕=%d\n",
            i, element.visible ? 1 : 0, (int)m_currentScreen, (int)element.screenType);
        OutputDebugStringA(debugVisibility);*/

        // 检查元素是否应该显示
        if (!element.visible) continue;
        if (element.screenType != m_currentScreen) continue;



        // 计算位置
        XMFLOAT2 pos;
        if (element.position == UI_POS_CUSTOM) {
            pos = element.customPosition;
        }
        else {
            pos = CalculatePosition(element.position, element.size);
        }

        // 应用当前alpha值
        XMFLOAT4 drawColor = element.color;
        if (m_isFading) {
            drawColor.w *= m_fadeAlpha;
        }

        // 根据元素类型绘制
        switch (element.type) {
        case UI_TEXT:
            // 直接绘制文本
            GeometricTextRenderer::DrawMinecraftText(
                pos.x, pos.y,
                element.text.c_str(),
                drawColor,
                element.scale * m_uiScale,
                false  // 不居中
            );
            break;

        case UI_PANEL: {
            // 绘制面板背景
            XMMATRIX panelWorld = XMMatrixTranslation(
                pos.x + element.size.x * 0.5f,
                pos.y + element.size.y * 0.5f,
                0.0f);
            SetWorldMatrix(panelWorld);

            // 设置材质
            MATERIAL material;
            ZeroMemory(&material, sizeof(MATERIAL));
            material.Diffuse = element.backgroundColor;
            SetMaterial(material);

            // 绘制背景
            DrawSprite(element.size, element.backgroundColor);

            // 绘制面板文本
            if (!element.text.empty()) {
                GeometricTextRenderer::DrawMinecraftText(
                    pos.x + 10.0f, pos.y + 10.0f,
                    element.text.c_str(),
                    drawColor,
                    element.scale * m_uiScale,
                    false
                );
            }
            break;
        }

        case UI_PROGRESS_BAR: {
            // 🔧 新增：绘制进度条
            DrawProgressBar(pos, element.size, element.currentValue, element.maxValue, drawColor);

            // 绘制进度条文本
            if (!element.text.empty()) {
                GeometricTextRenderer::DrawMinecraftText(
                    pos.x + 5, pos.y - 20,
                    element.text.c_str(),
                    drawColor,
                    element.scale * m_uiScale,
                    false
                );
            }
            break;
        }

        case UI_PLAYER_INFO: {
            // 🔧 新增：绘制玩家信息面板
            if (element.linkedPlayerID >= 0 && element.linkedPlayerID < 4) {
                DrawPlayerInfoPanel(m_playerInfos[element.linkedPlayerID], pos);
            }
            break;
        }
        case UI_BUTTON: {
            if (element.useTexture && element.texture) {
                XMMATRIX buttonWorld = XMMatrixTranslation(
                    pos.x + element.size.x * 0.5f,
                    pos.y + element.size.y * 0.5f,
                    0.0f);
                SetWorldMatrix(buttonWorld);

                MATERIAL material;
                ZeroMemory(&material, sizeof(MATERIAL));

                if (element.selected) {
                    material.Diffuse = element.pressedColor;
                }
                else {
                    material.Diffuse = element.color;
                }

                SetMaterial(material);
                GetDeviceContext()->PSSetShaderResources(0, 1, &element.texture);
                DrawSpriteUV(element.size, material.Diffuse, element.uvMin, element.uvMax);

                ID3D11ShaderResourceView* nullSRV = NULL;
                GetDeviceContext()->PSSetShaderResources(0, 1, &nullSRV);
            }
            else {
                // 绘制背景
                if (element.backgroundColor.w > 0) {
                    XMMATRIX buttonWorld = XMMatrixTranslation(
                        pos.x + element.size.x * 0.5f,
                        pos.y + element.size.y * 0.5f,
                        0.0f);
                    SetWorldMatrix(buttonWorld);

                    MATERIAL material;
                    ZeroMemory(&material, sizeof(MATERIAL));
                    material.Diffuse = element.backgroundColor;
                    SetMaterial(material);

                    DrawSprite(element.size, element.backgroundColor);
                }

                // 绘制文本
                if (!element.text.empty()) {
                    float textWidth = GeometricTextRenderer::MeasureText(
                        element.text.c_str(), element.scale).x;
                    float textX = pos.x + (element.size.x - textWidth) * 0.5f;
                    float textY = pos.y + (element.size.y - 30.0f) * 0.5f;

                    GeometricTextRenderer::DrawMinecraftText(
                        textX, textY,
                        element.text.c_str(),
                        drawColor,
                        element.scale * m_uiScale,
                        false
                    );
                }
            }
            break;
        }
        case UI_ICON_TEXT: {
            // 绘制图标
            if (element.icon) {
                XMMATRIX iconWorld = XMMatrixTranslation(
                    pos.x + 20.0f,
                    pos.y + 20.0f,
                    0.0f);
                SetWorldMatrix(iconWorld);

                // 设置材质
                MATERIAL material;
                ZeroMemory(&material, sizeof(MATERIAL));
                material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
                SetMaterial(material);

                // 绑定图标纹理
                GetDeviceContext()->PSSetShaderResources(0, 1, &element.icon);

                // 绘制图标
                DrawSprite(XMFLOAT2(30.0f, 30.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

                // 解绑纹理
                ID3D11ShaderResourceView* nullSRV = NULL;
                GetDeviceContext()->PSSetShaderResources(0, 1, &nullSRV);
            }

            // 绘制文本
            if (!element.text.empty()) {
                GeometricTextRenderer::DrawMinecraftText(
                    pos.x + 60.0f, pos.y + 10.0f,
                    element.text.c_str(),
                    drawColor,
                    element.scale * m_uiScale,
                    false
                );
            }
            break;
        }


        }
        // 对于临时UI元素
        for (const auto& element : m_temporaryElements) {
            XMFLOAT2 pos;
            if (element.position == UI_POS_CUSTOM) {
                pos = element.customPosition;
            }
            else {
                pos = CalculatePosition(element.position, element.size);
            }

            // 应用当前alpha值
            XMFLOAT4 drawColor = element.color;
            if (m_isFading) {
                drawColor.w *= m_fadeAlpha;
            }

            // 使用DrawMinecraftText替代DrawPixelText
            GeometricTextRenderer::DrawMinecraftText(
                pos.x, pos.y,
                element.text.c_str(),
                drawColor,
                element.scale * m_uiScale,
                false
            );
        }
        // 🔧 多人战斗屏幕特殊绘制
        if (m_currentScreen == UI_SCREEN_MULTIPLAYER_BATTLE) {
            // 绘制小地图
            DrawBattleMinimap();

            // 绘制额外的战斗效果
            // 例如：击杀提示、特殊效果等
        }

        // 恢复渲染状态
        SetDepthEnable(oldDepthEnable);
        SetBlendState(oldBlendEnable);
        SetCulingMode(oldCullMode);
    }
}

// 更新UI元素文本
void UIManager::UpdateElementText(int elementId, const wchar_t* newText) {
    if (elementId >= 0 && elementId < (int)m_elements.size()) {
        m_elements[elementId].text = newText;
    }
}

// 更新UI元素颜色
void UIManager::UpdateElementColor(int elementId, XMFLOAT4 newColor) {
    if (elementId >= 0 && elementId < (int)m_elements.size()) {
        m_elements[elementId].color = newColor;
    }
}

// 设置UI元素自定义位置
void UIManager::SetElementCustomPosition(int elementId, XMFLOAT2 position) {
    if (elementId >= 0 && elementId < (int)m_elements.size()) {
        m_elements[elementId].position = UI_POS_CUSTOM;
        m_elements[elementId].customPosition = position;
    }
}


//多人战斗方法实现UI
// 更新多人战斗UI
// 多人战斗UI公共方法实现

void UIManager::InitializeBattleUI() {
    OutputDebugStringA("🎮 初始化多人战斗UI\n");

    // 重置战斗状态
    m_battleUIState = BATTLE_UI_WAITING;
    m_battleTimer = 0.0f;
    m_battleRound = 1;
    m_showMinimap = true;

    // 重置玩家信息
    for (int i = 0; i < 4; i++) {
        m_playerInfos[i].health = m_playerInfos[i].maxHealth;
        m_playerInfos[i].score = 0;
        m_playerInfos[i].isAlive = true;
        m_playerInfos[i].isReady = false;
    }

    // 切换到多人战斗屏幕
    SwitchToScreen(UI_SCREEN_MULTIPLAYER_BATTLE);

    OutputDebugStringA("✅ 多人战斗UI初始化完成\n");
}

void UIManager::SetBattleUIState(BATTLE_UI_STATE state) {
    m_battleUIState = state;

    char debug[128];
    sprintf_s(debug, "🎮 战斗UI状态变更: %d\n", (int)state);
    OutputDebugStringA(debug);

    // 根据状态更新UI显示
    switch (state) {
    case BATTLE_UI_WAITING:
        ShowBattleMessage(L"Waiting for battle to start...", XMFLOAT4(1.0f, 1.0f, 0.2f, 1.0f), 3.0f);
        break;
    case BATTLE_UI_ACTIVE:
        ShowBattleMessage(L"Battle Started!", XMFLOAT4(0.2f, 1.0f, 0.2f, 1.0f), 2.0f);
        break;
    case BATTLE_UI_PAUSED:
        SwitchToScreen(UI_SCREEN_BATTLE_PAUSE);
        break;
    case BATTLE_UI_FINISHED:
        SwitchToScreen(UI_SCREEN_BATTLE_RESULT);
        break;
    }
}
void UIManager::UpdateBattleUI(float deltaTime) {
    // 更新战斗计时器
    m_battleTimer += deltaTime;

    // 更新计时器显示
    for (auto& element : m_elements) {
        if (element.screenType == UI_SCREEN_MULTIPLAYER_BATTLE &&
            element.text.find(L"TIME:") != std::wstring::npos) {
            int minutes = (int)m_battleTimer / 60;
            int seconds = (int)m_battleTimer % 60;
            wchar_t timeText[32];
            swprintf_s(timeText, L"TIME: %02d:%02d", minutes, seconds);
            element.text = timeText;
            break;
        }
    }

    // 更新玩家信息显示
    for (int i = 0; i < 4; i++) {
        UpdatePlayerUIElements(i);
    }

    // 更新动画效果
    UpdateBattleAnimations(deltaTime);
}

// 更新玩家UI元素
void UIManager::UpdatePlayerUIElements(int playerID) {
    if (playerID < 0 || playerID >= 4) return;

    const PlayerUIInfo& playerInfo = m_playerInfos[playerID];

    for (auto& element : m_elements) {
        if (element.linkedPlayerID == playerID &&
            element.screenType == UI_SCREEN_MULTIPLAYER_BATTLE) {

            // 更新玩家名称
            if (element.type == UI_TEXT &&
                element.text.find(L"Player") != std::wstring::npos) {
                element.text = playerInfo.playerName;
                element.color = playerInfo.playerColor;

                // 如果玩家死亡，显示灰色
                if (!playerInfo.isAlive) {
                    element.color = XMFLOAT4(0.5f, 0.5f, 0.5f, 0.7f);
                }
            }

            // 更新血量条
            else if (element.type == UI_PROGRESS_BAR) {
                element.currentValue = (float)playerInfo.health;
                element.maxValue = (float)playerInfo.maxHealth;

                wchar_t healthText[32];
                swprintf_s(healthText, L"HP: %d/%d", playerInfo.health, playerInfo.maxHealth);
                element.text = healthText;

                // 根据血量设置颜色
                float healthRatio = (float)playerInfo.health / (float)playerInfo.maxHealth;
                if (healthRatio > 0.6f) {
                    element.color = XMFLOAT4(0.2f, 1.0f, 0.2f, 1.0f); // 绿色
                }
                else if (healthRatio > 0.3f) {
                    element.color = XMFLOAT4(1.0f, 1.0f, 0.2f, 1.0f); // 黄色
                }
                else {
                    element.color = XMFLOAT4(1.0f, 0.2f, 0.2f, 1.0f); // 红色
                    element.isBlinking = true; // 低血量时闪烁
                }
            }

            // 更新分数显示
            else if (element.type == UI_TEXT &&
                element.text.find(L"Score:") != std::wstring::npos) {
                wchar_t scoreText[32];
                swprintf_s(scoreText, L"Score: %d", playerInfo.score);
                element.text = scoreText;
            }
        }
    }
}

// 更新战斗动画
void UIManager::UpdateBattleAnimations(float deltaTime) {
    for (auto& element : m_elements) {
        if (element.screenType != UI_SCREEN_MULTIPLAYER_BATTLE) continue;

        // 更新动画计时器
        element.animationTimer += deltaTime;

        // 闪烁效果
        if (element.isBlinking) {
            float blinkSpeed = 3.0f;
            float alpha = 0.5f + 0.5f * sinf(element.animationTimer * blinkSpeed);
            element.color.w = alpha;
        }

        // 脉冲效果（用于重要通知）
        if (element.type == UI_TEXT && element.text.find(L"WINNER:") != std::wstring::npos) {
            float pulseSpeed = 2.0f;
            float scale = 1.0f + 0.2f * sinf(element.animationTimer * pulseSpeed);
            element.scale = scale * 2.5f; // 基础缩放 * 脉冲效果
        }
    }
}

// 处理键盘输入
void UIManager::HandleKeyboardInput(float deltaTime) {
    // 处理键盘输入 - F1显示帮助
    static bool lastF1 = false;
    bool currentF1 = Keyboard_IsKeyDown(KK_F1);
    if (currentF1 && !lastF1) {
        if (m_currentScreen != UI_SCREEN_HELP) {
            SwitchToScreen(UI_SCREEN_HELP);
        }
        else {
            // 返回上一个屏幕
            if (GetCurrentAppMode() == APP_MODE_EDIT) {
                SwitchToScreen(UI_SCREEN_EDIT);
            }
            else if (GetCurrentAppMode() == APP_MODE_MULTIPLAYER_BATTLE) {
                SwitchToScreen(UI_SCREEN_MULTIPLAYER_BATTLE);
            }
            else {
                SwitchToScreen(UI_SCREEN_GAME);
            }
        }
    }
    lastF1 = currentF1;

    // 处理ESC键 - 暂停或返回
    static bool lastESC = false;
    bool currentESC = Keyboard_IsKeyDown(KK_ESCAPE);
    if (currentESC && !lastESC) {
        if (m_currentScreen == UI_SCREEN_MULTIPLAYER_BATTLE) {
            SwitchToScreen(UI_SCREEN_BATTLE_PAUSE);
        }
        else if (m_currentScreen == UI_SCREEN_BATTLE_PAUSE) {
            SwitchToScreen(UI_SCREEN_MULTIPLAYER_BATTLE);
        }
        else if (m_currentScreen != UI_SCREEN_PAUSE) {
            SwitchToScreen(UI_SCREEN_PAUSE);
        }
        else {
            // 返回上一个屏幕
            if (GetCurrentAppMode() == APP_MODE_EDIT) {
                SwitchToScreen(UI_SCREEN_EDIT);
            }
            else if (GetCurrentAppMode() == APP_MODE_MULTIPLAYER_BATTLE) {
                SwitchToScreen(UI_SCREEN_MULTIPLAYER_BATTLE);
            }
            else {
                SwitchToScreen(UI_SCREEN_GAME);
            }
        }
    }
    lastESC = currentESC;

    // 处理M键 - 切换小地图（仅在多人战斗中）
    if (m_currentScreen == UI_SCREEN_MULTIPLAYER_BATTLE) {
        static bool lastM = false;
        bool currentM = Keyboard_IsKeyDown(KK_M);
        if (currentM && !lastM) {
            ToggleMinimap();
        }
        lastM = currentM;
    }

    // 处理TAB键 - 显示统计信息（仅在多人战斗中）
    if (m_currentScreen == UI_SCREEN_MULTIPLAYER_BATTLE) {
        static bool lastTab = false;
        bool currentTab = Keyboard_IsKeyDown(KK_TAB);
        if (currentTab && !lastTab) {
            ShowBattleMessage(L"Statistics: Check player panels for details",
                XMFLOAT4(0.5f, 1.0f, 1.0f, 1.0f), 3.0f);
        }
        lastTab = currentTab;
    }

    // 处理F5/F8保存加载（仅在编辑模式）
    if (GetCurrentAppMode() == APP_MODE_EDIT) {
        // F5保存地图快捷键
        static bool lastF5 = false;
        bool currentF5 = Keyboard_IsKeyDown(KK_F5);
        if (currentF5 && !lastF5) {
            if (SaveCurrentMap()) {
                ShowStatusInfo(L"地图保存成功！", XMFLOAT4(0.2f, 1.0f, 0.2f, 1.0f), 3.0f);
            }
            else {
                ShowStatusInfo(L"地图保存失败", XMFLOAT4(1.0f, 0.2f, 0.2f, 1.0f), 3.0f);
            }
        }
        lastF5 = currentF5;

        // F8加载地图快捷键
        static bool lastF8 = false;
        bool currentF8 = Keyboard_IsKeyDown(KK_F8);
        if (currentF8 && !lastF8) {
            if (LoadCurrentMap()) {
                ShowStatusInfo(L"地图加载成功！", XMFLOAT4(0.2f, 1.0f, 0.2f, 1.0f), 3.0f);
            }
            else {
                ShowStatusInfo(L"地图加载失败", XMFLOAT4(1.0f, 0.2f, 0.2f, 1.0f), 3.0f);
            }
        }
        lastF8 = currentF8;
    }
}

// 处理按钮悬停效果
void UIManager::HandleButtonHoverEffects() {
    Mouse_State mouse;
    Mouse_GetState(&mouse);

    for (auto& element : m_elements) {
        if (element.visible && element.type == UI_BUTTON &&
            element.screenType == m_currentScreen) {

            // 计算位置
            XMFLOAT2 pos;
            if (element.position == UI_POS_CUSTOM) {
                pos = element.customPosition;
            }
            else {
                pos = CalculatePosition(element.position, element.size);
            }

            // 检查悬停
            bool isHovered = (mouse.x >= pos.x &&
                mouse.x <= pos.x + element.size.x &&
                mouse.y >= pos.y &&
                mouse.y <= pos.y + element.size.y);

            // 应用悬停效果
            if (isHovered && !element.selected) {
                element.backgroundColor.x = std::min(element.backgroundColor.x + 0.2f, 1.0f);
                element.backgroundColor.y = std::min(element.backgroundColor.y + 0.2f, 1.0f);
                element.backgroundColor.z = std::min(element.backgroundColor.z + 0.2f, 1.0f);
            }
            else if (!isHovered && !element.selected) {
                // 重置颜色
            }
        }
    }
}
void UIManager::UpdatePlayerInfo(int playerID, const wchar_t* name, XMFLOAT4 color) {
    if (playerID < 0 || playerID >= 4) return;

    m_playerInfos[playerID].playerName = name;
    m_playerInfos[playerID].playerColor = color;

    char debug[128];
    sprintf_s(debug, "🎮 更新玩家 %d 信息: %ls\n", playerID, name);
    OutputDebugStringA(debug);
}

void UIManager::UpdatePlayerStats(int playerID, int health, int maxHealth, int score) {
    if (playerID < 0 || playerID >= 4) return;

    m_playerInfos[playerID].health = health;
    m_playerInfos[playerID].maxHealth = maxHealth;
    m_playerInfos[playerID].score = score;

    // 如果血量为0，标记为死亡
    if (health <= 0) {
        m_playerInfos[playerID].isAlive = false;
    }
}

void UIManager::SetPlayerAlive(int playerID, bool alive) {
    if (playerID < 0 || playerID >= 4) return;

    m_playerInfos[playerID].isAlive = alive;

    if (!alive) {
        // 显示玩家死亡消息
        wchar_t deathMessage[64];
        swprintf_s(deathMessage, L"%s has been eliminated!", m_playerInfos[playerID].playerName.c_str());
        ShowBattleMessage(deathMessage, XMFLOAT4(1.0f, 0.3f, 0.3f, 1.0f), 4.0f);
    }
}

void UIManager::ShowBattleMessage(const wchar_t* message, XMFLOAT4 color, float duration) {
    // 在屏幕中央显示战斗消息
    AddTemporaryElement(
        UI_TEXT,
        UI_POS_MID_CENTER,
        message,
        color,
        duration,
        2.0f,
        true
    );

    char debug[256];
    sprintf_s(debug, "🎮 显示战斗消息: %ls\n", message);
    OutputDebugStringA(debug);
}

void UIManager::UpdateBattleTimer(float timeRemaining) {
    m_battleTimer = timeRemaining;
}

void UIManager::SetBattleRound(int round) {
    m_battleRound = round;

    // 更新回合显示
    for (auto& element : m_elements) {
        if (element.screenType == UI_SCREEN_MULTIPLAYER_BATTLE &&
            element.text.find(L"ROUND:") != std::wstring::npos) {
            wchar_t roundText[32];
            swprintf_s(roundText, L"ROUND: %d", round);
            element.text = roundText;
            break;
        }
    }

    char debug[64];
    sprintf_s(debug, "🎮 设置战斗回合: %d\n", round);
    OutputDebugStringA(debug);
}

void UIManager::ToggleMinimap() {
    m_showMinimap = !m_showMinimap;

    const wchar_t* message = m_showMinimap ? L"Minimap ON" : L"Minimap OFF";
    ShowStatusInfo(message, XMFLOAT4(0.5f, 1.0f, 1.0f, 1.0f), 2.0f);

    OutputDebugStringA(m_showMinimap ? "🗺️ 小地图已开启\n" : "🗺️ 小地图已关闭\n");
}

void UIManager::ShowBattleResults(int winnerID, const wchar_t* results) {
    // 更新获胜者显示
    for (auto& element : m_elements) {
        if (element.screenType == UI_SCREEN_BATTLE_RESULT &&
            element.text.find(L"WINNER:") != std::wstring::npos) {

            if (winnerID >= 0 && winnerID < 4) {
                wchar_t winnerText[64];
                swprintf_s(winnerText, L"WINNER: %s", m_playerInfos[winnerID].playerName.c_str());
                element.text = winnerText;
                element.color = m_playerInfos[winnerID].playerColor;
            }
            else {
                element.text = L"DRAW - No Winner";
                element.color = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
            }
            break;
        }
    }

    // 更新详细结果
    for (auto& element : m_elements) {
        if (element.screenType == UI_SCREEN_BATTLE_RESULT &&
            element.text.find(L"Battle Statistics:") != std::wstring::npos) {
            element.text = results;
            break;
        }
    }

    // 更新玩家排名
    int rankIndex = 0;
    for (auto& element : m_elements) {
        if (element.screenType == UI_SCREEN_BATTLE_RESULT &&
            element.linkedPlayerID >= 0 && element.linkedPlayerID < 4) {

            int playerID = element.linkedPlayerID;
            wchar_t rankText[64];
            swprintf_s(rankText, L"%d. %s - Score: %d",
                rankIndex + 1,
                m_playerInfos[playerID].playerName.c_str(),
                m_playerInfos[playerID].score);
            element.text = rankText;
            element.color = m_playerInfos[playerID].playerColor;
            rankIndex++;
        }
    }

    // 切换到结果屏幕
    SwitchToScreen(UI_SCREEN_BATTLE_RESULT);

    char debug[128];
    sprintf_s(debug, "🎮 显示战斗结果，获胜者ID: %d\n", winnerID);
    OutputDebugStringA(debug);
}

BATTLE_UI_STATE UIManager::GetBattleUIState()
{
    return m_battleUIState;
}

bool UIManager::IsMinimapVisible()
{
    return m_showMinimap;
}

int UIManager::GetBattleRound()
{
    return m_battleRound;
}

// 绘制进度条
void UIManager::DrawProgressBar(XMFLOAT2 position, XMFLOAT2 size, float value, float maxValue, XMFLOAT4 color) {
    if (maxValue <= 0) return;

    float progress = std::min(value / maxValue, 1.0f);

    // 绘制背景
    XMMATRIX bgWorld = XMMatrixTranslation(position.x + size.x * 0.5f, position.y + size.y * 0.5f, 0.0f);
    SetWorldMatrix(bgWorld);

    MATERIAL bgMaterial;
    ZeroMemory(&bgMaterial, sizeof(bgMaterial));
    bgMaterial.Diffuse = XMFLOAT4(0.2f, 0.2f, 0.2f, 0.8f);
    SetMaterial(bgMaterial);

    DrawSprite(size, bgMaterial.Diffuse);

    // 绘制进度
    if (progress > 0) {
        XMFLOAT2 progressSize = XMFLOAT2(size.x * progress, size.y);
        XMMATRIX progressWorld = XMMatrixTranslation(
            position.x + progressSize.x * 0.5f,
            position.y + size.y * 0.5f,
            0.0f);
        SetWorldMatrix(progressWorld);

        MATERIAL progressMaterial;
        ZeroMemory(&progressMaterial, sizeof(progressMaterial));
        progressMaterial.Diffuse = color;
        SetMaterial(progressMaterial);

        DrawSprite(progressSize, color);
    }
}

// 绘制玩家信息面板
void UIManager::DrawPlayerInfoPanel(const PlayerUIInfo& playerInfo, XMFLOAT2 position) {
    // 绘制面板背景
    XMFLOAT2 panelSize = XMFLOAT2(200, 80);
    XMMATRIX panelWorld = XMMatrixTranslation(
        position.x + panelSize.x * 0.5f,
        position.y + panelSize.y * 0.5f,
        0.0f);
    SetWorldMatrix(panelWorld);

    MATERIAL panelMaterial;
    ZeroMemory(&panelMaterial, sizeof(panelMaterial));
    panelMaterial.Diffuse = XMFLOAT4(0.1f, 0.1f, 0.2f, 0.8f);
    SetMaterial(panelMaterial);

    DrawSprite(panelSize, panelMaterial.Diffuse);

    // 绘制玩家名称
    GeometricTextRenderer::DrawMinecraftText(
        position.x + 5, position.y + 5,
        playerInfo.playerName.c_str(),
        playerInfo.isAlive ? playerInfo.playerColor : XMFLOAT4(0.5f, 0.5f, 0.5f, 0.7f),
        1.3f,
        false
    );

    // 绘制血量条
    DrawProgressBar(
        XMFLOAT2(position.x + 5, position.y + 30),
        XMFLOAT2(180, 15),
        (float)playerInfo.health,
        (float)playerInfo.maxHealth,
        playerInfo.isAlive ? XMFLOAT4(0.2f, 1.0f, 0.2f, 1.0f) : XMFLOAT4(0.5f, 0.5f, 0.5f, 0.5f)
    );

    // 绘制分数
    wchar_t scoreText[32];
    swprintf_s(scoreText, L"Score: %d", playerInfo.score);
    GeometricTextRenderer::DrawMinecraftText(
        position.x + 5, position.y + 55,
        scoreText,
        XMFLOAT4(1.0f, 1.0f, 0.2f, 1.0f),
        1.0f,
        false
    );
}

// 绘制战斗小地图
void UIManager::DrawBattleMinimap() {
    if (!m_showMinimap) return;

    // 小地图位置和大小
    XMFLOAT2 minimapPos = XMFLOAT2(SCREEN_WIDTH - 220, 20);
    XMFLOAT2 minimapSize = XMFLOAT2(200, 200);

    // 绘制小地图背景
    XMMATRIX minimapWorld = XMMatrixTranslation(
        minimapPos.x + minimapSize.x * 0.5f,
        minimapPos.y + minimapSize.y * 0.5f,
        0.0f);
    SetWorldMatrix(minimapWorld);

    MATERIAL minimapMaterial;
    ZeroMemory(&minimapMaterial, sizeof(minimapMaterial));
    minimapMaterial.Diffuse = XMFLOAT4(0.1f, 0.1f, 0.1f, 0.9f);
    SetMaterial(minimapMaterial);

    DrawSprite(minimapSize, minimapMaterial.Diffuse);

    // 绘制小地图边框
    // 这里可以添加更复杂的小地图渲染逻辑

    // 绘制小地图标题
    GeometricTextRenderer::DrawMinecraftText(
        minimapPos.x + 5, minimapPos.y + 5,
        L"Battle Map",
        XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
        1.0f,
        false
    );
}

