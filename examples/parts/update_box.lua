--[[
              update_box.lua
       
  Read more about chunk updates in MANUAL.txt.
  This function updates all chunks in given box.

]]

function update_box(x1,y1,z1,x2,y2,z2)
  for x0 = x1-32,x2+32,16 do
    for y0 = y1-32,y2+32,16 do
      for z0 = z1-32,z2+32,16 do
        if x0>0 and y0>0 and z0>0 then
          update_chunk(math.floor(x0/32)*32,
                       math.floor(y0/32)*32,
                       math.floor(z0/32)*32)		
        end
      end
    end
  end
end