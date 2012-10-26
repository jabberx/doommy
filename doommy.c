/*
  DOOMMY VOXEL ENGINE - voxel based game engine 
  which uses only CPU for rendering and Lua for scripting.

  COPYRIGHT (C) 2012 jabberx@ymail.com

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifdef WIN32
#include <windows.h>
#endif

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <SDL\SDL.h>
#include <SDL\SDL_thread.h>

#include <memory.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

SDL_Event event;
int MOUSEX, MOUSEY;
int KEY[128];
int BUTTON[5];
int exit_flag = 0;

enum {LB,MB,RB,WUP,WDOWN };

enum {BACKSPACE,TAB,CLEAR,ENTER,PAUSE,ESCAPE,SPACE,EXCLAIM,QUOTEDBL,
      HASH,DOLLAR,AMPERSAND,QUOTE,LEFTPAREN,RIGHTPAREN,ASTERISK,PLUS,
      COMMA,MINUS,PERIOD,SLASH,key_0,key_1,key_2,key_3,key_4,key_5,
      key_6,key_7,key_8,key_9,COLON,SEMICOLON,LESS,EQUALS,GREATER,
      QUESTION,AT,LEFTBRACKET,BACKSLASH,RIGHTBRACKET,CARET,UNDERSCORE,
      BACKQUOTE,key_a,key_b,key_c,key_d,key_e,key_f,key_g,key_h,key_i,
      key_j,key_k,key_l,key_m,key_n,key_o,key_p,key_q,key_r,key_s,
      key_t_,key_u,key_v,key_w,key_x,key_y,key_z,DEL,KP0,KP1,KP2,
      KP3,KP4,KP5,KP6,KP7,KP8,KP9,KP_PERIOD,KP_DIVIDE,KP_MULTIPLY,
      KP_MINUS,KP_PLUS,KP_ENTER,KP_EQUALS,UP,DOWN,RIGHT,LEFT,INSERT,
      HOME,END,PAGEUP,PAGEDOWN,F1,F2,F3,F4,F5,F6,F7,F8,F9,F10,
      F11,F12,F13,F14,F15,NUMLOCK,CAPSLOCK,SCROLLOCK,RSHIFT,LSHIFT,
      RCTRL,LCTRL,RALT,LALT };

void handle_events();

#define uchar unsigned char
#define uint unsigned int
#define bool int
#define cbool unsigned char
#define false 0
#define true 1
#define NEW(Type,Size) (Type*)malloc(Size*sizeof(Type))
#define inline __inline

#define asmRGB(R,G,B) SDL_MapRGB(screen->format, (R), (G), (B) )
#define getRGB(P,R,G,B) SDL_GetRGB((P),screen->format,&(R),&(G),&(B))

inline void pixel_set(int x, int y, uchar r, uchar g, uchar b);
void flip_scr(); long get_ticks(); void sleep_ms(long);
bool key_pressed(int); bool lmb_pressed(); bool rmb_pressed();
void set_cursor_pos(int, int); void get_cursor_pos(int *);
void show_cursor(); void hide_cursor();

uchar numbers[] = {
  1,1,1,1,0,1,1,0,1,1,0,1,1,1,1, 0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,
  1,1,1,0,0,1,1,1,1,1,0,0,1,1,1, 1,1,1,0,0,1,0,1,1,0,0,1,1,1,1,
  1,0,0,1,0,1,1,1,1,0,0,1,0,0,1, 1,1,1,1,0,0,1,1,1,0,0,1,1,1,1,
  1,1,1,1,0,0,1,1,1,1,0,1,1,1,1, 1,1,1,0,0,1,0,0,1,0,1,0,0,1,0,
  1,1,1,1,0,1,1,1,1,1,0,1,1,1,1, 1,1,1,1,0,1,1,1,1,0,0,1,1,1,1};

#define ABS(A) ((A)<=0?-(A):(A))
#define IS_POW_2(x) (((x) != 0) && !((x) & ((x) - 1)))
#define COPY3(D,S) memcpy((D),(S),3*sizeof(int))
#define CLAMP(A,B,C) (A)<=(B)?(A)=(B):0;(A)>(C)?(A)=(C):0;
#define SAT(A) (A)=(A)<0?0:(A);(A)=(A)>1?1:0
#define SIGN(A) ((A)>=0?1:-1)
#define NORM(A) {float l=1.0f/sqrtf(A.x*A.x+A.y*A.y+A.z*A.z); \
                                    A.x*=l;A.y*=l;A.z*=l;}
#define VEC_LEN(A) sqrtf(A.x*A.x+A.y*A.y+A.z*A.z)
#define DOT(A,B) (A).x*(B).x+(A).y*(B).y+(A).z*(B).z
#define VEC_SET(V,X,Y,Z) (V).x = (X); (V).y = (Y); (V).z = (Z)
#define RGB_SET(V,X,Y,Z) (V).r = (X); (V).g = (Y); (V).b = (Z)
#define DOTref(A,B) (A)->x*(B)->x+(A)->y*(B)->y+(A)->z*(B)->z
#define DIST2(A,B,C,D) (float)ABS(sqrtf(((D)-(B))*((D)-(B))+ \
                                         ((C)-(A))*((C)-(A))))
#define DISTxyz(X,Y,Z,XI,YI,ZI) sqrtf(powf((X)-(XI),2)+ \
                                powf((Y)-(YI),2)+powf((Z)-(ZI),2))
#define REPEAT(T,I) for(int(I)=0;(I)<(T);(I)++)

int XSIZE = 128, YSIZE = 128, ZSIZE = 128;

SDL_Surface *screen = NULL;
uint *img; int WIDTH=640, HEIGHT=480; const int FPS=98;

SDL_Rect scr_rect = {0,0,640,480 };
SDL_PixelFormat pif = {NULL, 32, 4, 0, 0, 8, 0, 0 };
SDL_Surface *fake_screen = NULL;

float *lbuffer;
const double DTR = 0.01745329238; const double RTD = 57.29578;
const double PI = 3.1415926535897932384626433832795;

uint desx,desy,desz;

struct camera {float x,y,z,yaw,pitch;};
struct camera cam = {0,0,0,0,0 };
struct vec3f {float x,y,z;};
struct rgb_color {uchar r,g,b;};

static uint seed=1000; inline void randomize(int s) {seed=s;}
inline int randm() {seed=(214013*seed+2531011); return((seed>>16)&0x7FFF);}

#define palette_i pal_i
#define palette_r pal_r
#define palette_g pal_g
#define palette_b pal_b

uchar pal_i[52*52*52]; uchar pal_r[256]; uchar pal_g[256]; uchar pal_b[256];

uchar default_pal[]={8,8,8,16,16,16,24,24,24,32,32,32,41,41,41,49,49,49,57,
57,57,65,65,65,74,74,74,82,82,82,90,90,90,98,98,98,106,106,106,115,115,115,
123,123,123,131,131,131,139,139,139,148,148,148,156,156,156,164,164,164,172,
172,172,180,180,180,189,189,189,197,197,197,205,205,205,213,213,213,222,222,
222,230,230,230,238,238,238,246,246,246,255,255,255,7,0,1,11,2,2,19,4,5,26,
7,8,33,10,12,41,12,14,48,15,17,56,17,20,63,20,23,71,22,26,77,26,29,86,29,33,
92,31,35,101,34,39,108,35,40,116,39,44,123,40,46,131,44,50,138,45,52,146,49,
56,152,51,58,160,55,62,168,57,70,175,60,73,183,62,79,190,65,74,195,70,79,200,
75,84,204,81,90,209,86,95,214,91,100,219,96,105,3,8,1,6,11,2,10,18,5,14,25,8,
20,31,12,24,39,14,28,45,18,33,52,21,38,59,24,43,66,27,46,72,31,53,80,35,57,86,
37,61,93,42,65,99,44,71,108,47,75,114,49,79,122,53,83,127,56,90,136,59,92,140,
63,97,149,66,106,155,70,113,162,73,115,169,76,118,176,79,120,182,83,127,188,87,
132,193,92,135,199,96,140,205,100,147,211,104,1,3,8,2,5,11,5,9,18,8,13,25,12,
18,31,14,21,39,18,26,45,21,30,52,24,35,59,27,39,66,31,43,72,35,49,80,37,52,86,
42,56,93,44,59,99,47,66,108,49,67,114,53,74,122,56,78,127,59,81,136,63,86,140,
66,92,149,70,98,155,73,102,162,76,107,169,79,106,176,83,111,182,87,118,188,92,
120,193,96,125,199,100,130,205,104,137,211,1,8,6,2,11,10,5,18,16,8,25,23,12,31,
29,14,39,36,18,45,41,21,52,48,24,59,54,27,66,61,31,72,68,35,80,75,37,86,80,42,
93,88,44,99,94,47,108,100,49,114,106,53,122,113,56,127,118,59,136,126,63,140,
132,66,149,138,70,155,140,73,162,147,76,169,153,79,176,166,83,182,172,87,188,
175,92,193,183,96,199,188,100,205,194,104,211,197,8,1,6,11,2,9,18,5,15,25,8,21,
31,12,26,39,14,33,45,18,38,52,21,45,59,24,50,66,27,56,72,31,62,80,35,70,86,37,
74,93,42,82,99,44,86,108,47,93,114,49,98,122,53,105,127,56,109,136,59,117,140,
63,123,149,66,130,155,70,140,162,73,147,169,76,151,176,79,152,182,83,158,188,
87,165,193,92,168,199,96,176,205,100,179,211,104,185,7,5,0,10,9,1,18,15,3,25,
22,6,32,30,9,41,35,10,48,42,13,55,48,16,63,56,18,70,64,21,77,69,24,86,76,27,
93,83,28,101,92,32,107,98,34,117,104,36,123,108,38,132,118,41,137,125,44,147,
129,46,152,134,49,161,142,52,169,148,54,176,155,57,184,162,59,191,168,62,197,
177,66,201,185,72,205,186,78,210,194,83,215,195,88,220,200,93,8,3,1,11,5,2,18,
9,5,25,13,8,31,18,12,39,22,14,45,26,18,52,30,21,59,35,24,66,38,27,72,43,31,80,
48,35,86,51,37,93,56,42,99,59,44,108,66,47,114,69,49,122,73,53,127,76,56,136,
81,59,140,86,63,149,90,66,155,90,70,162,94,73,169,98,76,176,109,79,182,111,
83,188,118,87,193,120,92,199,127,96,205,130,100,93,100,114};

uchar pal[255*3];

inline uchar rgb_to_8bit(uchar r, uchar g, uchar b) {
  r/=5; g/=5; b/=5; return pal_i[b+g*52+r*52*52]; }
void pal_init() {
  int i,r,g,b;
  for(i = 0; i<255; i++) {
    pal_r[i+1]=pal[i*3+0]; pal_g[i+1]=pal[i*3+1]; pal_b[i+1]=pal[i*3+2]; }
  for(r=0; r<52; r++) {
    for(g=0; g<52; g++) {
      for(b=0; b<52; b++) {
        uchar rr=r*5, gg=g*5, bb=b*5; int b_m = -1, m_d = 1000;
        for(i=0; i<255; i++) {
          int d=ABS(rr-pal_r[i+1])+ABS(gg-pal_g[i+1])+ABS(bb-pal_b[i+1]);
          if(d<m_d) {b_m=i+1; m_d=d;} } pal_i[b+g*52+r*52*52]=b_m; } } } }

// KEN PERLIN'S 3D NOISE. NOTHING TO ALTER HERE ANYMORE
int*p_p;float p_d(float t){return t*t*t*(t*(t*6-15)+10);}float p_l(float t,
float a,float b){return a+t*(b-a);}float p_g(int p_h,float x,float y,float z)
{int h=p_h&15;float u=h<8?x:y,v=h<4?y:h==12||h==14?x:z;return ((h&1)==0?u:-u)
+((h&2)==0?v:-v);}void noise_init(){int i;p_p=NEW(int,512);for (i=0;i<256;i++)
p_p[256+i]=p_p[i]=randm()%256;}float noise_3d(float x,float y,float z){int X=
(int)floor(x)&255,Y=(int)floor(y)&255,Z=(int)floor(z)&255;float u,v,w;int A,
AA,AB,B,BA,BB;x-=floor(x);y-=floor(y);z-=floor(z);u=p_d(x);v=p_d(y);w=p_d(z);
A=p_p[X]+Y; AA=p_p[A]+Z; AB=p_p[A+1]+Z;B=p_p[X+1]+Y; BA=p_p[B]+Z; BB=p_p[B+1]
+Z;return p_l(w,p_l(v,p_l(u,p_g(p_p[AA],x ,y ,z),p_g(p_p[BA],x-1,y ,z)),p_l(u,
p_g(p_p[AB],x ,y-1,z),p_g(p_p[BB],x-1,y-1,z))),p_l(v,p_l(u,p_g(p_p[AA+1],x,y,
z-1),p_g(p_p[BA+1],x-1,y ,z-1)),p_l(u,p_g(p_p[AB+1],x ,y-1,z-1),p_g(p_p[BB+1],
x-1,y-1,z-1))));}

// COMMON PROJECTION MATRIX ROUTINES. NOTHING TO ALTER HERE
int TX=3,TY=7,TZ=11,D0=0,D1=5,D2=10,D3=15,SX=0,SY=5,SZ=10,W=15;float cell[16],
t_cell[16];void identity(float *cell){cell[1]=cell[2]=cell[TX]=cell[4]=cell[6]
=cell[TY]=cell[8]=cell[9]=cell[TZ]=cell[12]=cell[13]=cell[14]=0;cell[D0]=cell
[D1]=cell[D2]=cell[W]=1;}void rotateX(float a_RX,float *cell){float sx=
(float)sin(a_RX*PI/180);float cx=(float)cos(a_RX*PI/180);identity(cell);cell[5]
=cx,cell[6]=sx,cell[9]=-sx,cell[10]=cx;}void rotateY(float a_RY,float *cell)
{float sy=(float)sin(a_RY*PI/180);float cy=(float)cos(a_RY*PI/180);identity
(cell);cell[0]=cy,cell[2]=-sy,cell[8]=sy,cell[10]=cy;}void rotateZ(float a_RZ,
float *cell){float sz=(float)sin(a_RZ*PI/180);float cz=(float)cos(a_RZ*PI/180);
identity(cell);cell[0]=cz,cell[1]=sz,cell[4]=-sz,cell[5]=cz;}void translate
(struct vec3f a_Pos,float *cell){cell[TX]+=a_Pos.x;cell[TY]+=a_Pos.y;cell[TZ]+=
a_Pos.z;}void concatenate(float *m2,float *cell){float res[16];int c,r;identity
(res);for(c=0;c<4;c++)for(r=0;r<4;r++)res[r*4+c]=cell[r*4]*m2[c]+cell[r*4+1]*m2
[c+4]+cell[r*4+2]*m2[c+8]+cell[r*4+3]*m2[c+12];for(c=0;c<16;c++)cell[c]=res[c];
}void rotate(struct vec3f a_Pos,float a_RX,float a_RY,float a_RZ){identity
(t_cell);rotateX(a_RZ,t_cell);rotateY(a_RY,cell);concatenate(t_cell,cell);
rotateZ(a_RX,t_cell);concatenate(t_cell,cell);translate(a_Pos,cell);}void 
transform(struct vec3f*v){float x,y,z;x=cell[0]*v->x+cell[1]*v->y+cell[2]*v->z+
cell[3];y=cell[4]*v->x+cell[5]*v->y+cell[6]*v->z+cell[7];z=cell[8]*v->x+cell[9]
*v->y+cell[10]*v->z+cell[11];v->x=x;v->y=y;v->z=z;}

inline struct rgb_color simple_shade(uchar r, uchar g, uchar b,
                                     struct vec3f *n, struct vec3f *l) {
  float lum = DOTref(n,l); struct rgb_color c = {r,g,b }; lum = (lum+1)/2.0f;
  c.r *= lum; c.g *= lum; c.b *= lum; return c; }

uchar *_1, *_2, *_4, *_8, *_16, *_32;
cbool *_32_upd;
uint xsize, ysize, zsize;
uint xsize32, ysize32, zsize32;
uint xsize16, ysize16, zsize16;
uint xsize8, ysize8, zsize8;
uint xsize4, ysize4, zsize4;
uint xsize2, ysize2, zsize2;

int level_init(uint xsize0, uint ysize0, uint zsize0) {
  xsize = xsize0;  ysize = ysize0;   zsize = zsize0;
  xsize32 = xsize/32; ysize32 = ysize/32; zsize32 = zsize/32;
  xsize16 = xsize/16; ysize16 = ysize/16; zsize16 = zsize/16;
  xsize8 = xsize/8;   ysize8 = ysize/8;   zsize8 = zsize/8;
  xsize4 = xsize/4;   ysize4 = ysize/4;   zsize4 = zsize/4;
  xsize2 = xsize/2;   ysize2 = ysize/2;   zsize2 = zsize/2;
  _1 = NEW(uchar,255*3+xsize*ysize*zsize*sizeof(uchar));
  memset(_1,  0, xsize*ysize*zsize);
  _2 = NEW(uchar, xsize*ysize*zsize/8);
  memset(_2,  0, xsize*ysize*zsize/8);
  _4 = NEW(uchar, xsize*ysize*zsize/64);
  memset(_4,  0, xsize*ysize*zsize/64);
  _8 = NEW(uchar, xsize*ysize*zsize/512);
  memset(_8,  0, xsize*ysize*zsize/512);
  _16 = NEW(uchar, xsize*ysize*zsize/4096);
  memset(_16, 0, xsize*ysize*zsize/4096);
  _32 = NEW(uchar, xsize*ysize*zsize/32768);
  memset(_32, 0, xsize*ysize*zsize/32768);
  _32_upd = NEW(cbool, xsize*ysize*zsize/32768);
  memset(_32_upd, 0, xsize*ysize*zsize/32768);
  return 0; }

void level_free() {
  free(_1); free(_2);
  free(_4); free(_8);
  free(_16); free(_32);
  free(_32_upd); }

bool load_level(const char *filename) {
  long len;
  FILE *fl = fopen(filename, "rb");
  if(!fl) return false;
  fseek(fl, 0, SEEK_END);
  len = ftell(fl);
  if(len!=XSIZE*YSIZE*ZSIZE && len!=XSIZE*YSIZE*ZSIZE+255*3) return false;
  fseek(fl, 0, SEEK_SET);
  fread(_1, 1, len, fl);
  if(len==XSIZE*YSIZE*ZSIZE+255*3) {
    int i;
    for(i=0; i<255*3; i++) {
      pal[i]=_1[XSIZE*YSIZE*ZSIZE+i]; }
    pal_init(); }
  fclose(fl);
  return true; }

bool save_level(const char *filename) {
  FILE *fl = fopen(filename, "wb");
  int i;
  for(i=0; i<255*3; i++) {
    _1[XSIZE*YSIZE*ZSIZE+i] = pal[i]; }
  fwrite(_1, 1, XSIZE*YSIZE*ZSIZE+255*3, fl);
  fclose(fl);
  return true; }

inline void _set(uint x, uint y, uint z, uchar val) {
_1[x*ysize+z*ysize*xsize+y]=val; }
inline uchar _get(uint x, uint y, uint z) {
return _1[x*ysize+z*ysize*xsize+y]; }
inline void _set2(uint x, uint y, uint z, uchar val) {
x/=2;y/=2;z/=2;_2[x*ysize2+z*ysize2*xsize2+y]=val; }
inline uchar _get2(uint x, uint y, uint z) {
x/=2;y/=2;z/=2;return _2[x*ysize2+z*ysize2*xsize2+y]; }
inline void _set4(uint x, uint y, uint z, uchar val) {
x/=4;y/=4;z/=4;_4[x*ysize4+z*ysize4*xsize4+y]=val; }
inline uchar _get4(uint x, uint y, uint z) {
x/=4;y/=4;z/=4;return _4[x*ysize4+z*ysize4*xsize4+y]; }
inline void _set8(uint x, uint y, uint z, uchar val) {
x/=8;y/=8;z/=8;_8[x*ysize8+z*ysize8*xsize8+y]=val; }
inline uchar _get8(uint x, uint y, uint z) {
x/=8;y/=8;z/=8;return _8[x*ysize8+z*ysize8*xsize8+y]; }
inline void _set16(uint x, uint y, uint z, uchar val) {
x/=16;y/=16;z/=16;_16[x*ysize16+z*ysize16*xsize16+y]=val; }
inline uchar _get16(uint x, uint y, uint z) {
x/=16;y/=16;z/=16;return _16[x*ysize16+z*ysize16*xsize16+y]; }
inline void _set32(uint x, uint y, uint z, uchar val) {
x/=32;y/=32;z/=32;_32[x*ysize32+z*ysize32*xsize32+y]=val; }
inline uchar _get32(uint x, uint y, uint z) {
x/=32;y/=32;z/=32;return _32[x*ysize32+z*ysize32*xsize32+y]; }
inline void _set32upd(uint x, uint y, uint z, cbool val) {
x/=32;y/=32;z/=32;_32_upd[x*ysize32+z*ysize32*xsize32+y]=val; }
inline cbool _get32upd(uint x, uint y, uint z) {
x/=32;y/=32;z/=32;return _32_upd[x*ysize32+z*ysize32*xsize32+y]; }

void lods_clear() {
  memset(_2, 0, xsize*ysize*zsize/8);
  memset(_4, 0, xsize*ysize*zsize/64);
  memset(_8, 0, xsize*ysize*zsize/512);
  memset(_16, 0, xsize*ysize*zsize/4096);
  memset(_32, 0, xsize*ysize*zsize/32768); }

#define CALC_LOD \
        v1 = _get(x,y,z); if (!v1) continue; \
        r = palette_r[v1]; g = palette_g[v1]; b = palette_b[v1]; \
        v2 = _get2(x,y,z); v4 = _get4(x,y,z); \
        v8 = _get8(x,y,z); v16 = _get16(x,y,z); \
        v32 = _get32(x,y,z); \
        if (!v2) v2 = v1; \
        rt = palette_r[v2]; gt = palette_g[v2]; bt = palette_b[v2]; \
        v2 = rgb_to_8bit((r+rt)/2,(g+gt)/2,(b+bt)/2); \
        CLAMP(v2,1,255); _set2(x,y,z,v2); \
        if (!v4) v4 = v1; \
        rt = palette_r[v4]; gt = palette_g[v4]; bt = palette_b[v4]; \
        v4 = rgb_to_8bit((r+rt)/2,(g+gt)/2,(b+bt)/2); \
        CLAMP(v4,1,255); _set4(x,y,z,v4); \
        if (!v8) { v8 = v1; _set8(x,y,z,v8);} \
        if (!v16) { v16 = v1; _set16(x,y,z,v16);} \
        if (!v32) { v32 = v1; _set32(x,y,z,v32);} 

void lods_fill(bool log) {
  int cnt=0; uint x,y,z;
  uchar v1,v2,v4,v8,v16,v32,r,g,b,rt,gt,bt;
  lods_clear();
  for(x = 0; x<xsize; x++) {
    if(log) {
      cnt++; if(cnt>5) {
        char buffer[256];
        float p=x; p/=xsize; p*=100; cnt=0;
        sprintf(buffer,"Updating chunks, please, wait... %.0f%c\0",p,'%');
        SDL_WM_SetCaption(buffer,NULL);
        handle_events(); sleep_ms(0); } }
    for(y = 0; y<ysize; y++) {
      for(z = 0; z<zsize; z++) {
        CALC_LOD } } } }

bool was_updates_on_frame = false;

void lod_cell_update(uint xc, uint yc, uint zc) {
  uint xs,ys,zs,xe,ye,ze, result;
  uint x,y,z; uchar v1,v2,v4,v8,v16,v32,r,g,b,rt,gt,bt;
  xc /= 32; xc *= 32; yc /= 32; yc *= 32; zc /= 32; zc *= 32;
  xs = xc; xe = xs + 32; ys = yc; ye = ys + 32; zs = zc; ze = zs + 32;
  xs<0?xs=0:0;  ys<0?ys=0:0;  zs<0?zs=0:0;
  was_updates_on_frame = true;
  xs>xsize-1?xs=xsize-1:0;
  ys>ysize-1?ys=ysize-1:0;
  zs>zsize-1?zs=zsize-1:0;
  xe>xsize?xe=xsize:0; ye>ysize?ye=ysize:0; ze>zsize?ze=zsize:0;
  if(_get32upd(xs,ys,zs)) return;
  _set32upd(xs,ys,zs,true);
  for(x = xs; x<xe; x++) {
    for(y = ys; y<ye; y++) {
      for(z = zs; z<ze; z++) {
        _set2(x,y,z,0); _set4(x,y,z,0); _set8(x,y,z,0);
        _set16(x,y,z,0); _set32(x,y,z,0); } } }
  for(x = xs; x<xe; x++) {
    for(y = ys; y<ye; y++) {
      for(z = zs; z<ze; z++) {
        CALC_LOD } } } }

// ROUGH ATTEMPT TO RECALCULATE ALL CHANGED LODS ACCORDING TO GIVEN B.BOX
// DEPRECATED, CAUSE IT FAILS FOR BIG BOXES!
void lods_update(uint x1, uint x2, uint y1, uint y2, uint z1, uint z2) {
  memset(_32_upd, false, xsize*ysize*zsize/32768);
  lod_cell_update(x1,y1,z1); lod_cell_update(x2,y1,z1);
  lod_cell_update(x1,y2,z1); lod_cell_update(x2,y2,z1);
  lod_cell_update(x1,y1,z2); lod_cell_update(x2,y1,z2);
  lod_cell_update(x1,y2,z2); lod_cell_update(x2,y2,z2); }
/*
BETTER USE SOMETHING LIKE THIS (MIGHT BE IMPLEMENTED IN YOUR GAME'S LUA CODE):
function update_box(x1,y1,z1,x2,y2,z2)
    for x = x1-32,x2+32,16 do
        for y = y1-32,y2+32,16 do
            for z = z1-32,z2+32,16 do
                update_chunk(x,y,z)
            end
        end
    end
end
*/

