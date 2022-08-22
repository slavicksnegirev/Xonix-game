//
//  main.cpp
//  Прогр. на ЯВУ
//
//  Created by Слава on 04.06.2021.
//
  
#include <map>
#include <math.h>
#include <string>
#include <fstream>
#include <iostream>
#include <SDL2/SDL.h>
#include <unordered_map>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
 
using namespace std;

/*=============================================================================================================================*/
/*=============================================================================================================================*/

const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;

SDL_Rect Rect;
SDL_Texture *texture;
TTF_Font *font = NULL;
SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;

/*=============================================================================================================================*/
/*=============================================================================================================================*/

class manager_input
{
private:
    SDL_Point mouse_coords; // положение мыши
    unordered_map<unsigned int, bool> key_map;
    unordered_map<unsigned int, bool> prev_key_map;
    
public:
    bool exit;
    
    manager_input();
    ~manager_input();

    void update(SDL_Event);
    void press_key(unsigned int);
    void release_key(unsigned int);
    void set_mouse_coords(int, int);

    bool is_key_down(unsigned int);    // возвращает истину, если клавиша удерживается
    bool is_key_pressed(unsigned int); // возвращает истину, если клавиша была просто нажата
    bool was_key_down(unsigned int);   // возвращает истину, если клавиша удерживается
};

/*=============================================================================================================================*/

manager_input::manager_input() : mouse_coords({ 0,0 })
{
    exit = false;
}
 
manager_input::~manager_input()
{
    key_map.clear();
    prev_key_map.clear();
}

void manager_input::update(SDL_Event event)
{
    for (auto &it : key_map)
    {
        prev_key_map[it.first] = it.second;
    }

    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            case SDL_QUIT:
                exit = true;
            
                break;

            case SDL_KEYDOWN:
                press_key(event.key.keysym.sym);
            
                break;

            case SDL_KEYUP:
                release_key(event.key.keysym.sym);
            
                break;

            case SDL_MOUSEMOTION:
                set_mouse_coords(event.motion.x, event.motion.y);
            
                break;

            case SDL_MOUSEBUTTONDOWN:
                press_key(event.button.button);
            
                break;

            case SDL_MOUSEBUTTONUP:
                release_key(event.button.button);
            
                break;
        }
    }
}

void manager_input::press_key(unsigned int key_ID)
{
    key_map[key_ID] = true;
}

void manager_input::release_key(unsigned int keyID)
{
    key_map[keyID] = false;
}

void manager_input::set_mouse_coords(int x, int y)
{
    mouse_coords.x = x;
    mouse_coords.y = y;
}

bool manager_input::is_key_down(unsigned int key_ID)
{
    auto it = key_map.find(key_ID);
    return it != key_map.end() ? it->second : false;
}

bool manager_input::is_key_pressed(unsigned int key_ID)
{
    if (is_key_down(key_ID) == true && was_key_down(key_ID) == false)
        return true;
    return false;
}

bool manager_input::was_key_down(unsigned int key_ID)
{
    auto it = prev_key_map.find(key_ID);
    return it != prev_key_map.end() ? it->second : false;
}

/*=============================================================================================================================*/
/*=============================================================================================================================*/

class manager_texture : public manager_input
{
private:
    int width;
    int height;
    
    SDL_Renderer *renderer;
    SDL_Texture *load_texture(string);
    map <string, SDL_Texture*> texture_map;
    
public:
    manager_texture();
    ~manager_texture();
    
    SDL_Texture *get_texture(string);
    void set_render(SDL_Renderer*);
    bool print_text(int, int, string, int, int, int);
};

/*=============================================================================================================================*/

manager_texture::manager_texture() {}

manager_texture::~manager_texture()
{
    for (map <string, SDL_Texture*> ::iterator i = texture_map.begin(); i != texture_map.end(); ++i)
    {
        SDL_DestroyTexture(i->second);
    }
    texture_map.clear();
}

void manager_texture::set_render(SDL_Renderer *renderer)
{
    this->renderer = renderer;
}

SDL_Texture* manager_texture::load_texture(string name)

{
    SDL_Surface *image = IMG_Load(name.c_str());
    SDL_Texture *new_texture = SDL_CreateTextureFromSurface(renderer, image);

    if (!image)
        printf("Не получается загрузить изображение.\n");
    else
    {
        if (!new_texture)
            printf("Не получается создать текстуру.\n");
        else
        {
            SDL_FreeSurface(image);
            return new_texture;
        }
    }
    return NULL;
}

