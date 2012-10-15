--[[
               example4.lua
       
  This example uses generator from "parts"
  folder and allows to fly in generated
  level with W/S keys. 
  take_screenshot bound to F2 here.

  You are free to use any part of this code 
  in your programs.
]]

ESCAPE, KEY_W, KEY_S, F2 = 5, 66, 62, 98
xsize, ysize, zsize = 256, 128, 256
camx, camy, camz, yaw, pitch = xsize/2, ysize*0.65, zsize/2, 0, 0

dofile('parts/generator1.lua')

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
  load_empty_level(xsize,ysize,zsize)
  generator1(xsize,ysize,zsize)
  update_all_chunks()
  set_window_title("example4")
  set_cam_pos(camx,camy,camz)
  set_cam_yaw(yaw)
  set_cam_pitch(pitch)
  set_max_dist(256)
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
    if is_key_pressed(KEY_W)==1  then fly_forward(delta/10) end
    if is_key_pressed(KEY_S)==1  then fly_forward(-delta/10) end  
    if is_key_pressed(ESCAPE)==1 then break end
  if is_key_pressed(F2)==1 then 
    take_screenshot('screen_'..ticks..'.bmp')
    fake_key_state(F2,0)
  end
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