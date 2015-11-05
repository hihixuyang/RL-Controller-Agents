#include "draw.h"


void drawMapASCII(Env * env)
{
    int i, j;

    for (i=0; i<GRID_SIZE * BLOCK_SIZE; i++){
        if ((i != 0) && (i % BLOCK_SIZE == 0))
            printf("\n");
        for (j=0; j<GRID_SIZE * BLOCK_SIZE; j++){
            if ((j != 0) && (j % BLOCK_SIZE == 0))
                printf(" ");
            printf("%d", env->block[i/BLOCK_SIZE][j/BLOCK_SIZE].cell[i%BLOCK_SIZE][j%BLOCK_SIZE]);
        }
        printf("\n");
    }
}

void startSDL()
{
    SDL_Init(SDL_INIT_EVERYTHING);

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    SDL_WM_SetCaption("Agents-Controller", NULL);

    SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 32, SDL_OPENGL);

    glClearColor(50, 50, 50, 1);

    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

    glShadeModel(GL_SMOOTH);

    glMatrixMode(GL_PROJECTION);

    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);
}

void drawMapSDL(Env * env)
{
    int row, col;
    int backClr = 0;

    glClear(GL_COLOR_BUFFER_BIT);
    glPushMatrix();
    glOrtho(0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, -1, 1);
    glEnable(GL_TEXTURE_2D);

    for (row=0; row<GRID_SIZE; row++){
        for (col=0; col<GRID_SIZE; col++){
            if (env->view == 0){
                if (env->blockStatus[env->teamShow][row][col] == UNEXPLORED)
                    drawUnexplored(&env->block[row][col], row, col, 1);
                else if (env->blockStatus[env->teamShow][row][col] == EXPLORED_NORMAL ||
                         env->blockStatus[env->teamShow][row][col] == HOME_ZONE ||
                         env->blockStatus[env->teamShow][row][col] == END_ZONE)
                    drawExplored(env, &env->block[row][col], row, col, 0);
                else
                    drawBlock(env, &env->block[row][col], row, col, backClr);
            }
            else if (env->view == 1){
                if (env->blockStatus[env->teamShow][row][col] == UNEXPLORED)
                    drawUnexplored(&env->block[row][col], row, col, 1);
                else
                    drawBlock(env, &env->block[row][col], row, col, backClr);
            }
            else{
                drawBlock(env, &env->block[row][col], row, col, backClr);
            }

            backClr = backClr == 0 ? 1 : 0;
        }
    }

    drawGrid();
    drawBooms(env);

    glDisable(GL_TEXTURE_2D);
    glPopMatrix();
    SDL_GL_SwapBuffers();
}

void drawUnexplored(Block * block, int row, int col, int backClr)
{
    int i, j;

    for (i=0; i<BLOCK_SIZE; i++)
        for (j=0; j<BLOCK_SIZE; j++)
            drawDark(row*CELL_SIZE*BLOCK_SIZE + i * CELL_SIZE, 
                     col*CELL_SIZE*BLOCK_SIZE + j * CELL_SIZE, backClr);
}

void drawExplored(Env * env, Block * block, int row, int col, int backClr)
{
    int i, j;

    for (i=0; i<BLOCK_SIZE; i++)
        for (j=0; j<BLOCK_SIZE; j++)
            switch(block->permanentValue[i][j]){
                case WALL       :
                    drawWall(row*CELL_SIZE*BLOCK_SIZE + i * CELL_SIZE, 
                             col*CELL_SIZE*BLOCK_SIZE + j * CELL_SIZE, backClr, 1);
                    break;
                case GREEN_ZONE :
                    drawGrnZone(env, row*CELL_SIZE*BLOCK_SIZE + i * CELL_SIZE, 
                                col*CELL_SIZE*BLOCK_SIZE + j * CELL_SIZE, backClr, 1);
                    break;
                case RED_ZONE   :
                    drawRedZone(env, row*CELL_SIZE*BLOCK_SIZE + i * CELL_SIZE, 
                                col*CELL_SIZE*BLOCK_SIZE + j * CELL_SIZE, backClr, 1);
                    break;
                default         :
                    drawEmpty(row*CELL_SIZE*BLOCK_SIZE + i * CELL_SIZE, 
                              col*CELL_SIZE*BLOCK_SIZE + j * CELL_SIZE, backClr, 1);
                    break;
            }
}

