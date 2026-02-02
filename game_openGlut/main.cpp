#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
typedef enum { STORY, PLAY, ENDING } State;
static State gameState = STORY;
static int currentLevel = 0;

#define NUM_LEVELS 5
typedef struct {
    int cols, rows;
    const char **layout_top_to_bottom;
    int startC, startR;
    int goalC, goalR;
    const char *title;
    const char **story;
    int storyLines;
} FixedLevel;
static int **tiles = NULL;
static int cols = 0, rows = 0;

static float worldLeft = -0.95f, worldRight = 0.95f;
static float worldTop = 0.85f, worldBottom = -0.85f;

static float tileW = 0.02f, tileH = 0.02f;

static int startC = 0, startR = 0;
static int goalC = 0, goalR = 0;

static int playerC = 0, playerR = 0;

static int playerStage = 1;

typedef enum { F_LEFT, F_RIGHT } Facing;
static Facing playerFacing = F_RIGHT;

static float glowTimer = 0.0f;
static float storyAlpha = 0.0f;
static float storyFadeSpeed = 0.6f;

static int storyReadyToContinue = 0;

static int inv_armour = 0, inv_shield = 0, inv_sword = 0;

static int bossC = -1, bossR = -1;
static int bossAlive = 0;
static int bossHP = 0;

static float clampf(float v, float a, float b) {
    return v < a ? a : (v > b ? b : v);
}
static void tileCenterToNDC(int c, int r, float *x, float *y) {
    *x = worldLeft + (c + 0.5f) * tileW;
    *y = worldBottom + (r + 0.5f) * tileH;
}

static void tileRectNDC(int c, int r, float *x1, float *y1, float *x2, float *y2) {
    float cx, cy;
    tileCenterToNDC(c, r, &cx, &cy);
    *x1 = cx - tileW * 0.5f;
    *y1 = cy - tileH * 0.5f;
    *x2 = cx + tileW * 0.5f;
    *y2 = cy + tileH * 0.5f;
}

static int inBounds(int c, int r) {
    return (c >= 0 && c < cols && r >= 0 && r < rows);
}

static const char *L1_layout[] = {
    "111111111111111111111111",
    "100000001111000000000001",
    "101111101111011111110101",
    "101000100001010000010101",
    "101011111101011111010101",
    "101010000101000001010101",
    "101010111101111101010101",
    "101010100000000101010101",
    "101010101111110101010101",
    "101000101000010100010001",
    "101111101011010111111101",
    "100000001000010000000001"
};

static const char *L2_layout[] = {
    "111111111111111111111111",
    "1S0000001111000000000001",
    "111011101001011111110111",
    "100010001001000000010001",
    "101110111101111011110101",
    "100000100001000010000001",
    "111110101111011110111101",
    "100000101000000000000001",
    "101111101011111111111101",
    "1000000000000000000000G1",
    "111111111111111111111111"
};

static const char *L3_layout[] = {
    "111111111111111111111111",
    "100000001111000000000001",
    "101111101011011111110101",
    "101000101000010000010001",
    "101011101111011111010101",
    "101010100001000001010101",
    "101010111101111101010101",
    "101010100000000101010101",
    "101010101111110101010101",
    "1S00000100000000000000G1",
    "111111111111111111111111"
};

static const char *L4_layout[] = {
    "11111111111111111111111111111111111111111111",
    "10000000000010000000000001000000000000000001",
    "10111111111010111111111101011111111111111011",
    "10100000001010100000000101010000000000001001",
    "10101111101010101111110101010111111111101011",
    "10101000101010101000010101010000000000101001",
    "10101011101010101111010101011111111110101011",
    "10101010001010100001010101000000000100101001",
    "10101010111010101111010101111101110101101011",
    "10100010100010100001010100001000100101001001",
    "10111110101110111101010101101111101101111011",
    "10000000100010000001000100000000001000000001",
    "11111111101011111101110101111111101111111111",
    "10000000001000000001000001000000001000000001",
    "10111111101111111101111101111111001111111011",
    "10000000000000000000000000000000000000000001",
    "10111111111111111111111111111111111111111101",
    "10000000000000000000000000000000000000000001"
};