SDL_Texture *manager_texture::get_texture(string name)
{
    map <string, SDL_Texture*> ::iterator it = texture_map.find(name);

    if (it == texture_map.end())
    {
        SDL_Texture *new_texture = load_texture(name);
        texture_map.insert(make_pair(name, new_texture));
        return new_texture;
    }

    return it->second;
}

bool manager_texture::print_text(int x, int y, string text_str, int r, int g, int b)//вывод текста
{
    if (texture != NULL)
    {
        SDL_DestroyTexture(texture);
        texture = NULL;
        width = 0;
        height = 0;
    }
    
    SDL_Color text_color = {static_cast<Uint8>(r), static_cast<Uint8>(g), static_cast<Uint8>(b)};
      
    SDL_Surface *text_surface = TTF_RenderText_Solid(font, text_str.c_str(), text_color);
    if (text_surface == NULL)
    {
        printf("Невозможно создать текстуру текста! SDL_ttf Error: %s\n", TTF_GetError());
    }
    else
    {
        texture = SDL_CreateTextureFromSurface(renderer, text_surface); // создание текстуры
        if (texture == NULL)
        {
            printf("Невозможно создать текстуру из данного текста! SDL Error: %s\n", SDL_GetError());
        }
        else
        {
            width = text_surface->w;
            height = text_surface->h;
            SDL_Rect renderQuad = { x, y, width, height };
            SDL_RenderCopy(renderer, texture, NULL, &renderQuad);
        }
        SDL_FreeSurface(text_surface);
    }
    return texture != NULL;
}

/*=============================================================================================================================*/
/*=============================================================================================================================*/

class SDL_Sprite
{
private:
    float           angel;
     
    SDL_Rect         rect;
    SDL_Color       color;
    SDL_FPoint      scale;
    SDL_FPoint     origin;
    SDL_FPoint   position;
    SDL_Texture  *texture;
    
    void init();
    
public:
    SDL_Sprite();
    ~SDL_Sprite();

    void draw(SDL_Renderer*);
    void rotate(float);
    
    /* сеттеры */
    void set_position(const float&, const float&);
    void set_origin(const float&, const float&);
    void set_texture_rect(SDL_Rect);
    void set_texture(SDL_Texture*);
};

SDL_Sprite::SDL_Sprite() {}
SDL_Sprite::~SDL_Sprite() {}

void SDL_Sprite::init()
{
    position = {0,0};
    scale = {1.0, 1.0};
    origin = {0.0, 0.0};
    color = {255, 255, 255, 255};
    angel = rect.x = rect.y = 0;
}

void SDL_Sprite::draw(SDL_Renderer *renderer)
{
    SDL_FPoint centre = origin;
    centre.x *= scale.x;
    centre.y *= scale.y;
    
    SDL_FRect rect_window
    {
        position.x - centre.x,
        position.y - centre.y,
        rect.w * scale.x,
        rect.h * scale.y
    };

    SDL_SetTextureColorMod(texture, color.r, color.g, color.b);
    SDL_SetTextureAlphaMod(texture, color.a);
    SDL_RenderCopyExF(renderer, texture, &rect, &rect_window, angel, &centre, SDL_FLIP_NONE);
}

void SDL_Sprite::rotate(float angel)
{
    this->angel += angel;
}

void SDL_Sprite::set_position(const float &x, const float &y)
{
    position = {x,y};
}

void SDL_Sprite::set_origin(const float &x, const float &y)
{
    origin = {x,y};
}

void SDL_Sprite::set_texture_rect(SDL_Rect rect)
{
    this->rect = rect;
}
 
void SDL_Sprite::set_texture(SDL_Texture *texture)
{
    this->texture = texture;
    SDL_QueryTexture(texture, NULL,NULL, &rect.w, &rect.h);
    init();
}

/*=============================================================================================================================*/
/*=============================================================================================================================*/

class game : public SDL_Sprite
{
private:
    static const int M = 40;
    static const int N = 60;
    
    int width;
    int height;
    
    int kol = 10;
    int area = 0;
    int lives = 3;
    int level = 1;
    int flag = false;
    int tile_size = 18;
    int enemy_count = 1;
    int grid[M][N] = {0};
    int square_w = SCREEN_WIDTH-30;
    int x = 0, y = 0, dx = 0, dy = 0;
     
    long score = 0;
    long record = 0;
    bool games = true;
    bool pause = false;
    bool game_loop = true;
    float timer = 0, delay = 0.1;
    
    string name = "|";
    //int choise = 1 + rand() % 5;

    struct enemy
    {
        int x, y, dx, dy;