// DEPRECATED!
void destroy_block_rgb(int x, int y, int z, uchar r, uchar g, uchar b) {
  int _16 = 18, _8 = 9, digx, digy, digz; uint _x, _y, _z;
  struct vec3f norm; struct rgb_color c; uchar _i;
  struct vec3f light = {0,-1,0 };
  for(digx=0; digx<_16; digx++) {
    for(digy=0; digy<_16; digy++) {
      for(digz=0; digz<_16; digz++) {
        _x = x+digx-_8; _y = y+digy-_8; _z = z+digz-_8;
        if(_x>0&&_x<xsize-1&&_y>0&&_y<ysize-1&&_z>0&&_z<zsize-1) {
          float len = sqrtf(powf(digx-_8,2) +
                            powf(digy-_8,2)+powf(digz-_8,2));
          if(len<_8 + 0.5f) {
            _i = _get(_x,_y,_z); _set(_x,_y,_z,0);
            if(ceil(len)>=_8-1 && _i) {
              VEC_SET(norm, digx-_8,digy-_8,digz-_8); NORM(norm);
              _i = rgb_to_8bit(r-randm()%16,g-randm()%16,b-randm()%16);
              c = simple_shade(palette_r[_i],
                               palette_g[_i],
                               palette_b[_i],
                               &norm,&light);
              c.r = palette_r[_i]*0.66f + c.r*0.33f;
              c.g = palette_g[_i]*0.66f + c.g*0.33f;
              c.b = palette_b[_i]*0.66f + c.b*0.33f;
              _set(_x,_y,_z,rgb_to_8bit(c.r,c.g,c.b)); } } } } } }
  lods_update(x-_8,x+_8,y-_8,y+_8,z-_8,z+_8); }
void destroy_block(int x, int y, int z) {
  destroy_block_rgb(x,y,z,255,255,255); }

#define N_NEXT(X) div = divs[(X)]; \
          nx=pos.x+dddx[(X)]; ny=pos.y+dddy[(X)]; nz=pos.z+dddz[(X)]; \
          CLAMP(nx,0,x_1); CLAMP(ny,0,y_1); CLAMP(nz,0,z_1)
int MAX_DIST = 512;
uchar fogR=128, fogG=128, fogB=128;
int quality = 1;
float view_ray_x, view_ray_y, view_ray_z;
int movelace = 0;
bool lace = true;
bool checkers = true, glob_checker = true;