static const char *L5_layout[] = {
    "11111111111111111111111111111111111111111111",
    "1S000001000100000001000000010000000100000001",
    "11011101011101111101011111010111110101111101",
    "11010001010001000101010000010100010100000101",
    "11010111010111011101010111110101110111110101",
    "11010100010100000101010100000101000100010101",
    "11010101110101110101010101110101011101010101",
    "11010100010101010101010100010101010001010101",
    "11010111010101010101010111010101010111010101",
    "11010001010101010101000001010101010100010101",
    "11011101010101010101111111010101010101110101",
    "11000101010001010100000001000101010100010101",
    "11110101011101010111111101111101010111010101",
    "10010101000101010000000101000001010000010101",
    "10110101110101011111110101111101011111110101",
    "10000100010001000000010100000101000000000101",
    "11110111011101111111010111110101111111110101",
    "100000000000000000000000000000000000000000G1"
};
static const char *L1_story[] = {
    "Lyra wakes up in a broken place.",
    "She feels lost and alone.",
    "She has no armour, no shield, and no sword.",
    "To get them back, she must enter the maze."
};
static const char *L2_story[] = {
    "Lyra comes to a quiet river.",
    "She sees something shining in the water.",
    "It is a piece of her armour.",
    "She must go inside the maze to take it."
};
static const char *L3_story[] = {
    "Lyra reaches an old broken fort.",
    "The place looks dark and scary.",
    "A part of her shield is inside the maze.",
    "She goes in to find it."
};
static const char *L4_story[] = {
    "Lyra walks into a hot, red mountain area.",
    "The ground is full of fire and smoke.",
    "Her sword is hidden in this maze.",
    "She must find it to fight the final monster."
};
static const char *L5_story[] = {
    "Lyra now has all her items.",
    "Only one last maze remains.",
    "A strong monster waits inside.",
    "Lyra must finish her journey now."
};

static FixedLevel fixedLevels[NUM_LEVELS];