        enemy()
        {
            x = 18 + rand() % (SCREEN_WIDTH-216);
            y = 18 + rand() % (SCREEN_HEIGHT-36);
            
            if ((1 + rand() % 2) == 1)
                dx = 3;
            else
                dx = -3;
            if ((1 + rand() % 2) == 1)
                dy = 3;
            else
                dy = -3;
        }

        void move()
        {
            x += dx;
            y += dy;
        }
    };
    
    struct player
    {
        long player_score;
        string player_name;
    };
    typedef struct player player;
    
    SDL_Event event;
    SDL_Sprite foe, tile, game_over, hearts, square, start, arrow, tiles;
    
    enemy a[100];
    player results[10];
    manager_input manager_input;
    manager_texture manager_texture;

    void game_render();
    void game_update();

public:
    game();
    ~game();
    
    void file_reading();
    void file_writing();
    void print();
    void records (int, int);
    void author(int, int);
    void create(int, int);
    void rules(int, int);
    void drop(int, int);
    void play();
};

/*=============================================================================================================================*/

game::game()
{
    for (int i = 0; i < kol; i++)
    {
        results[i].player_score = 0;
        results[i].player_name = "NoName";
    }
}

game::~game()
{
    if (renderer)
        SDL_DestroyRenderer(renderer);
    renderer = NULL;

    if (window)
        SDL_DestroyWindow(window);
    window = NULL;

    SDL_Quit();
}

void game::file_reading()
{
    int i = 0;
    ifstream file;
    file.open("/Users/slavick.snegirevicloud.com/Desktop/Прогр. на ЯВУ/Прогр. на ЯВУ/results.txt");
    game();
        
    if (!file.is_open())
    {
        cout << "Ошибка открытия файла.\n";
        exit(0);
    }
    else
    {
        cout << "Файл открыт.\n";
        while (file >> results[i].player_score >> results[i].player_name)
        {
            if (i == 0 &&  results[i].player_name != "NoName")
            {
                record = results[i].player_score;
            }
            i++;
        }
            
        file.clear();
        file.seekg(0, ios::beg);  // возвращаем курсор в начало файла
    }
    file.close();
}

void game::file_writing()
{
    ofstream file;
    file.open("/Users/slavick.snegirevicloud.com/Desktop/Прогр. на ЯВУ/Прогр. на ЯВУ/results.txt");
        
    if (!file.is_open())
    {
        cout << "Ошибка открытия файла.\n";
        exit(0);
    }
    else
    {
        cout << "Файл открыт.\n";
        for (int i = 0; i < kol; i++)
        {
            file << results[i].player_score << " " << results[i].player_name << endl;
        }
    }
    file.close();
}

void game::print()
{
    for (int i = 0; i < 10; i++)
        cout << results[i].player_score << " " << results[i].player_name << endl;
}

void game::game_render()
{
    for (int i = 0; i < M; i++)
        for (int j = 0; j < N; j++)
        {
            if (grid[i][j] == 0)
                continue;
            if (grid[i][j] == 1)
                tile.set_texture_rect({0, 0, tile_size, tile_size});
            if (grid[i][j] == 2)
                tile.set_texture_rect({54, 0, tile_size, tile_size});
            tile.set_position(j*tile_size, i*tile_size);
            tile.draw(renderer);
        }
    
    tile.set_texture_rect({36, 0, tile_size, tile_size});
    tile.set_position(x*tile_size, y*tile_size);
    tile.draw(renderer);
    foe.rotate(10);
    
    for (int i = 0; i < enemy_count; i++)
    {
        foe.set_position(a[i].x, a[i].y);
        foe.draw(renderer);
    } 

    if (!games && (lives <= 0))
    {
        manager_texture.print_text(530, 300, "GAME OVER", 250, 250, 250);
        manager_texture.print_text(400, 340, "Press 'Enter' to play again", 250, 250, 250);
        
        if (manager_input.is_key_pressed(SDLK_RETURN))
        {
            level = 1;
            lives = 3;
            flag = false;
            games = true;
            pause = false;
            enemy_count = 1;
            area = score = 0;
            x = y = dx = dy = 0;
            square_w = SCREEN_WIDTH-30;
            static_cast<void>(timer = 0), delay = 0.1;
            for (int i = 0; i < M; i++)
                for (int j = 0; j < N; j++)
                    if (i == 0 || j == 0 || i == M - 1 || j == N - 1)
                    {
                        grid[i][j] = 1;
                    }
                    else
                    {
                        grid[i][j] = 0;
                    }
            
            game_loop = true;
        }
    }
    
    hearts.set_position(SCREEN_WIDTH-200, 0);
    hearts.draw(renderer);
    
    square.set_position(square_w, 0);
    square.draw(renderer);

    manager_texture.print_text(1080, 70, "----------", 250, 250, 250);
    manager_texture.print_text(1090, 100, "LEVEL", 250, 250, 250);
    manager_texture.print_text(1220, 100, to_string(level), 250, 250, 250);
    manager_texture.print_text(1080, 130, "----------", 250, 250, 250);
    if (score > record)
        record = score;
    manager_texture.print_text(1090, 160, "RECORD", 250, 250, 250);
    manager_texture.print_text(1090, 200, to_string(record), 250, 250, 250);
    manager_texture.print_text(1090, 240, "SCORE", 250, 250, 250);
    manager_texture.print_text(1090, 280, to_string(score), 0, 250, 165);
    manager_texture.print_text(1080, 310, "----------", 250, 250, 250);
    manager_texture.print_text(1090, 550, "Press 'p'", 250, 250, 250);
    manager_texture.print_text(1090, 590, "for pause", 250, 250, 250);
    manager_texture.print_text(1090, 630, "or 'Esc'", 250, 250, 250);
    manager_texture.print_text(1090, 670, "to exit", 250, 250, 250);
    
    if (pause)
    {
        manager_texture.print_text(500, 300, "Game paused", 250, 250, 250);
        manager_texture.print_text(430, 340, "Press 'r' for resume", 250, 250, 250);
    }
     
    SDL_RenderPresent(renderer);
}