void drawBlock(Env * env, Block * block, int row, int col, int backClr)
{
    int i, j;

    for (i=0; i<BLOCK_SIZE; i++){
        for (j=0; j<BLOCK_SIZE; j++){
            switch (block->cell[i][j]){
                case EMPTY          :
                    drawEmpty(row*CELL_SIZE*BLOCK_SIZE + i * CELL_SIZE, 
                              col*CELL_SIZE*BLOCK_SIZE + j * CELL_SIZE, backClr, 0);
                    break;
                case WALL           :
                    drawWall(row*CELL_SIZE*BLOCK_SIZE + i * CELL_SIZE, 
                             col*CELL_SIZE*BLOCK_SIZE + j * CELL_SIZE, 0, 0);
                    break;
                case GREEN_AGENT_1  :
                    drawGrnAgent(row*CELL_SIZE*BLOCK_SIZE + i * CELL_SIZE, 
                                 col*CELL_SIZE*BLOCK_SIZE + j * CELL_SIZE, backClr);
                    break;
                case GREEN_AGENT_2  :
                    drawGrnAgent2(row*CELL_SIZE*BLOCK_SIZE + i * CELL_SIZE, 
                                  col*CELL_SIZE*BLOCK_SIZE + j * CELL_SIZE, backClr);
                    break;
                case RED_AGENT_1    :
                    drawRedAgent(row*CELL_SIZE*BLOCK_SIZE + i * CELL_SIZE, 
                                 col*CELL_SIZE*BLOCK_SIZE + j * CELL_SIZE, backClr);
                    break;
                case RED_AGENT_2    :
                    drawRedAgent2(row*CELL_SIZE*BLOCK_SIZE + i * CELL_SIZE, 
                                  col*CELL_SIZE*BLOCK_SIZE + j * CELL_SIZE, backClr);
                    break;
                case BULLET_LEFT    :
                case BULLET_RIGHT   :
                case BULLET_UP      :
                case BULLET_DOWN    :
                    switch(block->permanentValue[i][j]){
                        case EMPTY      :
                            drawEmpty(row*CELL_SIZE*BLOCK_SIZE + i * CELL_SIZE, 
                                      col*CELL_SIZE*BLOCK_SIZE + j * CELL_SIZE, backClr, 0);
                            break;
                        case GREEN_ZONE :
                            drawGrnZone(env, row*CELL_SIZE*BLOCK_SIZE + i * CELL_SIZE, 
                                        col*CELL_SIZE*BLOCK_SIZE + j * CELL_SIZE, backClr, 0);
                            break;
                        case RED_ZONE   :
                            drawRedZone(env, row*CELL_SIZE*BLOCK_SIZE + i * CELL_SIZE, 
                                        col*CELL_SIZE*BLOCK_SIZE + j * CELL_SIZE, backClr, 0);
                            break;
                        default         :
                            drawEmpty(row*CELL_SIZE*BLOCK_SIZE + i * CELL_SIZE, 
                                      col*CELL_SIZE*BLOCK_SIZE + j * CELL_SIZE, backClr, 0);
                            break;
                    }
                    drawBullet(row*CELL_SIZE*BLOCK_SIZE + i * CELL_SIZE, 
                               col*CELL_SIZE*BLOCK_SIZE + j * CELL_SIZE, backClr);
                    break;
                case GREEN_ZONE     :
                    drawGrnZone(env, row*CELL_SIZE*BLOCK_SIZE + i * CELL_SIZE, 
                                col*CELL_SIZE*BLOCK_SIZE + j * CELL_SIZE, backClr, 0);
                    break;
                case RED_ZONE       :
                    drawRedZone(env, row*CELL_SIZE*BLOCK_SIZE + i * CELL_SIZE, 
                                col*CELL_SIZE*BLOCK_SIZE + j * CELL_SIZE, backClr, 0);
                    break;
                default             :
                    break;
            }
        }
    }
}

void drawBackground(int yStart, int xStart, int backClr, int dark)
{
    glColor4ub(175, 175, 175, 255);

    glBegin(GL_QUADS);
    glVertex2f(xStart, yStart);
    glVertex2f(xStart + CELL_SIZE, yStart);
    glVertex2f(xStart + CELL_SIZE, yStart + CELL_SIZE);
    glVertex2f(xStart, yStart + CELL_SIZE);
    glEnd();
}

void drawDark(int yStart, int xStart, int backClr)
{
    drawBackground(yStart, xStart, backClr, 1);

    yStart += MARGIN;
    xStart += MARGIN;

    if (backClr == 0)
            glColor4ub(5, 5, 5, 255);
    else
            glColor4ub(15, 15, 15, 15);

    glBegin(GL_QUADS);
    glVertex2f(xStart, yStart);
    glVertex2f(xStart + CELL_SIZE-(MARGIN+1), yStart);
    glVertex2f(xStart + CELL_SIZE-(MARGIN+1), yStart + CELL_SIZE-(MARGIN+1));
    glVertex2f(xStart, yStart + CELL_SIZE-(MARGIN+1));
    glEnd();
}

