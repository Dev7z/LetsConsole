#include <iostream>
#include <ctime>
#include <unistd.h>
#include <cstdio>
#include <sys/ioctl.h>
#include <termios.h>
#include <vector>
#include <thread>
#include <future>

#include "ccolors.h"

using namespace std;

time_t launch = time(0);
time_t gameStart;

bool gameOver;
const int SCREEN_WIDTH = 64;
const int SCREEN_HEIGHT = 18;
int x, y, score;

int ticks = 0;
int interval = 10000;
int eInterval = 25;
int bInterval = 2;
int eShootProb = 3; // 1 in eShootProb chance of shooting

enum enemyTypes {NORMAL, KAMIKAZE};

enum eDirection {UP, DOWN, LEFT, RIGHT};
eDirection dir;
char dir2char[] = "^v<>";

Color::Modifier red(Color::FG_RED);
Color::Modifier green(Color::FG_GREEN);
Color::Modifier yellow(Color::FG_YELLOW);
Color::Modifier blue(Color::FG_BLUE);
Color::Modifier magenta(Color::FG_MAGENTA);
Color::Modifier cyan(Color::FG_CYAN);
Color::Modifier lightGray(Color::FG_LIGHTGRAY);
Color::Modifier darkGray(Color::FG_DARKGRAY);
Color::Modifier lightRed(Color::FG_LIGHTRED);
Color::Modifier lightGreen(Color::FG_LIGHTGREEN);
Color::Modifier lightYellow(Color::FG_LIGHTYELLOW);
Color::Modifier lightBlue(Color::FG_LIGHTBLUE);
Color::Modifier lightMagenta(Color::FG_LIGHTMAGENTA);
Color::Modifier lightCyan(Color::FG_LIGHTCYAN);
Color::Modifier def(Color::FG_DEFAULT);

Color::Modifier bgRed(Color::BG_RED);
Color::Modifier bgGreen(Color::BG_GREEN);
Color::Modifier bgYellow(Color::BG_YELLOW);
Color::Modifier bgBlue(Color::BG_BLUE);
Color::Modifier bgMagenta(Color::BG_MAGENTA);
Color::Modifier bgCyan(Color::BG_CYAN);
Color::Modifier bgLightGray(Color::BG_LIGHTGRAY);
Color::Modifier bgDarkGray(Color::BG_DARKGRAY);
Color::Modifier bgLightRed(Color::BG_LIGHTRED);
Color::Modifier bgLightGreen(Color::BG_LIGHTGREEN);
Color::Modifier bgLightYellow(Color::BG_LIGHTYELLOW);
Color::Modifier bgLightBlue(Color::BG_LIGHTBLUE);
Color::Modifier bgLightMagenta(Color::BG_LIGHTMAGENTA);
Color::Modifier bgLightCyan(Color::BG_LIGHTCYAN);
Color::Modifier bgDef(Color::BG_DEFAULT);

bool kbhit()
{
    termios term;
    tcgetattr(0, &term);

    termios term2 = term;
    term2.c_lflag &= ~ICANON;
    tcsetattr(0, TCSANOW, &term2);

    int byteswaiting;
    ioctl(0, FIONREAD, &byteswaiting);

    tcsetattr(0, TCSANOW, &term);

    return byteswaiting > 0;
}

char getch(void)
{
    char buf = 0;
    struct termios old = {0};
    fflush(stdout);
    if(tcgetattr(0, &old) < 0)
        perror("tcsetattr()");
    old.c_lflag &= ~ICANON;
    old.c_lflag &= ~ECHO;
    old.c_cc[VMIN] = 1;
    old.c_cc[VTIME] = 0;
    if(tcsetattr(0, TCSANOW, &old) < 0)
        perror("tcsetattr ICANON");
    if(read(0, &buf, 1) < 0)
        perror("read()");
    old.c_lflag |= ICANON;
    old.c_lflag |= ECHO;
    if(tcsetattr(0, TCSADRAIN, &old) < 0)
        perror("tcsetattr ~ICANON");
    return buf;
}

class bullet {
    public:
        bool ownedByPlayer;
        int x, y, bCooldown;
        eDirection bdir;
        bullet() {
            bCooldown = bCooldown;
            bdir = UP;
            x = 0;
            y = 0;
            ownedByPlayer = true;
        }
        bullet(eDirection dir, int bx, int by, bool isPlayer) {
            bdir = dir;
            x = bx;
            y = by;
            ownedByPlayer = isPlayer;
        }
};