void game::game_update()
{
    timer += 0.1f; // чтобы не происходило неявное преобразование типов из float в double на конце 'f'
    manager_input.update(event);
    
    if (manager_input.exit)
    {
        game_loop = false;
        exit(0);
    }

    if (manager_input.is_key_pressed(SDLK_ESCAPE))
    {
        level = 1;
        lives = 3;
        name = "|";
        flag = false;
        games = true;
        pause = false;
        enemy_count = 1;
        area = score = 0;
        x = y = dx = dy = 0;
        square_w = SCREEN_WIDTH-30;
        static_cast<void>(timer = 0), delay = 0.1;
        for (int i = 0; i < M; i++)
            for (int j = 0; j < N; j++)
                if (i == 0 || j == 0 || i == M - 1 || j == N - 1)
                {
                    grid[i][j] = 1;
                }
                else
                {
                    grid[i][j] = 0;
                }
        
        game_loop = false;
    }
    if (manager_input.is_key_down(SDLK_LEFT))
    {
        if (grid[y][x-1] != 2)
        {
            dx = -1;
            dy =  0;
        }
    }
    if (manager_input.is_key_down(SDLK_RIGHT))
    {
        if (grid[y][x+1] != 2)
        {
            dx = +1;
            dy =  0;
        }
    }
    if (manager_input.is_key_down(SDLK_UP))
    {
        if (grid[y-1][x] != 2)
        {
            dx =  0;
            dy = -1;
        }
    }
    if (manager_input.is_key_down(SDLK_DOWN))
    {
        if (grid[y+1][x] != 2)
        {
            dx =  0;
            dy = +1;
        }
    }
    if (manager_input.is_key_down(SDLK_p))
    {
        pause = true;
    }
    
    if (pause)
    {
        if (manager_input.is_key_down(SDLK_r))
        {
            pause = false;
        }
        return;
    }
    
    if (!games)
    {
        lives--;

        if (lives > 0)
        {
            square_w -= 50;
            x = y = dx = dy = 0;
            for (int i = 1; i < M-1; i++)
                for (int j = 1; j < N-1; j++)
                    if (grid[i][j] == 2)
                        grid[i][j] = 0;
            SDL_Delay(200);
            games = true;
        }
        if (lives == 0)
        {
            square_w -= 50;
            bool find = true;
            for (int i = 0; i < kol && find; i++)
            {
                if (score > results[i].player_score)
                {
                    int j = i;
                    find = false;
                    for (i = 8; i >= j; i--)
                    {
                        results[i+1].player_score = results[i].player_score;
                        results[i+1].player_name = results[i].player_name;
                    }
                    results[j].player_score = score;
                    results[j].player_name = name;
                }
            }
        }
        
        return;
    }
    
    if (timer > delay)
    {
        x += dx; // приращение по иксу
        y += dy; // приращение по игреку

        if (x < 0)
            x = 0;
        if (x > N - 1)
            x = N - 1;
        if (y < 0)
            y = 0;
        if (y > M - 1)
            y = M - 1;
        if (grid[y][x] == 2) // если сам в себя врезался, то игра заканчивается
            games = false;
        if (grid[y][x] == 0) // если начинаем закрашивать площадь
        {
            flag = true;
            grid[y][x] = 2;  // рисуем позади путь в виде зеленных клеточек
        }
        
        timer = 0;  
    }
 
    for (int i = 0; i < enemy_count; i++) // чтобы враги отскакивали от рамки обратно
    {
        a[i].move();
        if (grid[a[i].y / tile_size][a[i].x / tile_size] == 1)
        {
            a[i].dy = -a[i].dy;
            a[i].y += a[i].dy;
            if (grid[a[i].y / tile_size][a[i].x / tile_size] == 1)
            {
                a[i].dx = -a[i].dx;
                a[i].x += a[i].dx;
                a[i].dy = -a[i].dy;
                a[i].y += a[i].dy;
            }
        }
    }

    if (grid[y][x] == 1)
    {
        dx = dy = 0;
        
        if (flag)
        {
            for (int i = 0; i < enemy_count; i++)
                drop(a[i].y / tile_size, a[i].x / tile_size);

            for (int i = 1; i < M-1; i++)
                for (int j = 1; j < N-1; j++)
                    if (grid[i][j] == -1)
                        grid[i][j] = 0;
                    else
                    {
                        if (grid[i][j] != 1)
                        {
                            grid[i][j] = 1;
                            area++;
                            score++;
                        }
                    }
            
        }
        flag = false;
    }

    for (int i = 0; i < enemy_count; i++)
        if (grid[a[i].y / tile_size][a[i].x / tile_size] == 2) // если враги задели путь во время закрашивания
            games = false;
}

