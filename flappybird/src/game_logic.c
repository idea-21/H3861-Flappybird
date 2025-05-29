/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include "game_logic.h"
#include "oled.h"
#include "common_log.h"

typedef struct obstacleAttr {
    short X;
    short Y;
    short width;
    short height;
    _Bool Inverse; // 0 障碍物在 右半边，1 障碍物在 左半边
} ObstacleAttr;

static const unsigned char g_bitmapBird[] = {
    0X01,0X01,0X18,0X00,0X10,0X00,
    0X00,0X00,0X00,0X60,0X00,0XF8,
    0X0F,0XF6,0X10,0XF6,0X10,0XF6,
    0X6E,0XF6,0X80,0XF6,0X80,0XF6,
    0X80,0XFE,0XC1,0X69,0X9E,0X11,
    0X9C,0X11,0X80,0X01,0X80,0XE1,
    0X01,0X11,0X61,0X10,0X61,0X16,
    0X11,0X16,0X09,0X18,0X09,0X18,
    0X07,0X10,0X00,0XE0,0X00,0X00,
};

static const unsigned char g_bitmapBullet[] = {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
};

static const unsigned char g_bitmapObstacle1[] = {
    0x00, 0x00, 0x00, 0x00,
    0x03, 0xC0, 0x03, 0xC0,
    0x07, 0xE0, 0x07, 0xE0,
    0x0F, 0xF0, 0x0F, 0xF0,
    0x1F, 0xF8, 0x1F, 0xF8,
    0x3F, 0xFC, 0x3F, 0xFC,
    0x7F, 0xFE, 0x7F, 0xFE,
    0x3F, 0xFC, 0x00, 0x00
};

static const unsigned char g_bitmapObstacle2[] = {
    0x0F, 0xF0, 0x3F, 0xFC,
    0x7F, 0xFE, 0xFF, 0xFF,
    0xFF, 0xFF, 0x7F, 0xFE,
    0x3F, 0xFC, 0x0F, 0xF0
};

static const unsigned char g_bitmapObstacle3[] = {
    0x07, 0xE0, 0x1F, 0xF8,
    0x3F, 0xFC, 0x7F, 0xFE,
    0x7F, 0xFE, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0x7F, 0xFE,
    0x7F, 0xFE, 0x3F, 0xFC,
    0x1F, 0xF8, 0x07, 0xE0
};

#define BIRD_WIDTH  16
#define BIRD_HEIGHT  24
#define BIRD_DOWN_MARGIN  0
#define OBSTACLE_INTERVAL  40

#define PIPES_COUNT 3
#define BIRD_INIT_POSITION_X 52
#define BIRD_INIT_POSITION_Y 40
#define INIT_MOVE_SPEED_Y    1
#define INIT_MOVE_SPEED_X    2
#define OLED_WIDTH_DIVIDENDS 2
#define X_POSITION_SHOW_GAME_OVER  10
#define Y_POSITION_SHOW_GAME_OVER  20
#define SPEED_LEVLE_1 1
#define SPEED_LEVLE_2 2
#define FLY_DISTANCE_TAG_1 50
#define FLY_DISTANCE_TAG_2 200

static uint8 g_game_difficulty_flag = 0;
static uint8 g_isStarted = 0;
static short g_birdPositionX = BIRD_INIT_POSITION_X;
static short g_birdPositionY;
static float g_moveSpeedY = INIT_MOVE_SPEED_Y;
static float g_moveSpeedX = INIT_MOVE_SPEED_X;
static uint32 g_flyDistance = 0;

ObstacleAttr bullet;

static ObstacleAttr g_obstacles[PIPES_COUNT];

static short g_obstacleParm [3][2] = {
    {16, 16},
    {16, 8},
    {16, 14}
};

void SetRandPosition(ObstacleAttr *obstacle)
{
    if (obstacle->Inverse == 0) {
        obstacle->X = (SSD1306_WIDTH / OLED_WIDTH_DIVIDENDS +
            rand() % ((SSD1306_WIDTH / OLED_WIDTH_DIVIDENDS) - BIRD_WIDTH)) - 1;
    } else {
        obstacle->X = (rand() % (SSD1306_WIDTH / OLED_WIDTH_DIVIDENDS)) + 1;
    }
}

void GameInit(void)
{
    srand((unsigned)time(NULL)); // 用当前时间，设置种子
    for (short i = 0; i < PIPES_COUNT; i++) {
        g_obstacles[i].Y = -(i * OBSTACLE_INTERVAL);
        g_obstacles[i].Inverse = i % OLED_WIDTH_DIVIDENDS; // 0, 1, 0, 1, 左右 半个区域内
        g_obstacles[i].width = g_obstacleParm[i][0];
        g_obstacles[i].height = g_obstacleParm[i][1];
        SetRandPosition(&g_obstacles[i]); // 设置障碍物X轴上的随机位置
    }
    g_birdPositionX = BIRD_INIT_POSITION_X; // bird在X轴上的初始位置
    g_flyDistance = 0; // 飞行距离
}

void move(){
    g_birdPositionX += g_moveSpeedX;
}

void createbullet()
{
		bullet.X = g_birdPositionX + 8;
		bullet.Y = 0;
        bullet.width = BIRD_WIDTH/2;
        bullet.height = SSD1306_WIDTH/2;
}

void GenerateObstacle(ObstacleAttr *obstacle)
{
    obstacle->Y = -(OBSTACLE_INTERVAL * (PIPES_COUNT - 1));
    SetRandPosition(obstacle);
}

void GameStart(void)
{
    GameInit();
    g_isStarted = 1;
}