void ray_cast_thread(int startx, int endx) {
  float divs[] = {0.2,0.3,0.4,0.8,1.5,1.8 };
  float HQdiv2 = 7.5, HQdiv1 = 10, LQdiv2 = 1.5, LQdiv1 = 3;
  int x_1 = xsize-1, y_1 = ysize-1, z_1 = zsize-1;
  float LOD1 = 250-1*125, LOD2 = 500-1*250;
  int sx=0,sy=0; int xstep, ystep, txstep, tystep;
  struct vec3f pos0 = {0,0,0 };
  int checker = glob_checker;
  for(sx=0; sx<6; sx++) CLAMP(divs[sx],0.001f,4.0f);
  identity(cell); rotate(pos0,0,cam.yaw,cam.pitch);
  xstep = 2*quality;
  ystep = 2+(quality==2)*2;
  for(sx=startx; sx<endx-1; sx+=xstep) {
    checker = !checker;
    for(sy=0; sy<240; sy+=ystep) {
      float dddx[5], dddy[5], dddz[5], div = divs[5];
      float len=0, lim=MAX_DIST; bool pixelset = false;
      struct vec3f pos = {cam.x, cam.y, cam.z },
             dir = {sx-160, sy-120, 150 };
      if(checkers) {
        checker = !checker;
        if(checker) continue; }
      transform(&dir); NORM(dir);
      dddx[0] = dir.x/(divs[0]) + 0.5f;
      dddx[1] = dir.x/(divs[1]) + 0.5f; dddx[2] = dir.x/(divs[2]) + 0.5f;
      dddx[3] = dir.x/(divs[3]) + 0.5f; dddx[4] = dir.x/(divs[4]) + 0.5f;
      dddy[0] = dir.y/(divs[0]) + 0.5f;
      dddy[1] = dir.y/(divs[1]) + 0.5f; dddy[2] = dir.y/(divs[2]) + 0.5f;
      dddy[3] = dir.y/(divs[3]) + 0.5f; dddy[4] = dir.y/(divs[4]) + 0.5f;
      dddz[0] = dir.z/(divs[0]) + 0.5f;
      dddz[1] = dir.z/(divs[1]) + 0.5f; dddz[2] = dir.z/(divs[2]) + 0.5f;
      dddz[3] = dir.z/(divs[3]) + 0.5f; dddz[4] = dir.z/(divs[4]) + 0.5f;
      while(len<lim) {
        uchar next, i;
        int nx,ny,nz;
        int x = pos.x + 0.5f, y = pos.y + 0.5f, z = pos.z + 0.5f;
        len += 1.0f/div;
        if(x>=0 && x<xsize && y>=0 && y<ysize && z>=0 && z<zsize) {
          if(len<24) {
            divs[4] = HQdiv2; divs[5] = HQdiv1; }
          else {    divs[4] = LQdiv2; divs[5] = LQdiv1;}
          i = _get32(x,y,z);
          if(!i) {
            N_NEXT(0); next = _get32(nx,ny,nz);
            if(next) {
              N_NEXT(1); next = _get16(nx,ny,nz);
              if(next) {
                N_NEXT(2); next = _get8(nx,ny,nz);
                if(next) {
                  N_NEXT(3); next = _get4(nx,ny,nz);
                  if(next) {
                    N_NEXT(4); next=_get2(nx,ny,nz);
                    if(next) div = divs[5]; } } } } }
          else {
            i = _get16(x,y,z);
            if(!i) {
              N_NEXT(1); next = _get16(nx,ny,nz);
              if(next) {
                N_NEXT(2); next = _get8(nx,ny,nz);
                if(next) {
                  N_NEXT(3); next = _get4(nx,ny,nz);
                  if(next) {
                    N_NEXT(4); next=_get2(nx,ny,nz);
                    if(next) div = divs[5]; } } } }
            else {
              i = _get8(x,y,z);
              if(!i) {
                N_NEXT(2); next = _get8(nx,ny,nz);
                if(next) {
                  N_NEXT(3); next = _get4(nx,ny,nz);
                  if(next) {
                    N_NEXT(4); next=_get2(nx,ny,nz);
                    if(next) div = divs[5]; } } }
              else {
                i = _get4(x,y,z);
                if(!i) {
                  N_NEXT(3); next = _get4(nx,ny,nz);
                  if(next) {
                    N_NEXT(4); next=_get2(nx,ny,nz);
                    if(next) div = divs[5]; } }
                else {
                  i = _get2(x,y,z);
                  if(!i) {
                    N_NEXT(4);
                    if(len>LOD1 && len<=LOD2) {
                      next = _get2(nx,ny,nz);
                      if(next) div = divs[5];
                      goto skip1; } }
                  else {
                    if(len<=LOD1) {
                      i = _get(x,y,z);
                      div = divs[5]; } } } } } }
          skip1:
          if(i) {
            float fogpower; uchar r,g,b;
            if(len<=LOD1) {
              uchar ii = i;
              int cnt=0;
              while(ii==i) {
                float precision = 50;//40
                cnt++; if(cnt>precision) break;
                pos.x -= dir.x / precision;
                pos.y -= dir.y / precision;
                pos.z -= dir.z / precision;
                ii = _get(pos.x+0.5f,pos.y+0.5f,pos.z+0.5f); }
              if(ii) i = ii; }
            pixelset = true;
            r=palette_r[i]; g=palette_g[i]; b=palette_b[i];
            fogpower = ((float)len/(float)lim);
            b = b + (fogB - b)*fogpower;
            g = g + (fogG - g)*fogpower;
            r = r + (fogR - r)*fogpower;
            { int l=0,d=0;
              for(l=0; l<xstep*2; l++) {
                for(d=0; d<ystep*2; d++) {
                  pixel_set(sx*2+l,sy*2+d,r,g,b);
                  lbuffer[sx*2+l + (sy*2+d)*WIDTH] = len; } } }
            len=lim+1; }
            if (sx==160 && sy==120) {
              view_ray_x = dir.x;
              view_ray_y = dir.y;
              view_ray_z = dir.z;
              desx = x; desy = y; desz = z; }
          pos.x += dir.x / div;
          pos.y += dir.y / div;
          pos.z += dir.z / div; }
        else {
          len=lim+1; } } 
      if(!pixelset) {
        int l=0,d=0;
        for(l=0; l<xstep*2; l++) {
          for(d=0; d<ystep*2; d++) {
            pixel_set(sx*2+l,sy*2+d,fogR,fogG,fogB);
            lbuffer[sx*2+l + (sy*2+d)*WIDTH] = MAX_DIST; } } } } } }


void cast_ray_to_dir(float dirx, float diry, float dirz, 
              float posx, float posy, float posz, float max_d,
              uint *destx, uint *desty, uint *destz, uchar *destcol) {
  float len=0, lim=max_d; bool pixelset = false;
  float cox=0,coy=0,coz=1; int lax,lay,laz;
  struct vec3f pos = {posx, posy, posz },
               dir = {dirx, diry, dirz };
  pos.x += dir.x*2; pos.y += dir.y*2; pos.z += dir.z*2;
  dir.x /= 2.0f; dir.y /= 2.0f; dir.z /= 2.0f;
  if ((ABS(dir.x)>ABS(dir.y)) && (ABS(dir.x)>ABS(dir.z))){
    cox = -1*(SIGN(dir.x)); coy = 0; coz = 0; }
  if ((ABS(dir.y)>ABS(dir.z)) && (ABS(dir.y)>ABS(dir.x))){
    cox = 0; coy = -1*(SIGN(dir.y)); coz = 0; }
  if ((ABS(dir.z)>ABS(dir.y)) && (ABS(dir.z)>ABS(dir.x))){
    cox = 0; coy = 0; coz = -1*(SIGN(dir.z)); }
  lax = pos.x + 0.5f; lay = pos.y + 0.5f; laz = pos.z + 0.5f;
  while(len<lim) {
    uchar next, i, ii;
    int nx,ny,nz;
    int x = pos.x + 0.5f, y = pos.y + 0.5f, z = pos.z + 0.5f;
    len += 1;
    if(x>=1 && x<=xsize-2 && y>=1 && y<=ysize-2 && z>=1 && z<=zsize-2) {
    int changes = 0;
    i = _get(x,y,z);
    ii = i;
    changes = (x!=lax) + (y!=lay) + (z!=laz);
    if (changes>1){
      ii = _get(x+cox,y+coy,z+coz); }
      if(ii) {
        *destcol = ii;
        pixelset = true;
        len=lim+0.5f;
        *destx = x+cox; *desty = y+coy; *destz = z+coz; }
      if(i && !ii) {
        *destcol = i;
        pixelset = true;
        len=lim+0.5f;
        *destx = x; *desty = y; *destz = z; }
      pos.x += dir.x;
      pos.y += dir.y;
      pos.z += dir.z;
      lax = x; lay = y; laz = z; }
    else {
      len=lim+1; } }
  if(!pixelset) {
    *destx = dirx*max_d + posx;
    *desty = diry*max_d + posy;
    *destz = dirz*max_d + posz;
    *destcol = 0; } }

void cast_ray_to_pos(float lpx, float lpy, float lpz, 
              float posx, float posy, float posz,
              uint *destx, uint *desty, uint *destz, uchar *destcol) {
  float max_d=sqrtf(powf(lpx-posx,2)+powf(lpy-posy,2)+powf(lpz-posz,2));
  float len=0, lim=max_d; bool pixelset = false; 
  float cox=0,coy=0,coz=1; int lax,lay,laz;
  struct vec3f pos = {posx, posy, posz },
               dir = {lpx-posx, lpy-posy, lpz-posz };
  NORM(dir);
  pos.x += dir.x*2; pos.y += dir.y*2; pos.z += dir.z*2;   
  dir.x /= 2.0; dir.y /= 2.0; dir.z /= 2.0;   
  if ((ABS(dir.x)>ABS(dir.y)) && (ABS(dir.x)>ABS(dir.z))){
    cox = -1*(SIGN(dir.x)); coy = 0; coz = 0; }
  if ((ABS(dir.y)>ABS(dir.z)) && (ABS(dir.y)>ABS(dir.x))){
    cox = 0; coy = -1*(SIGN(dir.y)); coz = 0; }
  if ((ABS(dir.z)>ABS(dir.y)) && (ABS(dir.z)>ABS(dir.x))){
    cox = 0; coy = 0; coz = -1*(SIGN(dir.z)); }
  lax = pos.x + 0.5f; lay = pos.y + 0.5f; laz = pos.z + 0.5f;
  while(len<lim) {
    uchar next, i, ii;
    int nx,ny,nz;
    int x = pos.x + 0.5f, y = pos.y + 0.5f, z = pos.z + 0.5f;
    len += 0.5f;
    if(x>=1 && x<=xsize-2 && y>=1 && y<=ysize-2 && z>=1 && z<=zsize-2) {
  int changes = 0;  
    i = _get(x,y,z);
    ii = i;
    changes = (x!=lax) + (y!=lay) + (z!=laz);
    if (changes>1){
      ii = _get(x+cox,y+coy,z+coz);
    }
      if(ii) {
        *destcol = ii;
        pixelset = true;
        len=lim+0.5f;
        *destx = x+cox; *desty = y+coy; *destz = z+coz; }
      if(i && !ii) {
        *destcol = i;
        pixelset = true;
        len=lim+0.5f;
        *destx = x; *desty = y; *destz = z; }
      pos.x += dir.x;
      pos.y += dir.y;
      pos.z += dir.z;
      lax = x; lay = y; laz = z; }
    else {
      len=lim+1; } }
  if(!pixelset) {
    *destx = pos.x+0.5f;
    *desty = pos.y+0.5f;
    *destz = pos.z+0.5f;
    *destcol = 0; } }


int numCPU = 0;
void determ_CPU_count() {
#ifdef WIN32
  SYSTEM_INFO sysinfo;
  GetSystemInfo(&sysinfo);
  numCPU = sysinfo.dwNumberOfProcessors;
#else
  numCPU = get_nprocs();
#endif
}

int thr1(void *data) {ray_cast_thread(0,161); return 0;}
int thr2(void *data) {ray_cast_thread(160,320); return 0;}

void ray_cast_MT() {
  SDL_Thread *thread1 = SDL_CreateThread(thr1, NULL);
  SDL_Thread *thread2 = SDL_CreateThread(thr2, NULL);
  SDL_WaitThread(thread1, NULL);
  SDL_WaitThread(thread2, NULL); }

void ray_cast_ST() {
  ray_cast_thread(0,320); }

float cosyaw,cospitch,sinyaw,sinpitch;
void compute_cossins() {
  cosyaw = cos(cam.yaw*DTR);
  cospitch = cos(cam.pitch*DTR);
  sinyaw = sin(cam.yaw*DTR);
  sinpitch = sin(cam.pitch*DTR); }

float sprite_fov = 300.0f;

void camera_pos_draw();

SDL_Surface *image_load(const char *filename,  bool mTransp) {
  SDL_Surface *loadedImage = NULL;
  SDL_Surface *optimizedImage = NULL;
  loadedImage =  SDL_LoadBMP(filename);
  if(loadedImage != NULL) {
    if(!mTransp) optimizedImage =  SDL_DisplayFormatAlpha(loadedImage);
    if(mTransp) optimizedImage =  SDL_DisplayFormat(loadedImage);
    SDL_FreeSurface(loadedImage);
    if(optimizedImage != NULL) {
      Uint32 colorkey =  SDL_MapRGB(optimizedImage->format, 0xFF, 0,  0xFF);
      if(mTransp) {
        SDL_SetColorKey(optimizedImage, SDL_SRCCOLORKEY,  colorkey); } } }
        return optimizedImage; }

Uint32  getPixel(SDL_Surface *surface, int x, int y) {
  int bpp = surface->format->BytesPerPixel;
  Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;
  switch(bpp) {
  case 1: {
    return *p;
    break; } case 2: {
    return *(Uint16 *)p;
    break; } case 3: {
    if(SDL_BYTEORDER == SDL_BIG_ENDIAN) {
      return p[0] << 16 | p[1] << 8 | p[2]; }
    else {
      return p[0] | p[1] << 8 | p[2] << 16; } break; } case 4: {
    return *(Uint32 *)p;
    break; } default: {
    return 0; } } }

void  putPixel(SDL_Surface *surface, int x, int y,  Uint32 pixel) {
  int bpp = surface->format->BytesPerPixel;
  Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;
  switch(bpp) {
  case 1: {
    *p = pixel;
    break; }; case 2: {
    *(Uint16 *)p = pixel;
    break; }; case 3: {
    if(SDL_BYTEORDER == SDL_BIG_ENDIAN) {
      p[0] = (pixel >> 16) & 0xff;
      p[1] = (pixel >> 8) & 0xff;
      p[2] = pixel & 0xff; }
    else {
      p[0] = pixel & 0xff;
      p[1] = (pixel >> 8) & 0xff;
      p[2] = (pixel >> 16) & 0xff; } break; } case 4: {
    *(Uint32 *)p = pixel;
    break; } } }

void image_unload(SDL_Surface *i) {
  SDL_FreeSurface(i); }

SDL_Rect offset;
void image_draw(int x, int y, SDL_Surface *source) {
  offset.x = x;
  offset.y = y;
  SDL_BlitSurface(source, NULL, fake_screen, &offset); }

void  image_partly_draw(int x, int y, SDL_Surface *source, 
                        int clipx, int clipy, int clipw, int cliph) {
  SDL_Rect clip[1];
  clip[0].x = clipx;
  clip[0].y = clipy;
  clip[0].w = clipw;
  clip[0].h = cliph;
  offset.x = x; offset.y = y;
  SDL_BlitSurface(source, clip, fake_screen, &offset); }

void draw_line(int x1, int y1, int x2, int y2, Uint32 linecol);

void ray_cast() {
  glob_checker = !glob_checker;
  if(numCPU==0) determ_CPU_count();
  if(numCPU > 1) ray_cast_MT(); else ray_cast_ST();
  compute_cossins(); }

