--[[
                    draw_text.lua

    example using bimtap font located in parts/courier14.bmp
]]

font_txt = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890-=\\!@#$%^&*()_+|[]{};:'\",./<>?`~АБВГДЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЬЭЫЮЯабвгдеёжзийклмнопрстуфхцчшщъьэыюя "
char_w,char_h,char_m,char_cnt = 11,16,0,160
font_img = load_image('parts/courier14.bmp',1)

function draw_text(x,y,text)
  for i=1, #text do
    local ch = text:sub(i,i)
    for c=1, #font_txt do
      local fch = font_txt:sub(c,c)
      if fch == ch then
        if c<=char_cnt then
          draw_image_part(x+i*(char_w+char_m),y,(c-1)*char_w,0,char_w,char_h,font_img)
        end
        break
      end
    end
  end
end