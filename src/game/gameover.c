#include <string.h>
#include <math.h>
#include "based_raylib.h"
#include "based_basic.h"
#include "core_menu_state.h"
#include "state_transition.h"
#include "http.h"
#include "core_game_memory.h"
#include "raylib.h"
#include "based_core.h"
#include "ui_config.h"

#define HISCORE_SERVER_HOST "sochiscore.duckdns.org"
#define HISCORE_SERVER_PORT 49944
#define HISCORE_SERVER_ENDPOINT "/hiscores"

#define BG_COLOR BLACK
#define MAX_INPUT_CHARS 10
#define MAX_USERNAME_PLACEHOLDER "__________"

typedef struct
{
    const char* hiscoreHost;
    const char* endpoint;
    u16 port;
} HiscoreServerData;

static bool IsDigit(char c)
{
    return c >= '0' && c <= '9';
}

void InitGameOver(soc_GameMemory* memory)
{
    memory->gameoverData.state = GameoverState_InputScore;
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

static HiscoreServerData GetServerData()
{
    char* host = getenv("SOCHISCORE_HOST");
    if (!host)
    {
        host = "localhost";
    }

    char* endpoint = getenv("SOCHISCORE_ENDPOINT");
    if (!endpoint)
    {
        endpoint = HISCORE_SERVER_ENDPOINT;
    }

    char* portStr = getenv("SOCHISCORE_PORT");
    if (!portStr)
    {
        portStr = "49944";
    }

    s32 port = atoi(portStr);

    DBG_ASSERT_MSG(port <= 65535, "SOCHISCORE_PORT is out of bounds: %d", port);

    return (HiscoreServerData) {host, endpoint, port};
}

void GameoverLoadScores(GameoverData* data)
{
    Scoreboard* scoreboard = &data->scoreboard;

    HiscoreServerData serverData = GetServerData();

    data->gotScores = false;
    http_Connection* conn = {0}; // fix this mem leak
    http_Error err = http_ConnectionCreate(&conn);
    DBG_ASSERT_MSG(err == http_Success, "Connection setup failed");
    http_Request req = {0};
    req.method = http_MethodGET;
    req.body.str = NULL;
    req.port = serverData.port;
    // we're using snprintf here to stop MSVC from complaining about strcpy/strncpy deprecation.
    // kinda makes sense because strncpy doesn't do what i originally thought it did (it always copies n chars)
    snprintf(req.hostName, ArrayCount(req.hostName), "%s", serverData.hiscoreHost);
    snprintf(req.hostURL, ArrayCount(req.hostURL), "%s", serverData.endpoint);
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

            ParseScoresLine(&scoreboard->topScores[scoreIndex], curPos, lineLen);

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
    int len = snprintf(req.body.str, 256, "%s,%d\r\n", scoreboard->userScore.username, scoreboard->userScore.score);
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

void DrawGameOverText(f32 y)
{
    Color flashingBrightRed = PeriodicFade(bsd_BRIGHT_RED);
    DrawHCentreScreenText("GAME OVER", y, FONT_H1_SIZE, flashingBrightRed);
}

// TODO: fix all these magic numbers
void DrawScoreBoard(const Scoreboard* scoreboard, Vector2 topleft)
{
    DrawRectangle(topleft.x, topleft.y, 400, 400, BROWN);
    const int startX = topleft.x + 20;
    int startY = topleft.y + 20;
    const int rowHeight = 32;

    DrawText("Scores:", startX, startY, 40, RED);

    startY += 45;
    const int nameX = startX;
    const int scoreX = startX + 300;
    bool drawnUserScore = false;
    for (u32 i = 0; i < ArrayCount(scoreboard->topScores); i++)
    {
        int y = startY + i * rowHeight;

        if (!drawnUserScore && scoreboard->userScore.score > scoreboard->topScores[i].score)
        {
            DrawText(scoreboard->userScore.username, nameX, y, 24, ORANGE);
            DrawText(TextFormat("%u", scoreboard->userScore.score), scoreX, y, 24, YELLOW);
            drawnUserScore = true;
            continue;
        }

        int topScoresIdx = i;
        if (drawnUserScore)
        {
            topScoresIdx--;
        }
        if (scoreboard->topScores[topScoresIdx].username[0] != 0)
        {
            // username
            DrawText(scoreboard->topScores[topScoresIdx].username, nameX, y, 24, SKYBLUE);

            // score
            DrawText(TextFormat("%u", scoreboard->topScores[topScoresIdx].score), scoreX, y, 24, YELLOW);
        }
    }

    if (!drawnUserScore)
    {
        const int userScoreY = startY + ArrayCount(scoreboard->topScores) * rowHeight;
        DrawText(scoreboard->userScore.username, nameX, userScoreY, 24, ORANGE);
        DrawText(TextFormat("%u", scoreboard->userScore.score), scoreX, userScoreY, 24, YELLOW);
    }
}

void GameoverShowScores(GameoverData* data)
{
    int key = GetKeyPressed();
    if (key != 0)
    {
        TransitionToState(core_GameMemoryGet(), MenuState_Title);
    }

    BeginDrawing();
        ClearBackground(BG_COLOR);

        DrawScoreBoard(&data->scoreboard, (Vector2){200, 80});

        Color flashingBlue = PeriodicFade(SKYBLUE);
        DrawHCentreScreenText("Press any button to play again!", 520, FONT_BODY_SIZE, flashingBlue);
    EndDrawing();
}

void GameoverScreenNoScores(GameoverData* data)
{
    Scoreboard* scoreboard = &data->scoreboard;
    int key = GetKeyPressed();
    if (key != 0)
    {
        TransitionToState(core_GameMemoryGet(), MenuState_Title);
    }
    BeginDrawing();
        ClearBackground(BG_COLOR);
        DrawGameOverText(100);
        DrawHCentreScreenText("Failed to connect to hiscores server", 200, FONT_BODY_SIZE, DARKBLUE);
        DrawHCentreScreenText(TextFormat("Score: %d", scoreboard->userScore.score), 300, FONT_H2_SIZE, GREEN);

        Color flashingDarkBlue = PeriodicFade(DARKBLUE);
        DrawHCentreScreenText("PRESS ANY KEY TO RETURN TO TITLE SCREEN", 500, FONT_BODY_SIZE, flashingDarkBlue);
    EndDrawing();
}

void DrawUsernameTextbox(GameoverData* data)
{
    Scoreboard* scoreboard = &data->scoreboard;
    static int framesCounter = 0;
    int key = GetCharPressed();
    char* name = scoreboard->userScore.username;
    int screenWidth = GetScreenWidth();
    int textboxWidth = MeasureText(MAX_USERNAME_PLACEHOLDER, 40);

    f32 textboxPosX = CalculateCentredPosition(0, screenWidth, textboxWidth);
    Rectangle textBox = { textboxPosX, 380, textboxWidth, 50 };

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
    DrawRectangleRec(textBox, LIGHTGRAY);
    DrawRectangleLines((int)textBox.x, (int)textBox.y, (int)textBox.width, (int)textBox.height, RED);
    DrawText(name, (int)textBox.x + 5, (int)textBox.y + 8, 40, MAROON);
    if (data->usernameLen < MAX_INPUT_CHARS)
    {
        if (((framesCounter/20)%2) == 0)
            DrawText("_", (int)textBox.x + 8 + MeasureText(name, 40), (int)textBox.y + 12, 40, MAROON);
    }
}

bool AlphaNumeric(char c)
{
    return ('0' <= c && c <= '9')
        || ('a' <= c && c <= 'z')
        || ('A' <= c && c <= 'Z');
}

bool ValidateUsername(char username[USERNAME_MAX_LEN])
{
    for (int i = 0; i < USERNAME_MAX_LEN; i++)
    {
        char c = username[i];
        if (c == 0)
        {
            // at least 3 chars long
            return i >= 3;
        }

        if (!AlphaNumeric(c))
        {
            return false;
        }
    }
    return true;
}

void GameoverInputScore(GameoverData* data)
{
    static bool displayValidationFailMsg = false;
    if (IsKeyPressed(KEY_ENTER))
    {
        if (ValidateUsername(data->scoreboard.userScore.username))
        {
            // username should already be in the userScore struct
            data->scoreboard.userScore.score = (int)(core_GameMemoryGet()->levelTimer * 100.0f);
            data->state = GameoverState_LoadingScore;
        }
        else
        {
            displayValidationFailMsg = true;
        }
    }

    BeginDrawing();
        ClearBackground(BG_COLOR);

        DrawGameOverText(200);
        DrawHCentreScreenText("Enter your username to record your score!", 320, FONT_BODY_SIZE, RAYWHITE);
        DrawUsernameTextbox(data);
        if (displayValidationFailMsg)
        {
            DrawHCentreScreenText("Username needs to be alphanumeric and at least 3 characters!", 450, FONT_BODY_SIZE, RED);
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