#include "../MultiplayerBattle.h" 
#include <map>
#include <algorithm>
#include "MultiplayerMap.h"
#include "../FrameWork/TextureManager.h"

// ��?��??�ʒ�?
int MultiplayerBattleMap::s_battleGroundMap[BATTLE_MAP_SIZE_Z][BATTLE_MAP_SIZE_X];
int MultiplayerBattleMap::s_battleObstacleMap[BATTLE_MAP_SIZE_Z][BATTLE_MAP_SIZE_X];


extern MODEL* Tree;

void MultiplayerBattleMap::Init() {
    // ����?�l�n?
    memset(s_battleGroundMap, 0, sizeof(s_battleGroundMap));
    memset(s_battleObstacleMap, 0, sizeof(s_battleObstacleMap));

    OutputDebugStringA("? ?�l�n?�n?���n������\n");
}

void MultiplayerBattleMap::Uninit() {
    // �����n?����
    memset(s_battleGroundMap, 0, sizeof(s_battleGroundMap));
    memset(s_battleObstacleMap, 0, sizeof(s_battleObstacleMap));

    OutputDebugStringA("? ?�l�n?�n?��������\n");
}

bool MultiplayerBattleMap::MergeMapsForBattle(BattlePlayerInfo* players, int activePlayerCount) {
    OutputDebugStringA("??? ?�n����߉ƒn?��30x30?�l�n?...\n");

    // ����30x30?�l�n?
    memset(s_battleGroundMap, 0, sizeof(s_battleGroundMap));
    memset(s_battleObstacleMap, 0, sizeof(s_battleObstacleMap));

    // ?�擖�O??�풆�I�n?����
    extern int GROUND_MAP[MAPSIZE_Z][MAPSIZE_X];
    extern int OBSTACLE_MAP[MAPSIZE_Z][MAPSIZE_X];

    int mergedCount = 0;

    // ??���߉Ɛ����n?���e
    for (int playerID = 0; playerID < MAX_PLAYERS; playerID++) {
        if (!players[playerID].isActive) continue;

        int offsetX, offsetZ;
        GetPlayerMapOffset((PlayerID)playerID, &offsetX, &offsetZ);

        char offsetDebug[128];
        sprintf_s(offsetDebug, "??? �߉�%d�n?�Έ�: (%d, %d)\n", playerID, offsetX, offsetZ);
        OutputDebugStringA(offsetDebug);

        // ??���߉Ɛ��������I15x15�n?���
        for (int z = 0; z < SINGLE_MAP_SIZE; z++) {
            for (int x = 0; x < SINGLE_MAP_SIZE; x++) {
                int battleX = offsetX + x;
                int battleZ = offsetZ + z;

                if (battleX >= 0 && battleX < BATTLE_MAP_SIZE_X &&
                    battleZ >= 0 && battleZ < BATTLE_MAP_SIZE_Z) {

                    if (playerID == PLAYER_1) {
                        // �߉�1�g�p���O??�풆�I�n?
                        if (x < MAPSIZE_X && z < MAPSIZE_Z) {
                            s_battleGroundMap[battleZ][battleX] = GROUND_MAP[z][x];
                            s_battleObstacleMap[battleZ][battleX] = OBSTACLE_MAP[z][x];
                        }
                        else {
                            s_battleGroundMap[battleZ][battleX] = NORMAL_GROUND;
                        }
                    }
                    else {
                        // ?�����߉Ɛ�����?�n?
                        GenerateEnhancedPlayerMap(playerID, battleX, battleZ, x, z);
                    }
                }
            }
        }

        // ��?���߉Ƌ��Y���o���_
        AddPlayerSpawnPoint(playerID, offsetX, offsetZ);
        mergedCount++;
    }

    // �ݒn??�E�Y��?�ڋ��
    CreateConnectionAreas();

    char finalMsg[128];
    sprintf_s(finalMsg, "??? 30x30?�l�n?���󊮐��C������%d���߉ƒn?\n", mergedCount);
    OutputDebugStringA(finalMsg);

    int nonEmptyTiles = 0;
    for (int z = 0; z < BATTLE_MAP_SIZE_Z; z++) {
        for (int x = 0; x < BATTLE_MAP_SIZE_X; x++) {
            if (s_battleGroundMap[z][x] != 0) {
                nonEmptyTiles++;
            }
        }
    }

    bool success = (mergedCount >= 2) && (nonEmptyTiles > 0);

    char resultMsg[128];
    sprintf_s(resultMsg, "??? �n?����?��: %s (�߉�%d��, �n�`�i�q%d��)\n",
        success ? "����" : "��?", mergedCount, nonEmptyTiles);
    OutputDebugStringA(resultMsg);

    return success;
}

