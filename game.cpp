#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_image.h>
#include <stb_image_resize.h>
#include <stb_truetype.h>
#include <gl.h>
#include <gl.c>
#include <SDL.h>

#include "log.h"
#include "types.h"
#include "assets.h"
#include "shapes.h"
#include "application.h"
#include "coffee_cow.h"
#include "particles.h"

#include "assets.cpp"
#include "shapes.cpp"
#include "menu.cpp"
#include "particles.cpp"
#include "coffee_cow.cpp"

enum Game_Modes
{
    MAIN_MENU,
    MULTIPLAYER_MENU,
    COUNTDOWN,
    RESULTS,
    IN_GAME,
    PAUSED,
    GAME_OVER,
    SETTINGS,
    
    NUM_OF_GAME_MODES
};

enum In_Game_Modes
{
    SINGLEPLAYER,
    MULTIPLAYER,
};

struct Game_Data
{
    u32 music_index;

    // menu data
    u32 game_mode = MAIN_MENU;
    u32 in_game_mode;
    s32 active;
    Menu default_menu;

    // gameplay data
    v2s grid_dim;
    Coffee_Cow_Design designs[4];
    Coffee_Cow players[4];
    u32 num_of_players;
    Coffee coffees[10];
    u32 num_of_coffees;

    // singleplayer data
    u32 high_score;
    
    // multiplayer data
    u32 countdown;
    u32 count_start_time;
    Bitmap *winner;
    Bitmap *winner_outline;

    // settings
    b8 fullscreen;
    b8 show_fps;
};

/*
 TODO:
- vary particles life
- vary particles color

- animation? 
- crash into wall
- wagging of tail
- eyes moving

- make multiplayer good/fix bugs
- when they crash head on it ends with them a block away
*/

function void
init_controllers(Input *input)
{
    input->active_controller = &input->controllers[0];
    
    Controller *keyboard = &input->controllers[0];
    set(&keyboard->right,  SDLK_d);
    set(&keyboard->right,  SDLK_RIGHT);
    set(&keyboard->up,     SDLK_w);
    set(&keyboard->up,     SDLK_UP);
    set(&keyboard->left,   SDLK_a);
    set(&keyboard->left,   SDLK_LEFT);
    set(&keyboard->down,   SDLK_s);
    set(&keyboard->down,   SDLK_DOWN);
    set(&keyboard->select, SDLK_RETURN);
    set(&keyboard->pause,  SDLK_ESCAPE);

    set(&keyboard->show_fps,  SDLK_F5);
    
    input->num_of_controllers = 1; // keyboard
    
    log("Game Controllers:");
    SDL_Joystick **joysticks = input->joysticks;
    SDL_GameController **game_controllers = input->game_controllers;
    input->num_of_joysticks = SDL_NumJoysticks();
    for (u32 i = 0; i < input->num_of_joysticks; i++)
    {
        joysticks[i] = SDL_JoystickOpen(i);
        if (SDL_IsGameController(i))
        {
            log("%s", SDL_JoystickName(joysticks[i]));
            
            game_controllers[i] = SDL_GameControllerOpen(i);
            
            Controller *c = &input->controllers[i + 1];
            set(&c->right,  SDL_CONTROLLER_BUTTON_DPAD_RIGHT);
            set(&c->up,     SDL_CONTROLLER_BUTTON_DPAD_UP);
            set(&c->left,   SDL_CONTROLLER_BUTTON_DPAD_LEFT);
            set(&c->down,   SDL_CONTROLLER_BUTTON_DPAD_DOWN);
            set(&c->select, SDL_CONTROLLER_BUTTON_A);
            set(&c->pause,  SDL_CONTROLLER_BUTTON_START);
            
            input->num_of_controllers++;
        }
    }
}

function u32
load_high_score()
{
    u32 high_score = 0;
    File high_score_file = read_file("high_score.save");
    if (high_score_file.size == 0) return high_score;
    high_score = string_to_u32((char*)high_score_file.memory, high_score_file.size);
    return high_score;
}

function void
update_high_score(u32 score, u32 *high_score)
{
    if (score > *high_score)
    {
        *high_score = score;

        File file = {};
        String string = {};
        u32_to_string(&string, score);
        file.memory = (void*)string.data;
        file.size = string.length;
        write_file(&file, "high_score.save");
    }
}   