class enemy {
    public:
        int ex, ey, eCooldown;
        eDirection eDir;
        enemyTypes enemyType;
        enemy() {
            ex = 0;
            ey = 0;
            eDir = UP;
            enemyType = NORMAL;
            eCooldown = 0;
        }
};

vector<bullet> bullets;
vector<enemy> enemies;

void cleanup() {
    for(auto it = bullets.begin(); it != bullets.end(); ) {
        if(it->x < 0 || it->x > SCREEN_WIDTH || it->y > SCREEN_HEIGHT || it->y < 0) {
            it = bullets.erase(it);
        } else {
            ++it;
        }
    }
}

void term() {
    enemies.clear();
    bullets.clear();
}

void setup() {
    gameStart = time(0);
    srand(time(0));
    gameOver = false;
    ticks++;
    dir = UP;
    x = SCREEN_WIDTH / 2;
    y = SCREEN_HEIGHT / 2;

    for (int i = 0; i < 10; i++) {
        enemies.push_back(enemy());
    }
    for (auto& e : enemies) {
        int nsex = rand() % SCREEN_WIDTH;
        int nsey = rand() % SCREEN_HEIGHT;

        while(abs(nsex - x) > 3  && abs(nsey - y) > 3) {
            nsex = rand() % SCREEN_WIDTH;
            nsey = rand() % SCREEN_HEIGHT;
        }
        e.ex = nsex;
        e.ey = nsey;
        e.eDir = UP;
        e.enemyType = static_cast<enemyTypes>(rand() % 2);
    }
    score = 0;
}

void draw() {
    cout << "\033[2J\033[1;1H";
    for (int i = 0; i < SCREEN_WIDTH+2; i++) {
        cout << '#';
    }
    cout << '\n';

    for (int i = 0; i < SCREEN_HEIGHT; i++) {
        for (int j = 0; j < SCREEN_WIDTH+2; j++) {
            if(j == 0 || j == SCREEN_WIDTH+1) {
                cout << '#';
            }


            else if(i == y && j == x) {
                if(difftime( time(0), gameStart) <= 5) {
                    cout << dir2char[dir];
                }
                else {
                    cout << blue << dir2char[dir] << def;
                }
            }
            else {
                //Enemy
                bool EBdrawn = false;
                for (const auto& e : enemies) {
                    if (i == e.ey && j == e.ex) {
                        if(e.enemyType == KAMIKAZE) {
                            cout << red << 'K' << def;
                            EBdrawn = true;
                            break;
                        }
                        //else if (e.enemyType == NORMAL) {
                            cout << lightRed << dir2char[e.eDir] << def;
                            EBdrawn = true;
                            break;
                        //}
                    }
                }

                //Bullet
                for (const auto& b : bullets) {
                    if (i == b.y && j == b.x) {
                        if(b.ownedByPlayer) {
                            cout << 'O';
                            EBdrawn = true;
                            break;
                        }
                        else{ //if (b.owned by future shit idk){
                            cout << yellow << 'O' << def;
                            EBdrawn = true;
                            break;
                        }
                    }
                }
                if (!EBdrawn) {
                    cout << ' ';
                }
            }
        }
        cout << '\n';
    }

    for (int i = 0; i < SCREEN_WIDTH+2; i++) {
        cout << '#';
    }
    cout << '\n' << "Score: " << score << endl;
}