int IsCollision(void)
{
    if (g_birdPositionX < 0 || g_birdPositionX > SSD1306_WIDTH - BIRD_WIDTH) return 1; // 碰触边界
    for (int i = 0; i < PIPES_COUNT; i++) {
        if (((g_obstacles[i].Y) % SSD1306_WIDTH < SSD1306_HEIGHT) && // 在此条件下，才能发生碰撞
            ((g_obstacles[i].Y) % SSD1306_WIDTH > SSD1306_HEIGHT - BIRD_HEIGHT - g_obstacles[i].height)) {
            if ((g_birdPositionX > g_obstacles[i].X + g_obstacles[i].width)  ||
                (g_birdPositionX < g_obstacles[i].X - BIRD_WIDTH)) { // 非碰触情景
                return 0;
            } else {
                return 1;
            }
        }
    }
    return 0;
}

void IsBulletCollision()
{
    for (int i = 0; i < PIPES_COUNT; i++) {
        // 检查子弹与障碍物的碰撞
        if (bullet.X <= g_obstacles[i].X + g_obstacles[i].width && bullet.X >= g_obstacles[i].X - bullet.width) {
            // 子弹与障碍物发生碰撞
            g_obstacles[i].Y = SSD1306_HEIGHT + 1; // 将障碍物移除
            bullet.Y = SSD1306_HEIGHT; // 将子弹移除
        }
    }
}

void GameOver(void)
{
    g_isStarted = 0;
    ssd1306_SetCursor(X_POSITION_SHOW_GAME_OVER, Y_POSITION_SHOW_GAME_OVER);
    ssd1306_DrawString("== GAME OVER ==", sizeof("== GAME OVER =="), White);
    ssd1306_UpdateScreen();
    g_moveSpeedY = INIT_MOVE_SPEED_Y;
    g_moveSpeedX = INIT_MOVE_SPEED_X;
}

void RefreshGameDifficulty(void)
{
    if (g_game_difficulty_flag != SPEED_LEVLE_1) {
        if ((g_flyDistance > FLY_DISTANCE_TAG_1) && (g_flyDistance < FLY_DISTANCE_TAG_2)) {
            g_game_difficulty_flag = SPEED_LEVLE_1;
            g_moveSpeedY += SPEED_LEVLE_1;
            g_moveSpeedX += SPEED_LEVLE_1;
        }
    } else if (g_game_difficulty_flag != SPEED_LEVLE_2) {
        if (g_flyDistance > FLY_DISTANCE_TAG_2) {
            g_game_difficulty_flag = SPEED_LEVLE_2;
            g_moveSpeedY += SPEED_LEVLE_2;
            g_moveSpeedX += SPEED_LEVLE_2;
        }
    }
}

int GameLoop(void)
{
    if (g_isStarted) {
        g_flyDistance += g_moveSpeedY; // 统计飞行总距离，用来计算得分
        for (short i = 0; i < PIPES_COUNT; i++) {
            g_obstacles[i].Y += g_moveSpeedY; // 刷新障碍物的Y轴坐标
            if (g_obstacles[i].Y > SSD1306_HEIGHT) {
                GenerateObstacle(&g_obstacles[i]); // 重新生成障碍参数
            }
        }

        if (IsCollision()) { // 判断是否发生了碰撞
            LOG_I("=====Collision!!!======");
            return 1;
        } else {
            return 0;
        }
    }
    return 0;
}

void DrawObstacle(int x, ObstacleAttr obstacles)
{
    switch (x) {
        case OBSTACLE_1:
            ssd1306_DrawBitmapAtPosition(g_bitmapObstacle1, obstacles.width,
                obstacles.height, obstacles.X, obstacles.Y);
            break;
        case OBSTACLE_2:
            ssd1306_DrawBitmapAtPosition(g_bitmapObstacle2, obstacles.width,
                obstacles.height, obstacles.X, obstacles.Y);
            break;
        case OBSTACLE_3:
            ssd1306_DrawBitmapAtPosition(g_bitmapObstacle3, obstacles.width,
                obstacles.height, obstacles.X, obstacles.Y);
            break;
        default:
            break;
    }
}

void BirdMove(uint32 flag)
{
    if (flag == 0) {
        g_birdPositionX -= 2*g_moveSpeedX;  // move left
    } else {
        createbullet();
    }
}

#define SCORE_CONVERSION_PARM  10
#define SCORE_SHOW_PARM        30
#define EDGE_OF_OLED_X         127
#define EDGE_OF_OLED_Y         63

void DrawGameScreen(void)
{
    ssd1306_DrawBitmapAtPosition(g_bitmapBird, BIRD_WIDTH,
        BIRD_HEIGHT, g_birdPositionX, BIRD_INIT_POSITION_Y);
    for (short i = 0; i < PIPES_COUNT; i++) {
        DrawObstacle(i, g_obstacles[i]);
    }
    ssd1306_DrawBitmapAtPosition(g_bitmapBullet, bullet.width,
        bullet.height, bullet.X, bullet.Y);

    char buf[5];
    if (sprintf_s(buf, sizeof(buf), "%d", g_flyDistance / SCORE_CONVERSION_PARM) == -1) {
        LOG_E("setting the game score failed !\n");
    }
    ssd1306_SetCursor(SSD1306_WIDTH - SCORE_SHOW_PARM, 1);
    ssd1306_DrawString(buf, sizeof(buf), White);

    ssd1306_DrawLine(0, 0, 0, EDGE_OF_OLED_Y, White);
    ssd1306_DrawLine(EDGE_OF_OLED_X, 0, EDGE_OF_OLED_X, EDGE_OF_OLED_Y, White);
    ssd1306_UpdateScreen();
}