bool camera_collides() {
  int xx = cam.x + 0.5f, yy = cam.y + 0.5f, zz = cam.z + 0.5f, xm,ym,zm;
  bool retval = false;
  for(xm=-1; xm<2; xm++) {
    for(ym=-1; ym<2; ym++) {
      for(zm=-1; zm<2; zm++) {
        int x = xx+xm, y = yy+ym, z = zz+zm;
        if(x>0&&x<xsize-1&&y>0&&y<ysize-1&&z>0&&z<zsize-1) {
          if(_get(x,y,z)) {retval = true;   } }
        else {retval=true;} } } }
  return retval; }

void fps_counter_draw(float _fps) {
  int i,x,y,fps1,fps2,fps12[2],nux,nuy;
  uchar bit;
  if(_fps>99) _fps = 99;
  if(_fps<0) _fps = 0;
  for(x=0; x<8*2; x++) {
    for(y=0; y<12; y++) {
      pixel_set(x+3,HEIGHT-(y+3),0,0,0); }  }
  fps1 = _fps/10;
  fps2 = _fps-fps1*10;
  fps12[0] = fps1; fps12[1] = fps2;
  for(i=0; i<2; i++) {
    for(nux=0; nux<3; nux++) {
      for(nuy=0; nuy<5; nuy++) {
        bit = numbers[fps12[i]*15 + nux + nuy*3];
        if(bit) {
          pixel_set(4 + nux*2 + i*8, HEIGHT-(4 + nuy*2), 255, 255, 0);
          pixel_set(1+4 + nux*2 + i*8, HEIGHT-(4 + nuy*2), 255, 255, 0);
          pixel_set(4 + nux*2 + i*8, HEIGHT-(4 + nuy*2+1), 255, 255, 0);
          pixel_set(1+4 + nux*2 + i*8, HEIGHT-(4 + nuy*2+1),-1,-1,0); } } } } }

void camera_pos_draw() {
  int x,y,p,i,nux,nuy; uchar bit;
  int pp[] = {cam.x,cam.y,cam.z };
  for(i=0; i<3; i++)
    if(pp[i]>1000) pp[i]-=1000;
  for(x=24; x<90; x++) {for(y=0; y<7; y++) {pixel_set(x+3,HEIGHT-(y+3),0,0,0);}}
  for(p=0; p<3; p++) {
    int fps1 = pp[p]/100;
    int fps2 = pp[p]/10-fps1*10;
    int fps3 = pp[p] - fps1*100 - fps2*10;
    int fps12[] = {fps1,fps2,fps3 };
    for(i=0; i<3; i++) {
      for(nux=0; nux<3; nux++) {
        for(nuy=0; nuy<5; nuy++) {
          bit = numbers[fps12[i]*15 + nux + nuy*3];
          if(bit)
            pixel_set(32+24*p+nux+i*4,HEIGHT-(4 + nuy),255,255,0); } } } } }

bool need_to_free = false;

int engine_init(int xs, int ys, int zs, const char *filename) {
  XSIZE = xs; YSIZE = ys; ZSIZE = zs;
  if(need_to_free) level_free();
  level_init(XSIZE,YSIZE,ZSIZE);
  need_to_free = true;
  if(!load_level(filename)) return -1;
  lods_fill(true);
  return 0; }

int create_empty_level(int xs, int ys, int zs) {
  XSIZE = xs; YSIZE = ys; ZSIZE = zs;
  if(need_to_free) level_free();
  pal_init();
  level_init(XSIZE,YSIZE,ZSIZE);
  need_to_free = true;
  return 0; }

void flip_scr() {
  if(SDL_MUSTLOCK(screen)) if(SDL_LockSurface(screen)<0) return;
  memcpy(screen->pixels, img, WIDTH*HEIGHT*sizeof(int));
  if(SDL_MUSTLOCK(screen)) SDL_UnlockSurface(screen);
  SDL_Flip(screen);
  was_updates_on_frame = false; }

bool lmb_pressed() {return BUTTON[LB];} bool rmb_pressed() {return BUTTON[RB];}

bool key_pressed(int vKey) {return KEY[vKey];}

long get_ticks() {return SDL_GetTicks();}

void set_cursor_pos(int x,int y) {SDL_WarpMouse(x,y);}

void get_cursor_pos(int *pos) {pos[0]=MOUSEX; pos[1]=MOUSEY;}

void show_cursor() {SDL_ShowCursor(1);} void hide_cursor() {SDL_ShowCursor(0);}

void sleep_ms(long ms) {SDL_Delay(ms);}

inline void pixel_set(int x, int y, uchar r, uchar g, uchar b) {
  y = HEIGHT-1 - y;
  ((uchar *)img)[x*4 + y*WIDTH*4 + 0] = b;
  ((uchar *)img)[x*4 + y*WIDTH*4 + 1] = g;
  ((uchar *)img)[x*4 + y*WIDTH*4 + 2] = r; }

inline void pixel_set32(int x, int y, Uint32 col) {
  y = HEIGHT - y;
  img[x + y*WIDTH + 0] = col; }

void draw_line(int x1, int y1, int x2, int y2, Uint32 linecol) {
  int deltaX = ABS(x2 - x1);
  int deltaY = ABS(y2 - y1);
  int signX = x1 < x2 ? 1 : -1;
  int signY = y1 < y2 ? 1 : -1;
  int error2, error = deltaX - deltaY;
  while(true) {
    if(x1>0&&x1<screen->w&& y1>0&&y1<screen->h)
      pixel_set32(x1, y1, linecol);
    if(x1 == x2 && y1 == y2) break;
    error2 = error * 2;
    if(error2 > -deltaY) {error -= deltaY; x1 += signX;}
    if(error2 < deltaX) {error += deltaX; y1 += signY;} } }

void draw_circle(float x0, float y0, float radius, Uint32 linecol) {
  int x = 0;
  int y = radius;
  int delta = 2 - 2 * radius;
  int error = 0;
  while(y >= 0) {{{
        int cx = 0; int cy = 0;
        cx = x0+x; cy = y0+y;
        if(cx>0&&cx<screen->w&&cy>0&&cy<screen->h) {
          pixel_set32(x0 + x, y0 + y,linecol); }
        cx = x0+x; cy = y0-y;
        if(cx>0&&cx<screen->w&&cy>0&&cy<screen->h) {
          pixel_set32(x0 + x, y0 - y,linecol); }
        cx = x0-x; cy = y0+y;
        if(cx>0&&cx<screen->w&&cy>0&&cy<screen->h) {
          pixel_set32(x0 - x, y0 + y,linecol); }
        cx = x0-x; cy = y0-y;
        if(cx>0&&cx<screen->w&&cy>0&&cy<screen->h) {
          pixel_set32(x0 - x, y0 - y,linecol); }
        error = 2 * (delta + y) - 1;
        if(delta < 0 && error <= 0) {{{
              ++x; delta += 2 * x + 1; continue; } } }
        error = 2 * (delta - x) - 1;
        if(delta > 0 && error > 0) {{{ --y; delta += 1 - 2 * y; continue;}}}
        ++x; delta += 2 * (x - y); --y; } } } }

void pixel_draw_3d(float x, float y, float z, uchar r, uchar g, uchar b) {
  float tx,ty,tz; float newx,newz0,newy,newz; int sx,sy; int marg=0;
  tx=x-cam.x; ty=y-cam.y; tz=z-cam.z;
  newx = tx * cosyaw   + tz * sinyaw;
  newz0=-tx * sinyaw   + tz * cosyaw;
  newy = ty * cospitch - newz0 * sinpitch;
  newz = ty * sinpitch + newz0 * cospitch;
  if(newz>MAX_DIST || newz<0.05f) return;
  sx = WIDTH/2  + newx * sprite_fov/newz;
  sy = HEIGHT/2 + newy * sprite_fov/newz;
  if(sx<marg ||
      sx>WIDTH-marg-1 ||
      sy<marg ||
      sy>HEIGHT-marg-1) return;
  if(newz<lbuffer[sx+sy*WIDTH]) {
    lbuffer[sx+sy*WIDTH] = newz;
    pixel_set(sx,sy,r,g,b); } }

void quad_draw_3d(float x, float y, float z, uchar r, uchar g, uchar b, 
                  float scale_div, int max_size) {
  float tx,ty,tz; float newx,newz0,newy,newz; int sx,sy; int marg=0;
  tx=x-cam.x; ty=y-cam.y; tz=z-cam.z;
  newx = tx * cosyaw   + tz * sinyaw;
  newz0=-tx * sinyaw   + tz * cosyaw;
  newy = ty * cospitch - newz0 * sinpitch;
  newz = ty * sinpitch + newz0 * cospitch;
  if(newz>MAX_DIST || newz<0.05f) return;
  sx = WIDTH/2  + newx * sprite_fov/newz;
  sy = HEIGHT/2 + newy * sprite_fov/newz;
  marg = (MAX_DIST/newz)/scale_div;
  CLAMP(marg,1,max_size);
  if(sx<marg ||
      sx>WIDTH-marg-1 ||
      sy<marg ||
      sy>HEIGHT-marg-1) return;
  { int nx,ny;
    for(nx=-marg; nx<marg; nx++) {
      for(ny=-marg; ny<marg; ny++) {
        if(newz<lbuffer[sx+nx+(sy+ny)*WIDTH]) {
          lbuffer[sx+nx+(sy+ny)*WIDTH] = newz;
          pixel_set(sx+nx,sy+ny,r,g,b); } } } } }

void line_draw_3d(float x, float y, float z, 
                  float x2, float y2, float z2, uchar r, uchar g, uchar b) {
  float tx,ty,tz; float newx,newz0,newy,newz; int sx,sy,sx2,sy2;
  tx=x-cam.x; ty=y-cam.y; tz=z-cam.z;
  newx = tx * cosyaw   + tz * sinyaw;
  newz0=-tx * sinyaw   + tz * cosyaw;
  newy = ty * cospitch - newz0 * sinpitch;
  newz = ty * sinpitch + newz0 * cospitch;
  if(newz>MAX_DIST || newz<1) return;
  sx = WIDTH/2  + newx * sprite_fov/newz;
  sy = HEIGHT/2 + newy * sprite_fov/newz;
  tx=x2-cam.x; ty=y2-cam.y; tz=z2-cam.z;
  newx = tx * cosyaw   + tz * sinyaw;
  newz0=-tx * sinyaw   + tz * cosyaw;
  newy = ty * cospitch - newz0 * sinpitch;
  newz = ty * sinpitch + newz0 * cospitch;
  if(newz>MAX_DIST || newz<1) return;
  sx2 = WIDTH/2  + newx * sprite_fov/newz;
  sy2 = HEIGHT/2 + newy * sprite_fov/newz;
  draw_line(sx,sy,sx2,sy2,asmRGB(r,g,b)); }

lua_State *L;

static int setup_window(lua_State *L) {
  int n = lua_gettop(L);
  screen = SDL_SetVideoMode(WIDTH,HEIGHT,32,SDL_DOUBLEBUF);
  return 0; }

static int setup_fullscreen(lua_State *L) {
  int n = lua_gettop(L);
  screen = SDL_SetVideoMode(WIDTH,HEIGHT,32,SDL_DOUBLEBUF|SDL_FULLSCREEN);
  return 0; }

static int handle_sdl_events(lua_State *L) {
  int n = lua_gettop(L);
  handle_events();
  return 0; }

static int get_tick_count(lua_State *L) {
  int n = lua_gettop(L);
  lua_pushnumber(L, get_ticks());
  return 1; }

static int set_window_title(lua_State *L) {
  int n = lua_gettop(L);
  if(n!=1) return 0;
  SDL_WM_SetCaption(lua_tostring(L, 1),NULL);
  return 0; }

static int draw_fps(lua_State *L) {
  int n = lua_gettop(L);
  if(n!=1) return 0;
  fps_counter_draw(lua_tonumber(L, 1));
  return 0; }

static int draw_voxels(lua_State *L) {
  int n = lua_gettop(L);
  ray_cast();
  return 0; }

static int draw_camera_pos(lua_State *L) {
  int n = lua_gettop(L);
  camera_pos_draw();
  return 0; }

static int sdl_delay(lua_State *L) {
  int n = lua_gettop(L);
  if(n!=1) return 0;
  sleep_ms(lua_tonumber(L, 1));
  return 0; }

static int get_mouse_xy(lua_State *L) {
  int n = lua_gettop(L);
  int mouse[2];
  get_cursor_pos(mouse);
  lua_pushnumber(L, mouse[0]);
  lua_pushnumber(L, mouse[1]);
  return 2; }

static int set_mouse_xy(lua_State *L) {
  int n = lua_gettop(L);
  if(n!=2) return 0;
  set_cursor_pos(lua_tonumber(L, 1),lua_tonumber(L, 2));
  return 0; }

static int set_cam_yaw(lua_State *L) {
  int n = lua_gettop(L);
  if(n!=1) return 0;
  cam.yaw = lua_tonumber(L, 1);
  return 0; }

static int set_cam_pitch(lua_State *L) {
  int n = lua_gettop(L);
  if(n!=1) return 0;
  cam.pitch = lua_tonumber(L, 1);
  return 0; }

static int set_cam_pos(lua_State *L) {
  int n = lua_gettop(L);
  if(n!=3) return 0;
  cam.x = lua_tonumber(L, 1);
  cam.y = lua_tonumber(L, 2);
  cam.z = lua_tonumber(L, 3);
  return 0; }

static int cam_collides(lua_State *L) {
  int n = lua_gettop(L);
  lua_pushnumber(L,camera_collides());
  return 1; }

static int get_pointed_voxel_pos(lua_State *L) {
  int n = lua_gettop(L);
  lua_pushnumber(L,desx);
  lua_pushnumber(L,desy);
  lua_pushnumber(L,desz);
  return 3; }

static int is_voxel_empty(lua_State *L) {
  int n = lua_gettop(L); int x,y,z;
  if(n!=3) return 0;
  x = lua_tonumber(L, 1);
  y = lua_tonumber(L, 2);
  z = lua_tonumber(L, 3);
  lua_pushnumber(L,(_get(x,y,z)==0?1:0));
  return 1; }

static int erase_voxel(lua_State *L) {
  int n = lua_gettop(L); int x,y,z;
  if(n!=3) return 0;
  x = lua_tonumber(L, 1);
  y = lua_tonumber(L, 2);
  z = lua_tonumber(L, 3);
  _set(x,y,z,0);
  return 0; }

static int get_voxel_pal(lua_State *L) {
  int n = lua_gettop(L); int x,y,z;
  if(n!=3) return 0;
  x = lua_tonumber(L, 1);
  y = lua_tonumber(L, 2);
  z = lua_tonumber(L, 3);
  lua_pushnumber(L,_get(x,y,z));
  return 1; }