void game::records(int widht, int height)
{
    bool record_loop = true;

    this->width = widht;
    this->height = height;

    manager_texture.set_render(renderer);
    
    while (record_loop)
    {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        SDL_RenderClear(renderer);
        
        manager_input.update(event);
        
        if (manager_input.exit)
        {
            record_loop = false;
            exit(0);
        }

        if (manager_input.is_key_pressed(SDLK_ESCAPE))
        {
            record_loop = false;
        }
        
        manager_texture.print_text(500, 30, "TOP 10 RESULTS", 250, 250, 250);
        manager_texture.print_text(430, 90, "NAME            SCORE", 250, 0, 250);
        manager_texture.print_text(240, 150, "1st", 250, 250, 250);
        manager_texture.print_text(240, 200, "2nd", 250, 250, 250);
        manager_texture.print_text(240, 250, "3rd", 250, 250, 250);
        manager_texture.print_text(240, 300, "4th", 250, 250, 250);
        manager_texture.print_text(240, 350, "5th", 250, 250, 250);
        manager_texture.print_text(240, 400, "6th", 250, 250, 250);
        manager_texture.print_text(240, 450, "7th", 250, 250, 250);
        manager_texture.print_text(240, 500, "8th", 250, 250, 250);
        manager_texture.print_text(240, 550, "9th", 250, 250, 250);
        manager_texture.print_text(240, 600, "10th", 250, 250, 250);
        manager_texture.print_text(240, 670, "Press 'Esc' to return to the main menu", 250, 250, 250);
        
        manager_texture.print_text(430, 150, results[0].player_name, 250, 250, 250);
        manager_texture.print_text(765, 150, to_string(results[0].player_score), 250, 250, 250);
        manager_texture.print_text(430, 200, results[1].player_name, 250, 250, 250);
        manager_texture.print_text(765, 200, to_string(results[1].player_score), 250, 250, 250);
        manager_texture.print_text(430, 250, results[2].player_name, 250, 250, 250);
        manager_texture.print_text(765, 250, to_string(results[2].player_score), 250, 250, 250);
        manager_texture.print_text(430, 300, results[3].player_name, 250, 250, 250);
        manager_texture.print_text(765, 300, to_string(results[3].player_score), 250, 250, 250);
        manager_texture.print_text(430, 350, results[4].player_name, 250, 250, 250);
        manager_texture.print_text(765, 350, to_string(results[4].player_score), 250, 250, 250);
        manager_texture.print_text(430, 400, results[5].player_name, 250, 250, 250);
        manager_texture.print_text(765, 400, to_string(results[5].player_score), 250, 250, 250);
        manager_texture.print_text(430, 450, results[6].player_name, 250, 250, 250);
        manager_texture.print_text(765, 450, to_string(results[6].player_score), 250, 250, 250);
        manager_texture.print_text(430, 500, results[7].player_name, 250, 250, 250);
        manager_texture.print_text(765, 500, to_string(results[7].player_score), 250, 250, 250);
        manager_texture.print_text(430, 550, results[8].player_name, 250, 250, 250);
        manager_texture.print_text(765, 550, to_string(results[8].player_score), 250, 250, 250);
        manager_texture.print_text(430, 600, results[9].player_name, 250, 250, 250);
        manager_texture.print_text(765, 600, to_string(results[9].player_score), 250, 250, 250);
        
        
        SDL_RenderPresent(renderer);
    }
}

