#include "invader.hpp"
#include "assets.hpp"


#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 240
#define BONUS 5000


using namespace blit;

std::string message[]{
    "10 Points",
    "20 Points",
    "30 Points",
    "100 Points",
    "every 5000 Points"
};

Font font(font8x8);

struct Player 
{
    int type;
    int ani;
    int x;
    int y;
    bool shot;
    int shot_x;
    int shot_y;
    int live;
    int score;
    int bonus;
    int wave;
    int dT;
    float deadTimer;
};

struct Invader
{
    int x;
    int y;
    int dT;
    int ani;
    int left;
    int right;
    int bottom;
    int dx;
    int pic[9][5];
    int sum;
};

struct Alien_Shot
{
    int row[9];
    int x[9];
    int y[9];
    bool is;
};

struct Mother_Ship
{
    int x;
    int dx;
    int dt;
    int dT;
    int sT;
    int freq;
    int d_freq;
};

struct Explosion
{
    int x;
    int y;
    int type;
    int dT;
    int dS;
};

Player p;
Invader inv;
Alien_Shot bomb;
Mother_Ship ufo;
Explosion ex;

bool shield;
int wall[4][6][3];
int state;
int dT;
int ani;
uint32_t lastTime = 0;
int message_counter = 0;
int HighScore = 0;

void new_ufo()
{
    if (rand() % 2 > 0)
    {
        ufo.x = -16;
        ufo.dx = 1;
    }
    else 
    {
        ufo.x = 240;
        ufo.dx = -1;
    }
    ufo.dT = 0;
    ufo.sT = 500 + rand() % 1000;
    ufo.freq = 1000;
    ufo.d_freq = 80;
}

void new_wave()
{
    if (p.wave < 6)
    {
        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 6; j++)
            {
                for (int k = 0; k < 3; k++)
                {
                    wall[i][j][k] = 1;
                }
            }
            wall[i][2][2] = 0;
            wall[i][3][2] = 0;
        }
        shield = true;
    }
    else
    {
        shield = false;
    }

    inv.x = 0;
    inv.y = (p.wave * 16) + 32;
    if (inv.y > 144)
    {
        inv.y = 144;
    }
    inv.left = 0;
    inv.right = 8; //176;
    inv.bottom = 4; //60;
    inv.dx = 4;
    inv.ani = 0;
    inv.sum = 45;
    for (int i = 0; i < 9; i++)
    {
        inv.pic[i][0] = 10;
        inv.pic[i][1] = 6;
        inv.pic[i][2] = 6;
        inv.pic[i][3] = 2;
        inv.pic[i][4] = 2;

        bomb.row[i] = 5;
    }

    new_ufo();
}

void start_game() 
{
    p.type = 0;
    p.ani = 0;
    p.live = 2;
    p.x = 220 - (p.live * 15);
    p.shot = false;
    p.deadTimer = 0;
    p.score = 0;
    p.bonus = 0;
    p.wave = 0;

    new_wave();
}

void add_score(int pts)
{
    p.score += pts;
    if (p.score - p.bonus >= BONUS)
    {
        p.bonus += BONUS;
        p.live++;    
    }
}


//
// init()
//
// setup your game here
//

void init() 
{
    set_screen_mode(ScreenMode::lores);
    screen.sprites = Surface::load(asset_sprites);

    // Alien move
    channels[0].waveforms = Waveform::NOISE; 
    channels[0].frequency = 1000;
    channels[0].attack_ms = 5;
    channels[0].decay_ms = 50;
    channels[0].sustain = 0;
    channels[0].release_ms = 5;

    // Shot
    channels[1].waveforms = Waveform::SQUARE; 
    channels[1].frequency = 4000;
    channels[1].attack_ms = 10;
    channels[1].decay_ms = 200;
    channels[1].sustain = 0;
    channels[1].release_ms = 100;

    // Alien hit
    channels[2].waveforms = Waveform::SQUARE;
    channels[2].frequency = 0;
    channels[2].attack_ms = 10;
    channels[2].decay_ms = 200;
    channels[2].sustain = 0;
    channels[2].release_ms = 10; 

    // Ufo move
    channels[3].waveforms = Waveform::SQUARE;
    channels[3].frequency = 0;
    channels[3].attack_ms = 5;
    channels[3].decay_ms = 100;
    channels[3].sustain = 0;
    channels[3].release_ms = 5; 

    // Base destroy
    channels[4].waveforms = Waveform::SQUARE;
    channels[4].frequency = 0;
    channels[4].attack_ms = 5;
    channels[4].decay_ms = 400;
    channels[4].sustain = 0;
    channels[4].release_ms = 5;   

    // Bonus Base
    channels[5].waveforms = Waveform::SQUARE;
    channels[5].frequency = 0;
    channels[5].attack_ms = 5;
    channels[5].decay_ms = 100;
    channels[5].sustain = 0;
    channels[5].release_ms = 5; 

    if (read_save(HighScore))
    {
    }
    else 
    {
        HighScore = 0;
    }
}