static int get_voxel_rgb(lua_State *L) {
  int n = lua_gettop(L); int x,y,z; uchar i;
  if(n!=3) return 0;
  x = lua_tonumber(L, 1);
  y = lua_tonumber(L, 2);
  z = lua_tonumber(L, 3);
  i = _get(x,y,z);
  lua_pushnumber(L,pal_r[i]);
  lua_pushnumber(L,pal_g[i]);
  lua_pushnumber(L,pal_b[i]);
  return 3; }

static int set_voxel_pal(lua_State *L) {
  int n = lua_gettop(L); int x,y,z; uchar i;
  if(n!=4) return 0;
  x = lua_tonumber(L, 1);
  y = lua_tonumber(L, 2);
  z = lua_tonumber(L, 3);
  i = lua_tonumber(L, 4);
  _set(x,y,z,i);
  return 0; }

static int set_voxel_rgb(lua_State *L) {
  int n = lua_gettop(L); int x,y,z; uchar r,g,b;
  if(n!=6) return 0;
  x = lua_tonumber(L, 1);
  y = lua_tonumber(L, 2);
  z = lua_tonumber(L, 3);
  r = lua_tonumber(L, 4);
  g = lua_tonumber(L, 5);
  b = lua_tonumber(L, 6);
  _set(x,y,z,rgb_to_8bit(r,g,b));
  return 0; }

static int get_view_ray(lua_State *L) {
  int n = lua_gettop(L);
  lua_pushnumber(L,view_ray_x);
  lua_pushnumber(L,view_ray_y);
  lua_pushnumber(L,view_ray_z);
  return 3; }

static int announce_chunk_updates(lua_State *L) {
  int n = lua_gettop(L);
  memset(_32_upd, false, xsize*ysize*zsize/32768);
  return 0; }

static int update_chunk(lua_State *L) {
  int n = lua_gettop(L); int x,y,z;
  if(n!=3) return 0;
  x = lua_tonumber(L, 1);
  y = lua_tonumber(L, 2);
  z = lua_tonumber(L, 3);
  lod_cell_update(x,y,z);
  return 0; }

static int is_key_pressed(lua_State *L) {
  int n = lua_gettop(L); int i;
  if(n!=1) return 0;
  i = lua_tonumber(L, 1);
  lua_pushnumber(L,KEY[i]);
  return 1; }

static int fake_key_state(lua_State *L) {
  int n = lua_gettop(L); int i,s;
  if(n!=2) return 0;
  i = lua_tonumber(L, 1);
  s = lua_tonumber(L, 2);
  KEY[i] = s;
  return 0; }

static int is_button_pressed(lua_State *L) {
  int n = lua_gettop(L); int i;
  if(n!=1) return 0;
  i = lua_tonumber(L, 1);
  lua_pushnumber(L,BUTTON[i]);
  return 1; }

static int fake_button_state(lua_State *L) {
  int n = lua_gettop(L); int i,s;
  if(n!=2) return 0;
  i = lua_tonumber(L, 1);
  s = lua_tonumber(L, 2);
  BUTTON[i] = s;
  return 0; }

static int show_sdl_cursor(lua_State *L) {
  int n = lua_gettop(L);
  show_cursor();
  return 0; }

static int hide_sdl_cursor(lua_State *L) {
  int n = lua_gettop(L);
  hide_cursor();
  return 0; }

static int screen_set_pixel(lua_State *L) {
  int n = lua_gettop(L); int x,y,r,g,b;
  if(n!=5) return 0;
  x = lua_tonumber(L, 1);
  y = lua_tonumber(L, 2);
  r = lua_tonumber(L, 3);
  g = lua_tonumber(L, 4);
  b = lua_tonumber(L, 5);
  pixel_set(x, y, r, g, b);
  return 0; }

static int screen_set_pixel32(lua_State *L) {
  int n = lua_gettop(L); Uint32 x,y,col;
  if(n!=3) return 0;
  x = lua_tonumber(L, 1);
  y = lua_tonumber(L, 2);
  col = lua_tonumber(L, 3);
  pixel_set32(x, y, col);
  return 0; }

static int low_quality(lua_State *L) {
  int n = lua_gettop(L);
  quality = 2;
  return 0; }

static int enable_checkers(lua_State *L) {
  int n = lua_gettop(L);
  return 0; }

static int disable_checkers(lua_State *L) {
  int n = lua_gettop(L);
  return 0; }

static int force_single_thread(lua_State *L) {
  int n = lua_gettop(L);
  numCPU = 1;
  return 0; }

static int med_quality(lua_State *L) {
  int n = lua_gettop(L);
  quality = 1;
  return 0; }

static int screen_clear(lua_State *L) {
  int n = lua_gettop(L); int r,g,b;
  if(n!=3) return 0;
  r = lua_tonumber(L, 1);
  g = lua_tonumber(L, 2);
  b = lua_tonumber(L, 3);
  SDL_FillRect(fake_screen,&fake_screen->clip_rect,asmRGB(r,g,b));
  return 0; }

static int screen_fill_rect(lua_State *L) {
  int n = lua_gettop(L); int x,y,w,h,r,g,b;
  SDL_Rect rect;
  if(n!=7 && n!=5) return 0;
  x = lua_tonumber(L, 1);
  y = lua_tonumber(L, 2);
  w = lua_tonumber(L, 3);
  h = lua_tonumber(L, 4);
  rect.x = x; rect.y = y; rect.w = w; rect.h = h;
  r = lua_tonumber(L, 5);
  if(n==7) {
    g = lua_tonumber(L, 6);
    b = lua_tonumber(L, 7);
    SDL_FillRect(fake_screen,&rect,asmRGB(r,g,b)); }
  else {
    SDL_FillRect(fake_screen,&rect,r); }
  return 0; }

static int screen_draw_circle(lua_State *L) {
  int n = lua_gettop(L); int x,y,radius,r,g,b;
  if(n!=6 && n!=4) return 0;
  x = lua_tonumber(L, 1);
  y = lua_tonumber(L, 2);
  radius = lua_tonumber(L, 3);
  r = lua_tonumber(L, 4);
  if(n==6) {
    g = lua_tonumber(L, 5);
    b = lua_tonumber(L, 6);
    draw_circle(x,y,radius,asmRGB(r,g,b)); }
  else {
    draw_circle(x,y,radius,r); }
  return 0; }

static int screen_draw_line(lua_State *L) {
  int n = lua_gettop(L); int x,y,x2,y2,r,g,b;
  if(n!=7 && n!=5) return 0;
  x = lua_tonumber(L, 1);
  y = lua_tonumber(L, 2);
  x2 = lua_tonumber(L, 3);
  y2 = lua_tonumber(L, 4);
  r = lua_tonumber(L, 5);
  if(n==7) {
    g = lua_tonumber(L, 5);
    b = lua_tonumber(L, 6);
    draw_line(x,y,x2,y2,asmRGB(r,g,b)); }
  else {
    draw_line(x,y,x2,y2,r); }
  return 0; }

static int load_level_from_file(lua_State *L) {
  int n = lua_gettop(L); int x,y,z; int res;
  if(n!=4) return 0;
  x = lua_tonumber(L, 1);
  y = lua_tonumber(L, 2);
  z = lua_tonumber(L, 3);
  res = engine_init(x,y,z, lua_tostring(L, 4));
  lua_pushnumber(L,res);
  return 1; }

static int swap_buffers(lua_State *L) {
  int n = lua_gettop(L);
  flip_scr();
  return 0; }

static int set_palette(lua_State *L) {
  int n = lua_gettop(L); uchar i,r,g,b;
  if(n!=4) return 0;
  i = lua_tonumber(L, 1);
  r = lua_tonumber(L, 2);
  g = lua_tonumber(L, 3);
  b = lua_tonumber(L, 4);
  if(i==0) return 0;
  i--;
  pal[i*3 + 0] = r;
  pal[i*3 + 1] = g;
  pal[i*3 + 2] = b;
  return 0; }

static int reinit_palette(lua_State *L) {
  int n = lua_gettop(L);
  pal_init();
  return 0; }

static int draw_3d_pixel(lua_State *L) {
  int n = lua_gettop(L); float x,y,z; uchar r,g,b;
  if(n!=6) return 0;
  x = lua_tonumber(L, 1);
  y = lua_tonumber(L, 2);
  z = lua_tonumber(L, 3);
  r = lua_tonumber(L, 4);
  g = lua_tonumber(L, 5);
  b = lua_tonumber(L, 6);
  pixel_draw_3d(x,y,z,r,g,b);
  return 0; }

static int draw_3d_quad(lua_State *L) {
  int n = lua_gettop(L); float x,y,z; uchar r,g,b; float sc_div; int max_sz;
  if(n!=8) return 0;
  x = lua_tonumber(L, 1);
  y = lua_tonumber(L, 2);
  z = lua_tonumber(L, 3);
  r = lua_tonumber(L, 4);
  g = lua_tonumber(L, 5);
  b = lua_tonumber(L, 6);
  sc_div = lua_tonumber(L, 7);
  max_sz = lua_tonumber(L, 8);
  quad_draw_3d(x,y,z,r,g,b,sc_div,max_sz);
  return 0; }

static int draw_3d_wire(lua_State *L) {
  int n = lua_gettop(L); float x,y,z,x2,y2,z2; uchar r,g,b;
  if(n!=9) return 0;
  x = lua_tonumber(L, 1);
  y = lua_tonumber(L, 2);
  z = lua_tonumber(L, 3);
  x2 = lua_tonumber(L, 4);
  y2 = lua_tonumber(L, 5);
  z2 = lua_tonumber(L, 6);
  r = lua_tonumber(L, 7);
  g = lua_tonumber(L, 8);
  b = lua_tonumber(L, 9);
  line_draw_3d(x,y,z,x2,y2,z2,r,g,b);
  return 0; }

static int load_empty_level(lua_State *L) {
  int n = lua_gettop(L); int x,y,z;
  if(n!=3) return 0;
  x = lua_tonumber(L, 1);
  y = lua_tonumber(L, 2);
  z = lua_tonumber(L, 3);
  create_empty_level(x,y,z);
  return 0; }

static int erase_box(lua_State *L) {
  int n = lua_gettop(L); int x,y,z,x2,y2,z2; int xi,yi,zi;
  if(n!=6) return 0;
  x = lua_tonumber(L, 1);
  y = lua_tonumber(L, 2);
  z = lua_tonumber(L, 3);
  x2 = lua_tonumber(L, 4);
  y2 = lua_tonumber(L, 5);
  z2 = lua_tonumber(L, 6);
  for(xi=x; xi<=x2; xi++) {
    for(yi=y; yi<=y2; yi++) {
      for(zi=z; zi<=z2; zi++) {
        _set(xi,yi,zi,0); } } }
  return 0; }

static int save_level_to_file(lua_State *L) {
  int n = lua_gettop(L);
  if(n!=1) return 0;
  save_level(lua_tostring(L,1));
  return 0; }

static int save_patch_to_file(lua_State *L) {
  int n = lua_gettop(L);
  int x,y,z,x2,y2,z2; int xi,yi,zi; FILE *fl;
  if(n!=7) return 0;
  x = lua_tonumber(L, 1);
  y = lua_tonumber(L, 2);
  z = lua_tonumber(L, 3);
  x2 = lua_tonumber(L, 4);
  y2 = lua_tonumber(L, 5);
  z2 = lua_tonumber(L, 6);
  fl = fopen(lua_tostring(L,7), "wb");
  for(xi=x; xi<=x2; xi++) {
    for(yi=y; yi<=y2; yi++) {
      for(zi=z; zi<=z2; zi++) {
        uchar i = _get(xi,yi,zi);
        fwrite(&i,1,1,fl); } } }
  fclose(fl);
  return 0; }

static int load_patch_from_file(lua_State *L) {
  int n = lua_gettop(L);
  int x,y,z,x2,y2,z2; int xi,yi,zi; FILE *fl;
  if(n!=7) return 0;
  x = lua_tonumber(L, 1);
  y = lua_tonumber(L, 2);
  z = lua_tonumber(L, 3);
  x2 = lua_tonumber(L, 4);
  y2 = lua_tonumber(L, 5);
  z2 = lua_tonumber(L, 6);
  fl = fopen(lua_tostring(L,7), "rb");
  for(xi=x; xi<=x2; xi++) {
    for(yi=y; yi<=y2; yi++) {
      for(zi=z; zi<=z2; zi++) {
        uchar i;
        fread(&i,1,1,fl);
        _set(xi,yi,zi,i); } } }
  fclose(fl);
  return 0; }

static int fill_box_with_rgb(lua_State *L) {
  int n = lua_gettop(L); int x,y,z,x2,y2,z2,r,g,b;
  uchar c; int xi,yi,zi;
  if(n!=9) return 0;
  x = lua_tonumber(L, 1);
  y = lua_tonumber(L, 2);
  z = lua_tonumber(L, 3);
  x2 = lua_tonumber(L, 4);
  y2 = lua_tonumber(L, 5);
  z2 = lua_tonumber(L, 6);
  r = lua_tonumber(L, 7);
  g = lua_tonumber(L, 8);
  b = lua_tonumber(L, 9);
  c = rgb_to_8bit(r,g,b);
  for(xi=x; xi<=x2; xi++) {
    for(yi=y; yi<=y2; yi++) {
      for(zi=z; zi<=z2; zi++) {
        _set(xi,yi,zi,c); } } }
  return 0; }

static int fill_box_with_index(lua_State *L) {
  int n = lua_gettop(L); int x,y,z,x2,y2,z2; uchar i;
  int xi,yi,zi;
  if(n!=7) return 0;
  x = lua_tonumber(L, 1);
  y = lua_tonumber(L, 2);
  z = lua_tonumber(L, 3);
  x2 = lua_tonumber(L, 4);
  y2 = lua_tonumber(L, 5);
  z2 = lua_tonumber(L, 6);
  i = lua_tonumber(L, 7);
  for(xi=x; xi<=x2; xi++) {
    for(yi=y; yi<=y2; yi++) {
      for(zi=z; zi<=z2; zi++) {
        _set(xi,yi,zi,i); } } }
  return 0; }

static int fill_box_with_noisy_rgb(lua_State *L) {
  int n = lua_gettop(L); int x,y,z,x2,y2,z2,r,g,b,intensity;
  uchar c; int xi,yi,zi;
  if(n!=10) return 0;
  x = lua_tonumber(L, 1);
  y = lua_tonumber(L, 2);
  z = lua_tonumber(L, 3);
  x2 = lua_tonumber(L, 4);
  y2 = lua_tonumber(L, 5);
  z2 = lua_tonumber(L, 6);
  r = lua_tonumber(L, 7);
  g = lua_tonumber(L, 8);
  b = lua_tonumber(L, 9);
  intensity = lua_tonumber(L, 10);
  c = rgb_to_8bit(r,g,b);
  for(xi=x; xi<=x2; xi++) {
    for(yi=y; yi<=y2; yi++) {
      for(zi=z; zi<=z2; zi++) {
        uchar col = c-randm()%intensity;
        CLAMP(col,1,255);
        _set(xi,yi,zi,col); } } }
  return 0; }