static void setupFixedLevels(void) {

    fixedLevels[0].cols = 24;
    fixedLevels[0].rows = 12;
    fixedLevels[0].layout_top_to_bottom = L1_layout;
    fixedLevels[0].startC = 0;
    fixedLevels[0].startR = 10;
    fixedLevels[0].goalC = 23;
    fixedLevels[0].goalR = 1;
    fixedLevels[0].title = "Whispering Forest - I";
    fixedLevels[0].story = L1_story;
    fixedLevels[0].storyLines = 4;

    fixedLevels[1].cols = 24;
    fixedLevels[1].rows = 11;
    fixedLevels[1].layout_top_to_bottom = L2_layout;
    fixedLevels[1].startC = 1;
    fixedLevels[1].startR = 9;
    fixedLevels[1].goalC = 22;
    fixedLevels[1].goalR = 1;
    fixedLevels[1].title = "River of Reflections - II";
    fixedLevels[1].story = L2_story;
    fixedLevels[1].storyLines = 4;

    fixedLevels[2].cols = 24;
    fixedLevels[2].rows = 11;
    fixedLevels[2].layout_top_to_bottom = L3_layout;
    fixedLevels[2].startC = 1;
    fixedLevels[2].startR = 1;
    fixedLevels[2].goalC = 22;
    fixedLevels[2].goalR = 1;
    fixedLevels[2].title = "Fallen Fortress - III";
    fixedLevels[2].story = L3_story;
    fixedLevels[2].storyLines = 4;

    fixedLevels[3].cols = 44;
    fixedLevels[3].rows = 18;
    fixedLevels[3].layout_top_to_bottom = L4_layout;
    fixedLevels[3].startC = 0;
    fixedLevels[3].startR = 16;
    fixedLevels[3].goalC = 43;
    fixedLevels[3].goalR = 1;
    fixedLevels[3].title = "Crimson Peaks - IV";
    fixedLevels[3].story = L4_story;
    fixedLevels[3].storyLines = 4;

    fixedLevels[4].cols = 54;
    fixedLevels[4].rows = 20;
    fixedLevels[4].layout_top_to_bottom = L5_layout;
    fixedLevels[4].startC = 0;
    fixedLevels[4].startR = 18;
    fixedLevels[4].goalC = 53;
    fixedLevels[4].goalR = 1;
    fixedLevels[4].title = "Final Ascent - V";
    fixedLevels[4].story = L5_story;
    fixedLevels[4].storyLines = 4;
}
static void allocateTiles(int r, int c) {
    if (tiles) {
        for (int i = 0; i < rows; ++i)
            free(tiles[i]);
        free(tiles);
    }
    rows = r;
    cols = c;

    tiles = (int**) malloc(sizeof(int*) * rows);
    for (int i = 0; i < rows; ++i) {
        tiles[i] = (int*) malloc(sizeof(int) * cols);
        for (int j = 0; j < cols; ++j)
            tiles[i][j] = 1;
    }
}
static void findMarkersInLayout(FixedLevel *F, int *outStartC, int *outStartR,
                                int *outGoalC, int *outGoalR) {

    int sc=-1, sr=-1, gc=-1, gr=-1;

    for (int i=0;i<F->rows;++i) {
        const char *row = F->layout_top_to_bottom[i];
        int targetR = F->rows - 1 - i;

        for (int c=0; c<F->cols && row[c]; ++c) {
            if (row[c] == 'S') { sc=c; sr=targetR; }
            if (row[c] == 'G') { gc=c; gr=targetR; }
        }
    }

    if (sc>=0) { *outStartC = sc; *outStartR = sr; }
    else { *outStartC = F->startC; *outStartR = F->startR; }

    if (gc>=0) { *outGoalC = gc; *outGoalR = gr; }
    else { *outGoalC = F->goalC; *outGoalR = F->goalR; }
}

static int checkReachable(int sc, int sr, int gc, int gr) {

    if (!inBounds(sc,sr) || !inBounds(gc,gr)) return 0;
    if (tiles[sr][sc] != 0 || tiles[gr][gc] != 0) return 0;

    int *visited = (int*) calloc(rows*cols, sizeof(int));
    int *qx = (int*) malloc(sizeof(int)*rows*cols);
    int *qy = (int*) malloc(sizeof(int)*rows*cols);

    int qh=0, qt=0;
    qx[qt] = sc; qy[qt] = sr; qt++;
    visited[sr*cols + sc] = 1;

    const int dx[4] = {1,-1,0,0};
    const int dy[4] = {0,0,1,-1};

    while (qh < qt) {

        int x = qx[qh], y = qy[qh];
        qh++;

        if (x == gc && y == gr) {
            free(visited); free(qx); free(qy);
            return 1;
        }

        for (int k=0;k<4;++k) {
            int nx = x + dx[k], ny = y + dy[k];

            if (nx>=0 && nx<cols && ny>=0 && ny<rows &&
                !visited[ny*cols + nx] && tiles[ny][nx]==0)
            {
                visited[ny*cols + nx] = 1;
                qx[qt] = nx; qy[qt] = ny; qt++;
            }
        }
    }

    free(visited); free(qx); free(qy);
    return 0;
}
static void carveCorridor(int sc, int sr, int gc, int gr) {
    int x = sc, y = sr;
    tiles[y][x] = 0;

    while (x != gc) {
        x += (gc > x ? 1 : -1);
        tiles[y][x] = 0;
    }
    while (y != gr) {
        y += (gr > y ? 1 : -1);
        tiles[y][x] = 0;
    }
}
static void closeFullOpenColumnsSafely(void) {

    for (int c = 0; c < cols; ++c) {

        int allOpen = 1;
        for (int r = 0; r < rows; ++r)
            if (tiles[r][c] != 0) { allOpen = 0; break; }

        if (!allOpen) continue;
        if (c == startC || c == goalC) continue;

        int *backup = (int*) malloc(sizeof(int) * rows);
        for (int r = 0; r < rows; ++r) backup[r] = tiles[r][c];

        for (int r = 0; r < rows; ++r) tiles[r][c] = 1;

        int ok = checkReachable(startC,startR,goalC,goalR);

        if (!ok) {
            for (int r=0;r<rows;++r) tiles[r][c] = backup[r];
        }

        free(backup);
    }
}

