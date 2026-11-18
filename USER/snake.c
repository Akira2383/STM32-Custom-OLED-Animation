#include "snake.h"
#include "stm32f10x.h"
#include "oled.h"
#include "key.h"
#include "timer.h"
#include "stdlib.h"

/*==================== 游戏参数 ====================*/
#define GRID_W          32      /* 横向格子数 32*4 = 128 像素  */
#define GRID_H          16      /* 纵向格子数 16*4 = 64 像素   */
#define CELL_PX         4       /* 每格 4x4 像素                */
#define SNAKE_MAX_LEN   100     /* 蛇最大长度                    */
#define MOVE_INTERVAL   150     /* 移动间隔 ms                   */
typedef enum { DIR_UP = 0, DIR_DOWN, DIR_LEFT, DIR_RIGHT } SnakeDir;

typedef enum {
    SNAKE_OK = 0,
    SNAKE_WIN,
    SNAKE_HIT_WALL,
    SNAKE_HIT_SELF
} SnakeResult;

/*==================== 游戏数据（静态，省栈空间） ====================*/
static u8  snake_x[SNAKE_MAX_LEN];
static u8  snake_y[SNAKE_MAX_LEN];
static u8  snake_len;
static SnakeDir cur_dir;
static SnakeDir nxt_dir;
static u8  food_x, food_y;
static u32 last_move_ms;

/*==================== 内部函数 ====================*/

/* 读取 4 个方向键，写入 nxt_dir */
static void Snake_ReadKeys(void)
{
    Key_Scan(&key_up);
    Key_Scan(&key_next);    /* “下”方向复用菜单下翻键（PA1） */
    Key_Scan(&key_left);
    Key_Scan(&key_right);

    if (Key_GetEvent(&key_up)    == KEY_EVENT_DOWN) nxt_dir = DIR_UP;
    if (Key_GetEvent(&key_next)  == KEY_EVENT_DOWN) nxt_dir = DIR_DOWN;
    if (Key_GetEvent(&key_left)  == KEY_EVENT_DOWN) nxt_dir = DIR_LEFT;
    if (Key_GetEvent(&key_right) == KEY_EVENT_DOWN) nxt_dir = DIR_RIGHT;
}

/* 判断两个方向是否相反 */
static u8 Snake_IsReverse(SnakeDir a, SnakeDir b)
{
    return (a == DIR_UP    && b == DIR_DOWN)  ||
           (a == DIR_DOWN  && b == DIR_UP)    ||
           (a == DIR_LEFT  && b == DIR_RIGHT) ||
           (a == DIR_RIGHT && b == DIR_LEFT);
}

/* 判断 (x,y) 是否是蛇身的一部分 */
static u8 Snake_HitBody(u8 x, u8 y)
{
    u8 i;
    for (i = 0; i < snake_len; i++)
    {
        if (snake_x[i] == x && snake_y[i] == y) return 1;
    }
    return 0;
}

/* 在空白格子里随机放食物 */
static void Snake_PlaceFood(void)
{
    u8 fx, fy;
    do {
        fx = (u8)(rand() % GRID_W);
        fy = (u8)(rand() % GRID_H);
    } while (Snake_HitBody(fx, fy));   /* 不放在蛇身上 */

    food_x = fx;
    food_y = fy;
}

/* 画一个 4x4 实心方块（格子坐标 → 像素坐标） */
static void Snake_DrawCell(u8 gx, u8 gy)
{
    u8 px = gx * CELL_PX;
    u8 py = gy * CELL_PX;
    u8 i, j;
    for (i = 0; i < CELL_PX; i++)
        for (j = 0; j < CELL_PX; j++)
            OLED_DrawPoint(px + i, py + j, 1);
}

static void Snake_DrawBorder(void)
{
    OLED_DrawLine(0,   0,   127, 0,   1);   /* 上 */
    OLED_DrawLine(0,   63,  127, 63, 1);   /* 下 */
    OLED_DrawLine(0,   0,   0,   63, 1);   /* 左 */
    OLED_DrawLine(127, 0,   127, 63, 1);   /* 右 */
}

