--[[
                       shade1.lua
					   
  Shadowing algorithm. Works fine only with default palette 
or any palette with compatible scheme (1-32 - from dark* to 
light*, 33-64 - from dark* to light* etc).

  lx,ly,lz - light position in world coordinates.
             Can be outside of level boundaries.
  depth    - shadow hardness (1 - 32)
  *size    - current level's dimensions                     

]]
function shadow1(lx,ly,lz,depth,xsize,ysize,zsize)
  for x=0,xsize-1 do
    handle_sdl_events()
    set_window_title("shadow1 "..math.floor(x/xsize*100).."%")
    for y=0,ysize-1 do
      for z=0,zsize-1 do
        voxel = get_voxel_pal(x,y,z)
        color_margin = math.floor((voxel-1)/32)*32 + 1
        if voxel~=0 then
          hx,hy,hz,i = throw_ray_to_pos(lx,ly,lz,x,y,z)        
          if i~=0 then
            voxel = voxel - depth
            if voxel < color_margin then voxel = color_margin end
            set_voxel_pal(x,y,z,voxel)
          end
        end
      end
    end
  end
end