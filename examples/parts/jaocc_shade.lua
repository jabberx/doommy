--[[
           jaocc_shade.lua
	
	This shading algorithm slow and has errors with thin objects, 
	but the effect is very smooth for count>2
	
    count - 1-... (the more the slower and smoother result)
	depth - from 0.01 to 1 (hardest)
	*size - level dimensions
]]

function clamp(val,min,max)
  if val<min then return min end
  if val>max then return max end
  return val
end

function jaocc_shade(count,depth,xsize,ysize,zsize)  
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
    handle_sdl_events()
    set_window_title("jaocc_shade "..(math.floor(x/xsize*100)).."%")
    for y=count,ysize-count-1 do
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
          dim = clamp((1.0-(clamp(dim,0,max_dim)/max_dim))*depth + (1-depth),0,1)
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