static void loadFixedLevel(int levelIndex) {

    if (levelIndex < 0 || levelIndex >= NUM_LEVELS) return;

    FixedLevel *F = &fixedLevels[levelIndex];

    allocateTiles(F->rows, F->cols);

    for (int i = 0; i < F->rows; ++i) {

        const char *rowStr = F->layout_top_to_bottom[i];
        int targetR = F->rows - 1 - i;
        int len = strlen(rowStr);

        for (int c=0; c < F->cols && c < len; ++c) {
            char ch = rowStr[c];
            tiles[targetR][c] = (ch == '0' || ch=='S' || ch=='G') ? 0 : 1;
        }
        for (int c=len;c<F->cols;++c)
            tiles[targetR][c] = 1;
    }

    int sc, sr, gc, gr;
    findMarkersInLayout(F, &sc, &sr, &gc, &gr);

    startC = sc; startR = sr;
    goalC = gc; goalR = gr;

    tiles[startR][startC] = 0;
    tiles[goalR][goalC]  = 0;

    bossAlive = 0;
    bossHP = 0;
    bossC = bossR = -1;
    if (levelIndex == 4) {

        int reachable = checkReachable(startC, startR, goalC, goalR);
        if (!reachable)
            carveCorridor(startC,startR,goalC,goalR);

        bossC = goalC;
        bossR = goalR;
        bossAlive = 1;
        bossHP = 4;

        tiles[bossR][bossC] = 1;

        const int dx[4] = {1,-1,0,0};
        const int dy[4] = {0,0,1,-1};

        int bestN=-1, bestD=999999, opened=0;

        for (int k=0;k<4;++k) {
            int nx=bossC+dx[k], ny=bossR+dy[k];
            if (!inBounds(nx,ny)) continue;

            if (tiles[ny][nx]==0) { opened=1; break; }

            int d = abs(nx-startC) + abs(ny-startR);
            if (d < bestD) { bestD = d; bestN = k; }
        }

        if (!opened && bestN>=0) {
            int nx=bossC+dx[bestN], ny=bossR+dy[bestN];
            tiles[ny][nx] = 0;
        }

        closeFullOpenColumnsSafely();
    }
}

static void drawQuad(float x1,float y1,float x2,float y2) {
    glBegin(GL_QUADS);
        glVertex2f(x1,y1);
        glVertex2f(x2,y1);
        glVertex2f(x2,y2);
        glVertex2f(x1,y2);
    glEnd();
}
static void drawBorderBox(void) {
    float thicknessX = (worldRight - worldLeft)*0.01f;
    float thicknessY = (worldTop - worldBottom)*0.01f;

    glColor3f(0.02f,0.02f,0.02f);
    drawQuad(worldLeft, worldTop - thicknessY, worldRight, worldTop);
    drawQuad(worldLeft, worldBottom, worldRight, worldBottom + thicknessY);
    drawQuad(worldLeft, worldBottom, worldLeft + thicknessX, worldTop);
    drawQuad(worldRight - thicknessX, worldBottom, worldRight, worldTop);
}