static int fill_box_with_noisy_index(lua_State *L) {
  int n = lua_gettop(L); int x,y,z,x2,y2,z2,intensity; uchar i;
  int xi,yi,zi;
  if(n!=8) return 0;
  x = lua_tonumber(L, 1);
  y = lua_tonumber(L, 2);
  z = lua_tonumber(L, 3);
  x2 = lua_tonumber(L, 4);
  y2 = lua_tonumber(L, 5);
  z2 = lua_tonumber(L, 6);
  i = lua_tonumber(L, 7);
  intensity = lua_tonumber(L, 8);
  for(xi=x; xi<=x2; xi++) {
    for(yi=y; yi<=y2; yi++) {
      for(zi=z; zi<=z2; zi++) {
        uchar col = i-randm()%intensity;
        CLAMP(col,1,255);
        _set(xi,yi,zi,col); } } }
  return 0; }

static int erase_sphere(lua_State *L) {
  int n = lua_gettop(L); int x,y,z; float rad; int xi,yi,zi;
  if(n!=4) return 0;
  x = lua_tonumber(L, 1);
  y = lua_tonumber(L, 2);
  z = lua_tonumber(L, 3);
  rad = lua_tonumber(L, 4);
  for(xi=x-rad; xi<=x+rad; xi++) {
    for(yi=y-rad; yi<=y+rad; yi++) {
      for(zi=z-rad; zi<=z+rad; zi++) {
        float len = sqrtf(powf(x-xi,2) + powf(y-yi,2)+powf(z-zi,2));
        if(xi>0 && xi<xsize-1 &&
            yi>0 && yi<ysize-1 &&
            zi>0 && zi<zsize-1) {
          if(len<=rad)
            _set(xi,yi,zi,0); } } } }
  return 0; }

static int erase_sphere_shaded(lua_State *L) {
  int n = lua_gettop(L); int x,y,z,r,g,b;
  if(n!=6) return 0;
  x = lua_tonumber(L, 1);
  y = lua_tonumber(L, 2);
  z = lua_tonumber(L, 3);
  r = lua_tonumber(L, 4);
  g = lua_tonumber(L, 5);
  b = lua_tonumber(L, 6);
  destroy_block_rgb(x,y,z,r,g,b);
  return 0; }

static int fill_sphere_with_rgb(lua_State *L) {
  int n = lua_gettop(L); int x,y,z,r,g,b;
  float rad; uchar i; int xi,yi,zi;
  if(n!=7) return 0;
  x = lua_tonumber(L, 1);
  y = lua_tonumber(L, 2);
  z = lua_tonumber(L, 3);
  rad = lua_tonumber(L, 4);
  r = lua_tonumber(L, 5);
  g = lua_tonumber(L, 6);
  b = lua_tonumber(L, 7);
  i = rgb_to_8bit(r,g,b);
  for(xi=x-rad; xi<=x+rad; xi++) {
    for(yi=y-rad; yi<=y+rad; yi++) {
      for(zi=z-rad; zi<=z+rad; zi++) {
        float len = sqrtf(powf(x-xi,2) + powf(y-yi,2)+powf(z-zi,2));
        if(xi>0 && xi<xsize-1 &&
            yi>0 && yi<ysize-1 &&
            zi>0 && zi<zsize-1) {
          if(len<=rad)
            _set(xi,yi,zi,i); } } } }
  return 0; }

static int fill_sphere_with_index(lua_State *L) {
  int n = lua_gettop(L); int x,y,z;
  float rad; uchar i; int xi,yi,zi;
  if(n!=5) return 0;
  x = lua_tonumber(L, 1);
  y = lua_tonumber(L, 2);
  z = lua_tonumber(L, 3);
  rad = lua_tonumber(L, 4);
  i = lua_tonumber(L, 5);
  for(xi=x-rad; xi<=x+rad; xi++) {
    for(yi=y-rad; yi<=y+rad; yi++) {
      for(zi=z-rad; zi<=z+rad; zi++) {
        float len = sqrtf(powf(x-xi,2) + powf(y-yi,2)+powf(z-zi,2));
        if(xi>0 && xi<xsize-1 &&
            yi>0 && yi<ysize-1 &&
            zi>0 && zi<zsize-1) {
          if(len<=rad)
            _set(xi,yi,zi,i); } } } }
  return 0; }

static int fill_sphere_with_noisy_rgb(lua_State *L) {
  int n = lua_gettop(L); int x,y,z,r,g,b,intensity;
  float rad; uchar i; int xi,yi,zi;
  if(n!=8) return 0;
  x = lua_tonumber(L, 1);
  y = lua_tonumber(L, 2);
  z = lua_tonumber(L, 3);
  rad = lua_tonumber(L, 4);
  r = lua_tonumber(L, 5);
  g = lua_tonumber(L, 6);
  b = lua_tonumber(L, 7);
  intensity = lua_tonumber(L, 8);
  i = rgb_to_8bit(r,g,b);
  for(xi=x-rad; xi<=x+rad; xi++) {
    for(yi=y-rad; yi<=y+rad; yi++) {
      for(zi=z-rad; zi<=z+rad; zi++) {
        float len = sqrtf(powf(x-xi,2) + powf(y-yi,2)+powf(z-zi,2));
        if(xi>0 && xi<xsize-1 &&
            yi>0 && yi<ysize-1 &&
            zi>0 && zi<zsize-1) {
          if(len<=rad) {
            uchar col = i-randm()%intensity;
            CLAMP(col,1,255);
            _set(xi,yi,zi,col); } } } } }
  return 0; }

static int fill_sphere_with_noisy_index(lua_State *L) {
  int n = lua_gettop(L); int x,y,z,intensity;
  float rad; uchar i; int xi,yi,zi;
  if(n!=6) return 0;
  x = lua_tonumber(L, 1);
  y = lua_tonumber(L, 2);
  z = lua_tonumber(L, 3);
  rad = lua_tonumber(L, 4);
  i = lua_tonumber(L, 5);
  intensity = lua_tonumber(L, 6);
  for(xi=x-rad; xi<=x+rad; xi++) {
    for(yi=y-rad; yi<=y+rad; yi++) {
      for(zi=z-rad; zi<=z+rad; zi++) {
        float len = sqrtf(powf(x-xi,2) + powf(y-yi,2)+powf(z-zi,2));
        if(xi>0 && xi<xsize-1 &&
            yi>0 && yi<ysize-1 &&
            zi>0 && zi<zsize-1) {
          if(len<=rad) {
            uchar col = i-randm()%intensity;
            CLAMP(col,1,255);
            _set(xi,yi,zi,col); } } } } }
  return 0; }

static int take_screenshot(lua_State *L) {
  int n = lua_gettop(L);
  if(n!=1) return 0;
  SDL_SaveBMP(screen,lua_tostring(L,1));
  return 0; }

static int reset_palette(lua_State *L) {
  int n = lua_gettop(L);
  memcpy(pal,default_pal,255*3);
  pal_init();
  return 0; }

static int remove_isolated_voxels(lua_State *L) {
  int n = lua_gettop(L); int x,y,z; int cnt=0;
  uchar *voxels2 = (uchar *)malloc(xsize*ysize*zsize*sizeof(uchar));
  memcpy(voxels2,_1,xsize*ysize*zsize*sizeof(uchar));
  for(x=1; x<xsize-1; x++) {
    cnt++; if(cnt>5) {
      char buffer[256];
      float p=x; p/=xsize; p*=100; cnt=0; sprintf(buffer,
        "Removing isolated voxels, please, wait... %.0f%c\0", p,'%');
      SDL_WM_SetCaption(buffer,NULL);
      handle_events(); sleep_ms(0); }
    for(y=1; y<ysize-1; y++) {
      for(z=1; z<zsize-1; z++) {
        int nx,ny,nz, has_air_nei=0;
        for(nx=-1; nx<=1; nx++) {
          for(ny=-1; ny<=1; ny++) {
            for(nz=-1; nz<=1; nz++) {
              if(_get(x+nx,y+ny,z+nz)==0) {
                has_air_nei = 1; nx=2; ny=2; nz=2; } } } }
        if(has_air_nei==0) voxels2[x*ysize+y+z*ysize*xsize]=0; } } }
  memcpy(_1,voxels2,xsize*ysize*zsize*sizeof(uchar));
  free(voxels2);
  return 0; }

static int update_all_chunks(lua_State *L) {
  int n = lua_gettop(L);
  lods_fill(true);
  return 0; }

static int set_max_dist(lua_State *L) {
  int n = lua_gettop(L);
  if(n!=1) return 0;
  MAX_DIST = lua_tonumber(L, 1);
  return 0; }

static int set_fog_rgb(lua_State *L) {
  int n = lua_gettop(L);
  if(n!=3) return 0;
  fogR = lua_tonumber(L, 1);
  fogG = lua_tonumber(L, 2);
  fogB = lua_tonumber(L, 3);
  return 0; }

SDL_Surface *loaded_images[10000];
int liptr=0;

static int load_image(lua_State *L) {
  int n = lua_gettop(L); bool tran=false;
  if(n!=2 && n!=1) return 0;
  if(n==2) tran = lua_tonumber(L, 2);
  loaded_images[liptr] = image_load(lua_tostring(L,1),tran);
  lua_pushnumber(L,liptr);
  liptr++; if(liptr>=10000) liptr=0;
  return 1; }

static int draw_image(lua_State *L) {
  int n = lua_gettop(L); int ptr=0; int x,y;
  if(n!=3) return 0;
  x = lua_tonumber(L, 1);
  y = lua_tonumber(L, 2);
  ptr = lua_tonumber(L, 3);
  image_draw(x,y,loaded_images[ptr]);
  return 0; }

static int draw_image_part(lua_State *L) {
  int n = lua_gettop(L); int ptr=0; int x,y,cx,cy,cw,ch;
  if(n!=7) return 0;
  x = lua_tonumber(L, 1);
  y = lua_tonumber(L, 2);
  cx = lua_tonumber(L, 3);
  cy = lua_tonumber(L, 4);
  cw = lua_tonumber(L, 5);
  ch = lua_tonumber(L, 6);
  ptr = lua_tonumber(L, 7);
  image_partly_draw(x,y,loaded_images[ptr],cx,cy,cw,ch);
  return 0; }

static int unload_image(lua_State *L) {
  int n = lua_gettop(L); int ptr=0;
  if(n!=1) return 0;
  ptr = lua_tonumber(L, 1);
  image_unload(loaded_images[ptr]);
  return 0; }

static int set_image_pixel(lua_State *L) {
  int n = lua_gettop(L); int ptr=0; int x,y; uchar r,g,b;
  if(n!=6) return 0;
  x = lua_tonumber(L, 1);
  y = lua_tonumber(L, 2);
  r = lua_tonumber(L, 3);
  g = lua_tonumber(L, 4);
  b = lua_tonumber(L, 5);
  ptr = lua_tonumber(L, 6);
  putPixel(loaded_images[ptr],x,y,asmRGB(r,g,b));
  return 0; }

static int get_image_pixel(lua_State *L) {
  int n = lua_gettop(L); int ptr=0; int x,y; Uint32 col; uchar r,g,b;
  if(n!=3) return 0;
  x = lua_tonumber(L, 1);
  y = lua_tonumber(L, 2);
  ptr = lua_tonumber(L, 3);
  col = getPixel(loaded_images[ptr],x,y);
  getRGB(col,r,g,b);
  lua_pushnumber(L,r);
  lua_pushnumber(L,g);
  lua_pushnumber(L,b);
  return 3; }

static int perlin_noise_3d(lua_State *L) {
  int n = lua_gettop(L); float x,y,z;
  if(n!=3) return 0;
  x = lua_tonumber(L, 1);
  y = lua_tonumber(L, 2);
  z = lua_tonumber(L, 3);
  lua_pushnumber(L,noise_3d(x,y,z));
  return 1; }

static int throw_ray(lua_State *L) {
  int n = lua_gettop(L); float dx,dy,dz, px,py,pz, md;
  uint rx,ry,rz; uchar ri;
  if(n!=7) return 0;
  dx = lua_tonumber(L, 1);
  dy = lua_tonumber(L, 2);
  dz = lua_tonumber(L, 3);
  px = lua_tonumber(L, 4);
  py = lua_tonumber(L, 5);
  pz = lua_tonumber(L, 6);
  md = lua_tonumber(L, 7);
  cast_ray_to_dir(dx,dy,dz,px,py,pz,md,&rx,&ry,&rz,&ri);
  lua_pushnumber(L,rx);
  lua_pushnumber(L,ry);
  lua_pushnumber(L,rz);
  lua_pushnumber(L,ri);
  return 4; }

static int throw_ray_to_pos(lua_State *L) {
  int n = lua_gettop(L); float lx,ly,lz, px,py,pz;
  uint rx,ry,rz; uchar ri;
  if(n!=6) return 0;
  lx = lua_tonumber(L, 1);
  ly = lua_tonumber(L, 2);
  lz = lua_tonumber(L, 3);
  px = lua_tonumber(L, 4);
  py = lua_tonumber(L, 5);
  pz = lua_tonumber(L, 6);
  cast_ray_to_pos(lx,ly,lz,px,py,pz,&rx,&ry,&rz,&ri);
  lua_pushnumber(L,rx);
  lua_pushnumber(L,ry);
  lua_pushnumber(L,rz);
  lua_pushnumber(L,ri);
  return 4; }

static int is_focused(lua_State *L) {
  int n = lua_gettop(L);
  lua_pushnumber(L,((SDL_GetAppState() & SDL_APPINPUTFOCUS)?1:0));
  return 1; }

void log_error(const char *m) {
  int ln=strlen(m);
  FILE *fl = fopen("error.log", "wb");
  fwrite(m, 1, ln, fl);
  fclose(fl); }

void report_errors(lua_State *L, int status) {
  if(status!=0) {
    log_error("error!\n");
    log_error(lua_tostring(L, -1));
    lua_pop(L, 1); } }

