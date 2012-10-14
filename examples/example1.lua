--[[
  This example generates a simple level,
  applies simple shading to it
  and allows to simulate walking 
  with collision detection, 
  but without jumps or normal gravity.

  You are free to use any part of this code 
  in your programs.
]]

ESCAPE, KEY_W, KEY_S, KEY_A, KEY_D = 5, 66, 62, 44, 47
xsize, ysize, zsize = 256, 128, 256
camx, camy, camz, yaw, pitch = 128, 90, 128, 0, 0
prevx,prevy,prevz = camx,camy,camz -- for collision detection
cam_h = 10                        --_//
prevdist = cam_h                  --_/
is_falling = false

function generate()
    load_empty_level(xsize,ysize,zsize)
    for x=0,xsize-1 do
    handle_sdl_events()
    set_window_title("Generating level "..math.floor(x/xsize*50).."%")
        for y=0,ysize-16 do
            for z=0,zsize-1 do    
                if y<64 or perlin_noise_3d(x/30.0, y/30.0, z/30.0)<=-0.25 then
                    set_voxel_pal(x, y, z, 255 - math.random(2))
                end
            end 
        end 
    end
end

function shade()
    for x=1,xsize-2 do
    handle_sdl_events()
    set_window_title("Shading level "..(math.floor(x/xsize*50)+50).."%")
        for y=1,ysize-16 do
            for z=1,zsize-2 do
                if y>62 then
                    voxel = get_voxel_pal(x,y,z)
                    if voxel ~= 0 then
                        if get_voxel_pal(x,y+1,z)>0 then voxel = voxel - 6 end                        
                        if get_voxel_pal(x+1,y+1,z)>0 then voxel = voxel - 2 end
                        if get_voxel_pal(x-1,y+1,z)>0 then voxel = voxel - 2 end
                        if get_voxel_pal(x+1,y+1,z+1)>0 then voxel = voxel - 2 end
                        if get_voxel_pal(x+1,y+1,z-1)>0 then voxel = voxel - 2 end                        
                        if get_voxel_pal(x+1,y,z+1)>0 then voxel = voxel - 1 end
                        if get_voxel_pal(x-1,y,z-1)>0 then voxel = voxel - 1 end
                        if get_voxel_pal(x+1,y,z-1)>0 then voxel = voxel - 1 end
                        if get_voxel_pal(x-1,y,z+1)>0 then voxel = voxel - 1 end
                        --[[ note, that we may occasionally step back to another tone (if using default palette)
                          in this case this won't occur, cause we know our color is in range 253-255 and
                          maximum fall is 6+2+2+2+2+1+1+1+1=18
                          but for other cases we need to check:
                          first, determine to which tone this index belongs to
                          second, compute its lower margin (for our case - 225)
                          then apply clamping function before next call:]]
                        set_voxel_pal(x,y,z,voxel)
                    end
                end
            end 
        end 
    end
end

function mouse_look()
    mx,my = get_mouse_xy()
    dx,dy = mx-320, my-240
    yaw,pitch = yaw-dx*0.25, pitch-dy*0.25    
    while yaw>360 do yaw=yaw-360 end
    while yaw<0 do yaw=yaw+360 end
    if pitch>90 then pitch=89 end
    if pitch<-90 then pitch=-89 end
    set_cam_yaw(yaw)
    set_cam_pitch(pitch)
    set_mouse_xy(320,240)
end

function distance(x,y,z, x2,y2,z2)
    return math.sqrt(math.pow(x-x2,2)+math.pow(y-y2,2)+math.pow(z-z2,2))
end

function clamp(val,min,max)
    if val<min then return min end
    if val>max then return max end
    return val
end

function down_ray(x,y,z,m)
    len,i = 0,0
    while len<m do
        i = get_voxel_pal(x,y,z)
        y = y - 1
        if i ~= 0 then break end
        len = len + 1
    end
    return x,y,z,i
end

function fall(speed)    
    result = true
    x,y,z,i = down_ray(camx,camy,camz,cam_h)
    if i==0 then -- air under feet, fall
        camy = camy - speed        
        set_cam_pos(camx,camy,camz)
        if (is_voxel_empty(camx+0.5,camy+0.5,camz+0.5)~=1) then 
            result = false 
        else
            is_falling = true
        end
    else -- probably stuck in something, computing new height:
        dist = distance(camx,camy,camz,x,y,z)
        if dist<prevdist then -- comparing with previous height
            if prevdist-dist>cam_h/3 then -- if the difference is too high:
                result = false -- we've hit something
            else -- difference isn't to high, probably it's just a stair step or small rock
                camy = camy + (cam_h-dist) -- restoring normal height
                set_cam_pos(camx,camy,camz)
                if (is_voxel_empty(camx+0.5,camy+0.5,camz+0.5)~=1) then 
                    result = false 
                end
            end
        end        
    end
    if result then prevdist = distance(camx,camy,camz,x,y,z) end
    return result