//
// render(time)
//
// This function is called to perform rendering of the game. time is the 
// amount if milliseconds elapsed since the start of your game
//

void render(uint32_t time) 
{

    // clear the screen -- screen is a reference to the frame buffer and can be used to draw all things with the 32blit
    screen.clear();
    screen.alpha = 255;
    screen.mask = nullptr;

    if (state == 0) 
    {
        screen.pen = Pen(75, 75, 75);
        screen.rectangle(Rect(0, 0, 120, 16));
        screen.rectangle(Rect(0, 112, 120, 8));

        screen.text(message[message_counter], minimal_font, Point(59,91), true, TextAlign::center_center);

        screen.pen = Pen(0, 0, 0);
        screen.text("SCORE: " + std::to_string(p.score), minimal_font, Point(60, 1), true, TextAlign::top_center);
        screen.text("HIGH SCORE: " + std::to_string(HighScore), minimal_font, Point(60, 9), true, TextAlign::top_center);
        screen.text("PRESS A TO START", minimal_font, Point(60, 113), true, TextAlign::top_center);

        screen.pen = Pen(255, 255, 255);
        screen.text("SCORE: " + std::to_string(p.score), minimal_font, Point(59, 0), true, TextAlign::top_center);
        screen.text("HIGH SCORE: " + std::to_string(HighScore), minimal_font, Point(59, 8), true, TextAlign::top_center);
        screen.text(message[message_counter], minimal_font, Point(59,90), true, TextAlign::center_center);
        screen.text("PRESS A TO START", minimal_font, Point(59, 112), true, TextAlign::top_center);

        screen.sprite(Rect(0, 6, 12, 5), Point(11, 31));
        screen.sprite(message_counter * 4, Point(51, 77));
        screen.sprite(message_counter * 4 + 1, Point(59, 77));        
    }
    else if (state == 1 || state == 2) 
    {

        screen.pen = Pen(255, 255, 255);
        screen.text("SCORE: " + std::to_string(p.score), font, Point(6, 227));

        if (shield)
        {
            for (int i = 0; i < 4; i ++)
            {
                screen.sprite(Rect(0, 3, 3, 2), Point(24 + (i * 55), 172));
                for (int j = 0; j < 6; j++)
                {
                    for (int k = 0; k < 3; k++)
                    {
                        if (wall[i][j][k] > 1)
                        {
                            screen.sprite(34 + wall[i][j][k], Point(22 + (i * 55) + (j * 4), 171 + (k * 6)));
                        }                   
                    }
                }
            }
        }


        for (int i = 0; i < 9; i++)
        { 
            for (int j = 0; j < 5; j++)
            {
                if (inv.pic[i][j] > 0)
                {
                    screen.sprite(inv.pic[i][j] - inv.ani, Point(inv.x + (i * 20), inv.y + (j * 12)));
                    screen.sprite(inv.pic[i][j] - inv.ani + 1, Point(inv.x + (i * 20) + 8, inv.y + (j * 12)));
                }
            }
            if (bomb.y[i] > 0)
            {
                screen.sprite(32 + ani, Point(bomb.x[i], bomb.y[i]));
            }
        }

        if (ufo.dT > ufo.sT)
        {
            screen.sprite(12, Point(ufo.x, 8));
            screen.sprite(13, Point(ufo.x + 8, 8));
        }

        if (p.shot)
        {
            screen.pen = Pen(255, 255, 255);
            screen.line(Point(p.shot_x, p.shot_y), Point(p.shot_x, p.shot_y + 5));
        }

        if (ex.dT > 0)
        {
            ex.dT--;
            screen.sprite(ex.type, Point(ex.x, ex.y));
            screen.sprite(ex.type + 1, Point(ex.x + 8, ex.y));

            ex.dS++;
            if (ex.dS > 8)
            { 
                ex.dS = 0;
                channels[2].trigger_attack();
            }
            channels[2].frequency = ex.dS * 200;
        }
        
        screen.sprite(16 + p.ani, Point(p.x, 210));
        screen.sprite(17 + p.ani, Point(p.x + 8, 210));
        
        for (int i = 0; i < p.live; i++)
        {
            screen.sprite(16, Point(220 - (i * 15), 227));
            screen.sprite(17, Point(228 - (i * 15), 227));            
        }
        
        screen.pen = Pen(255, 0, 255);
        screen.line(Point(0, 219), Point(SCREEN_WIDTH, 219));
    }
    screen.pen = Pen(0, 0, 0);
}


//
// update(time)
//
// This is called to update your game state. time is the 
// amount if milliseconds elapsed since the start of your game
//

