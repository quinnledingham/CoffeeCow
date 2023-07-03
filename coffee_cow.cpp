function void
add_node(Coffee_Cow *cow, v2s grid_coords)
{
    Coffee_Cow_Node *new_node = &cow->nodes[cow->num_of_nodes++];
    new_node->coords = grid_coords;
    new_node->direction = cow->direction;
    new_node->last_direction = cow->direction;
}

function void
add_node_to_end(Coffee_Cow *cow)
{
    Coffee_Cow_Node *tail = &cow->nodes[cow->num_of_nodes - 1];
    add_node(cow, cow->add_coords);
    Coffee_Cow_Node *new_node = &cow->nodes[cow->num_of_nodes - 1];
    new_node->direction = cow->add_direction;
    new_node->last_direction = cow->add_direction;
    new_node->max_transition = true;
    cow->add = false;
}

function s32
random(s32 lower, s32 upper)
{
    return lower + (rand() % (upper - lower));
}

function v2s
random_coords(v2s lower, v2s upper)
{
    v2s coords = {};
    coords.x = random(lower.x, upper.x);
    coords.y = random(lower.y, upper.y);
    return coords;
}

function b8
valid_cc_coords(v2s coords, Coffee_Cow *other)
{
    for (u32 i = 0; i < other->num_of_nodes; i++)
    {
        Coffee_Cow_Node *o = &other->nodes[i];
        if (o->coords == coords) return false;
    }
    return true;
}

function b8
valid_coords_all(v2s coords, Coffee_Cow *cows, u32 num_of_cows)
{
    for (u32 i = 0; i < num_of_cows; i++)
    {
        if (!valid_cc_coords(coords, &cows[i])) return false;
    }
    return true;
}

function void
random_cc_location(Coffee_Cow *player, Coffee_Cow *all, u32 num_of_cows, v2s grid_dim)
{
    b8 valid_location = false;
    v2s locations[4] = {};
    
    SDL_memset(player->nodes, 0, sizeof(Coffee_Cow_Node) * 400); 
    player->num_of_nodes = 0;
    
    while(!valid_location)
    {   
        locations[0].x = random(4, grid_dim.x - 4);
        locations[0].y = random(4, grid_dim.y - 4);
        valid_location = true;
        
        if (locations[0].x < grid_dim.x / 2.0f)
        {
            player->direction = RIGHT_V;
            locations[1] = locations[0] - v2s{ 1, 0 };
            locations[2] = locations[0] - v2s{ 2, 0 };
            locations[3] = locations[0] - v2s{ 3, 0 };
        }
        else
        {
            player->direction = LEFT_V;
            locations[1] = locations[0] + v2s{ 1, 0 };
            locations[2] = locations[0] + v2s{ 2, 0 };
            locations[3] = locations[0] + v2s{ 3, 0 };
        }
        
        for (u32 i = 0; i < num_of_cows; i++)
        {
            Coffee_Cow *other = &all[i];
            valid_cc_coords(locations[0], other);
            valid_cc_coords(locations[1], other);
            valid_cc_coords(locations[2], other);
            valid_cc_coords(locations[3], other);
        }
    }
    
    add_node(player, locations[0]);
    add_node(player, locations[1]);
    add_node(player, locations[2]);
    add_node(player, locations[3]);
}

function void
random_coffee_locaton(v2s *coffee, v2s grid_dim, Coffee_Cow *cows, u32 num_of_cows)
{
    do
    {
        *coffee = random_coords({ 0, 0 }, grid_dim);
    } while (!valid_coords_all(*coffee, cows, num_of_cows));
}

function void
init_cc(Coffee_Cow *cow, Coffee_Cow *cows, Coffee_Cow_Design design, Controller *controller, v2s grid_dim)
{
    *cow = {};
    cow->design = design;
    cow->controller = controller;
    random_cc_location(cow, cows, 0, grid_dim);
    cow->score = 0;
}

function b8
cc_check_on_cc(Coffee_Cow *cow, Coffee_Cow *other)
{
    for (u32 i = 0; i < cow->num_of_nodes; i++)
    {
        for (u32 j = 0; j < other->num_of_nodes; j++)
        {
            if (i == j) continue;
            
            Coffee_Cow_Node *c = &cow->nodes[i];
            Coffee_Cow_Node *o = &other->nodes[j];
            if (c->coords == o->coords)
                return false;
        }
    }
    return true;
}