end

function move(speed) 
    is_falling = false
    prevx,prevy,prevz = camx,camy,camz
    camx = camx + speed*math.cos((yaw+90)*math.pi/180)
    if not fall(math.abs(speed)) then
        camx,camy,camz = prevx,prevy,prevz
        set_cam_pos(camx,camy,camz)
    end    
    prevx,prevy,prevz = camx,camy,camz
    camz = camz + speed*math.sin((yaw+90)*math.pi/180)
    if not fall(math.abs(speed)) then
        camx,camy,camz = prevx,prevy,prevz
    end
    set_cam_pos(camx,camy,camz)
    return distance(prevx,prevy,prevz, camx,camy,camz)
end

function init_game()
    generate()
    shade()
    update_all_chunks()
    set_window_title("doommy voxel engine")
    set_cam_pos(camx,camy,camz)
    set_cam_yaw(yaw)
    set_cam_pitch(pitch)
    set_max_dist(128)
    -- low_quality() -- uncomment for slow PC
end

function main()
    init_game()
    last_ticks,delta = get_tick_count(),16
    fps = 60
    diag = math.cos(45*math.pi/180)
    hide_sdl_cursor()
    bob_dir,bob_h,bob_min,bob_max,bob_speed = 1,0,-1,0.5,0.005
    while true do
        handle_sdl_events()    
        mouse_look()   
                
        is_falling = false
        walked_dist = 0
        
        -- check for diagonal motion and alter delta value
        if (is_key_pressed(KEY_S)==1 or is_key_pressed(KEY_W)==1) and
            (is_key_pressed(KEY_A)==1 or is_key_pressed(KEY_D)==1)
            then delta = delta*diag end
        
        -- check for back key and alter delta sign
        if is_key_pressed(KEY_S)==1  then delta = -delta end        
        
        -- f/b motion
        if is_key_pressed(KEY_W)==1 or is_key_pressed(KEY_S)==1
            then walked_dist = move(delta/30) end
        
        -- restoring delta sign
        if is_key_pressed(KEY_S)==1  then delta = -delta end
        
        -- check for left/right keys and alter yaw
        if is_key_pressed(KEY_A)==1  then yaw = yaw + 90 end
        if is_key_pressed(KEY_D)==1  then yaw = yaw - 90 end        
                
        -- l/r motion
        if is_key_pressed(KEY_A)==1 or is_key_pressed(KEY_D)==1
            then walked_dist = walked_dist + move(delta/30) end
        
        -- restoring yaw
        if is_key_pressed(KEY_A)==1  then yaw = yaw - 90 end
        if is_key_pressed(KEY_D)==1  then yaw = yaw + 90 end
        
        -- restoring delta value
        if (is_key_pressed(KEY_S)==1 or is_key_pressed(KEY_W)==1) and
            (is_key_pressed(KEY_A)==1 or is_key_pressed(KEY_D)==1)
            then delta = delta/diag end
        
        -- world margin
        camx=clamp(camx,1,xsize-2)
        camy=clamp(camy,1,ysize-2)
        camz=clamp(camz,1,zsize-2)
        
        -- quit if escape was pressed
        if is_key_pressed(ESCAPE)==1 then break end
        
        -- head bobbing
        bob_h = bob_h + (delta*bob_speed)*bob_dir
        if bob_dir==1 then
            if bob_h>bob_max then
                bob_h = bob_h - (delta*bob_speed)
                bob_dir = -1
            end
        else
            if bob_h<bob_min then
                bob_h = bob_h + (delta*bob_speed)
                bob_dir = 1
            end
        end
        
        if (not is_falling) and (walked_dist~=0) and 
            ((is_key_pressed(KEY_S)==1 or is_key_pressed(KEY_W)==1) or
            (is_key_pressed(KEY_A)==1 or is_key_pressed(KEY_D)==1)) then
                set_cam_pos(camx,camy+bob_h,camz)
        end
        
        -- drawing voxels
        draw_voxels()
                
        -- draw anything else:
        -- 
        
        -- swapping buffers
        swap_buffers()
        
        -- computing fps
        fps = fps*0.9 + (1000/delta)*0.1
        draw_fps(fps)              
        ticks = get_tick_count()
        delta =  ticks - last_ticks
        last_ticks = get_tick_count()
        if (delta<16) then sdl_delay(16-delta) end
    end
end

main()