void input() {
    if(kbhit()) {
        bool notTouchingEnemy = true;
        switch (getch()) {
            case 'u':
                dir = UP;
                break;
            case 'k':
                dir = RIGHT;
                break;
            case 'j':
                dir = DOWN;
                break;
            case 'h':
                dir = LEFT;
                break;

            case 'w':
                for (auto & e : enemies) {
                    if(y-1 == e.ey && x == e.ex) {
                        notTouchingEnemy = false;
                    }
                }
                if (notTouchingEnemy) {
                    if(y <= 0) {
                        y = SCREEN_HEIGHT-1;
                    }
                    else {
                        y--;
                    }
                }
                break;
            case 'd':
                for(auto & e : enemies) {
                    if(y == e.ey && x+1 == e.ex) {
                        notTouchingEnemy = false;
                    }
                }
                if (notTouchingEnemy) {
                    if(x >= SCREEN_WIDTH) {
                        x = 1;
                    }
                    else {
                        x++;
                    }
                }
                break;
            case 's':
                for(auto & e: enemies) {
                    if(y+1 == e.ey && x == e.ex) {
                        notTouchingEnemy = false;
                    }
                }
                if (notTouchingEnemy) {
                    if(y >= SCREEN_HEIGHT-1) {
                        y = 0;
                    }
                    else {
                        y++;
                    }
                }

                break;
            case 'a':
                for(auto & e : enemies) {
                    if(y == e.ey && x-1 == e.ex) {
                        notTouchingEnemy = false;
                        break;
                    }
                }
                if (notTouchingEnemy) {
                    if(x <= 1) {
                        x = SCREEN_WIDTH;
                    }
                    else {
                        x--;
                    }
                }
                break;

            case ' ':
                bullets.push_back(bullet(dir, x, y, true));
                break;


            case 27: //etc
                gameOver = true;
                break;

            default:
                break;
        }
    }
}

void fbullet() {
    for (auto& b : bullets) {
        if(b.bCooldown < bInterval ) {
            b.bCooldown++;
        }
        else {
            if(b.ownedByPlayer) {
                for (auto & e : enemies) {
                    if((b.x == e.ex || b.x == e.ex+1 || b.x == e.ex-1) && (b.y == e.ey || b.y == e.ey+1 || b.y == e.ey-1)) {
                        score++;

                        int nsex = rand() % SCREEN_WIDTH;
                        int nsey = rand() % SCREEN_HEIGHT;

                        while(abs(nsex - x) > 3  && abs(nsey - y) > 3) {
                            nsex = rand() % SCREEN_WIDTH;
                            nsey = rand() % SCREEN_HEIGHT;
                        }
                        e.ex = nsex;
                        e.ey = nsey;
                        e.enemyType = static_cast<enemyTypes>(rand() % 2);
                    }
                }
            }

            if(b.x == x && b.y == y && !b.ownedByPlayer && difftime( time(0), gameStart) > 5) {
                gameOver = true;
            }

            switch (b.bdir) {
                case UP:
                    b.y--;
                    break;
                case RIGHT:
                    b.x++;
                    break;
                case DOWN:
                    b.y++;
                    break;
                case LEFT:
                    b.x--;
                    break;
            }
            b.bCooldown = 0;
        }
    }
}

void enemy() {
    for (auto & e : enemies) {
        if(e.eCooldown < eInterval ) {
            e.eCooldown++;
        }
        else {
            e.eDir = static_cast<eDirection>(rand() % 4);
            if(e.enemyType == NORMAL) {
                if(rand() % eShootProb == 0) {
                    bullets.push_back(bullet(e.eDir, e.ex, e.ey, false));
                }
                switch(e.eDir) {
                    case UP:
                        e.ey--;
                        break;
                    case RIGHT:
                        e.ex++;
                        break;
                    case DOWN:
                        e.ey++;
                        break;
                    case LEFT:
                        e.ex--;
                        break;
                }
            }
            else if(e.enemyType == KAMIKAZE) {
                if (e.ex > x) {
                    e.eDir = LEFT;
                    e.ex--;
                }
                else if (e.ex < x) {
                    e.eDir = RIGHT;
                    e.ex++;
                }
                if (e.ey > y) {
                    e.eDir = UP;
                    e.ey--;
                }
                else if (e.ey < y) {
                    e.eDir = DOWN;
                    e.ey++;
                }
            }
            e.eCooldown = 0;
            if(e.ex == x && e.ey == y && difftime( time(0), gameStart) > 5) {
                gameOver = true;
            }
        }
    }
}

int main() {
    cout << "Hello, World!" << endl;
    setup();

    while (!gameOver) {
        input();
        draw();
        fbullet();
        enemy();
        usleep(interval);
        cleanup();
    }
    cout << endl << "Game Over, not so easy right?" << endl;
    term();
    return 0;
}

/*
 *  TODO:
 *  sidebar with score & info
 *  turn gameOver into pause menu + proper gameOver screen
 *
 *  windows build/code, should be an easy port
 */