function b8
coffee_cow_can_move(Coffee_Cow *cow, v2s grid_dim)
{
    Coffee_Cow_Node *head_node = &cow->nodes[0];
    v2s head = head_node->coords + cow->direction;
    
    // checking if it will hit the wall
    if      (head.x < 0 || head.x > (grid_dim.x - 1)) return false;
    else if (head.y < 0 || head.y > (grid_dim.y - 1)) return false;
    
    if (!valid_cc_coords(head, cow)) return false;
    
    return true;
}

function void
coffee_cows_on_coffee(Coffee_Cow *cows, u32 num_of_cows, Coffee *coffees, u32 num_of_coffees, v2s grid_dim)
{
    for (u32 i = 0; i < num_of_coffees; i++)
    {
        Coffee *coffee = &coffees[i];
        v2s *coffee_coords = &coffees[i].coords;
        
        for (u32 j = 0; j < num_of_cows; j++)
        {
            Coffee_Cow *cow = &cows[j];
            Coffee_Cow_Node *head = &cow->nodes[0];
            Coffee_Cow_Node *next_to_head = &cow->nodes[1];
            
            if (head->coords == *coffee_coords && !coffee->consumed)
            {
                coffee->consumed = true;
                cow->score++;
                
                // adding node to end of coffee
                Coffee_Cow_Node *tail = &cow->nodes[cow->num_of_nodes - 1];
                cow->add = true;   
                cow->add_coords = tail->coords;
                cow->add_direction = tail->direction;
            }
            else if (next_to_head->coords == *coffee_coords)
            {
                coffee->consumed = false;
                random_coffee_locaton(coffee_coords, grid_dim, cows, num_of_cows);
            }
        }
    }
}

function void
coffee_cow_add_input(Coffee_Cow *cow, v2s dir)
{
    cow->inputs[cow->num_of_inputs++] = dir;
}

function v2s
coffee_cow_next_direction(Coffee_Cow *cow)
{
    if (cow->num_of_inputs == 0) return cow->direction;

    v2s input = cow->inputs[0];
    for (s32 i = 0; i < cow->num_of_inputs - 1; i++) cow->inputs[i] = cow->inputs[i + 1]; // shift inputs
    cow->num_of_inputs--;
    return input;
}

function void
update_coffee_cow(Coffee_Cow *cow, r32 frame_time_s, v2s grid_dim)
{
    // read new inputs for cow
    Controller *controller = cow->controller;

    if (cow->num_of_inputs < 4)
    {
        v2s last_direction = {};
        if      (cow->num_of_inputs == 0) last_direction = cow->direction;
        else if (cow->num_of_inputs != 0) last_direction = cow->inputs[cow->num_of_inputs - 1];
            
        if (on_down(controller->right) && last_direction != LEFT_V  && last_direction != RIGHT_V) coffee_cow_add_input(cow, RIGHT_V);
        if (on_down(controller->up)    && last_direction != DOWN_V  && last_direction != UP_V)    coffee_cow_add_input(cow, UP_V);
        if (on_down(controller->left)  && last_direction != RIGHT_V && last_direction != LEFT_V)  coffee_cow_add_input(cow, LEFT_V);
        if (on_down(controller->down)  && last_direction != UP_V    && last_direction != DOWN_V)  coffee_cow_add_input(cow, DOWN_V);
    }

    r32 speed = 7.0f; // m/s
    r32 slight_boost_speed = speed + 1.0f; // for when inputs are stacked

    r32 next_transition                   = cow->transition + (speed * frame_time_s);
    r32 next_transition_with_slight_boost = cow->transition + (slight_boost_speed * frame_time_s);
    
    if (next_transition >= 1.0f) // changes cells
    { 
        cow->direction = coffee_cow_next_direction(cow);        
        if (!coffee_cow_can_move(cow, grid_dim)) cow->dead = true;

        if (cow->dead)return;
        cow->transition = next_transition - 1.0f;

        // update the direction and coords of all the nodes
        cow->nodes[0].direction = cow->direction;
        for (u32 i = 0; i < cow->num_of_nodes; i++)
        {
            cow->nodes[i].coords += cow->nodes[i].direction;
            cow->nodes[i].last_direction = cow->nodes[i].direction;
            cow->nodes[i].max_transition = false;
            if (i != 0) cow->nodes[i].direction = cow->nodes[i - 1].last_direction;
        }
        
        if (cow->add) add_node_to_end(cow);
    }
    else
    {
        if      (cow->num_of_inputs == 0) cow->transition = next_transition;
        else if (cow->num_of_inputs > 0)  cow->transition = next_transition_with_slight_boost;
    }
}