static void drawGoalTile(int c, int r, float t) {
    if (!inBounds(c,r)) return;
    if (tiles[r][c] != 0) return;

    float cx, cy; tileCenterToNDC(c,r,&cx,&cy);

    float pulse = 0.5f*(1.0f + sinf(t*3.0f));

    float R = 0.3f+0.5f*pulse;
    float G = 0.6f+0.4f*pulse;

    glColor3f(R,G,1.0f);
    float s = fminf(tileW,tileH)*0.45f;
    drawQuad(cx-s,cy-s,cx+s,cy+s);

    glColor3f(1,1,1);
    drawQuad(cx-s*0.35f, cy-s*0.35f, cx+s*0.35f, cy+s*0.35f);
}
static void drawLyraAtTile(int c, int r, int stage, Facing facing) {

    float cx, cy; tileCenterToNDC(c,r,&cx,&cy);

    float h = tileH * 1.0f;
    float w = h * 0.45f;
    float feetY = cy - tileH*0.5f;
    float centerY = feetY + h*0.5f;
    glColor3f(0.18f,0.02f,0.06f);
    drawQuad(cx - w*0.6f, centerY - h*0.45f,
             cx - w*0.05f, centerY + h*0.45f);
    if (stage >= 2) glColor3f(0.78f,0.78f,0.82f);
    else            glColor3f(0.56f,0.58f,0.62f);
    drawQuad(cx - w*0.5f, centerY - h*0.2f,
             cx + w*0.5f, centerY + h*0.15f);
    glColor3f(0.92f,0.9f,0.88f);
    drawQuad(cx - w*0.18f, centerY + h*0.25f,
             cx + w*0.18f, centerY + h*0.45f);

    glColor3f(0.06f,0.06f,0.08f);
    drawQuad(cx - w*0.12f, centerY + h*0.32f,
             cx + w*0.12f, centerY + h*0.36f);

    glColor3f(0.9f,0.9f,1.0f);
    drawQuad(cx - w*0.06f, centerY + h*0.33f,
             cx - w*0.03f, centerY + h*0.35f);
    drawQuad(cx + w*0.03f, centerY + h*0.33f,
             cx + w*0.06f, centerY + h*0.35f);

    glColor3f(0.2f,0.2f,0.22f);
    drawQuad(cx - w*0.35f, centerY - h*0.55f,
             cx - w*0.05f, centerY - h*0.2f);
    drawQuad(cx + w*0.05f, centerY - h*0.55f,
             cx + w*0.35f, centerY - h*0.2f);
    glColor3f(0.06f,0.06f,0.08f);
    drawQuad(cx - w*0.35f, centerY - h*0.75f,
             cx - w*0.05f, centerY - h*0.55f);
    drawQuad(cx + w*0.05f, centerY - h*0.75f,
             cx + w*0.35f, centerY - h*0.55f);

    if (stage >= 3) {
        glColor3f(0.6f,0.6f,0.68f);
        drawQuad(cx - w*0.95f, centerY - h*0.05f,
                 cx - w*0.65f, centerY + h*0.25f);
    }

    if (stage >= 4) {
        glColor3f(0.96f,0.86f,0.28f);
        drawQuad(cx + w*0.65f, centerY + h*0.0f,
                 cx + w*0.78f, centerY + h*0.06f);

        glColor3f(0.9f,0.92f,0.98f);
        drawQuad(cx + w*0.78f, centerY + h*0.0f,
                 cx + w*1.05f, centerY + h*0.02f);
    }
}

static void drawBossAtTile(int c,int r,int hp) {

    if (!inBounds(c,r)) return;

    float cx,cy;
    tileCenterToNDC(c,r,&cx,&cy);

    float s = fminf(tileW,tileH)*0.46f;
    glColor3f(0.12f,0.06f,0.08f);
    drawQuad(cx-s,cy-s,cx+s,cy+s);

    for (int i=0;i<3;i++) {
        float px = cx - s + i*(2*s/2.0f);
        glColor3f(0.5f,0.08f,0.08f);
        drawQuad(px, cy+s*0.45f, px+s*0.4f, cy+s*0.9f);
    }

    glColor3f(1,0.6f,0.2f);
    drawQuad(cx-s*0.4f, cy+s*0.05f, cx-s*0.14f, cy+s*0.25f);
    drawQuad(cx+s*0.14f, cy+s*0.05f, cx+s*0.4f, cy+s*0.25f);

    glColor3f(0.02f,0.02f,0.02f);
    drawQuad(cx-s*0.35f, cy-s*0.4f, cx+s*0.35f, cy-s*0.25f);

    float bw=s*1.6f, bh=s*0.18f;
    float bx1=cx-bw*0.5f, by1=cy+s+0.02f;
    float bx2 = bx1 + bw*((float)hp/4.0f);

    glColor3f(0.8f,0.12f,0.12f);
    drawQuad(bx1,by1,bx2,by1+bh);

    glColor3f(0.2f,0.2f,0.2f);
    drawQuad(bx2,by1,cx+bw*0.5f,by1+bh);
}

