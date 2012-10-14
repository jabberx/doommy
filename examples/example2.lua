--[[
  This example generates a simple level,
  applies JAOCC shading to it
  and allows to fly using W/S keys
  without collision detection.

  JAOCC algorithm is ugly and very slow, 
  so reduced level size is used.

  You are free to use any part of this code 
  in your programs.
]]

ESCAPE, KEY_W, KEY_S = 5, 66, 62
xsize, ysize, zsize = 128, 128, 128
camx, camy, camz, yaw, pitch = xsize/2, 90, zsize/2, 0, 0

function clamp(val,min,max)
    if val<min then return min end
    if val>max then return max end
    return val
end

function jaocc(count,ystart)    
    max_dim = 0    
    for xn=-count,count do
        for yn=-count,count do
            yk1,yk2 = 0,0
            if yn>0 then yk1 = 1 end
            if yn<0 then yk2 = 1 end
            for zn=-count,count do                
                max_dim = max_dim + 1 + yk1 - yk2
            end
        end
    end
    for x=count,xsize-count-1 do
        -- always do this in expensive loops so user can see progress and move the window:
         handle_sdl_events()
         set_window_title("JAOCC shading "..(math.floor(x/xsize*100)).."%")
        for y=ystart+count,ysize-count-1 do
            for z=count,zsize-count-1 do
                if (is_voxel_empty(x,y,z)~=1) then
                    dim = 0
                    for xn=-count,count do
                        for yn=-count,count do
                            yk1,yk2 = 0,0
                            if yn>0 then yk1 = 1 end
                            if yn<0 then yk2 = 1 end
                            for zn=-count,count do    
                                if is_voxel_empty(x+xn,y+yn,z+zn)~=1 then
                                    dim = dim + 1 + yk1 - yk2
                                end
                            end
                        end
                    end                    
                    dim = clamp((1.0-(clamp(dim,0,max_dim)/max_dim))/2 + 0.5,0,1)
                    r,g,b = get_voxel_rgb(x,y,z)
                    r = clamp(r*dim,0,255) 
                    g = clamp(g*dim,0,255) 
                    b = clamp(b*dim,0,255)
                    set_voxel_rgb(x,y,z,r,g,b)                    
                end
            end
        end
    end
end

function generate()
    load_empty_level(xsize,ysize,zsize)
    for x=0,xsize-1 do
    handle_sdl_events()
    set_window_title("Generating level "..math.floor(x/xsize*50).."%")
        for y=0,ysize-16 do
            for z=0,zsize-1 do    
                if y<64 or perlin_noise_3d(x/30.0, y/30.0, z/30.0)<=-0.25 then
                    set_voxel_pal(x, y, z, 255 - math.random(1))
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

function fly_forward(speed) -- no collision checks here
    camx = camx + speed*math.cos((yaw+90)*math.pi/180) * math.cos(pitch*math.pi/180)
    camy = camy + speed*math.sin(pitch*math.pi/180)
    camz = camz + speed*math.sin((yaw+90)*math.pi/180) * math.cos(pitch*math.pi/180)
    set_cam_pos(camx,camy,camz)
end

function init_game()
    generate()
    jaocc(3,60)    -- start from 60 (almost our ground level)
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
    hide_sdl_cursor()
    while true do
        handle_sdl_events()    
        mouse_look()    
        if is_key_pressed(KEY_W)==1  then fly_forward(delta/30) end
        if is_key_pressed(KEY_S)==1  then fly_forward(-delta/30) end    
        if is_key_pressed(ESCAPE)==1 then break end
        draw_voxels()
        fps = fps*0.9 + (1000/delta)*0.1
        draw_fps(fps)
        swap_buffers()        
        ticks = get_tick_count()
        delta =  ticks - last_ticks
        last_ticks = get_tick_count()
        if (delta<16) then sdl_delay(16-delta) end
    end
end

main()