function void*
init_game_data(Assets *assets)
{
    Game_Data *data = (Game_Data*)malloc(sizeof(Game_Data));
    *data = {};
    data->grid_dim = { 12, 12 };
    
    Coffee_Cow_Design *designs = data->designs;

    designs[0].bitmaps[ASSET_COW_HEAD]         = find_bitmap(assets, "COW1_HEAD");
    designs[0].bitmaps[ASSET_COW_HEAD_OUTLINE] = find_bitmap(assets, "COW1_HEAD_OUTLINE");
    designs[0].bitmaps[ASSET_COW_MOUTH]        = find_bitmap(assets, "COW1_MOUTH");
    designs[0].bitmaps[ASSET_COW_TAIL]         = find_bitmap(assets, "COW1_TAIL");
    designs[0].bitmaps[ASSET_COW_SPOT]         = find_bitmap(assets, "COW1_SPOT1");
    designs[0].bitmaps[ASSET_COW_SPOT + 1]     = find_bitmap(assets, "COW1_SPOT2");
    designs[0].bitmaps[ASSET_COW_SPOT + 2]     = find_bitmap(assets, "COW1_SPOT3");
    designs[0].color = { 255, 255, 255, 1 };
    designs[0].outline_color = { 0, 0, 0, 1 };

    designs[1].bitmaps[ASSET_COW_HEAD]         = find_bitmap(assets, "COW2_HEAD");
    designs[1].bitmaps[ASSET_COW_HEAD_OUTLINE] = find_bitmap(assets, "COW2_HEAD_OUTLINE");
    designs[1].bitmaps[ASSET_COW_MOUTH]        = find_bitmap(assets, "COW2_MOUTH");
    designs[1].bitmaps[ASSET_COW_TAIL]         = find_bitmap(assets, "COW2_TAIL");
    designs[1].bitmaps[ASSET_COW_SPOT]         = find_bitmap(assets, "COW2_SPOT1");
    designs[1].bitmaps[ASSET_COW_SPOT + 1]     = find_bitmap(assets, "COW2_SPOT2");
    designs[1].bitmaps[ASSET_COW_SPOT + 2]     = find_bitmap(assets, "COW2_SPOT3");
    designs[1].color = { 108, 108, 108, 1 };
    designs[1].outline_color = { 0, 0, 0, 1 };

    designs[2].bitmaps[ASSET_COW_HEAD]         = find_bitmap(assets, "COW3_HEAD");
    designs[2].bitmaps[ASSET_COW_HEAD_OUTLINE] = find_bitmap(assets, "COW3_HEAD_OUTLINE");
    designs[2].bitmaps[ASSET_COW_MOUTH]        = find_bitmap(assets, "COW3_MOUTH");
    designs[2].bitmaps[ASSET_COW_TAIL]         = find_bitmap(assets, "COW3_TAIL");
    designs[2].bitmaps[ASSET_COW_SPOT]         = find_bitmap(assets, "COW3_SPOT1");
    designs[2].bitmaps[ASSET_COW_SPOT + 1]     = find_bitmap(assets, "COW3_SPOT2");
    designs[2].bitmaps[ASSET_COW_SPOT + 2]     = find_bitmap(assets, "COW3_SPOT3");
    designs[2].color = { 252, 231, 213, 1 };
    designs[2].outline_color = { 0, 0, 0, 1 };

    designs[3].bitmaps[ASSET_COW_HEAD]         = find_bitmap(assets, "COW4_HEAD");
    designs[3].bitmaps[ASSET_COW_HEAD_OUTLINE] = find_bitmap(assets, "COW4_HEAD_OUTLINE");
    designs[3].bitmaps[ASSET_COW_MOUTH]        = find_bitmap(assets, "COW4_MOUTH");
    designs[3].bitmaps[ASSET_COW_TAIL]         = find_bitmap(assets, "COW4_TAIL");
    designs[3].bitmaps[ASSET_COW_SPOT]         = find_bitmap(assets, "COW4_SPOT1");
    designs[3].bitmaps[ASSET_COW_SPOT + 1]     = find_bitmap(assets, "COW4_SPOT2");
    designs[3].bitmaps[ASSET_COW_SPOT + 2]     = find_bitmap(assets, "COW4_SPOT3");
    designs[3].color = { 197, 64, 48, 1 };
    designs[3].outline_color = { 0, 0, 0, 1 };

    // default menu
    Menu *default_menu = &data->default_menu;
    default_menu->font = find_font(assets, "RUBIK");
    
    default_menu->button.back_color = {0, 0, 0, 1};
    default_menu->button.active_back_color = {222, 201, 179, 1};
    default_menu->button.text_color = {255, 255, 255, 1};
    default_menu->button.active_text_color = {0, 0, 0, 1};
    
    default_menu->window_percent = 0.7f;
    default_menu->padding_percent = 0.0179f;
    default_menu->button_percent = 0.15f;
    default_menu->pixel_height_percent = 0.6f;
    
    data->high_score = load_high_score();

    data->music_index = random(0, 3);

    return (void*)data;
}

// assigned controller to a design
function void
multiplayer_assign_controller(Controller *controller, Coffee_Cow_Design *players, u32 index)
{
    if (players[index].controller == 0)
    {
        for (u32 i = 0; i < 4; i++) if (players[i].controller == controller) players[i].controller = 0; // free controller from another spot
        players[index].controller = controller;
    }
    else if (players[index].controller == controller)
    {
        players[index].controller = 0;
    }
}