static int canMoveTo(int nc,int nr) {
    if (!inBounds(nc,nr)) return 0;
    if (tiles[nr][nc] != 0) return 0;
    return 1;
}

static void pickupItemForLevel(int level) {
    if (level == 0) {
        inv_armour = 1;
    }
    else if (level == 2) {
        inv_shield = 1;
    }
    else if (level == 4) {
        inv_sword = 1;
    }
}

static void tryAdvanceIfAtGoal(void) {

    if (playerC == goalC && playerR == goalR) {

        if (currentLevel == 4 && bossAlive) return;

        pickupItemForLevel(currentLevel);

        currentLevel++;
playerStage = 1;
if (inv_armour) playerStage = 2;
if (inv_shield) playerStage = 3;
if (inv_sword)  playerStage = 5;


        if (currentLevel >= NUM_LEVELS)
            gameState = ENDING;
        else {
            gameState = STORY;
            storyAlpha = 0.0f;
            storyReadyToContinue = 0;
        }
    }
}

static void movePlayerDelta(int dc,int dr) {

    int nc = playerC + dc;
    int nr = playerR + dr;

    if (canMoveTo(nc,nr)) {
        playerC = nc;
        playerR = nr;
        if (dc<0) playerFacing = F_LEFT;
        if (dc>0) playerFacing = F_RIGHT;
    }

    tryAdvanceIfAtGoal();
}
static void tryPlayerAttack(void) {

    if (gameState != PLAY) return;
    if (!bossAlive) return;

    int dx=abs(playerC-bossC);
    int dy=abs(playerR-bossR);

    if ((dx+dy)<=1) {

        bossHP--;

        if (bossHP <= 0) {
            bossAlive = 0;
            tiles[bossR][bossC] = 0;
            tryAdvanceIfAtGoal();
        }

    }
}
static void keyboardChar(unsigned char key, int x, int y) {
    (void)x; (void)y;

    if (key == 27) exit(0);

    if (key == 13) {
        if (gameState == STORY) {

            if (!storyReadyToContinue) {
                storyAlpha = 1.0f;
                storyReadyToContinue = 1;
            }
            else {
                loadFixedLevel(currentLevel);
                tileW = (worldRight - worldLeft) / (float)cols;
                tileH = (worldTop - worldBottom) / (float)rows;

                playerStage = currentLevel + 1;
                if (playerStage > 4) playerStage = 4;
                if (playerStage < 1) playerStage = 1;

                playerC = startC;
                playerR = startR;
                playerFacing = F_RIGHT;

                gameState = PLAY;
                glowTimer = 0.0f;
            }
        }

        else if (gameState == ENDING) {
            exit(0);
        }
    }

    if (key == 'a' || key == 'A')
        tryPlayerAttack();
}

static void specialKey(int key, int x, int y) {
    (void)x; (void)y;

    if (gameState != PLAY) return;

    if (key == GLUT_KEY_LEFT)  movePlayerDelta(-1, 0);
    if (key == GLUT_KEY_RIGHT) movePlayerDelta(1, 0);
    if (key == GLUT_KEY_UP)    movePlayerDelta(0, 1);
    if (key == GLUT_KEY_DOWN)  movePlayerDelta(0, -1);

    glutPostRedisplay();
}

static void renderText(float x, float y, const char *s) {
    glRasterPos2f(x, y);
    for (const char *p = s; *p; ++p)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *p);
}

