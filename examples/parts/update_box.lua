--[[
              update_box.lua
       
  Read more about chunk updates in MANUAL.txt.
  This function updates all chunks in given box.

]]

function update_box(x1,y1,z1,x2,y2,z2)
  for x = x1-32,x2+32,16 do
    for y = y1-32,y2+32,16 do
      for z = z1-32,z2+32,16 do
        update_chunk(x,y,z)
      end
    end
  end
end