void update(uint32_t time) 
{
    if (state == 0)
    {
        if (buttons & Button::A)
        {
            set_screen_mode(ScreenMode::hires);
            screen.sprites = Surface::load(asset_sprites);

            state = 1;
            start_game();
            dT = 0;
        }

        dT++;
        if (dT > 250)
        {
            dT = 0;
            message_counter++;
            if (message_counter > 4)
            {
                message_counter = 0;
            }
        } 
    }
    else 
    {
        if (state == 1)
        {
            inv.dT++;
            if (inv.dT > 50)
            {
                inv.dT = 0;
                inv.x = 0;
                inv.dx--;
                if (inv.dx < 0)
                {
                    inv.dx = 4;
                    state = 2;
                }
            }
            else if (inv.dT == 25)
            {
                inv.x = 320;
            }
        }
        else if (state == 2)
        {
            if (inv.sum > 0)
            {
                inv.dT++;
                if (inv.dT > 4 + (inv.sum * .6))
                {
                    inv.dT = 0;
        
                    inv.ani += 2;
                    if (inv.ani > 2)
                    {
                        inv.ani = 0;
                    }
        
                    if (p.type == 0)
                    {
                        inv.x += inv.dx;
                        if (inv.x + (inv.left * 20) < 0 || inv.x + (inv.right * 20 +16) > SCREEN_WIDTH + 2)
                        {
                            inv.dx = -inv.dx;
                            inv.x += inv.dx;
                            inv.y += 8;
                            if (inv.y + (inv.bottom * 12) > 202)
                            {
                                p.live = 0;
                                p.type = 1;
                            }
                            else if (inv.y + (inv.bottom * 12) > 162)
                            {
                                shield = false;
                            }
                        }
                    
                        if (std::rand() % 9 == 0)
                        {
                            int i = inv.left + std::rand() %(inv.right - inv.left + 1);
                            while (bomb.row[i] == 0)
                            {
                                i++;
                            }
                            if (bomb.y[i] == 0)
                            {
                                bomb.x[i] = inv.x + (i * 20) + 5;
                                bomb.y[i] = inv.y + (bomb.row[i] * 12) - 4;
                            }
                        }
                    }
                }
            }
            else if ( ex.dT == 0 && p.shot == false && bomb.is == false && ufo.sT > ufo.dT) //next Wave
            {
                p.wave++;
                new_wave();
                state = 1;
            }

            if (p.shot)
            {
                p.shot_y -= 2; 
                if (p.shot_y < 0)
                {
                    p.shot = false;
                }
                else if (shield && p.shot_y < 188 && p.shot_y > 172)
                {
                    for (int i = 0; i < 4; i ++)
                    {
                        if (p.shot_x < 48 + (i * 55) && p.shot_x > 24 + (i * 55))
                        {
                            int xx = (p.shot_x - (24 + (i * 55))) * .25;
                            int yy = (p.shot_y - 173) * .2;
                            if (wall[i][xx][yy] == 1)
                            {
                                wall[i][xx][yy] = 2 + rand() % 3;
                                p.shot = false;
                                break;
                            }
                        }
                    }
                }
                else
                {
                    for (int i = 0; i < 9; i++)
                    { 
                        for (int j = 0; j < 5; j++)
                        {
                            if (inv.pic[i][j] > 0)
                            {
                                if (p.shot_x < (i * 20 + 13 + inv.x) && p.shot_x > (i * 20 + inv.x) && p.shot_y < (j * 12 + 8 + inv.y) && p.shot_y +3 > (j * 12 + inv.y))
                                {
                                    p.shot = false;
                                    add_score((inv.pic[i][j] + 2) * 2.5);
                                    inv.pic[i][j] = 0;
        
                                    ex.x = inv.x + (i * 20);
                                    ex.y = inv.y + (j * 12);
                                    ex.type = 39;
                                    ex.dT = 8;
                                    ex.dS = 0;

                                    if (ufo.dT < ufo.sT)
                                    {
                                        channels[2].trigger_attack();
                                    }
        
                                    inv.sum--;
                                    if (inv.sum > 0)
                                    {
                                        while (inv.pic[i][bomb.row[i] - 1] == 0 && bomb.row[i] > 0)
                                        {
                                            bomb.row[i]--;
                                        }
                        
                                        for (int k = inv.left; k < inv.right + 1; k++)
                                        {
                                            int sum = 0;
                                            for (int l = 0; l < inv.bottom + 1; l++)
                                            {
                                                sum += inv.pic[k][l];
                                            }
                                            if (sum > 0)
                                            {
                                                inv.left = k;
                                                break;
                                            }
                                        }
                                        for (int k = inv.right; k > inv.left - 1; k--)
                                        {
                                            int sum = 0;
                                            for (int l = 0; l < 5; l++)
                                            {
                                                sum += inv.pic[k][l];
                                            }
                                            if (sum > 0)
                                            {
                                                inv.right = k;
                                                break;
                                            }
                                        }
                                        for (int k = inv.bottom; k > -1; k--)
                                        {
                                            int sum = 0;
                                            for (int l = inv.left; l < inv.right + 1; l++)
                                            {
                                                sum += inv.pic[l][k];
                                            }
                                            if (sum > 0)
                                            {
                                                inv.bottom = k;
                                                break;
                                            }
                                        }
        
                                        break;
                                    }
                                    else
                                    {
     
                                    }
                                }
                            }
                        }
                        if (p.shot == false)
                        {
                            break;
                        }
                    }
                }
            }
        }
        
        ufo.dT++;
        if (ufo.dT > ufo.sT)
        {
            ufo.dt++;
            if (ufo.dt > 1)
            {
                ufo.dt = 0;
                ufo.x += ufo.dx;

                ufo.freq += ufo.d_freq;
                if (ufo.freq < 400 || ufo.freq > 1200)
                {
                    ufo.d_freq=-ufo.d_freq;
                }

                channels[3].frequency = ufo.freq;
                if (p.type == 0)
                {
                    channels[3].trigger_attack();
                }

                if (ufo.x < -16 || ufo.x > 240)
                {
                    new_ufo();
                }
            }
            if (p.shot && p.shot_x < ufo.x + 16 && p.shot_x > ufo.x && p.shot_y < 16 && p.shot_y > 4)
            {
                ex.x = ufo.x;
                ex.y = 8;
                ex.type = 14;
                ex.dT = 24;
                ex.dS = 0;

                channels[2].trigger_attack();

                new_ufo();
                p.shot = false;
                add_score(100);
            }
        }

        bomb.is = false;
        for (int i = 0; i < 9; i++)
        { 
            if (bomb.y[i] > 0)
            {
                bomb.is = true;
                bomb.y[i]++;
                if (bomb.y[i] > 212)
                {   
                    bomb.y[i] = 0;
                }
                else if (bomb.y[i] < 218 && bomb.y[i] > 204 && bomb.x[i] < p.x + 12 && bomb.x[i] + 3 > p.x)
                {
                    bomb.y[i] = 0;
                    p.type = 1;
                }
                else if (shield && bomb.y[i] < 181 && bomb.y[i] > 165)
                {
                    for (int j = 0; j < 4; j ++)
                    {
                        if (bomb.x[i] < 48 + (j * 55) && bomb.x[i] > 20 + (j * 55))
                        {
                            int xx = (bomb.x[i] - 22 - (j * 55)) * .25;
                            int yy = (bomb.y[i] - 166) * .2;
                            if (wall[j][xx][yy] == 1)
                            {
                                wall[j][xx][yy] = 2 + rand() % 3;
                                bomb.y[i] = 0;
                                break;
                            }
                        }
                    }
                }
                else if (p.shot && p.shot_x < bomb.x[i] + 4 && p.shot_x > bomb.x[i] && p.shot_y < bomb.y[i] + 6 && p.shot_y + 6 > bomb.y[i])
                {
                    ex.x = bomb.x[i] - 1;
                    ex.y = bomb.y[i] - 1;
                    ex.type = 41;
                    ex.dT = 8;

                    bomb.y[i] = 0;
                    p.shot = false;
                    add_score(5);
                }
            }
        }

        ani++;
        if (ani > 3)
        {
            ani = 0;
        }

        if (p.type == 0)
        {
            if (p.shot == false && buttons & Button::A && state == 2)
            {
                p.shot = true;
                p.shot_x = p.x + 6;
                p.shot_y = 206;

                if (ufo.dT < ufo.sT)
                {
                    channels[1].trigger_attack();
                }
            }
            if (buttons & Button::DPAD_RIGHT && p.x < SCREEN_WIDTH - 13)
            {
                p.x++;
            }
            if (buttons & Button::DPAD_LEFT && p.x > 0)
            {
                p.x--;
            }
        }
        else if (p.type == 1)
        {
            p.ani = 2 * (rand() % 3) + 2;
            p.dT++;

            if (p.dT < 100)
            {
                channels[4].frequency = 1600 - (p.dT * 15);
                channels[4].trigger_attack();
            }
            else if (p.dT > 200)
            {
                p.dT = 0;
                if (p.live == 0)
                {
                    if (p.score > HighScore)
                    {
                        HighScore = p.score;
                        write_save(HighScore);
                    }
                    set_screen_mode(ScreenMode::lores);
                    screen.sprites = Surface::load(asset_sprites);
                    state = 0;
                }
                else
                {
                    p.live--;
                    p.x = 220 - (p.live * 15);
                    p.type = 0;
                    p.ani = 0;
                }
            }
        }
    }
}