void MultiplayerBattleMap::ApplyBattleMapToWorld() {
    OutputDebugStringA("??? ?�n?�p30x30?�l�n?�����E\n");

    BoxObject* box = GetBox();
    BoxObject* obstacle = GetObstacle();

    for (int i = 0; i < BATTLE_MAP_SIZE_X * BATTLE_MAP_SIZE_Z; i++) {
        box[i] = BoxObject{};
        obstacle[i] = BoxObject{};
    }

    int createdBoxes = 0;
    int createdObstacles = 0;

    for (int z = 0; z < BATTLE_MAP_SIZE_Z; z++) {
        for (int x = 0; x < BATTLE_MAP_SIZE_X; x++) {
            int i = z * BATTLE_MAP_SIZE_X + x;

            // ?���n��?��
            if (s_battleGroundMap[z][x] != 0) {
                box[i].ObjectNo = s_battleGroundMap[z][x];
                box[i].Use = true;
                box[i].position.x = x * BOXSIZE_X + (BOXSIZE_X / 2);
                box[i].position.y = 0.0f - (BOXSIZE_Y / 2);
                box[i].position.z = -z * BOXSIZE_Z + (BOXSIZE_Z / 2);
                box[i].rotate = XMFLOAT3(0.0f, 0.0f, 0.0f);
                box[i].scale = XMFLOAT3(1.0f, 1.0f, 1.0f);
                box[i].Radius = BOX_RADIUS;
                box[i].SizeMin = XMFLOAT3(-BOXSIZE_X / 2, -BOXSIZE_Y / 2, -BOXSIZE_Z / 2);
                box[i].SizeMax = XMFLOAT3(BOXSIZE_X / 2, BOXSIZE_Y / 2, BOXSIZE_Z / 2);

                // ?�u?��
                switch (s_battleGroundMap[z][x]) {
                case NORMAL_GROUND: box[i].TexID = GET_TEXTURE(GROUND); break;
                case ICE_GROUND: box[i].TexID = GET_TEXTURE(ICE); break;
                case SAND_GROUND: box[i].TexID = GET_TEXTURE(SAND); break;
                case GRASS_GROUND: box[i].TexID = GET_TEXTURE(GRASS); break;
                case WATER: box[i].TexID = GET_TEXTURE(WATER); break;
                default: box[i].TexID = GET_TEXTURE(GROUND); break;
                }

                createdBoxes++;
            }

            // ?����V��?��
            if (s_battleObstacleMap[z][x] != 0) {
                obstacle[i].ObjectNo = s_battleObstacleMap[z][x];
                obstacle[i].Use = true;
                obstacle[i].position.x = x * BOXSIZE_X + (BOXSIZE_X / 2);
                obstacle[i].position.z = -z * BOXSIZE_Z + (BOXSIZE_Z / 2);
                obstacle[i].rotate = XMFLOAT3(0.0f, 0.0f, 0.0f);
                obstacle[i].scale = XMFLOAT3(1.0f, 1.0f, 1.0f);
                obstacle[i].Radius = BOX_RADIUS;

                // ����?�^?�u����
                switch (s_battleObstacleMap[z][x]) {
                case WALL:
                    obstacle[i].position.y = (BOXSIZE_Y / 2 + 0.01f);
                    obstacle[i].TexID = GET_TEXTURE(WALL);
                    break;
                case TREE:
                    obstacle[i].position.y = 0.5f;
                    obstacle[i].model = Tree;
                    break;
                case START:
                    obstacle[i].position.y = (BOXSIZE_Y / 2);
                    obstacle[i].TexID = GET_TEXTURE(START);
                    break;
                case COIN:
                    obstacle[i].position.y = (BOXSIZE_Y / 2);
                    obstacle[i].TexID = GET_TEXTURE(COIN);
                    obstacle[i].useProcedural = true;
                    obstacle[i].proceduralType = 1;
                    break;
                case BOMB:
                    obstacle[i].position.y = (BOXSIZE_Y / 2);
                    obstacle[i].TexID = GET_TEXTURE(BOMB);
                    obstacle[i].useProcedural = true;
                    obstacle[i].proceduralType = 2;
                    break;
                }

                obstacle[i].SizeMin = XMFLOAT3(-BOXSIZE_X / 2, -BOXSIZE_Y / 2, -BOXSIZE_Z / 2);
                obstacle[i].SizeMax = XMFLOAT3(BOXSIZE_X / 2, BOXSIZE_Y / 2, BOXSIZE_Z / 2);
                createdObstacles++;
            }

                obstacle[i].SizeMin = XMFLOAT3(-BOXSIZE_X / 2, -BOXSIZE_Y / 2, -BOXSIZE_Z / 2);
                obstacle[i].SizeMax = XMFLOAT3(BOXSIZE_X / 2, BOXSIZE_Y / 2, BOXSIZE_Z / 2);
                createdObstacles++;
            }
        }
    }

    