/* 更新蛇状态：OK / 撞墙 / 撞自己 */
static SnakeResult Snake_Update(void)
{
    int nx, ny;
    u8  i;

    /* 第一步：过滤反向 */
    if (!Snake_IsReverse(cur_dir, nxt_dir)) cur_dir = nxt_dir;

    /* 第二步：算新蛇头 */
    nx = (int)snake_x[0];
    ny = (int)snake_y[0];
    switch (cur_dir)
    {
        case DIR_UP:    ny--; break;
        case DIR_DOWN:  ny++; break;
        case DIR_LEFT:  nx--; break;
        case DIR_RIGHT: nx++; break;
    }

    /* 第三步：撞墙 */
    if (nx < 0 || nx >= GRID_W || ny < 0 || ny >= GRID_H) return SNAKE_HIT_WALL;

    /* 第四步：撞自己 */
    for (i = 0; i < snake_len; i++)
    {
        if (snake_x[i] == (u8)nx && snake_y[i] == (u8)ny) return SNAKE_HIT_SELF;
    }

    /* 第五步：移动蛇身 */
    if ((u8)nx == food_x && (u8)ny == food_y)
    {
        if (snake_len < SNAKE_MAX_LEN) snake_len++;
        Snake_PlaceFood();
    }

    for (i = snake_len - 1; i > 0; i--)
    {
        snake_x[i] = snake_x[i - 1];
        snake_y[i] = snake_y[i - 1];
    }
    snake_x[0] = (u8)nx;
    snake_y[0] = (u8)ny;

    if (snake_len >= SNAKE_MAX_LEN) return SNAKE_WIN;
    return SNAKE_OK;
}

/* 画一整帧游戏画面 */
static void Snake_Draw(void)
{
    u8 i;

    OLED_Clear_GRAM();
    Snake_DrawBorder();
    for (i = 0; i < snake_len; i++) Snake_DrawCell(snake_x[i], snake_y[i]);
    Snake_DrawCell(food_x, food_y);
    OLED_Refresh();
}

static void Snake_DrawEndMessage(const char *message)
{
    OLED_Clear_GRAM();
    OLED_ShowString(24, 24, (u8 *)message, 16, 1);
    OLED_Refresh();
}

/* 扫描全部按键，任意键按下返回 1 */
static u8 Snake_AnyKey(void)
{
    Key_Scan(&key_next);  Key_Scan(&key_enter);
    Key_Scan(&key_up);
    Key_Scan(&key_left);  Key_Scan(&key_right);

    if (Key_GetEvent(&key_next)  == KEY_EVENT_DOWN) return 1;
    if (Key_GetEvent(&key_enter) == KEY_EVENT_DOWN) return 1;
    if (Key_GetEvent(&key_up)    == KEY_EVENT_DOWN) return 1;
    if (Key_GetEvent(&key_left)  == KEY_EVENT_DOWN) return 1;
    if (Key_GetEvent(&key_right) == KEY_EVENT_DOWN) return 1;
    return 0;
}

static void Snake_Init(void)
{
    snake_len = 3;
    snake_x[0] = 16; snake_y[0] = 8;    /* 蛇头 */
    snake_x[1] = 15; snake_y[1] = 8;
    snake_x[2] = 14; snake_y[2] = 8;

    cur_dir    = DIR_RIGHT;
    nxt_dir    = DIR_RIGHT;

    srand(Timer_GetMs());

    Snake_PlaceFood();
    last_move_ms = Timer_GetMs();
    OLED_Clear();
}

static void Snake_ShowEndScreen(const char *message, u16 blink_interval)
{
    u32 last_toggle = Timer_GetMs();
    u8 visible = 1;

    Snake_DrawEndMessage(message);
    while (!Snake_AnyKey())
    {
        if (Timer_GetMs() - last_toggle >= blink_interval)
        {
            last_toggle = Timer_GetMs();
            visible = !visible;
            if (visible)
            {
                Snake_DrawEndMessage(message);
            }
            else
            {
                OLED_Clear();
            }
        }
    }
}

/*==================== 对外入口 ====================*/
void Snake_Run(void)
{
    SnakeResult result;

    Snake_Init();

    while (1)
    {
        Snake_ReadKeys();

        if (Timer_GetMs() - last_move_ms >= MOVE_INTERVAL)
        {
            last_move_ms = Timer_GetMs();
            result = Snake_Update();
            if (result != SNAKE_OK)
            {
                if (result == SNAKE_WIN)
                {
                    Snake_ShowEndScreen("YOU WIN!", 400);
                }
                else
                {
                    Snake_ShowEndScreen("GAME OVER", 300);
                }
                break;
            }
        }
        Snake_Draw();
    }

    OLED_Clear();
}