int init_lua(const char *filename) {
  int status;
  log_error("No errors. Engine's version: 0.9993 beta\n");
  L = lua_open();
  luaL_openlibs(L);
  lua_register(L, "setup_fullscreen", setup_fullscreen);
  lua_register(L, "setup_window", setup_window);
  lua_register(L, "handle_sdl_events",handle_sdl_events);
  lua_register(L, "get_tick_count", get_tick_count);
  lua_register(L, "set_window_title", set_window_title);
  lua_register(L, "draw_fps", draw_fps);
  lua_register(L, "draw_voxels", draw_voxels);
  lua_register(L, "draw_camera_pos", draw_camera_pos);
  lua_register(L, "sdl_delay", sdl_delay);
  lua_register(L, "get_mouse_xy", get_mouse_xy);
  lua_register(L, "set_mouse_xy", set_mouse_xy);
  lua_register(L, "set_cam_yaw", set_cam_yaw);
  lua_register(L, "set_cam_pitch", set_cam_pitch);
  lua_register(L, "set_cam_pos", set_cam_pos);
  lua_register(L, "cam_collides", cam_collides);
  lua_register(L, "get_pointed_voxel_pos", get_pointed_voxel_pos);
  lua_register(L, "is_voxel_empty", is_voxel_empty);
  lua_register(L, "erase_voxel", erase_voxel);
  lua_register(L, "get_voxel_pal", get_voxel_pal);
  lua_register(L, "get_voxel_rgb", get_voxel_rgb);
  lua_register(L, "set_voxel_pal", set_voxel_pal);
  lua_register(L, "set_voxel_rgb", set_voxel_rgb);
  lua_register(L, "get_view_ray", get_view_ray);
  lua_register(L, "announce_chunk_updates", announce_chunk_updates);
  lua_register(L, "update_chunk", update_chunk);
  lua_register(L, "update_all_chunks",update_all_chunks);
  lua_register(L, "remove_isolated_voxels",remove_isolated_voxels);
  lua_register(L, "is_key_pressed", is_key_pressed);
  lua_register(L, "fake_key_state", fake_key_state);
  lua_register(L, "is_button_pressed",is_button_pressed);
  lua_register(L, "fake_button_state",fake_button_state);
  lua_register(L, "show_sdl_cursor", show_sdl_cursor);
  lua_register(L, "hide_sdl_cursor", hide_sdl_cursor);
  lua_register(L, "screen_set_pixel", screen_set_pixel);
  lua_register(L, "screen_set_pixel32",screen_set_pixel32);
  lua_register(L, "screen_clear", screen_clear);
  lua_register(L, "screen_fill_rect", screen_fill_rect); 
  lua_register(L, "screen_draw_circle",screen_draw_circle);
  lua_register(L, "screen_draw_line", screen_draw_line); 
  lua_register(L, "load_level_from_file",load_level_from_file);
  lua_register(L, "load_empty_level", load_empty_level);
  lua_register(L, "save_level_to_file",save_level_to_file);
  lua_register(L, "save_patch_to_file",save_patch_to_file);
  lua_register(L, "load_patch_from_file",load_patch_from_file);
  lua_register(L, "erase_box", erase_box);
  lua_register(L, "fill_box_with_rgb",fill_box_with_rgb);
  lua_register(L, "fill_box_with_index",fill_box_with_index);
  lua_register(L, "fill_box_with_noisy_rgb",fill_box_with_noisy_rgb); 
  lua_register(L, "fill_box_with_noisy_index",fill_box_with_noisy_index);
  lua_register(L, "erase_sphere", erase_sphere);
  lua_register(L, "erase_sphere_shaded",erase_sphere_shaded);
  lua_register(L, "fill_sphere_with_rgb",fill_sphere_with_rgb);
  lua_register(L, "fill_sphere_with_index",fill_sphere_with_index);
  lua_register(L, "fill_sphere_with_noisy_rgb",fill_sphere_with_noisy_rgb);
  lua_register(L, "fill_sphere_with_noisy_index",fill_sphere_with_noisy_index);
  lua_register(L, "set_max_dist", set_max_dist);
  lua_register(L, "set_fog_rgb", set_fog_rgb);
  lua_register(L, "swap_buffers", swap_buffers);
  lua_register(L, "set_palette", set_palette);
  lua_register(L, "reinit_palette", reinit_palette);
  lua_register(L, "draw_3d_pixel", draw_3d_pixel);
  lua_register(L, "draw_3d_quad", draw_3d_quad);
  lua_register(L, "draw_3d_wire", draw_3d_wire);
  lua_register(L, "low_quality", low_quality);
  lua_register(L, "med_quality", med_quality);
  lua_register(L, "take_screenshot", take_screenshot);
  lua_register(L, "reset_palette", reset_palette);
  lua_register(L, "force_single_thread",force_single_thread);
  lua_register(L, "load_image", load_image);
  lua_register(L, "draw_image", draw_image);
  lua_register(L, "draw_image_part", draw_image_part);
  lua_register(L, "unload_image", unload_image);
  lua_register(L, "set_image_pixel", set_image_pixel);
  lua_register(L, "get_image_pixel", get_image_pixel);
  lua_register(L, "perlin_noise_3d", perlin_noise_3d);
  lua_register(L, "is_focused", is_focused);
  lua_register(L, "enable_checkers", enable_checkers);
  lua_register(L, "disable_checkers", disable_checkers);
  lua_register(L, "throw_ray", throw_ray);
  lua_register(L, "throw_ray_to_dir", throw_ray);
  lua_register(L, "throw_ray_to_pos", throw_ray_to_pos);
  status = luaL_dofile(L, filename);
  report_errors(L, status);
  lua_close(L);
  return status; }

int main(int argc, char *argv[]) {
  memcpy(pal,default_pal,255*3);
  pal_init(); noise_init();
  screen = SDL_SetVideoMode(WIDTH,HEIGHT,32,SDL_DOUBLEBUF);
  img = NEW(unsigned int, WIDTH*HEIGHT);
  fake_screen = SDL_CreateRGBSurface(SDL_SWSURFACE,640,480,32,0,0,0,0);
  fake_screen->pixels = (void *)img;
  lbuffer = NEW(float, WIDTH*HEIGHT);
  SDL_FillRect(screen,&screen->clip_rect,0);
  SDL_WM_SetCaption("Please, wait...",NULL);
  if(argc==2) {
    init_lua(argv[1]); }
  else {
    init_lua("game.lua"); }
  return 0; }

