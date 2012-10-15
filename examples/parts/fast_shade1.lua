--[[
                    fast_shade1.lua
					   
  Fast shading algorithm (assumes vertical sun rays). 
  Works fine only with default palette or any palette with 
  compatible scheme (1-32 - from dark* to light*, 33-64 - 
  from dark* to light* etc).
  
  *size    - current level's dimensions                     

]]
function fast_shade1(xsize,ysize,zsize)
    for x=1,xsize-2 do
    handle_sdl_events()
    set_window_title("fast_shade1 "..(math.floor(x/xsize*100)).."%")
        for y=1,ysize-2 do
            for z=1,zsize-2 do
                voxel = get_voxel_pal(x,y,z)
				color_margin = math.floor((voxel-1)/32)*32 + 1
                if voxel ~= 0 then
                    if get_voxel_pal(x,y+1,z)>0 then voxel = voxel - 3 end                        
                    if get_voxel_pal(x+1,y+1,z)>0 then voxel = voxel - 2 end                    
                    if get_voxel_pal(x+1,y,z+1)>0 then voxel = voxel - 1 end
                    if get_voxel_pal(x-1,y,z-1)>0 then voxel = voxel - 1 end
					if voxel < color_margin then voxel = color_margin end
                    set_voxel_pal(x,y,z,voxel)
                end
            end
        end 
    end
end
