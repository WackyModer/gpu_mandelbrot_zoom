
#ifndef SHARED_H
#define SHARED_H

void disk_thread();

/*
 0 = Compute thread is using
 1 = Write thread is using
 2 = END PROGRAM
 */
extern int side;


extern int width;
extern int height;

extern unsigned int framenum;

extern unsigned char img[256][256][3];


/*
int side = 0;


int width = 32;
int height = 32;

int framenum = 0;
int maxframes = 1440;

char img[3][32][32];
*/

#endif