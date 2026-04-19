#include "based_basic.h"
#include "http.h"
#include "core_game_memory.h"
#include <string.h>
#include <math.h>

#define HISCORE_SERVER_HOST "sochiscore.duckdns.org"
#define HISCORE_SERVER_PORT 49944
#define HISCORE_SERVER_ENDPOINT "/hiscores"

static bool IsDigit(char c)
{
    return c >= '0' && c <= '9';
}

s32 ParseScoresLine(ScoreInfo* info, char* buf, u32 len)
{
    char* comma = memchr(buf, ',', len);
    if (!comma)
    {
        return 1;
    }

    u32 usernameLen = comma - buf;

    if (usernameLen >= sizeof(info->username))
    {
        BSD_ERR("Username too long!");
        return 1;
    }

    char* curPos = comma+1;
    u32 score = 0;
    while (IsDigit(*curPos))
    {
        score *= 10;
        score += *curPos++ - '0';
    }
    memcpy(info->username, buf, usernameLen);
    info->score = score;
    return 0;
}


void GameoverLoadScores(GameoverData* data)
{
    data->gotScores = false;
    http_Connection* conn = {0}; // fix this mem leak
    http_Error err = http_ConnectionCreate(&conn);
    DBG_ASSERT_MSG(err == http_Success, "Connection setup failed");
    http_Request req = {0};
    req.method = http_MethodGET;
    req.body.str = NULL;
    req.port = HISCORE_SERVER_PORT;
    // we're using snprintf here to stop MSVC from complaining about strcpy/strncpy deprecation.
    // kinda makes sense because strncpy doesn't do what i originally thought it did (it always copies n chars)
    snprintf(req.hostName, ArrayCount(req.hostName), HISCORE_SERVER_HOST);
    snprintf(req.hostURL, ArrayCount(req.hostURL), HISCORE_SERVER_ENDPOINT);
    http_Response resp = {0};
    err = http_ReqAndWaitForResp(conn, &req, &resp);
    if (err)
    {
        BSD_ERR("HTTP req failed with error %d", err);
    }
    else if (resp.status != 200)
    {
        BSD_ERR("HTTP response returned non-success status code: %d, body: %.*s", resp.status, resp.content.len, resp.content.str);
        http_ResponseFree(&resp);
    }
    else
    {
        // got response, will parse csv body into ScoreInfo

        char* curPos = resp.content.str;
        u32 remaining = resp.content.len;
        
        u32 scoreIndex = 0;
        while (curPos < curPos + resp.content.len && scoreIndex < 10)
        {
            char* nextLine = memchr(curPos, '\n', remaining);
            if (!nextLine)
            {
                BSD_WARN("Possibility of truncated line");
                break;
            }

            // move to start of next line
            nextLine += 1;

            u32 lineLen = nextLine - curPos;

            ParseScoresLine(&data->topScores[scoreIndex], curPos, lineLen);

            scoreIndex++;
            curPos = nextLine;
            data->gotScores = true;
        }
        http_ResponseFree(&resp);
    }

    http_ConnectionClose(conn);
    conn = NULL;
    // TODO: please use the same connection and sort out the net code to be idempotent!!!
    // currently doing this cos it throws an already connected err
    http_ConnectionCreate(&conn);

    // NOTE: the hostname/url fields will be the same so we use the same struct
    // TODO: just use a freaking arena
    req.body.str = MemAlloc(256);
    int len = snprintf(req.body.str, 256, "%s,%d\r\n", data->userScore.username, data->userScore.score);
    req.body.len = len;
    req.method = http_MethodPOST;

    http_Response postResp = {0};
    err = http_ReqAndWaitForResp(conn, &req, &resp);
    if (err)
    {
        BSD_ERR("HTTP POST req failed with error %d", err);
    }
    else if (resp.status != 200)
    {
        BSD_ERR("HTTP POST req response returned non-success status code: %d", resp.status);
    }
    else
    {
        BSD_INF("Posted score to server!");
        http_ResponseFree(&postResp);
    }
    http_ConnectionClose(conn);

    if (data->gotScores)
    {
        data->state = GameoverState_ShowScores;
    }
    else
    {
        data->state = GameoverState_NoScores;
    }
}