function void
update_music(Audio_Player *player, Assets *assets, u32 *music_index)
{
    Playing_Audio *music = &player->audios[0];
    if (music->length_remaining <= 0)
    {
        play_audio(player, find_audio(assets, (*music_index)++), AUDIO_MUSIC);
        if (*music_index > 2) *music_index = 0;
    }
}

function void
draw_singleplayer_scoreboard(Font *font, v2s window_dim, u32 score, u32 high_score)
{
    r32 pixel_height = window_dim.y * 0.07f;
    v4 color = { 0, 0, 0, 1 };
    v2 padding = { 5, 10 };

    char *score_str = u32_to_string(score);
    v2 score_dim = get_string_dim(font, score_str, pixel_height, color);
    draw_string(font, score_str, { 5, score_dim.y + 10 }, pixel_height, color);
    SDL_free((void*)score_str);
    
    const char *high_score_str = u32_to_string(high_score);
    v2 high_score_dim = get_string_dim(font, high_score_str, pixel_height, color);
    v2 high_score_coords = { window_dim.x - high_score_dim.x - padding.x, high_score_dim.y + padding.y };
    draw_string(font, high_score_str, high_score_coords, pixel_height, color);
    SDL_free((void*)high_score_str);

    const char *high_score_text = "High Score: ";
    v2 high_score_text_dim = get_string_dim(font, high_score_text, pixel_height, color);
    v2 high_score_text_coords = { window_dim.x - high_score_text_dim.x - 10 - high_score_dim.x - padding.x, high_score_text_dim.y + padding.y };
    draw_string(font, high_score_text, high_score_text_coords, pixel_height, color);
}

function void
draw_multiplayer_scoreboard(Font *font, v2s window_dim, Coffee_Cow *players, u32 num_of_players)
{
    r32 pixel_height = window_dim.y * 0.07f;
    v4 color = { 0, 0, 0, 1 };
    
    r32 bit_dim_temp = window_dim.y * 0.07f;
    v2 bitmap_dim = { bit_dim_temp, bit_dim_temp };

    Rect rect = {};
    rect.dim = { bitmap_dim.x + 100, bitmap_dim.y * num_of_players };
    rect.coords = { 20, 20 };

    v2 coords = { 5, 5 };
    for (u32 i = 0; i < num_of_players; i++)
    {
        char *str = u32_to_string(players[i].wins);
        v2 str_dim = get_string_dim(font, str, pixel_height, color);
        v2 str_coords = { coords.x + bitmap_dim.x + 5,  coords.y + (bitmap_dim.y/2.0f) + (str_dim.y/2.0f) };

        draw_rect(coords, 0, bitmap_dim, players[i].design.bitmaps[ASSET_COW_HEAD_OUTLINE]);       
        draw_rect(coords, 0, bitmap_dim, players[i].design.bitmaps[ASSET_COW_HEAD]);
        draw_string(font, str, str_coords, pixel_height, color);

        coords.y += bitmap_dim.y;
    }
}

