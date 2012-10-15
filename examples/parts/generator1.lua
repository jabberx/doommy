--[[
               generator1.lua
       
  generator1 produces desert-like scene and
  applies shading and shadowing from "parts"

  You are free to use any part of this code 
  in your programs.
]]

dofile('parts/fast_shade1.lua')
dofile('parts/shadow1.lua')

function generator1(xsize,ysize,zsize)
  sca = 0.5
    for x=0,xsize-1 do
    handle_sdl_events()
    set_window_title("generator1 "..math.floor(x/xsize*50).."%")
        for z=0,zsize-1 do
      color = 255
      height = perlin_noise_3d(sca*x/30,1,sca*z/30)*16 -
           math.abs(perlin_noise_3d(sca*x/5,10,sca*z/5)*4)
      mountain = perlin_noise_3d(sca*x/15,77,sca*z/15)*8
      if height+mountain > 0 then
        height = height * 2
        color = 245
      end
      mod2 = perlin_noise_3d(sca*x/15,20,sca*z/15)
      if mod2>-0.1 then
        mod2 = 0
      end
      height = height + mod2*4 + 32
            for y=0,ysize-1 do    
                if math.abs(y-height)<4 then
          set_voxel_pal(x,y,z,color-math.random(2))
        end
            end 
        end 
    end  
  
  sca = 1
    for x=0,xsize-1 do
    handle_sdl_events()
    set_window_title("generator1 "..math.floor(x/xsize*50).."%")
        for y=0,48 do
            for z=0,zsize-1 do    
                if perlin_noise_3d(sca*x/30.0, sca*y/30.0, sca*z/30.0)<=-0.45 then
                    set_voxel_pal(x, y, z, 255 - math.random(1)) -- was 32
                end
            end 
        end 
    end
  
  fast_shade1(xsize,ysize,zsize)  
  shadow1(1000,4000,1000, 5, xsize,ysize,zsize)  
end