// �n?����??����
int MultiplayerBattleMap::GetBattleGroundTile(int x, int z) {
    if (x >= 0 && x < BATTLE_MAP_SIZE_X && z >= 0 && z < BATTLE_MAP_SIZE_Z) {
        return s_battleGroundMap[z][x];
    }
    return 0;
}

int MultiplayerBattleMap::GetBattleObstacleTile(int x, int z) {
    if (x >= 0 && x < BATTLE_MAP_SIZE_X && z >= 0 && z < BATTLE_MAP_SIZE_Z) {
        return s_battleObstacleMap[z][x];
    }
    return 0;
}

void MultiplayerBattleMap::SetBattleGroundTile(int x, int z, int value) {
    if (x >= 0 && x < BATTLE_MAP_SIZE_X && z >= 0 && z < BATTLE_MAP_SIZE_Z) {
        s_battleGroundMap[z][x] = value;
    }
}

void MultiplayerBattleMap::SetBattleObstacleTile(int x, int z, int value) {
    if (x >= 0 && x < BATTLE_MAP_SIZE_X && z >= 0 && z < BATTLE_MAP_SIZE_Z) {
        s_battleObstacleMap[z][x] = value;
    }
}
void MultiplayerBattleMap::GetPlayerMapOffset(PlayerID playerID, int* offsetX, int* offsetZ) {
    switch (playerID) {
    case PLAYER_1: // ����p
        *offsetX = 0;
        *offsetZ = 0;
        break;
    case PLAYER_2: // �E��p
        *offsetX = SINGLE_MAP_SIZE;
        *offsetZ = 0;
        break;
    case PLAYER_3: // �����p
        *offsetX = 0;
        *offsetZ = SINGLE_MAP_SIZE;
        break;
    case PLAYER_4: // �E���p
        *offsetX = SINGLE_MAP_SIZE;
        *offsetZ = SINGLE_MAP_SIZE;
        break;
    default:
        *offsetX = 0;
        *offsetZ = 0;
        break;
    }
}