function b32
update(Application *app)
{
    Game_Data *data = (Game_Data*)app->data;
    Controller *menu_controller = app->input.active_controller;
    
    Coffee_Cow *players = data->players;
    v2s window_dim = app->window.dim;
    Font *rubik = find_font(&app->assets, "RUBIK");
    
    // UPDATE
    update_particles(&particles, app->time.frame_time_s);
    if (app->time.run_time_s > 4.0f) update_music(&app->player, &app->assets, &data->music_index);

    switch(data->game_mode)
    {
        case MAIN_MENU:
        {
            menu_update_active(&data->active, 0, 3, menu_controller->down, menu_controller->up);
        } break;
        
        case MULTIPLAYER_MENU:
        {
            menu_update_active(&data->active, 0, 5, menu_controller->down, menu_controller->up);
        } break;

        case SETTINGS:
        {
            menu_update_active(&data->active, 0, 4, menu_controller->down, menu_controller->up);
        } break;

        case PAUSED:
        case GAME_OVER:
        {
            menu_update_active(&data->active, 0, 1, menu_controller->down, menu_controller->up);
            update_coffee_cow_mouth(&players[0], app->time.frame_time_s, 7.0f);

            if (on_down(menu_controller->pause)) data->game_mode = IN_GAME;
        } break;

        case COUNTDOWN:
        {
            r32 diff = app->time.run_time_s - data->count_start_time;
            u32 count = 3 - (u32)diff;
            data->countdown = count;
            if (count == 0) data->game_mode = IN_GAME;
        } break;

        case RESULTS:
        {
            r32 diff = app->time.run_time_s - data->count_start_time;
            u32 count = 2 - (u32)diff;
            if (count == 0) data->game_mode = GAME_OVER;
        } break;
        
        case IN_GAME:
        {
            update_high_score(players[0].score, &data->high_score);

            update_coffee_cows(players, data->num_of_players, app->time.frame_time_s, data->grid_dim);
            update_coffees(data->coffees, data->num_of_coffees);
            if (coffee_cows_on_coffee(players, data->num_of_players, data->coffees, data->num_of_coffees, data->grid_dim))
                play_audio(&app->player, find_audio(&app->assets, "GULP"), AUDIO_SOUND);

            if (data->in_game_mode == SINGLEPLAYER)
            {
                if (players[0].dead) data->game_mode = GAME_OVER;
            }
            else if (data->in_game_mode == MULTIPLAYER)
            {
                u32 players_alive = 0;
                Coffee_Cow *alive = 0;
                for (u32 i = 0; i < data->num_of_players; i++) if (!players[i].dead) { players_alive++; alive = &players[i]; }
                if (players_alive == 0)
                {
                    data->game_mode = RESULTS;
                    data->count_start_time = app->time.run_time_s;
                    data->winner = 0;
                    data->winner_outline = 0;
                }
                else if (players_alive == 1)
                {
                    alive->wins++;
                    data->game_mode = RESULTS;
                    data->count_start_time = app->time.run_time_s;
                    data->winner = alive->design.bitmaps[ASSET_COW_HEAD];
                    data->winner_outline = alive->design.bitmaps[ASSET_COW_HEAD_OUTLINE];
                }
            }
                                 
            if (on_down(menu_controller->pause)) data->game_mode = PAUSED;
        } break;
    }
    
    // DRAW
    u32 gl_clear_flags = 
        GL_COLOR_BUFFER_BIT | 
        GL_DEPTH_BUFFER_BIT | 
        GL_STENCIL_BUFFER_BIT;
    
    glClear(gl_clear_flags);
    
    Rect window_rect = Rect{{0, 0}, cv2(window_dim)};
    
    switch (data->game_mode)
    {
        case MAIN_MENU:
        {
            Bitmap *logo = find_bitmap(&app->assets, "LOGO");
            Bitmap *main_menu_back = find_bitmap(&app->assets, "MAIN_MENU_BACK");
            
            Menu main_menu = data->default_menu;
            b32 select = on_down(menu_controller->select);
            
            resize_menu(&main_menu, window_rect, logo->dim, 1, 4);
            
            draw_rect(window_rect, main_menu_back);
            
            draw_rect(main_menu.rect.coords, 0, main_menu.bitmap.dim, logo);
            main_menu.rect.coords.y += main_menu.bitmap.dim.y + main_menu.padding.y;
            
            u32 index = 0;
            if (menu_button(&main_menu, "Singleplayer", index++, data->active, select))
            {
                data->game_mode = IN_GAME;
                data->in_game_mode = SINGLEPLAYER;
                data->active = 0;
                data->num_of_players = 1;
                data->num_of_coffees = 1;
                reset_all_coffee_cows(players, data->num_of_players);
                players[0].design_index = 0;
                players[0].controller = app->input.active_controller;
                init_all_coffee_cows(players, data->num_of_players, data->designs, data->grid_dim);
                random_coffee_locaton(&data->coffees[0].coords, data->grid_dim, players, data->num_of_players);  
            }
            
            if (menu_button(&main_menu, "Multiplayer", index++, data->active, select))
            {
                data->game_mode = MULTIPLAYER_MENU;
                data->in_game_mode = MULTIPLAYER;
                data->active = 0;
                play_audio(&app->player, find_audio(&app->assets, "BLOOP"), AUDIO_SOUND);
                for (u32 i = 0; i < 4; i++) data->designs[i].controller = 0;
            }

            if (menu_button(&main_menu, "Settings", index++, data->active, select))
            {
                data->game_mode = SETTINGS;
                data->active = 0;
            }
            
            if (menu_button(&main_menu, "Quit", index++, data->active, select))
            {
                return true;
            }

            draw_particles(&particles, &app->assets);
        } break;

        case MULTIPLAYER_MENU:
        {
            Bitmap *multi_menu_back = find_bitmap(&app->assets, "MAIN_MENU_BACK");

            Bitmap *char1              = find_bitmap(&app->assets, "COW1_JOIN");
            Bitmap *char1_hover        = find_bitmap(&app->assets, "COW1_JOIN_HOVER");
            Bitmap *char1_select       = find_bitmap(&app->assets, "COW1_SELECT");
            Bitmap *char1_select_hover = find_bitmap(&app->assets, "COW1_SELECT_HOVER");

            Bitmap *char2              = find_bitmap(&app->assets, "COW2_JOIN");
            Bitmap *char2_hover        = find_bitmap(&app->assets, "COW2_JOIN_HOVER");
            Bitmap *char2_select       = find_bitmap(&app->assets, "COW2_SELECT");
            Bitmap *char2_select_hover = find_bitmap(&app->assets, "COW2_SELECT_HOVER");

            Bitmap *char3              = find_bitmap(&app->assets, "COW3_JOIN");
            Bitmap *char3_hover        = find_bitmap(&app->assets, "COW3_JOIN_HOVER");
            Bitmap *char3_select       = find_bitmap(&app->assets, "COW3_SELECT");
            Bitmap *char3_select_hover = find_bitmap(&app->assets, "COW3_SELECT_HOVER");

            Bitmap *char4              = find_bitmap(&app->assets, "COW4_JOIN");
            Bitmap *char4_hover        = find_bitmap(&app->assets, "COW4_JOIN_HOVER");
            Bitmap *char4_select       = find_bitmap(&app->assets, "COW4_SELECT");
            Bitmap *char4_select_hover = find_bitmap(&app->assets, "COW4_SELECT_HOVER");

            b32 select = on_down(menu_controller->select);

            Menu multi_menu = data->default_menu;
            Rect bounds = get_centered_square(window_rect, multi_menu.window_percent);

            resize_menu(&multi_menu, window_rect, {char1->dim.x, char1->dim.y/4}, 1, 2);
            draw_rect(window_rect, multi_menu_back);

            u32 index = 0;

            v2 character_dim = { (r32)bounds.dim.x/4.0f, (r32)bounds.dim.y * (2.0f/4.0f) };
            v2 character_coords = multi_menu.rect.coords;

            if (menu_multiplayer_selector(&multi_menu, index++, data->active, select, &character_coords, character_dim, data->designs[0].controller,
                                          char1, char1_hover, char1_select, char1_select_hover))
            {
                multiplayer_assign_controller(menu_controller, data->designs, 0);
            }

            if (menu_multiplayer_selector(&multi_menu, index++, data->active, select, &character_coords, character_dim, data->designs[1].controller, 
                                          char2, char2_hover, char2_select, char2_select_hover))
            {
                multiplayer_assign_controller(menu_controller, data->designs, 1);
            }

            if (menu_multiplayer_selector(&multi_menu, index++, data->active, select, &character_coords, character_dim, data->designs[2].controller, 
                                          char3, char3_hover, char3_select, char3_select_hover))
            {
                multiplayer_assign_controller(menu_controller, data->designs, 2);
            }

            if (menu_multiplayer_selector(&multi_menu, index++, data->active, select, &character_coords, character_dim, data->designs[3].controller, 
                                          char4, char4_hover, char4_select, char4_select_hover))
            {
                multiplayer_assign_controller(menu_controller, data->designs, 3);
            }

            multi_menu.rect.coords.y += multi_menu.bitmap.dim.y + multi_menu.padding.y;

            
            if (menu_button(&multi_menu, "Start", index++, data->active, select))
            {
                data->num_of_players = 0;
                for (u32 i = 0; i < 4; i++) if (data->designs[i].controller != 0) data->num_of_players++;

                if (data->num_of_players >= 2) 
                {
                    data->active = 0;
                    data->game_mode = COUNTDOWN;
                    data->count_start_time = app->time.run_time_s;
                    data->num_of_coffees = 1;
                    reset_all_coffee_cows(players, data->num_of_players);

                    // loading the controllers from the designs to the players array
                    // no spaces in the player array so can loop from 0 - num of players to update all

                    u32 design_index = 0;
                    for (u32 i = 0; i < data->num_of_players; i++)
                    {
                        while(data->designs[design_index].controller == 0) design_index++;
                        players[i].design_index = design_index; // change to = design_index when there is more than one
                        players[i].controller = data->designs[design_index].controller;
                        design_index++;
                    }

                    init_all_coffee_cows(players, data->num_of_players, data->designs, data->grid_dim);
                }
            }

            if (menu_button(&multi_menu, "Back", index++, data->active, select))
            {
                data->game_mode = MAIN_MENU;
                data->active = 0;
            }
        } break;

        case SETTINGS:
        {
            Bitmap *settings_menu_back = find_bitmap(&app->assets, "MAIN_MENU_BACK");
            b32 select = on_down(menu_controller->select);
            Menu settings_menu = data->default_menu;
            Rect bounds = get_centered_square(window_rect, settings_menu.window_percent);
            u32 index = 0;
            resize_menu(&settings_menu, window_rect, {(s32)settings_menu.button.dim.x, 0}, 0, 4);
            draw_rect(window_rect, settings_menu_back);

            const char *sounds_str = 0;
            if      (app->player.sound_volume == 0) sounds_str = "Unmute Sounds";
            else if (app->player.sound_volume != 0) sounds_str = "Mute Sounds";

            if (menu_button(&settings_menu, sounds_str, index++, data->active, select))
            {
                if      (app->player.sound_volume == 0) app->player.sound_volume = 0.5f;
                else if (app->player.sound_volume != 0) app->player.sound_volume = 0.0f;
            }

            const char *music_str = 0;
            if      (app->player.music_volume == 0) music_str = "Unmute Music";
            else if (app->player.music_volume != 0) music_str = "Mute Music";

            if (menu_button(&settings_menu, music_str, index++, data->active, select))
            {
                if      (app->player.music_volume == 0) app->player.music_volume = 0.5f;
                else if (app->player.music_volume != 0) app->player.music_volume = 0.0f;
            }

            if (menu_button(&settings_menu, "Toggle Fullscreen", index++, data->active, select))
            {
                data->fullscreen = !data->fullscreen;
                SDL_PauseAudioDevice(app->player.device_id, 1);
                if (data->fullscreen) SDL_SetWindowFullscreen(app->window.sdl, SDL_WINDOW_FULLSCREEN_DESKTOP);
                else                  SDL_SetWindowFullscreen(app->window.sdl, 0);
                SDL_PauseAudioDevice(app->player.device_id, 0);
            }

            if (menu_button(&settings_menu, "Back", index++, data->active, select))
            {
                data->game_mode = MAIN_MENU;
                data->active = 0;
            }
        } break;
        
        case IN_GAME:
        case COUNTDOWN:
        case RESULTS:
        case PAUSED:
        case GAME_OVER:
        {
            Bitmap *game_back = find_bitmap(&app->assets, "GAME_BACK");
            Bitmap *grass = find_bitmap(&app->assets, "GRASS");
            Bitmap *grid = find_bitmap(&app->assets, "GRID");
            Bitmap *rocks = find_bitmap(&app->assets, "ROCKS");
            
            draw_rect(window_rect, game_back);
            
            Rect rocks_rect = get_centered_square(window_rect, 0.95f);
            Rect grass_rect = get_centered_rect(rocks_rect, 0.75265f, 0.75265f);
            
            draw_rect(grass_rect, grass);
            
            v2 grid_size = grass_rect.dim / cv2(data->grid_dim);
            
            // drawing grid
            for (s32 i = 0; i < data->grid_dim.x; i++)
            {
                for (s32 j = 0; j < data->grid_dim.y; j++) 
                {
                    draw_rect({ grass_rect.coords.x + (i * grid_size.x), grass_rect.coords.y + (j * grid_size.y)}, 0, grid_size, grid);
                }
            }
            
            draw_rect(rocks_rect, rocks);

            if      (data->in_game_mode == SINGLEPLAYER) draw_singleplayer_scoreboard(rubik, window_dim, players[0].score, data->high_score);
            else if (data->in_game_mode == MULTIPLAYER)  draw_multiplayer_scoreboard(rubik, window_dim, data->players, data->num_of_players);

            for (u32 i = 0; i < data->num_of_players; i++) draw_coffee_cow_mouth(&players[i], grass_rect.coords, grid_size.x);
            for (u32 i = 0; i < data->num_of_coffees; i++) draw_rect(grass_rect.coords + (cv2(data->coffees[i].coords) * grid_size), DEG2RAD * data->coffees[i].rotation, grid_size, find_bitmap(&app->assets, "COFFEE"));
            draw_particles(&particles, &app->assets);
            for (u32 i = 0; i < data->num_of_players; i++) draw_coffee_cow(&players[i], grass_rect.coords, grid_size.x);
            //for (u32 i = 0; i < num_of_players; i++) draw_coffee_cow_debug(&players[i], grass_rect.coords, grid_size.x);
            
            if (data->game_mode == PAUSED || data->game_mode == GAME_OVER)
            {
                Menu pause_menu = data->default_menu;
                b32 select = on_down(menu_controller->select);
                
                pause_menu.button.back_color = { 0, 0, 0, 0.5f };
                pause_menu.button.active_back_color = { 0, 0, 0, 1.0f };
                pause_menu.button.text_color = { 255, 255, 255, 1 };
                pause_menu.button.active_text_color = { 255, 255, 255, 1 };
                
                pause_menu.window_percent = 0.6f;
                
                resize_menu(&pause_menu, window_rect, {1,1}, 0, 2);
                
                Rect back_rect = get_centered_rect_pad(pause_menu.rect, pause_menu.padding * 2);
                draw_rect(back_rect, { 0, 0, 0, 0.5f });

                u32 index = 0;
                
                if (menu_button(&pause_menu, "Play Again", index++, data->active, select))
                {
                    data->active = 0;
                    if (data->in_game_mode == SINGLEPLAYER)
                    {
                        data->game_mode = IN_GAME;
                    }
                    else if (data->in_game_mode == MULTIPLAYER) 
                    {
                        data->game_mode = COUNTDOWN;
                        data->count_start_time = app->time.run_time_s;
                    }
                    
                    init_all_coffee_cows(players, data->num_of_players, data->designs, data->grid_dim);   
                    random_coffee_locaton(&data->coffees[0].coords, data->grid_dim, players, data->num_of_players);                
                }
                
                if (menu_button(&pause_menu, "Menu", index++, data->active, select))
                {                    
                    data->active = 0;
                    data->game_mode = MAIN_MENU;
                    
                    init_all_coffee_cows(players, data->num_of_players, data->designs, data->grid_dim);
                    random_coffee_locaton(&data->coffees[0].coords, data->grid_dim, players, data->num_of_players);
                }
            }
            else if (data->game_mode == COUNTDOWN)
            {
                r32 pixel_height = window_dim.y * 0.2f;
                v4 color = { 0, 0, 0, 1 };
                char *countdown_str = u32_to_string(data->countdown);
                v2 countdown_dim = get_string_dim(rubik, countdown_str, pixel_height, color);
                v2 countdown_coords = { (window_dim.x / 2.0f) - (countdown_dim.x / 2.0f), (window_dim.y / 2.0f) + (countdown_dim.y / 2.0f) };
                draw_string(rubik, countdown_str, countdown_coords, pixel_height, color);
                SDL_free((void*)countdown_str);
            }
            else if (data->game_mode == RESULTS)
            {                
                r32 bit_dim_temp = window_dim.y * 0.2f;
                v2 bitmap_dim = { bit_dim_temp, bit_dim_temp };
                r32 pixel_height = window_dim.y * 0.2f;
                v4 color = { 255, 255, 255, 1 };

                if (data->winner)
                {
                    char *winner_str = "Wins";
                    v2 str_dim = get_string_dim(rubik, winner_str, pixel_height, color);

                    Rect rect = {};
                    rect.dim = { bitmap_dim.x + str_dim.x, pixel_height };
                    rect.coords = get_centered(rect, window_rect);
                    v2 str_coords = { rect.coords.x + bitmap_dim.x, rect.coords.y + (rect.dim.y/2.0f) + (str_dim.y/2.0f) };

                    draw_rect(get_centered_rect_pad(rect, cv2(window_dim) * 0.0179f * 2.0f), { 0, 0, 0, 0.5f });
                    draw_rect(rect.coords, 0, bitmap_dim, data->winner_outline);
                    draw_rect(rect.coords, 0, bitmap_dim, data->winner);       
                    draw_string(rubik, winner_str, str_coords, pixel_height, color);         
                }
                else
                {
                    char *tie_str = "Tie";
                    v2 str_dim = get_string_dim(rubik, tie_str, pixel_height, color);
                    Rect rect = {};
                    rect.dim = str_dim;
                    rect.coords = get_centered(rect, window_rect);
                    v2 str_coords = { rect.coords.x,  rect.coords.y + (rect.dim.y/2.0f) + (str_dim.y/2.0f) };
                    draw_rect(get_centered_rect_pad(rect, cv2(window_dim) * 0.0179f * 2.0f), { 0, 0, 0, 0.5f });
                    draw_string(rubik, tie_str, str_coords, pixel_height, color);    
                }
            }
        
        } break;
    }
    
    if (on_down(app->input.active_controller->show_fps)) data->show_fps = !data->show_fps;
    if (data->show_fps) draw_string(rubik, ftos(app->time.frames_per_s), { 100, 100 }, 50, { 255, 150, 0, 1 });

    return false;
}