void game::author(int widht, int height)
{
    bool author_loop = true;

    this->width = widht;
    this->height = height;
 
    manager_texture.set_render(renderer);
    
    while (author_loop)
    {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        SDL_RenderClear(renderer);
        
        manager_input.update(event);
        
        if (manager_input.exit)
        {
            author_loop = false;
            exit(0);
        }
        
        if (manager_input.is_key_pressed(SDLK_ESCAPE))
        {
            author_loop = false;
        }
        
        manager_texture.print_text(450, 50, "ABOUT THE AUTHOR", 0, 200, 200);
        
        manager_texture.print_text(150, 150, "Game was created in 1984 for the PC platform as", 250, 250, 250);
        manager_texture.print_text(150, 200, "a clone of the game Qix, which appeared earlier", 250, 250, 250);
        manager_texture.print_text(150, 250, "on arcade machines. Invented by Ilan Rab and Dani", 250, 250, 250);
        manager_texture.print_text(150, 300, "Katz. It was popular in the 1980s and 1990s.", 250, 250, 250);
        manager_texture.print_text(150, 350, "Specifically, this version of the game was written", 250, 250, 250);
        manager_texture.print_text(150, 400, "by a student from the University 'VOENMEH' Snegirev", 250, 250, 250);
        manager_texture.print_text(150, 450, "Svyatoslav (group i508b).", 250, 250, 250);

        manager_texture.print_text(150, 600, "Press 'Esc' to return to the main menu", 250, 250, 250);
        SDL_RenderPresent(renderer);
    }
}


void game::create(int widht, int height)
{
    SDL_Event event;
    game_loop = true;
    bool flag = false;
    bool quit = false;
    
    this->width = widht;
    this->height = height;
    
    manager_texture.set_render(renderer);
    
    SDL_StartTextInput();
    while (!quit)
    {
        while (SDL_PollEvent(&event) != 0)
        {
            if (event.type == SDL_KEYDOWN)
            {
                switch (event.key.keysym.sym)
                {
                case SDLK_RETURN:
                        if (name != "|")
                            quit = true;
                        break;
                
                case SDLK_ESCAPE:
                        game_loop = false;
                        quit = true;
                        break;
                        
                case SDLK_BACKSPACE:
                        if (name.length() != 0)
                            name.pop_back();
                        else
                        {
                            name = "|";
                            flag = false;
                        }
                        break;
                }
            }
            else
            {
                if (event.type == SDL_TEXTINPUT)
                {
                    if (!flag)
                    {
                        name.pop_back();
                        flag = true;
                    }
                    
                    if (name.length() < 17)
                        name += event.text.text;
                }
                if (event.type == SDL_QUIT)
                {
                    quit = true;
                    exit(0);
                }
            }
        }
        
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        SDL_RenderClear(renderer);
        manager_texture.print_text(480, 20, "Enter your name:", 250, 250, 250);
        start.set_texture(manager_texture.get_texture("/Users/slavick.snegirevicloud.com/Desktop/Прогр. на ЯВУ/Прогр. на ЯВУ/input_name.jpg"));
        start.set_position(45, 70);
        start.draw(renderer);
        
        if (name.length() != 0)
            manager_texture.print_text(450, 100, name, 0, 200, 200);
        else
        {
            name = "|";
            flag = false;
        }
        
        manager_texture.print_text(415, 180, "Press 'Enter' to continue", 250, 250, 250);
            
        SDL_RenderPresent(renderer);
    }
    SDL_StopTextInput();
    
    
    foe.set_texture(manager_texture.get_texture("/Users/slavick.snegirevicloud.com/Desktop/Прогр. на ЯВУ/Прогр. на ЯВУ/enemy.png"));
    tile.set_texture(manager_texture.get_texture("/Users/slavick.snegirevicloud.com/Desktop/Прогр. на ЯВУ/Прогр. на ЯВУ/tiles.png"));
    hearts.set_texture(manager_texture.get_texture("/Users/slavick.snegirevicloud.com/Desktop/Прогр. на ЯВУ/Прогр. на ЯВУ/hearts.png"));
    square.set_texture(manager_texture.get_texture("/Users/slavick.snegirevicloud.com/Desktop/Прогр. на ЯВУ/Прогр. на ЯВУ/square.png"));
    
    foe.set_origin(20, 20);

    for (int i = 0; i < M; i++)
        for (int j = 0; j < N; j++)
            if (i == 0 || j == 0 || i == M - 1 || j == N - 1)
                grid[i][j] = 1;
}