XMFLOAT3 MultiplayerBattleMap::GetPlayerSpawnPosition(PlayerID playerID) {
    int offsetX, offsetZ;
    GetPlayerMapOffset(playerID, &offsetX, &offsetZ);

    // ��30x30�I�o���_��????���E��?
    float displayX = (offsetX + SINGLE_MAP_SIZE * 0.5f);
    float displayZ = (offsetZ + SINGLE_MAP_SIZE * 0.5f);

    // ???���E��?
    float spawnX = displayX * BOXSIZE_X;
    float spawnZ = -displayZ * BOXSIZE_Z; // ����Z?����

    char debugSpawn[256];
    sprintf_s(debugSpawn, "?? �߉�%d�o���_?�Z: �Έ�(%d,%d) -> ?��(%.1f,%.1f) -> ���E(%.1f, %.1f)\n",
        playerID, offsetX, offsetZ, displayX, displayZ, spawnX, spawnZ);
    OutputDebugStringA(debugSpawn);

    return XMFLOAT3(spawnX, BOXSIZE_Y, spawnZ);
}

void MultiplayerBattleMap::GenerateTestMapForPlayer(int playerID, int battleX, int battleZ, int localX, int localZ) {
    // ?AI�߉Ɛ���??�I??�n?
    switch (playerID) {
    case PLAYER_2:
        s_battleGroundMap[battleZ][battleX] = ICE_GROUND;
        break;
    case PLAYER_3:
        s_battleGroundMap[battleZ][battleX] = SAND_GROUND;
        break;
    case PLAYER_4:
        s_battleGroundMap[battleZ][battleX] = GRASS_GROUND;
        break;
    default:
        s_battleGroundMap[battleZ][battleX] = NORMAL_GROUND;
        break;
    }

    // ��??���u�ꍱ?
    if (localX == 0 || localX == SINGLE_MAP_SIZE - 1 ||
        localZ == 0 || localZ == SINGLE_MAP_SIZE - 1) {
        if ((localX + localZ) % 3 == 0) {
            s_battleObstacleMap[battleZ][battleX] = WALL;
        }
    }
}

void MultiplayerBattleMap::GenerateEnhancedPlayerMap(int playerID, int battleX, int battleZ, int localX, int localZ) {
    // ??��AI�߉�??�s���I�n�`���F
    switch (playerID) {
    case PLAYER_2: // �u���?
        if (localX < 5 || localX > 10 || localZ < 5 || localZ > 10) {
            s_battleGroundMap[battleZ][battleX] = ICE_GROUND;
        }
        else {
            s_battleGroundMap[battleZ][battleX] = NORMAL_GROUND;
        }

        // �Y���ꍱ?��?�h��
        if ((localX + localZ) % 4 == 0 && localX > 2 && localX < 12 && localZ > 2 && localZ < 12) {
            s_battleObstacleMap[battleZ][battleX] = WALL;
        }
        break;

    case PLAYER_3: // ������?
        if ((localX + localZ) % 3 == 0) {
            s_battleGroundMap[battleZ][battleX] = SAND_GROUND;
        }
        else {
            s_battleGroundMap[battleZ][battleX] = NORMAL_GROUND;
        }

        // �Y���ꍱ��?
        if ((localX * localZ) % 11 == 0 && localX > 3 && localX < 11) {
            s_battleObstacleMap[battleZ][battleX] = COIN;
        }
        break;

    case PLAYER_4: // �X�ю�?
        if (localX % 2 == localZ % 2) {
            s_battleGroundMap[battleZ][battleX] = GRASS_GROUND;
        }
        else {
            s_battleGroundMap[battleZ][battleX] = NORMAL_GROUND;
        }

        // �Y��?��
        if ((localX + localZ * 2) % 7 == 0 && localX > 1 && localX < 13) {
            s_battleObstacleMap[battleZ][battleX] = TREE;
        }
        break;

    default:
        s_battleGroundMap[battleZ][battleX] = NORMAL_GROUND;
        break;
    }
}