function void
prepare_controller_for_input(Controller *controller)
{
    for (u32 j = 0; j < ARRAY_COUNT(controller->buttons); j++)
            controller->buttons[j].previous_state = controller->buttons[j].current_state;

    for (u32 j = 0; j < ARRAY_COUNT(controller->debug_buttons); j++)
            controller->debug_buttons[j].previous_state = controller->debug_buttons[j].current_state;            
}

function void
controller_process_input(Controller *controller, s32 id, b32 state)
{
    for (u32 i = 0; i < ARRAY_COUNT(controller->buttons); i++)
    {
        // loop through all ids associated with button
        for (u32 j = 0; j < controller->buttons[i].num_of_ids; j++)
        {
            if (id == controller->buttons[i].ids[j])
                controller->buttons[i].current_state = state;
        }
    }

    for (u32 i = 0; i < ARRAY_COUNT(controller->debug_buttons); i++)
    {
        // loop through all ids associated with button
        for (u32 j = 0; j < controller->debug_buttons[i].num_of_ids; j++)
        {
            if (id == controller->debug_buttons[i].ids[j])
                controller->debug_buttons[i].current_state = state;
        }
    }
}

function b32
process_input(v2s *window_dim, Input *input)
{
    for (u32 i = 0; i < input->num_of_controllers; i++) prepare_controller_for_input(&input->controllers[i]);
    
    SDL_Event event;
    while(SDL_PollEvent(&event))
    {
        switch(event.type)
        {
            case SDL_QUIT: return true;
            
            case SDL_WINDOWEVENT:
            {
                SDL_WindowEvent *window_event = &event.window;
                
                switch(window_event->event)
                {
                    case SDL_WINDOWEVENT_RESIZED:
                    case SDL_WINDOWEVENT_SIZE_CHANGED:
                    {
                        window_dim->width = window_event->data1;
                        window_dim->height = window_event->data2;
                        glViewport(0, 0, window_dim->width, window_dim->height);
                    } break;
                }
            } break;
            
            case SDL_KEYDOWN:
            case SDL_KEYUP:
            {
                input->active_controller = &input->controllers[0];
                SDL_KeyboardEvent *keyboard_event = &event.key;
                s32 key_id = keyboard_event->keysym.sym;
                b32 state = false;
                if (keyboard_event->state == SDL_PRESSED)
                    state = true;
                
                controller_process_input(&input->controllers[0], key_id, state);
            } break;
            
            case SDL_CONTROLLERBUTTONUP:
            case SDL_CONTROLLERBUTTONDOWN:
            {
                SDL_ControllerButtonEvent *button_event = &event.cbutton;
                for (u32 i = 0; i < input->num_of_joysticks; i++)
                {
                    if (SDL_JoystickInstanceID(input->joysticks[i]) == button_event->which)
                    {
                        s32 button_id = button_event->button;
                        b32 state = false;
                        if (button_event->state == SDL_PRESSED)
                            state = true;
                        
                        controller_process_input(&input->controllers[i + 1], button_id, state);
                        input->active_controller = &input->controllers[i + 1];
                    }
                }
            } break;

            case SDL_MOUSEBUTTONDOWN:
            {
                SDL_MouseButtonEvent *mouse_event = &event.button;
                add_particle(&particles, { (r32)mouse_event->x, (r32)mouse_event->y, 0.0f }, 0.0f, 100.0f);
            } break;
        }
    }
    
    return false;
}