void game::rules(int widht, int height)
{
    bool rule_loop = true;

    this->width = widht;
    this->height = height;

    manager_texture.set_render(renderer);
    
    while (rule_loop)
    {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        SDL_RenderClear(renderer);
        
        manager_input.update(event);
        
        if (manager_input.exit)
        {
            rule_loop = false;
            exit(0);
        }

        if (manager_input.is_key_pressed(SDLK_ESCAPE))
        {
            rule_loop = false;
        }
        manager_texture.print_text(525, 50, "GAME RULES", 200, 200, 0);
        manager_texture.print_text(150, 150, "Click on the menu item 'new game', then enter the", 250, 250, 250);
        manager_texture.print_text(150, 200, "player's name. After you have decided on the name,", 250, 250, 250);
        manager_texture.print_text(150, 250, "click 'enter'. To move in the game, use the arrow", 250, 250, 250);
        manager_texture.print_text(150, 300, "keys (up, down, left, right). The game is endless,", 250, 250, 250);
        manager_texture.print_text(150, 350, "to go to the next level, you need to paint over more", 250, 250, 250);
        manager_texture.print_text(150, 400, "than 3/4 of the space. With each new level, one enemy", 250, 250, 250);
        manager_texture.print_text(150, 450, "will be added to the game. The game ends if your green", 250, 250, 250);
        manager_texture.print_text(150, 500, "line is crossed more than three times.", 250, 250, 250);
        
        manager_texture.print_text(150, 600, "Press 'Esc' to return to the main menu", 250, 250, 250);
        SDL_RenderPresent(renderer);
    }
}

void game::drop(int y, int x)
{
    if (grid[y][x] == 0)
        grid[y][x] = -1;
    if (grid[y-1][x] == 0)
        drop(y-1, x);
    if (grid[y+1][x] == 0)
        drop(y+1, x);
    if (grid[y][x-1] == 0)
        drop(y, x-1);
    if (grid[y][x+1] == 0)
        drop(y, x+1);
}

void game::play()
{
    while (game_loop)
    {
        if (area < ((N-2)*(M-2)*0.75))
        {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
            SDL_RenderClear(renderer);
            
            game_update();
            game_render();
        }
        else
        {
            for (int i = 0; i < M; i++)
                for (int j = 0; j < N; j++)
                {
                    if (i == 0 || j == 0 || i == M - 1 || j == N - 1)
                        grid[i][j] = 1;
                    else
                        grid[i][j] = 0;
                }
            x = y = dx = dy = 0;
            tile.set_texture_rect({36, 0, tile_size, tile_size});
            tile.set_position(x*tile_size, y*tile_size);
            SDL_Delay(200);
                
            area = 0;
            level++;
            enemy_count++;
            //choise = 1 + rand() % 5;
        }
    }
}

/*=============================================================================================================================*/
/*=============================================================================================================================*/

bool initialization();
void free_memory();

/*=============================================================================================================================*/
/*=============================================================================================================================*/