void drawEmpty(int yStart, int xStart, int backClr, int dark)
{
    drawBackground(yStart, xStart, backClr, dark);

    yStart += MARGIN;
    xStart += MARGIN;

    if (backClr == 0){
        if (dark)
            glColor4ub(125, 125, 125, 255);
        else
            glColor4ub(255, 255, 255, 255);
    }
    else{
        if (dark)
            glColor4ub(125, 125, 125, 255);
        else
            glColor4ub(255, 255, 255, 255);
    }

    glBegin(GL_QUADS);
    glVertex2f(xStart, yStart);
    glVertex2f(xStart + CELL_SIZE-(MARGIN+1), yStart);
    glVertex2f(xStart + CELL_SIZE-(MARGIN+1), yStart + CELL_SIZE-(MARGIN+1));
    glVertex2f(xStart, yStart + CELL_SIZE-(MARGIN+1));
    glEnd();
}

void drawWall(int yStart, int xStart, int backClr, int dark)
{
    drawBackground(yStart, xStart, backClr, 0);

    yStart += MARGIN;
    xStart += MARGIN;

    glColor4ub(0, 0, 0, 255);

    glBegin(GL_QUADS);
    glVertex2f(xStart, yStart);
    glVertex2f(xStart + CELL_SIZE-(MARGIN+1), yStart);
    glVertex2f(xStart + CELL_SIZE-(MARGIN+1), yStart + CELL_SIZE-(MARGIN+1));
    glVertex2f(xStart, yStart + CELL_SIZE-(MARGIN+1));
    glEnd();
}

void drawGrnAgent(int yStart, int xStart, int backClr)
{
    drawBackground(yStart, xStart, backClr, 0);

    yStart += MARGIN;
    xStart += MARGIN;

    glColor4ub(0, 205, 0, 255);

    glBegin(GL_QUADS);
    glVertex2f(xStart, yStart);
    glVertex2f(xStart + CELL_SIZE-(MARGIN+1), yStart);
    glVertex2f(xStart + CELL_SIZE-(MARGIN+1), yStart + CELL_SIZE-(MARGIN+1));
    glVertex2f(xStart, yStart + CELL_SIZE-(MARGIN+1));
    glEnd();
}

void drawGrnAgent2(int yStart, int xStart, int backClr)
{
    drawBackground(yStart, xStart, backClr, 0);

    yStart += MARGIN;
    xStart += MARGIN;

    glColor4ub(0, 135, 0, 255);

    glBegin(GL_QUADS);
    glVertex2f(xStart, yStart);
    glVertex2f(xStart + CELL_SIZE-(MARGIN+1), yStart);
    glVertex2f(xStart + CELL_SIZE-(MARGIN+1), yStart + CELL_SIZE-(MARGIN+1));
    glVertex2f(xStart, yStart + CELL_SIZE-(MARGIN+1));
    glEnd();
}

void drawRedAgent(int yStart, int xStart, int backClr)
{
    drawBackground(yStart, xStart, backClr, 0);

    yStart += MARGIN;
    xStart += MARGIN;

    glColor4ub(205, 0, 0, 255);

    glBegin(GL_QUADS);
    glVertex2f(xStart, yStart);
    glVertex2f(xStart + CELL_SIZE-(MARGIN+1), yStart);
    glVertex2f(xStart + CELL_SIZE-(MARGIN+1), yStart + CELL_SIZE-(MARGIN+1));
    glVertex2f(xStart, yStart + CELL_SIZE-(MARGIN+1));
    glEnd();
}

void drawRedAgent2(int yStart, int xStart, int backClr)
{
    drawBackground(yStart, xStart, backClr, 0);

    yStart += MARGIN;
    xStart += MARGIN;

    glColor4ub(205, 0, 0, 255);

    glBegin(GL_QUADS);
    glVertex2f(xStart, yStart);
    glVertex2f(xStart + CELL_SIZE-(MARGIN+1), yStart);
    glVertex2f(xStart + CELL_SIZE-(MARGIN+1), yStart + CELL_SIZE-(MARGIN+1));
    glVertex2f(xStart, yStart + CELL_SIZE-(MARGIN+1));
    glEnd();
}