function void
update_time(Time *time)
{
    u32 last_run_time_ms = time->run_time_ms;
    
    time->run_time_ms = SDL_GetTicks();
    time->run_time_s = (f32)time->run_time_ms / 1000.0f;
    time->frame_time_ms = time->run_time_ms - last_run_time_ms;
    time->frame_time_s = (f32)time->frame_time_ms / 1000.0f;
    
    // get fps
    time->frames_per_s = 1000.0f;
    if (time->frame_time_s > 0.0f) time->frames_per_s = 1.0f / time->frame_time_s;
}

function int
main_loop(Application *app)
{
    srand(SDL_GetTicks());

    init_controllers(&app->input);
    app->data = init_game_data(&app->assets);

    Bitmap *icon = find_bitmap(&app->assets, "ICON");
    SDL_Surface *icon_surface = SDL_CreateRGBSurfaceFrom(icon->memory, icon->dim.width, icon->dim.height, 32, icon->pitch, 0x00000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
    SDL_SetWindowIcon(app->window.sdl, icon_surface);

    //Particles particles = {};
    init_particles(&particles, 200);
    init_shapes();

    while(1)
    {
        if (process_input(&app->window.dim, &app->input)) return 0; // quit if input to quit
    
        set_orthographic_matrix(app->window.dim);
        
        update_time(&app->time);

        if (update(app)) return 0; // quit if update says to quit

        // Audio
        mix_audio(&app->player, app->time.frame_time_s);
        if (SDL_QueueAudio(app->player.device_id, app->player.buffer, app->player.length)) log("%s", SDL_GetError());

        SDL_memset(app->player.buffer, 0, app->player.max_length);
        swap_window(&app->window);
    }
}

function void
init_opengl(Window *window)
{
    SDL_GL_LoadLibrary(NULL);
    
    // Request an OpenGL 4.6 context (should be core)
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    
    // Also request a depth buffer
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    
    SDL_GLContext Context = SDL_GL_CreateContext(window->sdl);
    SDL_GL_SetSwapInterval(0);
    
    // Check OpenGL properties
    gladLoadGL((GLADloadfunc) SDL_GL_GetProcAddress);
    log("OpenGL loaded:");
    log("Vendor:   %s", glGetString(GL_VENDOR));
    log("Renderer: %s", glGetString(GL_RENDERER));
    log("Version:  %s", glGetString(GL_VERSION));
    
    SDL_GetWindowSize(window->sdl, &window->dim.width, &window->dim.height);
    glViewport(0, 0, window->dim.width, window->dim.height);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
}

function void
init_window(Window *window)
{
    u32 sdl_init_flags = 
        SDL_INIT_VIDEO | 
        SDL_INIT_GAMECONTROLLER | 
        SDL_INIT_AUDIO;
    
    u32 sdl_window_flags = 
        SDL_WINDOW_RESIZABLE | 
        SDL_WINDOW_OPENGL;
    
    SDL_Init(sdl_init_flags);

    window->sdl = SDL_CreateWindow("Coffee Cow", 
                                   SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                   900, 800, 
                                   sdl_window_flags);
    init_opengl(window);
    SDL_GetWindowSize(window->sdl, &window->dim.width, &window->dim.height);
}

function int
application()
{
    Application app = {};
    init_window(&app.window);
    //if (load_assets(&app.assets, "../assets.ethan")) return 1;
    //save_assets(&app.assets, "assets.save");
    if (load_saved_assets(&app.assets, "assets.save")) return 1;
    init_audio_player(&app.player);
    return main_loop(&app);
}

int main(int argc, char *argv[]) { return application(); }