function void
update_coffee_cows(Coffee_Cow *cows, u32 num_of_cows, r32 frame_time_s, v2s grid_dim)
{
    for (u32 i = 0; i < num_of_cows; i++)
    {
        Coffee_Cow *cow = &cows[i];
        update_coffee_cow(cow, frame_time_s, grid_dim);
    }
}

function void
update_coffees(Coffee *coffees, u32 num_of_coffees)
{
    for (u32 i = 0; i < num_of_coffees; i++) 
    {
        coffees[i].rotation += 0.1f;
        if (coffees[i].rotation >= 360.0f)
            coffees[i].rotation -= 360.0f;
    }
}

//
// Drawing
//

function Rect
get_cc_body_rect(u32 dir, r32 min, r32 max, Rect og)
{
    Rect cow_node = {};
    if (dir == RIGHT) cow_node = get_centered_rect(og, max, min);
    else if (dir == UP) cow_node = get_centered_rect(og, min, max);
    else if (dir == LEFT) cow_node = get_centered_rect(og, max, min);
    else if (dir == DOWN) cow_node = get_centered_rect(og, min, max);
    return cow_node;
}

function Rect
get_cc_outline_rect(v2 point, v2 current, v2 last, r32 grid_size)
{
    Rect rect = {};
    if (point.x == 0 && point.y < 0)
    {
        rect.dim = { grid_size, -point.y };
        rect.coords.x = current.x - (grid_size/2.0f);
        rect.coords.y = current.y;
    }
    else if (point.x == 0 && point.y > 0)
    {
        rect.dim = { grid_size, point.y };
        rect.coords.x = last.x - (grid_size/2.0f);
        rect.coords.y = last.y;
    }
    else if (point.x > 0 && point.y == 0)
    {
        rect.dim = { point.x, grid_size };
        rect.coords.x = last.x;
        rect.coords.y = last.y - (grid_size / 2.0f);
    }
    else if (point.x < 0 && point.y == 0)
    {
        rect.dim = { -point.x, grid_size };
        rect.coords.x = current.x;
        rect.coords.y = current.y - (grid_size / 2.0f);
    }
    return rect;
}

function u32
get_direction(v2s dir)
{
    if (dir == RIGHT_V) return RIGHT;
    else if (dir == UP_V) return UP;
    else if (dir == LEFT_V) return LEFT;
    else  return DOWN;
}