void drawBullet(int yStart, int xStart, int backClr)
{
    yStart += BULLET_MARGIN;
    xStart += BULLET_MARGIN;

    glColor4ub(rand() % 50, rand() % 50, (rand() % 155)+100, 255); 

    glBegin(GL_QUADS);
    glVertex2f(xStart, yStart);
    glVertex2f(xStart + CELL_SIZE-(BULLET_MARGIN*2), yStart);
    glVertex2f(xStart + CELL_SIZE-(BULLET_MARGIN*2), yStart + CELL_SIZE-(BULLET_MARGIN*2));
    glVertex2f(xStart, yStart + CELL_SIZE-(BULLET_MARGIN*2));
    glEnd();
}

void drawGrnZone(Env * env, int yStart, int xStart, int backClr, int dark)
{
    drawBackground(yStart, xStart, backClr, dark);

    yStart += MARGIN;
    xStart += MARGIN;

    if (env->flash[GRN] > 0)
        glColor4ub(0, 255, 0, 255);
    else
        glColor4ub(150, 255, 150, 255);

    env->flash[GRN] = env->flash[GRN] - 1;

    glBegin(GL_QUADS);
    glVertex2f(xStart, yStart);
    glVertex2f(xStart + CELL_SIZE-(MARGIN+1), yStart);
    glVertex2f(xStart + CELL_SIZE-(MARGIN+1), yStart + CELL_SIZE-(MARGIN+1));
    glVertex2f(xStart, yStart + CELL_SIZE-(MARGIN+1));
    glEnd();
}

void drawRedZone(Env * env, int yStart, int xStart, int backClr, int dark)
{
    drawBackground(yStart, xStart, backClr, dark);

    yStart += MARGIN;
    xStart += MARGIN;

    if (env->flash[RED] > 0)
        glColor4ub(255, 0, 0, 255);
    else
        glColor4ub(255, 150, 150, 255);

    env->flash[RED] = env->flash[RED] - 1;

    glBegin(GL_QUADS);
    glVertex2f(xStart, yStart);
    glVertex2f(xStart + CELL_SIZE-(MARGIN+1), yStart);
    glVertex2f(xStart + CELL_SIZE-(MARGIN+1), yStart + CELL_SIZE-(MARGIN+1));
    glVertex2f(xStart, yStart + CELL_SIZE-(MARGIN+1));
    glEnd();
}

void drawGrid()
{
    int xStart, yStart;
    int i, j;

    glColor4ub(50, 0, 50, 255);

    for (i=0; i<GRID_SIZE; i++){
        for (j=0; j<GRID_SIZE; j++){
            xStart = i * CELL_SIZE * BLOCK_SIZE;
            yStart = j * CELL_SIZE * BLOCK_SIZE;

            glBegin(GL_LINES);
            glVertex2f(xStart+1, yStart+1);
            glVertex2f(xStart + CELL_SIZE*BLOCK_SIZE-1, yStart+1);

            glVertex2f(xStart + CELL_SIZE*BLOCK_SIZE-1, yStart+1);
            glVertex2f(xStart + CELL_SIZE*BLOCK_SIZE-1, yStart + CELL_SIZE*BLOCK_SIZE-1);

            glVertex2f(xStart + CELL_SIZE*BLOCK_SIZE-1, yStart + CELL_SIZE*BLOCK_SIZE-1);
            glVertex2f(xStart+1, yStart + CELL_SIZE*BLOCK_SIZE-1);

            glVertex2f(xStart+1, yStart + CELL_SIZE*BLOCK_SIZE-1);
            glVertex2f(xStart+1, yStart+1);
            glEnd();
        }
    }
}

void drawBooms(Env * env)
{
    int xStart, yStart, i, j;

    for (i=0; i<5; i++){
        if (env->boom[i][0] > 0){
            if (env->view == 2 || env->blockStatus[env->teamShow][env->boom[i][1]][env->boom[i][2]] > 3 ||
                (env->view == 1 && env->blockStatus[env->teamShow][env->boom[i][1]][env->boom[i][2]] != UNEXPLORED)){
                for (j=0; j<(env->boom[i][0]*3); j++){
                    yStart = (env->boom[i][1] * (CELL_SIZE*BLOCK_SIZE)) + (env->boom[i][3] * CELL_SIZE) + rand() % CELL_SIZE-3;
                    xStart = (env->boom[i][2] * (CELL_SIZE*BLOCK_SIZE)) + (env->boom[i][4] * CELL_SIZE) + rand() % CELL_SIZE-3;
                    glColor4ub(rand() % 255, rand() % 255, rand() % 255, 255);
                    glBegin(GL_QUADS);
                    glVertex2f(xStart, yStart);
                    glVertex2f(xStart + 3, yStart);
                    glVertex2f(xStart + 3, yStart + 3);
                    glVertex2f(xStart, yStart + 3);
                    glEnd();
                }
            }
            env->boom[i][0] = env->boom[i][0] - 1;
        }
    }
}