void MultiplayerBattleMap::AddPlayerSpawnPoint(int playerID, int offsetX, int offsetZ) {
    // ��?���߉Ƌ��I���S���ߕ��u�o���_
    int spawnX = offsetX + SINGLE_MAP_SIZE / 2;
    int spawnZ = offsetZ + SINGLE_MAP_SIZE / 2;

    // ?�ۏo���_��?�E��
    if (spawnX >= 0 && spawnX < BATTLE_MAP_SIZE_X &&
        spawnZ >= 0 && spawnZ < BATTLE_MAP_SIZE_Z) {

        s_battleObstacleMap[spawnZ][spawnX] = START;
        s_battleGroundMap[spawnZ][spawnX] = NORMAL_GROUND;

        char spawnDebug[128];
        sprintf_s(spawnDebug, "?? �߉�%d�o���_: (%d, %d)\n", playerID, spawnX, spawnZ);
        OutputDebugStringA(spawnDebug);
    }
}

void MultiplayerBattleMap::CreateConnectionAreas() {
    OutputDebugStringA("?? ?���߉Ƌ��?�I?�ڒʓ�\n");

    // ?������?�ڒʓ� (?�ڍ��E���)
    for (int z = SINGLE_MAP_SIZE - 2; z <= SINGLE_MAP_SIZE + 1; z++) {
        for (int x = SINGLE_MAP_SIZE - 2; x <= SINGLE_MAP_SIZE + 1; x++) {
            if (x >= 0 && x < BATTLE_MAP_SIZE_X && z >= 0 && z < BATTLE_MAP_SIZE_Z) {
                s_battleGroundMap[z][x] = NORMAL_GROUND;
                // ��?�ڋ����u�ꍱ��?�a�y?��?��?�_
                if ((x + z) % 8 == 0) {
                    s_battleObstacleMap[z][x] = COIN;
                }
                else if ((x + z) % 13 == 0) {
                    s_battleObstacleMap[z][x] = BOMB;
                }
            }
        }
    }
}

int MultiplayerBattleMap::SampleBattleMapTerrain(int centerX, int centerZ) {
    // ??2x2�����e?�n�`?�^�I����
    std::map<int, int> terrainCount;

    for (int dz = 0; dz < 2; dz++) {
        for (int dx = 0; dx < 2; dx++) {
            int x = centerX + dx;
            int z = centerZ + dz;

            if (x < BATTLE_MAP_SIZE_X && z < BATTLE_MAP_SIZE_Z) {
                int terrain = s_battleGroundMap[z][x];
                terrainCount[terrain]++;
            }
        }
    }

    // �ԉ�o?�����ő��I�n�`?�^
    int maxCount = 0;
    int dominantTerrain = NORMAL_GROUND;

    for (auto& pair : terrainCount) {
        if (pair.second > maxCount) {
            maxCount = pair.second;
            dominantTerrain = pair.first;
        }
    }

    return dominantTerrain;
}

int MultiplayerBattleMap::SampleBattleMapObstacle(int centerX, int centerZ) {
    // ?��??�d�v��V���i�@�o���_�A��?�A�y?�j
    for (int dz = 0; dz < 2; dz++) {
        for (int dx = 0; dx < 2; dx++) {
            int x = centerX + dx;
            int z = centerZ + dz;

            if (x < BATTLE_MAP_SIZE_X && z < BATTLE_MAP_SIZE_Z) {
                int obstacle = s_battleObstacleMap[z][x];

                // ?��ۗ��d�v��V��
                if (obstacle == START || obstacle == COIN || obstacle == BOMB) {
                    return obstacle;
                }
            }
        }
    }

    // �@�ʖv�L�d�v��V���C??��꘢����V��
    for (int dz = 0; dz < 2; dz++) {
        for (int dx = 0; dx < 2; dx++) {
            int x = centerX + dx;
            int z = centerZ + dz;

            if (x < BATTLE_MAP_SIZE_X && z < BATTLE_MAP_SIZE_Z) {
                int obstacle = s_battleObstacleMap[z][x];
                if (obstacle != 0) {
                    return obstacle;
                }
            }
        }
    }

    return 0; 
}