function void
draw_coffee_cow(Coffee_Cow *cow, v2 grid_coords, r32 grid_size)
{
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    
    v2 grid_s = { grid_size, grid_size };
    
    s32 head_index = 0;
    s32 tail_index =  cow->num_of_nodes - 1;
    
    v2s grid_coords_last = { 0, 0 };
    v2 coords_of_last_cir = { 0, 0 };
    
    // o = 0 drawing the outline - first layer
    // o = 1 drawing the body - second layer
    for (s32 o = 0; o < 2; o++)
    {
        for (s32 i = tail_index; i >= 0; i--)
        {
            Coffee_Cow_Node *node = &cow->nodes[i];
            v2 coords = cv2(node->coords);
            coords *= grid_size;
            coords += grid_coords;
            v2 t_coords = coords - ((cv2(node->last_direction) * (1.0f - cow->transition)) * grid_size);
            
            u32 dir = get_direction(node->direction);
            u32 rev_dir = get_direction(node->direction * -1);
            
            if (i == head_index)
            {
                dir = get_direction(cow->direction);
                
                r32 rot = 0.0f; // DOWN
                switch(dir)
                {
                    case RIGHT: rot = DEG2RAD * 90.0f; break;
                    case UP: rot = DEG2RAD * 180.0f; break;
                    case LEFT: rot = DEG2RAD * 270.0f; break;
                    case DOWN: rot = 0.0f; break;
                }
                
                v2 coords_of_cir = get_center(Rect{t_coords, grid_s});
                v2 point = coords_of_cir - coords_of_last_cir;
                Rect rect = get_cc_outline_rect(point, coords_of_cir, coords_of_last_cir, grid_size);
                v2s point_dir = normalized(grid_coords_last - node->coords);
                
                if (o == 0) 
                {
                    draw_rect(t_coords, rot, grid_s, cow->design.bitmaps[ASSET_COW_HEAD_OUTLINE]);
                    draw_rect(rect, cow->design.outline_color);
                }
                else if (o == 1)
                {
                    Rect w = get_cc_body_rect(get_direction(point_dir), 0.9f, 1.0f, rect);
                    draw_rect(w, cow->design.color);
                    draw_rect(t_coords, rot, grid_s, cow->design.bitmaps[ASSET_COW_HEAD]);
                }
                
                coords_of_last_cir = coords_of_cir;
                grid_coords_last = node->coords;
            }
            else if (i == tail_index)
            {
                b32 add = true;
                v2 size = grid_s;

                v2 second_layer_coords = t_coords;
                v2 second_layer_size = size;              
                
                if (node->max_transition) 
                {
                    Rect new_tail = get_centered_square(Rect{t_coords, grid_s}, cow->transition);
                    second_layer_coords = new_tail.coords;
                    second_layer_size = size * cow->transition;          
                }

                v2 next_coords = get_center(Rect{coords, grid_s});
                v2 current_coords = get_center(Rect{t_coords, grid_s});
                
                v2 point = next_coords - current_coords;
                Rect gap = get_cc_outline_rect(point, next_coords, current_coords, grid_size);
                u32 last_dir = get_direction(node->last_direction);
                Rect w = get_cc_body_rect(last_dir, 0.9f, 1.0f, gap);     

                if (o == 0)
                {
                    draw_rect(t_coords, 0, size, cow->design.bitmaps[ASSET_COW_CIRCLE_OUTLINE]);
                    draw_rect(coords, 0, size, cow->design.bitmaps[ASSET_COW_CIRCLE_OUTLINE]);
                    draw_rect(gap, cow->design.outline_color);
                }
                else if (o == 1)
                {
                    draw_rect(second_layer_coords, 0, second_layer_size, cow->design.bitmaps[ASSET_COW_CIRCLE]);
                    draw_rect(coords, 0, size, cow->design.bitmaps[ASSET_COW_CIRCLE]);
                    draw_rect(w, cow->design.color);
                }
                
                coords_of_last_cir = next_coords;
                grid_coords_last = node->coords;
            }
            else if (cow->nodes[i + 1].direction != node->direction)
            {
                u32 n_dir = get_direction(cow->nodes[i + 1].last_direction);
                
                v2 coords_of_cir = get_center(Rect{coords, grid_s});
                v2 point = coords_of_cir - coords_of_last_cir;
                Rect rect = get_cc_outline_rect(point, coords_of_cir, coords_of_last_cir, grid_size);
                v2s point_dir = normalized(grid_coords_last - node->coords);
                Rect w = get_cc_body_rect(get_direction(point_dir), 0.9f, 1.0f, rect);
                
                if (o == 0)
                {
                    draw_rect(coords, 0, grid_s, cow->design.bitmaps[ASSET_COW_CIRCLE_OUTLINE]);
                    draw_rect(rect, cow->design.outline_color);
                }
                else if (o == 1)
                {
                    draw_rect(coords, 0, grid_s, cow->design.bitmaps[ASSET_COW_CIRCLE]);
                    draw_rect(w, cow->design.color);
                }
                
                coords_of_last_cir = coords_of_cir;
                grid_coords_last = node->coords;
            }
        }
    }
    
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

function v2
grid_coords_to_screen_coords(v2s coords, v2 coords_of_grid, r32 grid_size)
{
    v2 screen_coords = cv2(coords);
    screen_coords *= grid_size;
    screen_coords += coords_of_grid;
    return screen_coords;
}

function void
draw_coffee_cow_debug(Coffee_Cow *cow, v2 grid_coords, r32 grid_size)
{
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    v2 grid_s = { grid_size, grid_size };

    for (u32 i = 0; i < cow->num_of_nodes; i++)
    {
        Coffee_Cow_Node *node = &cow->nodes[i];
        v2 coords = cv2(node->coords);
        coords *= grid_size;
        coords += grid_coords;
        v4 color = { 255.0f, 0.0f, 0.0f, 1.0f};
        draw_rect(coords, 0.0f, grid_s / 5.0f, color);
    }

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