int main(int argc, char *argv[])
{
    int selector = 1;
    int height = 720;
    
    game game;
    SDL_Sprite start_screen;
    manager_texture manager_texture;
    
    game.file_reading();
    
    if (!initialization())
    {
        cout << "\nОшибка инициализации." << endl;
    }
    else
    {
        SDL_Event event;
        bool quit = false;
            
        while (!quit)
        {
            while (SDL_PollEvent(&event) != 0)
            {
                if (event.type == SDL_QUIT)
                {
                    quit = true;
                }
                else
                if (event.type == SDL_KEYDOWN)
                {
                    switch (event.key.keysym.sym)
                    {
                    case SDLK_ESCAPE:
                            quit = true;
                            break;
                            
                    case SDLK_RETURN:
                            if (height <= 15)
                            {
                                if (selector == 1)
                                {
                                    game.create(SCREEN_WIDTH, SCREEN_HEIGHT);
                                    game.play();
                                }
                                if (selector == 2)
                                {
                                    game.records(SCREEN_WIDTH, SCREEN_HEIGHT);
                                }
                                if (selector == 3)
                                {
                                    game.rules(SCREEN_WIDTH, SCREEN_HEIGHT);
                                }
                                if (selector == 4)
                                {
                                    game.author(SCREEN_WIDTH, SCREEN_HEIGHT);
                                }
                                if (selector == 5)
                                {
                                    quit = true;
                                }
                            }
                            break;
                            
                    case SDLK_s:
                            if (selector < 5)
                                selector++;
                            break;
                            
                    case SDLK_w:
                            if (selector > 1)
                                selector--;
                            break;
                            
                    case SDLK_DOWN:
                            if (selector < 5)
                                selector++;
                            break;
                                
                    case SDLK_UP:
                            if (selector > 1)
                                selector--;
                            break;

                    default:
                            break;
                    }
                }
            }
            
            manager_texture.set_render(renderer);
            SDL_RenderClear(renderer);
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
            start_screen.set_texture(manager_texture.get_texture("/Users/slavick.snegirevicloud.com/Desktop/Прогр. на ЯВУ/Прогр. на ЯВУ/strart_screen.png"));
            manager_texture.print_text(550, 1085, "NEW GAME", 250, 250, 250);
            manager_texture.print_text(560, 1135, "RECORDS", 250, 250, 250);
            manager_texture.print_text(585, 1185, "RULES", 250, 250, 250);
            manager_texture.print_text(455, 1235, "ABOUT THE AUTHOR", 250, 250, 250);
            manager_texture.print_text(600, 1285, "EXIT", 250, 250, 250);
            
            if (height > 15)
            {
                start_screen.set_position(320, height -= 5);
                manager_texture.print_text(550, height + 340, "NEW GAME", 250, 250, 250);
                manager_texture.print_text(560, height + 400, "RECORDS", 250, 250, 250);
                manager_texture.print_text(585, height + 460, "RULES", 250, 250, 250);
                manager_texture.print_text(455, height + 520, "ABOUT THE AUTHOR", 250, 250, 250);
                manager_texture.print_text(600, height + 580, "EXIT", 250, 250, 250);
            }
            else
            {
                start_screen.set_position(320, height);
                if (selector == 1)
                    manager_texture.print_text(550, height + 340, "NEW GAME", 0, 250, 250);
                else
                    manager_texture.print_text(550, height + 340, "NEW GAME", 250, 250, 250);
                if (selector == 2)
                    manager_texture.print_text(560, height + 400, "RECORDS", 0, 250, 250);
                else
                    manager_texture.print_text(560, height + 400, "RECORDS", 250, 250, 250);
                if (selector == 3)
                    manager_texture.print_text(585, height + 460, "RULES", 0, 250, 250);
                else
                    manager_texture.print_text(585, height + 460, "RULES", 250, 250, 250);
                if (selector == 4)
                    manager_texture.print_text(455, height + 520, "ABOUT THE AUTHOR", 0, 250, 250);
                else
                    manager_texture.print_text(455, height + 520, "ABOUT THE AUTHOR", 250, 250, 250);
                if (selector == 5)
                    manager_texture.print_text(600, height + 580, "EXIT", 0, 250, 250);
                else
                    manager_texture.print_text(600, height + 580, "EXIT", 250, 250, 250);
            }
            start_screen.draw(renderer);
            SDL_RenderPresent(renderer);
        }
    }
    game.file_writing();
    free_memory();

    return 0;
}

/*=============================================================================================================================*/
/*=============================================================================================================================*/

bool initialization()
{
    bool success = true;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) // инициализация SDL
    {
        cout << "\nНевозможно инициализировать SDL. Ошибка: " << SDL_GetError() << "." << endl;
        success = false;
    }
    else
    {
        window = SDL_CreateWindow("'Xonix'. И508Б Снегирев С.И.", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
        
        if (window == NULL)
        {
            cout << "\nОкно не может быть создано. Ошибка: " << SDL_GetError() << "." << endl;
            success = false;
        }
        else
        {
            renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
            if (renderer == NULL)
            {
                cout << "\nНевозможно создать рендерер. Ошибка: " << SDL_GetError() << "." << endl;
                success = false;
            }
            else
            {
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0); // черный цвет зливки экрана

                int image_flag = IMG_INIT_PNG; // загрузка png картинок
                if (!(IMG_Init(image_flag) & image_flag))
                {
                    cout << "\nНевозможно инициализировать SDL_Image. Ошибка: " << IMG_GetError() << "." << endl;
                    success = false;
                }
                else
                {
                    if (TTF_Init() < 0)
                    {
                        cout << "\nНевозможно инициализировать SDL_ttf. Ошибка: " << TTF_GetError() << "." << endl;
                        success = false;
                    }
                    else
                        font = TTF_OpenFont("/Users/slavick.snegirevicloud.com/Desktop/Прогр. на ЯВУ/Прогр. на ЯВУ/20944.ttf", 40);
                }
            }
        }
    }
    return success;
}

void free_memory() // очистка памяти и выход из подсистем
{   
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    
    font = NULL;
    window = NULL;
    renderer = NULL;

    IMG_Quit();
    TTF_Quit();
    SDL_Quit();
}