static void drawInventoryUI(void) {

    float startX = worldRight - 0.18f;
    float y = worldTop - 0.06f;
    float size = 0.045f;

    glColor3f(0.02f,0.02f,0.02f);
    drawQuad(startX - 0.02f, y - 0.06f,
             startX + 0.18f, y + 0.02f);

    glColor3f(inv_armour ? 0.78f : 0.18f,
              inv_armour ? 0.78f : 0.18f,
              inv_armour ? 0.82f : 0.18f);
    drawQuad(startX, y - 0.02f,
             startX + size, y + size - 0.02f);
    renderText(startX + size + 0.01f, y, "Armour");

    glColor3f(inv_shield ? 0.6f : 0.18f,
              inv_shield ? 0.6f : 0.18f,
              inv_shield ? 0.68f : 0.18f);
    drawQuad(startX, y - 0.06f - size,
             startX + size, y - 0.02f - size);
    renderText(startX + size + 0.01f,
               y - 0.06f - size + 0.02f, "Shield");

    glColor3f(inv_sword ? 0.9f : 0.18f,
              inv_sword ? 0.92f : 0.18f,
              inv_sword ? 0.98f : 0.18f);
    drawQuad(startX,
             y - 0.12f - 2*size,
             startX + size,
             y - 0.06f - 2*size);
    renderText(startX + size + 0.01f,
               y - 0.12f - 2*size + 0.02f, "Sword");
}
static void drawLevel(void) {

    if (currentLevel == 0) glClearColor(0.02,0.22,0.08,1);
    else if (currentLevel == 1) glClearColor(0.05,0.18,0.28,1);
    else if (currentLevel == 2) glClearColor(0.09,0.09,0.11,1);
    else if (currentLevel == 3) glClearColor(0.28,0.09,0.07,1);
    else                       glClearColor(0.18,0.14,0.22,1);

    glClear(GL_COLOR_BUFFER_BIT);

    drawBorderBox();

    for (int r=0; r<rows; ++r) {
        for (int c=0; c<cols; ++c) {
            if (tiles[r][c] == 1) {

                float x1,y1,x2,y2;
                tileRectNDC(c,r,&x1,&y1,&x2,&y2);

                float shade = 0.25f + 0.45f*(float)r/(float)rows;

                glColor3f(0.09f*shade,0.11f*shade,0.13f*shade);
                drawQuad(x1,y1,x2,y2);

                glColor3f(0.16f*shade,0.18f*shade,0.2f*shade);
                drawQuad(x1, y2-(y2-y1)*0.14f, x2, y2);
            }
        }
    }

    drawGoalTile(goalC,goalR,glowTimer);

    if (bossAlive)
        drawBossAtTile(bossC,bossR,bossHP);

    drawLyraAtTile(playerC,playerR,playerStage,playerFacing);

    glColor3f(1,1,1);
    char buf[64];
    snprintf(buf,sizeof(buf), "Level %d/%d",
             currentLevel+1, NUM_LEVELS);
    renderText(-0.94f,0.92f,buf);

    renderText(-0.94f,0.85f, fixedLevels[currentLevel].title);

    drawInventoryUI();

    if (bossAlive && currentLevel == 4) {
        glColor3f(1,1,1);
        renderText(-0.94f,0.78f,
            "Boss: Shadow Gate Warden (Press A to attack)");
    }
}

