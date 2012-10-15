--[[
          draw_wire_box.lua

    w,h,d - width, height, depth of the box
	x,y,z - corner with lower coordinates
	r,g,b - color of lines
]]

function draw_wire_box(x,y,z,w,h,d,r,g,b)
  draw_3d_wire(x,y,z, x+w,y,z, r,g,b)
  draw_3d_wire(x,y,z, x,y+h,z, r,g,b)
  draw_3d_wire(x+w,y,z, x+w,y+h,z, r,g,b)
  draw_3d_wire(x,y+h,z, x+w,y+h,z, r,g,b)  
  draw_3d_wire(x,y,z+d, x+w,y,z+d, r,g,b)
  draw_3d_wire(x,y,z+d, x,y+h,z+d, r,g,b)
  draw_3d_wire(x+w,y,z+d, x+w,y+h,z+d, r,g,b)
  draw_3d_wire(x,y+h,z+d, x+w,y+h,z+d, r,g,b)  
  draw_3d_wire(x,y,z, x,y,z+d, r,g,b)
  draw_3d_wire(x+w,y,z, x+w,y,z+d, r,g,b)
  draw_3d_wire(x,y+h,z, x,y+h,z+d, r,g,b)
  draw_3d_wire(x+w,y+h,z, x+w,y+h,z+d, r,g,b)
end