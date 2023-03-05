function u32
get_direction(v2s dir)
{
    if (dir == RIGHT_V) return RIGHT;
    else if (dir == UP_V) return UP;
    else if (dir == LEFT_V) return LEFT;
    else  return DOWN;
}

function void
add_node(Coffee_Cow *cow, v2s grid_coords)
{
    Coffee_Cow_Node *new_node = &cow->nodes[cow->num_of_nodes++];
    new_node->coords = grid_coords;
    new_node->direction = cow->direction;
    new_node->last_direction = cow->direction;
}

function b8
cc_check_bounds(Coffee_Cow *cow, v2s grid_dim)
{
    Coffee_Cow_Node *head_node = &cow->nodes[0];
    v2s head = head_node->coords;
    head += cow->direction;
    if (head.x < 0 || head.x > (grid_dim.x - 1))
        return false;
    else if (head.y < 0 || head.y > (grid_dim.y - 1))
        return false;
    
    for (u32 i = 1; i < cow->num_of_nodes; i++)
    {
        Coffee_Cow_Node *node = &cow->nodes[i];
        if (head == node->coords)
            return false;
    }
    
    return true;
}

function void
update_cc(Coffee_Cow *cow, r32 frame_time_s, v2s grid_dim)
{
    r32 speed = 7.0f;
    if (cow->transition + (speed * frame_time_s) >= 1.0f)
    {
        if (cow->num_of_inputs != 0)
        {
            cow->direction = cow->inputs[0];
            for (s32 i = 0; i < cow->num_of_inputs - 1; i++)
            {
                cow->inputs[i] = cow->inputs[i + 1];
            }
            cow->num_of_inputs--;
        }
        
        cow->first_input_of_transition = true;
        
        if (!cc_check_bounds(cow, grid_dim))
            return;
        
        cow->transition = (cow->transition + (speed * frame_time_s)) - 1.0f;
        
        cow->nodes[0].direction = cow->direction;
        
        for (u32 i = 0; i < cow->num_of_nodes; i++)
        {
            cow->nodes[i].coords += cow->nodes[i].direction;
            cow->nodes[i].last_direction = cow->nodes[i].direction;
            if (i != 0)
                cow->nodes[i].direction = cow->nodes[i - 1].last_direction;
        }
    }
    else
        cow->transition += speed * frame_time_s;
}

function void
cc_add_input(Coffee_Cow *cow, v2s dir)
{
    if (cow->first_input_of_transition)
    {
        cow->num_of_inputs = 0;
        cow->first_input_of_transition = false;
    }
    cow->inputs[cow->num_of_inputs++] = dir;
}

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



function void
draw_coffee_cow(Coffee_Cow *cow, v2 grid_coords, r32 grid_size)
{
    v2 grid_s = { grid_size, grid_size };
    
    s32 head_index = 0;
    s32 tail_index =  cow->num_of_nodes - 1;
    
    v2s grid_coords_last = { 0, 0 };
    v2 coords_of_last_cir = { 0, 0 };
    
    for (s32 o = 0; o < 2; o++)
    {
        v4 color = {};
        if (o == 0)
        {
            color = { 0, 0, 0, 1 };
        }
        else if (o == 1)
        {
            color = {255, 255, 255, 1};
        }
        
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
                    draw_rect(t_coords, rot, grid_s, &cow->bitmaps[ASSET_COW_HEAD_OUTLINE]);
                    draw_rect(rect, color);
                }
                else if (o == 1)
                {
                    Rect w = get_cc_body_rect(get_direction(point_dir), 0.9f, 1.0f, rect);
                    draw_rect(w, color);
                    draw_rect(t_coords, rot, grid_s, &cow->bitmaps[ASSET_COW_HEAD]);
                }
                
                coords_of_last_cir = coords_of_cir;
                grid_coords_last = node->coords;
            }
            else if (i == tail_index)
            {
                b32 add = true;
                
                v2 next_coords = get_center(Rect{coords, grid_s});
                v2 current_coords = get_center(Rect{t_coords, grid_s});
                
                v2 point = next_coords - current_coords;
                Rect gap = get_cc_outline_rect(point, next_coords, current_coords, grid_size);
                u32 last_dir = get_direction(node->last_direction);
                Rect w = get_cc_body_rect(last_dir, 0.9f, 1.0f, gap);
                
                if (o == 0)
                {
                    draw_rect(t_coords, 0, grid_s, &cow->bitmaps[ASSET_COW_CIRCLE_OUTLINE]);
                    draw_rect(coords, 0, grid_s, &cow->bitmaps[ASSET_COW_CIRCLE_OUTLINE]);
                    draw_rect(gap, color);
                }
                else if (o == 1)
                {
                    draw_rect(t_coords, 0, grid_s, &cow->bitmaps[ASSET_COW_CIRCLE]);
                    draw_rect(coords, 0, grid_s, &cow->bitmaps[ASSET_COW_CIRCLE]);
                    draw_rect(w, color);
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
                    draw_rect(coords, 0, grid_s, &cow->bitmaps[ASSET_COW_CIRCLE_OUTLINE]);
                    draw_rect(rect, color);
                }
                else if (o == 1)
                {
                    draw_rect(coords, 0, grid_s, &cow->bitmaps[ASSET_COW_CIRCLE]);
                    draw_rect(w, color);
                }
                
                coords_of_last_cir = coords_of_cir;
                grid_coords_last = node->coords;
            }
        }
    }
}