static void drawStoryScreen(void) {
    if (currentLevel == 0) glClearColor(0.02f, 0.22f, 0.08f, 1.0f);
    else if (currentLevel == 1) glClearColor(0.05f, 0.18f, 0.28f, 1.0f);
    else if (currentLevel == 2) glClearColor(0.09f, 0.09f, 0.11f, 1.0f);
    else if (currentLevel == 3) glClearColor(0.28f, 0.09f, 0.07f, 1.0f);
    else                       glClearColor(0.18f, 0.14f, 0.22f, 1.0f);

    glClear(GL_COLOR_BUFFER_BIT);

    float a = clampf(storyAlpha, 0.0f, 1.0f);
    glColor4f(1.0f, 1.0f, 1.0f, a);
    int lines = fixedLevels[currentLevel].storyLines;

    float lineSpacing = 0.12f;
    float totalHeight = lines * lineSpacing;
    float y = totalHeight * 0.25f;
    renderText(-0.25f, 0.60f, fixedLevels[currentLevel].title);

    for (int i = 0; i < lines; i++) {
        renderText(-0.55f, y, fixedLevels[currentLevel].story[i]);
        y -= lineSpacing;
    }

    if (a >= 1.0f) {
        renderText(-0.25f, -0.75f, "Press ENTER to continue");
        storyReadyToContinue = 1;
    }
}

static void display(void) {
    glLoadIdentity();

    if (gameState == STORY)
        drawStoryScreen();

    else if (gameState == PLAY)
        drawLevel();

    else {
        glClearColor(0.03f,0.03f,0.06f,1);
        glClear(GL_COLOR_BUFFER_BIT);

        glColor3f(1,1,1);
        renderText(-0.6f,0.3f,
        "The Shadow Beast falls. Light returns.");
        renderText(-0.6f,0.15f,
        "Lyra stands strong as a new dawn rises.");
        renderText(-0.3f,-0.7f,
        "Press ENTER to exit");
    }

    glutSwapBuffers();
}

static void idleTimer(int) {

    float dt = 0.03f;
    glowTimer += dt;

    if (gameState == STORY && !storyReadyToContinue) {
        storyAlpha += storyFadeSpeed * dt;
        if (storyAlpha >= 1.0f) {
            storyAlpha = 1.0f;
            storyReadyToContinue = 1;
        }
    }

    glutPostRedisplay();
    glutTimerFunc(30, idleTimer, 0);
}
static void reshape(int w, int h) {

    if (w <= 0) w=1;
    if (h <= 0) h=1;

    glViewport(0,0,w,h);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    float mazeAspect = (cols>0 && rows>0)
                       ? ((float)cols/(float)rows)
                       : 1.0f;
    float windowAspect = (float)w/(float)h;

    if (windowAspect >= mazeAspect) {
        float viewHalfY = 1.0f;
        float viewHalfX = mazeAspect * viewHalfY;
        float scale = windowAspect / mazeAspect;
        viewHalfX *= scale;

        worldLeft = -viewHalfX;
        worldRight = viewHalfX;
        worldBottom = -viewHalfY;
        worldTop = viewHalfY;
    }
    else {
        float viewHalfX = 1.0f;
        float viewHalfY = (1.0f/mazeAspect)*viewHalfX;
        float scale = mazeAspect / windowAspect;
        viewHalfY *= (1.0f/scale);

        worldLeft = -viewHalfX;
        worldRight = viewHalfX;
        worldBottom = -viewHalfY;
        worldTop = viewHalfY;
    }

    glOrtho(worldLeft,worldRight,
            worldBottom,worldTop,-1,1);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    if (cols>0 && rows>0) {
        tileW = (worldRight-worldLeft)/cols;
        tileH = (worldTop-worldBottom)/rows;
    }
}

int main(int argc, char **argv) {

    glutInit(&argc,argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(1100,700);
    glutCreateWindow("Lyra - Maze of Destiny");

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    setupFixedLevels();

    cols = fixedLevels[0].cols;
    rows = fixedLevels[0].rows;

    allocateTiles(rows, cols);

    tileW = (worldRight-worldLeft)/cols;
    tileH = (worldTop-worldBottom)/rows;

    storyAlpha = 0;
    storyReadyToContinue = 0;

    inv_armour = inv_shield = inv_sword = 0;
    bossAlive = 0;
    bossHP = 0;
    bossC = bossR = -1;

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboardChar);
    glutSpecialFunc(specialKey);
    glutTimerFunc(30, idleTimer, 0);

    glutMainLoop();
    return 0;
}
