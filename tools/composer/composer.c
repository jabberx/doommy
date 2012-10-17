/*
  composer - raw indexed slices to Doommy Voxel Engine level converter

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

#include <memory.h>
#include <stdio.h>
#include <malloc.h>

unsigned char pal[]={8,8,8,16,16,16,24,24,24,32,32,32,41,41,41,49,49,49,57,
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

unsigned char *level, *slice;

int xsize, ysize, zsize, slices;

int load_slice(const char *filename) {
  long len;
  FILE *fl = fopen(filename, "rb");
  if (!fl) return -1;
  fseek(fl, 0, SEEK_END);
  len = ftell(fl);
  if (len!=xsize*zsize) return -1;
  fseek(fl, 0, SEEK_SET);
  fread(slice, 1, len, fl);
  fclose(fl);
  return 0; }

void save_level(const char *filename) {
  FILE *fl = fopen(filename, "wb");
  int i;
  for(i=0; i<255*3; i++) {
    level[xsize*ysize*zsize+i] = pal[i]; }
  fwrite(level, 1, xsize*ysize*zsize+255*3, fl);
  fclose(fl); }
  
void save_palette() {
  FILE *fl = fopen("color_table.act", "wb");
  int i;
  unsigned char empty[] = {0xFF,0,0xFF};
  fwrite(empty, 1, 3, fl);
  fwrite(pal, 1, 255*3, fl);
  fclose(fl); }
  
void read_palette() {
  FILE *fl = fopen("color_table.act", "rb");
  if (!fl) return;
  int i;
  unsigned char empty[] = {0xFF,0,0xFF};
  fread(empty, 1, 3, fl);
  fread(pal, 1, 255*3, fl);
  fclose(fl); }

int main(int argc, char **argv) {
    int i;
    
    if (argc!=6){
        printf("usage:   %s slice_w slice_h slices_count world_h filename\n", 
                                                                       argv[0]);
        printf("example: %s 256 256 32 128 level.dat\n\n",argv[0]);
        printf("slices must be in raw indexed format without header,\n");
        printf("named 0.raw, 1.raw, etc.\n");
        printf("Use color_table.act as palette\n");
        printf("(first color (magenta,FF00FF) is for empty).\n");
        printf("Palette will be saved now as color_table.act\n");
		printf("You can alter this palette and it will be added to level.\n");
        save_palette();
        return 0; }
	
	read_palette();
    xsize = atoi(argv[1]);
    zsize = atoi(argv[2]);
    slices = atoi(argv[3]);
    ysize = atoi(argv[4]);
    
    if (xsize<32 || zsize<32 || ysize<32 ) {
        printf("Size error. Values greater than 32 expected.\n");
        return -1; }

    level = (unsigned char*)malloc(xsize*ysize*zsize+255*3);
    slice = (unsigned char*)malloc(xsize*zsize); 
    
    memset(level,0,xsize*ysize*zsize+255*3);
    memset(slice,0,xsize*zsize);
    
    for (i=0;i<slices;i++){
        int x,z; char buffer[256];
        memset(slice,0,xsize*zsize);
        sprintf(buffer,"%d.raw",i);
        
        if (load_slice(buffer)!=0){
            printf("error reading %d.raw\n",i);
            return -1; }
            
        for (x=0;x<xsize;x++) {
            for (z=0;z<zsize;z++) {
                level[x*ysize+z*ysize*xsize+i] = slice[x+z*xsize]; } } }

    save_level(argv[5]);
    printf("Done!\n");
    
    return 0; }