void handle_events() {
  while(SDL_PollEvent(&event)) {
    if(event.type==SDL_MOUSEMOTION) {
      MOUSEX=event.motion.x; MOUSEY=event.motion.y; }
    if(event.type==SDL_QUIT) {
      exit_flag=1;
      SDL_Quit();
      exit(1); }
    if(event.type==SDL_MOUSEBUTTONDOWN) {
      MOUSEX=event.motion.x; MOUSEY=event.motion.y;
      if(event.button.button==SDL_BUTTON_LEFT)BUTTON[LB]=1;
      if(event.button.button==SDL_BUTTON_MIDDLE)BUTTON[MB]=1;
      if(event.button.button==SDL_BUTTON_RIGHT)BUTTON[RB]=1;
      if(event.button.button==SDL_BUTTON_WHEELUP)BUTTON[WUP]=1;
      if(event.button.button==SDL_BUTTON_WHEELDOWN)BUTTON[WDOWN]=1; }
    if(event.type==SDL_MOUSEBUTTONUP) {
      MOUSEX=event.motion.x; MOUSEY=event.motion.y;
      if(event.button.button==SDL_BUTTON_LEFT)BUTTON[LB]=0;
      if(event.button.button==SDL_BUTTON_MIDDLE)BUTTON[MB]=0;
      if(event.button.button==SDL_BUTTON_RIGHT)BUTTON[RB]=0;
      if(event.button.button==SDL_BUTTON_WHEELUP)BUTTON[WUP]=0;
      if(event.button.button==SDL_BUTTON_WHEELDOWN)BUTTON[WDOWN]=0; }
    if(event.type==SDL_KEYDOWN) {
      int bset=1;
      if(event.key.keysym.sym==SDLK_BACKSPACE)KEY[BACKSPACE]=bset;
      if(event.key.keysym.sym==SDLK_TAB)KEY[TAB]=bset;
      if(event.key.keysym.sym==SDLK_CLEAR)KEY[CLEAR]=bset;
      if(event.key.keysym.sym==SDLK_RETURN)KEY[ENTER]=bset;
      if(event.key.keysym.sym==SDLK_PAUSE)KEY[PAUSE]=bset;
      if(event.key.keysym.sym==SDLK_ESCAPE)KEY[ESCAPE]=bset;
      if(event.key.keysym.sym==SDLK_SPACE)KEY[SPACE]=bset;
      if(event.key.keysym.sym==SDLK_EXCLAIM)KEY[EXCLAIM]=bset;
      if(event.key.keysym.sym==SDLK_QUOTEDBL)KEY[QUOTEDBL]=bset;
      if(event.key.keysym.sym==SDLK_HASH)KEY[HASH]=bset;
      if(event.key.keysym.sym==SDLK_DOLLAR)KEY[DOLLAR]=bset;
      if(event.key.keysym.sym==SDLK_AMPERSAND)KEY[AMPERSAND]=bset;
      if(event.key.keysym.sym==SDLK_QUOTE)KEY[QUOTE]=bset;
      if(event.key.keysym.sym==SDLK_LEFTPAREN)KEY[LEFTPAREN]=bset;
      if(event.key.keysym.sym==SDLK_RIGHTPAREN)KEY[RIGHTPAREN]=bset;
      if(event.key.keysym.sym==SDLK_ASTERISK)KEY[ASTERISK]=bset;
      if(event.key.keysym.sym==SDLK_PLUS)KEY[PLUS]=bset;
      if(event.key.keysym.sym==SDLK_COMMA)KEY[COMMA]=bset;
      if(event.key.keysym.sym==SDLK_MINUS)KEY[MINUS]=bset;
      if(event.key.keysym.sym==SDLK_PERIOD)KEY[PERIOD]=bset;
      if(event.key.keysym.sym==SDLK_SLASH)KEY[SLASH]=bset;
      if(event.key.keysym.sym==SDLK_0)KEY[key_0]=bset;
      if(event.key.keysym.sym==SDLK_1)KEY[key_1]=bset;
      if(event.key.keysym.sym==SDLK_2)KEY[key_2]=bset;
      if(event.key.keysym.sym==SDLK_3)KEY[key_3]=bset;
      if(event.key.keysym.sym==SDLK_4)KEY[key_4]=bset;
      if(event.key.keysym.sym==SDLK_5)KEY[key_5]=bset;
      if(event.key.keysym.sym==SDLK_6)KEY[key_6]=bset;
      if(event.key.keysym.sym==SDLK_7)KEY[key_7]=bset;
      if(event.key.keysym.sym==SDLK_8)KEY[key_8]=bset;
      if(event.key.keysym.sym==SDLK_9)KEY[key_9]=bset;
      if(event.key.keysym.sym==SDLK_COLON)KEY[COLON]=bset;
      if(event.key.keysym.sym==SDLK_SEMICOLON)KEY[SEMICOLON]=bset;
      if(event.key.keysym.sym==SDLK_LESS)KEY[LESS]=bset;
      if(event.key.keysym.sym==SDLK_EQUALS)KEY[EQUALS]=bset;
      if(event.key.keysym.sym==SDLK_GREATER)KEY[GREATER]=bset;
      if(event.key.keysym.sym==SDLK_QUESTION)KEY[QUESTION]=bset;
      if(event.key.keysym.sym==SDLK_AT)KEY[AT]=bset;
      if(event.key.keysym.sym==SDLK_LEFTBRACKET)KEY[LEFTBRACKET]=bset;
      if(event.key.keysym.sym==SDLK_BACKSLASH)KEY[BACKSLASH]=bset;
      if(event.key.keysym.sym==SDLK_RIGHTBRACKET)KEY[RIGHTBRACKET]=bset;
      if(event.key.keysym.sym==SDLK_CARET)KEY[CARET]=bset;
      if(event.key.keysym.sym==SDLK_UNDERSCORE)KEY[UNDERSCORE]=bset;
      if(event.key.keysym.sym==SDLK_BACKQUOTE)KEY[BACKQUOTE]=bset;
      if(event.key.keysym.sym==SDLK_a)KEY[key_a]=bset;
      if(event.key.keysym.sym==SDLK_b)KEY[key_b]=bset;
      if(event.key.keysym.sym==SDLK_c)KEY[key_c]=bset;
      if(event.key.keysym.sym==SDLK_d)KEY[key_d]=bset;
      if(event.key.keysym.sym==SDLK_e)KEY[key_e]=bset;
      if(event.key.keysym.sym==SDLK_f)KEY[key_f]=bset;
      if(event.key.keysym.sym==SDLK_g)KEY[key_g]=bset;
      if(event.key.keysym.sym==SDLK_h)KEY[key_h]=bset;
      if(event.key.keysym.sym==SDLK_i)KEY[key_i]=bset;
      if(event.key.keysym.sym==SDLK_j)KEY[key_j]=bset;
      if(event.key.keysym.sym==SDLK_k)KEY[key_k]=bset;
      if(event.key.keysym.sym==SDLK_l)KEY[key_l]=bset;
      if(event.key.keysym.sym==SDLK_m)KEY[key_m]=bset;
      if(event.key.keysym.sym==SDLK_n)KEY[key_n]=bset;
      if(event.key.keysym.sym==SDLK_o)KEY[key_o]=bset;
      if(event.key.keysym.sym==SDLK_p)KEY[key_p]=bset;
      if(event.key.keysym.sym==SDLK_q)KEY[key_q]=bset;
      if(event.key.keysym.sym==SDLK_r)KEY[key_r]=bset;
      if(event.key.keysym.sym==SDLK_s)KEY[key_s]=bset;
      if(event.key.keysym.sym==SDLK_t)KEY[key_t_]=bset;
      if(event.key.keysym.sym==SDLK_u)KEY[key_u]=bset;
      if(event.key.keysym.sym==SDLK_v)KEY[key_v]=bset;
      if(event.key.keysym.sym==SDLK_w)KEY[key_w]=bset;
      if(event.key.keysym.sym==SDLK_x)KEY[key_x]=bset;
      if(event.key.keysym.sym==SDLK_y)KEY[key_y]=bset;
      if(event.key.keysym.sym==SDLK_z)KEY[key_z]=bset;
      if(event.key.keysym.sym==SDLK_DELETE)KEY[DEL]=bset;
      if(event.key.keysym.sym==SDLK_KP0)KEY[KP0]=bset;
      if(event.key.keysym.sym==SDLK_KP1)KEY[KP1]=bset;
      if(event.key.keysym.sym==SDLK_KP2)KEY[KP2]=bset;
      if(event.key.keysym.sym==SDLK_KP3)KEY[KP3]=bset;
      if(event.key.keysym.sym==SDLK_KP4)KEY[KP4]=bset;
      if(event.key.keysym.sym==SDLK_KP5)KEY[KP5]=bset;
      if(event.key.keysym.sym==SDLK_KP6)KEY[KP6]=bset;
      if(event.key.keysym.sym==SDLK_KP7)KEY[KP7]=bset;
      if(event.key.keysym.sym==SDLK_KP8)KEY[KP8]=bset;
      if(event.key.keysym.sym==SDLK_KP9)KEY[KP9]=bset;
      if(event.key.keysym.sym==SDLK_KP_PERIOD)KEY[KP_PERIOD]=bset;
      if(event.key.keysym.sym==SDLK_KP_DIVIDE)KEY[KP_DIVIDE]=bset;
      if(event.key.keysym.sym==SDLK_KP_MULTIPLY)KEY[KP_MULTIPLY]=bset;
      if(event.key.keysym.sym==SDLK_KP_MINUS)KEY[KP_MINUS]=bset;
      if(event.key.keysym.sym==SDLK_KP_PLUS)KEY[KP_PLUS]=bset;
      if(event.key.keysym.sym==SDLK_KP_ENTER)KEY[KP_ENTER]=bset;
      if(event.key.keysym.sym==SDLK_KP_EQUALS)KEY[KP_EQUALS]=bset;
      if(event.key.keysym.sym==SDLK_UP)KEY[UP]=bset;
      if(event.key.keysym.sym==SDLK_DOWN)KEY[DOWN]=bset;
      if(event.key.keysym.sym==SDLK_RIGHT)KEY[RIGHT]=bset;
      if(event.key.keysym.sym==SDLK_LEFT)KEY[LEFT]=bset;
      if(event.key.keysym.sym==SDLK_INSERT)KEY[INSERT]=bset;
      if(event.key.keysym.sym==SDLK_HOME)KEY[HOME]=bset;
      if(event.key.keysym.sym==SDLK_END)KEY[END]=bset;
      if(event.key.keysym.sym==SDLK_PAGEUP)KEY[PAGEUP]=bset;
      if(event.key.keysym.sym==SDLK_PAGEDOWN)KEY[PAGEDOWN]=bset;
      if(event.key.keysym.sym==SDLK_F1)KEY[F1]=bset;
      if(event.key.keysym.sym==SDLK_F2)KEY[F2]=bset;
      if(event.key.keysym.sym==SDLK_F3)KEY[F3]=bset;
      if(event.key.keysym.sym==SDLK_F4)KEY[F4]=bset;
      if(event.key.keysym.sym==SDLK_F5)KEY[F5]=bset;
      if(event.key.keysym.sym==SDLK_F6)KEY[F6]=bset;
      if(event.key.keysym.sym==SDLK_F7)KEY[F7]=bset;
      if(event.key.keysym.sym==SDLK_F8)KEY[F8]=bset;
      if(event.key.keysym.sym==SDLK_F9)KEY[F9]=bset;
      if(event.key.keysym.sym==SDLK_F10)KEY[F10]=bset;
      if(event.key.keysym.sym==SDLK_F11)KEY[F11]=bset;
      if(event.key.keysym.sym==SDLK_F12)KEY[F12]=bset;
      if(event.key.keysym.sym==SDLK_F13)KEY[F13]=bset;
      if(event.key.keysym.sym==SDLK_F14)KEY[F14]=bset;
      if(event.key.keysym.sym==SDLK_F15)KEY[F15]=bset;
      if(event.key.keysym.sym==SDLK_NUMLOCK)KEY[NUMLOCK]=bset;
      if(event.key.keysym.sym==SDLK_CAPSLOCK)KEY[CAPSLOCK]=bset;
      if(event.key.keysym.sym==SDLK_SCROLLOCK)KEY[SCROLLOCK]=bset;
      if(event.key.keysym.sym==SDLK_RSHIFT)KEY[RSHIFT]=bset;
      if(event.key.keysym.sym==SDLK_LSHIFT)KEY[LSHIFT]=bset;
      if(event.key.keysym.sym==SDLK_RCTRL)KEY[RCTRL]=bset;
      if(event.key.keysym.sym==SDLK_LCTRL)KEY[LCTRL]=bset;
      if(event.key.keysym.sym==SDLK_RALT)KEY[RALT]=bset;
      if(event.key.keysym.sym==SDLK_LALT)KEY[LALT]=bset; }
    if(event.type==SDL_KEYUP) {
      int bset=0;
      if(event.key.keysym.sym==SDLK_BACKSPACE)KEY[BACKSPACE]=bset;
      if(event.key.keysym.sym==SDLK_TAB)KEY[TAB]=bset;
      if(event.key.keysym.sym==SDLK_CLEAR)KEY[CLEAR]=bset;
      if(event.key.keysym.sym==SDLK_RETURN)KEY[ENTER]=bset;
      if(event.key.keysym.sym==SDLK_PAUSE)KEY[PAUSE]=bset;
      if(event.key.keysym.sym==SDLK_ESCAPE)KEY[ESCAPE]=bset;
      if(event.key.keysym.sym==SDLK_SPACE)KEY[SPACE]=bset;
      if(event.key.keysym.sym==SDLK_EXCLAIM)KEY[EXCLAIM]=bset;
      if(event.key.keysym.sym==SDLK_QUOTEDBL)KEY[QUOTEDBL]=bset;
      if(event.key.keysym.sym==SDLK_HASH)KEY[HASH]=bset;
      if(event.key.keysym.sym==SDLK_DOLLAR)KEY[DOLLAR]=bset;
      if(event.key.keysym.sym==SDLK_AMPERSAND)KEY[AMPERSAND]=bset;
      if(event.key.keysym.sym==SDLK_QUOTE)KEY[QUOTE]=bset;
      if(event.key.keysym.sym==SDLK_LEFTPAREN)KEY[LEFTPAREN]=bset;
      if(event.key.keysym.sym==SDLK_RIGHTPAREN)KEY[RIGHTPAREN]=bset;
      if(event.key.keysym.sym==SDLK_ASTERISK)KEY[ASTERISK]=bset;
      if(event.key.keysym.sym==SDLK_PLUS)KEY[PLUS]=bset;
      if(event.key.keysym.sym==SDLK_COMMA)KEY[COMMA]=bset;
      if(event.key.keysym.sym==SDLK_MINUS)KEY[MINUS]=bset;
      if(event.key.keysym.sym==SDLK_PERIOD)KEY[PERIOD]=bset;
      if(event.key.keysym.sym==SDLK_SLASH)KEY[SLASH]=bset;
      if(event.key.keysym.sym==SDLK_0)KEY[key_0]=bset;
      if(event.key.keysym.sym==SDLK_1)KEY[key_1]=bset;
      if(event.key.keysym.sym==SDLK_2)KEY[key_2]=bset;
      if(event.key.keysym.sym==SDLK_3)KEY[key_3]=bset;
      if(event.key.keysym.sym==SDLK_4)KEY[key_4]=bset;
      if(event.key.keysym.sym==SDLK_5)KEY[key_5]=bset;
      if(event.key.keysym.sym==SDLK_6)KEY[key_6]=bset;
      if(event.key.keysym.sym==SDLK_7)KEY[key_7]=bset;
      if(event.key.keysym.sym==SDLK_8)KEY[key_8]=bset;
      if(event.key.keysym.sym==SDLK_9)KEY[key_9]=bset;
      if(event.key.keysym.sym==SDLK_COLON)KEY[COLON]=bset;
      if(event.key.keysym.sym==SDLK_SEMICOLON)KEY[SEMICOLON]=bset;
      if(event.key.keysym.sym==SDLK_LESS)KEY[LESS]=bset;
      if(event.key.keysym.sym==SDLK_EQUALS)KEY[EQUALS]=bset;
      if(event.key.keysym.sym==SDLK_GREATER)KEY[GREATER]=bset;
      if(event.key.keysym.sym==SDLK_QUESTION)KEY[QUESTION]=bset;
      if(event.key.keysym.sym==SDLK_AT)KEY[AT]=bset;
      if(event.key.keysym.sym==SDLK_LEFTBRACKET)KEY[LEFTBRACKET]=bset;
      if(event.key.keysym.sym==SDLK_BACKSLASH)KEY[BACKSLASH]=bset;
      if(event.key.keysym.sym==SDLK_RIGHTBRACKET)KEY[RIGHTBRACKET]=bset;
      if(event.key.keysym.sym==SDLK_CARET)KEY[CARET]=bset;
      if(event.key.keysym.sym==SDLK_UNDERSCORE)KEY[UNDERSCORE]=bset;
      if(event.key.keysym.sym==SDLK_BACKQUOTE)KEY[BACKQUOTE]=bset;
      if(event.key.keysym.sym==SDLK_a)KEY[key_a]=bset;
      if(event.key.keysym.sym==SDLK_b)KEY[key_b]=bset;
      if(event.key.keysym.sym==SDLK_c)KEY[key_c]=bset;
      if(event.key.keysym.sym==SDLK_d)KEY[key_d]=bset;
      if(event.key.keysym.sym==SDLK_e)KEY[key_e]=bset;
      if(event.key.keysym.sym==SDLK_f)KEY[key_f]=bset;
      if(event.key.keysym.sym==SDLK_g)KEY[key_g]=bset;
      if(event.key.keysym.sym==SDLK_h)KEY[key_h]=bset;
      if(event.key.keysym.sym==SDLK_i)KEY[key_i]=bset;
      if(event.key.keysym.sym==SDLK_j)KEY[key_j]=bset;
      if(event.key.keysym.sym==SDLK_k)KEY[key_k]=bset;
      if(event.key.keysym.sym==SDLK_l)KEY[key_l]=bset;
      if(event.key.keysym.sym==SDLK_m)KEY[key_m]=bset;
      if(event.key.keysym.sym==SDLK_n)KEY[key_n]=bset;
      if(event.key.keysym.sym==SDLK_o)KEY[key_o]=bset;
      if(event.key.keysym.sym==SDLK_p)KEY[key_p]=bset;
      if(event.key.keysym.sym==SDLK_q)KEY[key_q]=bset;
      if(event.key.keysym.sym==SDLK_r)KEY[key_r]=bset;
      if(event.key.keysym.sym==SDLK_s)KEY[key_s]=bset;
      if(event.key.keysym.sym==SDLK_t)KEY[key_t_]=bset;
      if(event.key.keysym.sym==SDLK_u)KEY[key_u]=bset;
      if(event.key.keysym.sym==SDLK_v)KEY[key_v]=bset;
      if(event.key.keysym.sym==SDLK_w)KEY[key_w]=bset;
      if(event.key.keysym.sym==SDLK_x)KEY[key_x]=bset;
      if(event.key.keysym.sym==SDLK_y)KEY[key_y]=bset;
      if(event.key.keysym.sym==SDLK_z)KEY[key_z]=bset;
      if(event.key.keysym.sym==SDLK_DELETE)KEY[DEL]=bset;
      if(event.key.keysym.sym==SDLK_KP0)KEY[KP0]=bset;
      if(event.key.keysym.sym==SDLK_KP1)KEY[KP1]=bset;
      if(event.key.keysym.sym==SDLK_KP2)KEY[KP2]=bset;
      if(event.key.keysym.sym==SDLK_KP3)KEY[KP3]=bset;
      if(event.key.keysym.sym==SDLK_KP4)KEY[KP4]=bset;
      if(event.key.keysym.sym==SDLK_KP5)KEY[KP5]=bset;
      if(event.key.keysym.sym==SDLK_KP6)KEY[KP6]=bset;
      if(event.key.keysym.sym==SDLK_KP7)KEY[KP7]=bset;
      if(event.key.keysym.sym==SDLK_KP8)KEY[KP8]=bset;
      if(event.key.keysym.sym==SDLK_KP9)KEY[KP9]=bset;
      if(event.key.keysym.sym==SDLK_KP_PERIOD)KEY[KP_PERIOD]=bset;
      if(event.key.keysym.sym==SDLK_KP_DIVIDE)KEY[KP_DIVIDE]=bset;
      if(event.key.keysym.sym==SDLK_KP_MULTIPLY)KEY[KP_MULTIPLY]=bset;
      if(event.key.keysym.sym==SDLK_KP_MINUS)KEY[KP_MINUS]=bset;
      if(event.key.keysym.sym==SDLK_KP_PLUS)KEY[KP_PLUS]=bset;
      if(event.key.keysym.sym==SDLK_KP_ENTER)KEY[KP_ENTER]=bset;
      if(event.key.keysym.sym==SDLK_KP_EQUALS)KEY[KP_EQUALS]=bset;
      if(event.key.keysym.sym==SDLK_UP)KEY[UP]=bset;
      if(event.key.keysym.sym==SDLK_DOWN)KEY[DOWN]=bset;
      if(event.key.keysym.sym==SDLK_RIGHT)KEY[RIGHT]=bset;
      if(event.key.keysym.sym==SDLK_LEFT)KEY[LEFT]=bset;
      if(event.key.keysym.sym==SDLK_INSERT)KEY[INSERT]=bset;
      if(event.key.keysym.sym==SDLK_HOME)KEY[HOME]=bset;
      if(event.key.keysym.sym==SDLK_END)KEY[END]=bset;
      if(event.key.keysym.sym==SDLK_PAGEUP)KEY[PAGEUP]=bset;
      if(event.key.keysym.sym==SDLK_PAGEDOWN)KEY[PAGEDOWN]=bset;
      if(event.key.keysym.sym==SDLK_F1)KEY[F1]=bset;
      if(event.key.keysym.sym==SDLK_F2)KEY[F2]=bset;
      if(event.key.keysym.sym==SDLK_F3)KEY[F3]=bset;
      if(event.key.keysym.sym==SDLK_F4)KEY[F4]=bset;
      if(event.key.keysym.sym==SDLK_F5)KEY[F5]=bset;
      if(event.key.keysym.sym==SDLK_F6)KEY[F6]=bset;
      if(event.key.keysym.sym==SDLK_F7)KEY[F7]=bset;
      if(event.key.keysym.sym==SDLK_F8)KEY[F8]=bset;
      if(event.key.keysym.sym==SDLK_F9)KEY[F9]=bset;
      if(event.key.keysym.sym==SDLK_F10)KEY[F10]=bset;
      if(event.key.keysym.sym==SDLK_F11)KEY[F11]=bset;
      if(event.key.keysym.sym==SDLK_F12)KEY[F12]=bset;
      if(event.key.keysym.sym==SDLK_F13)KEY[F13]=bset;
      if(event.key.keysym.sym==SDLK_F14)KEY[F14]=bset;
      if(event.key.keysym.sym==SDLK_F15)KEY[F15]=bset;
      if(event.key.keysym.sym==SDLK_NUMLOCK)KEY[NUMLOCK]=bset;
      if(event.key.keysym.sym==SDLK_CAPSLOCK)KEY[CAPSLOCK]=bset;
      if(event.key.keysym.sym==SDLK_SCROLLOCK)KEY[SCROLLOCK]=bset;
      if(event.key.keysym.sym==SDLK_RSHIFT)KEY[RSHIFT]=bset;
      if(event.key.keysym.sym==SDLK_LSHIFT)KEY[LSHIFT]=bset;
      if(event.key.keysym.sym==SDLK_RCTRL)KEY[RCTRL]=bset;
      if(event.key.keysym.sym==SDLK_LCTRL)KEY[LCTRL]=bset;
      if(event.key.keysym.sym==SDLK_RALT)KEY[RALT]=bset;
      if(event.key.keysym.sym==SDLK_LALT)KEY[LALT]=bset; } } }