void GameoverShowScores(GameoverData* data)
{
    int key = GetKeyPressed();
    if (key != 0)
    {
        core_GameMemoryGet()->menuState = MenuState_Title;
    }
    BeginDrawing();
        ClearBackground(BLUE);
        const int startX = 100;
        const int startY = 120;
        const int rowHeight = 32;

        const int nameX = startX;
        const int scoreX = startX + 300;

        DrawText("Game Over!", startX, 60, 40, RAYWHITE);

        bool drawnUserScore = false;
        for (u32 i = 0; i < ArrayCount(data->topScores); i++)
        {
            int y = startY + i * rowHeight;

            if (!drawnUserScore && data->userScore.score > data->topScores[i].score)
            {
                DrawText(data->userScore.username, nameX, y, 24, ORANGE);
                DrawText(TextFormat("%u", data->userScore.score), scoreX, y, 24, YELLOW);
                drawnUserScore = true;
                continue;
            }

            int topScoresIdx = i;
            if (drawnUserScore)
            {
                topScoresIdx--;
            }
            if (data->topScores[topScoresIdx].username[0] != 0)
            {
                // username
                DrawText(data->topScores[topScoresIdx].username, nameX, y, 24, WHITE);

                // score
                DrawText(TextFormat("%u", data->topScores[topScoresIdx].score), scoreX, y, 24, YELLOW);
            }
        }

        if (!drawnUserScore)
        {
            const int userScoreY = startY + ArrayCount(data->topScores) * rowHeight;
            DrawText(data->userScore.username, nameX, userScoreY, 24, ORANGE);
            DrawText(TextFormat("%u", data->userScore.score), scoreX, userScoreY, 24, YELLOW);
        }

    EndDrawing();
}

void GameoverScreenNoScores(GameoverData* data)
{
    int key = GetKeyPressed();
    if (key != 0)
    {
        core_GameMemoryGet()->menuState = MenuState_Title;
    }
    static int alphaCount = 0;
    float alpha = ( (sinf((float)alphaCount / 10.0f) + 1.0f )* 0.5f );
    BeginDrawing();
        ClearBackground(BLUE);
        DrawText("GAME OVER", 200, 200, 60, DARKBLUE);
        DrawText("Failed to connect to hiscores server...", 200, 100, 20, DARKBLUE);
        DrawText(TextFormat("Score: %d", data->userScore.score), 200, 300, 40, GREEN);
        DrawText("PRESS ANY KEY TO RETURN TO TITLE SCREEN", 120, 500, 20, Fade(DARKBLUE, alpha));
    EndDrawing();
    alphaCount++;
}

#define MAX_INPUT_CHARS 10
void GameoverInputScore(GameoverData* data)
{
    static int framesCounter = 0;
    int key = GetCharPressed();
    char* name = data->userScore.username;
    int screenWidth = GetScreenWidth();
    Rectangle textBox = { screenWidth/2.0f - 100, 180, 225, 50 };

    while (key > 0)
    {
        if (key >= 32 && key <= 125 && data->usernameLen < MAX_INPUT_CHARS)
        {
            name[data->usernameLen] = (char)key;
            name[data->usernameLen+1] = 0;
            data->usernameLen++;
        }
        key = GetCharPressed();
    }

    if (IsKeyPressed(KEY_BACKSPACE))
    {
        data->usernameLen--;
        if (data->usernameLen < 0) data->usernameLen = 0;
        name[data->usernameLen] = 0;
    }
    if (IsKeyPressed(KEY_ENTER))
    {
        // username should already be in the userScore struct
        data->userScore.score = (int)(core_GameMemoryGet()->levelTimer * 100.0f);
        data->state = GameoverState_LoadingScore;
    }

    BeginDrawing();
        ClearBackground(BLUE);
        DrawText("Game Over!", 100, 60, 40, RAYWHITE);
        DrawText("Enter your username to record your score!", 200, 150, 20, RAYWHITE);
        DrawRectangleRec(textBox, LIGHTGRAY);
        DrawRectangleLines((int)textBox.x, (int)textBox.y, (int)textBox.width, (int)textBox.height, RED);
        DrawText(name, (int)textBox.x + 5, (int)textBox.y + 8, 40, MAROON);
        if (data->usernameLen < MAX_INPUT_CHARS)
        {
            // Draw blinking underscore char
            if (((framesCounter/20)%2) == 0) DrawText("_", (int)textBox.x + 8 + MeasureText(name, 40), (int)textBox.y + 12, 40, MAROON);
        }
    EndDrawing();
}

void UpdateGameoverData(GameoverData* data)
{
    switch(data->state)
    {
        case GameoverState_InputScore:
        {
            GameoverInputScore(data);
        } break;
        case GameoverState_LoadingScore:
        {
            GameoverLoadScores(data);
        } break;
        case GameoverState_ShowScores:
        {
            GameoverShowScores(data);
        } break;
        case GameoverState_NoScores:
            GameoverScreenNoScores(data);
        break;
    }
}