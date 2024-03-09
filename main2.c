#include <X11/Xlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <unistd.h>

#define W 1920
#define H 1080

Display* display;		//initialize display object
int screen=0;		

int main(){
	
	for (int i = 0; i < 10; i++){


	//screen nummer
Pixmap pixmap;			//Pixmap initialization (set of pixels to be displayed on the screen)
int width;
int height;
int depth;
XVisualInfo vinfo;		//visual information actually I have no idea, what is inside
Visual* visual;			//some part of previous object
int dataSize;			//size of data to create image template
XImage* image;			//the template
unsigned long gcmask = GCBackground | GCForeground;//some shit which defines, how imagemask is applied
XGCValues values;		//I think... The-e Mask?
GC gc;				//decides how to apply image
int toGo;

void generateAnImage(XImage*,int,int,unsigned int [H][W][3]);
int setRootAtoms(Pixmap);
unsigned int frame[H][W][3] = {0};



	display = XOpenDisplay(NULL);
	width = DisplayWidth(display, screen);
	height = DisplayHeight(display, screen);
	depth = DefaultDepth(display, screen);
	printf("%d\n%d\n", width, height);
	if (!display || !width || !height || !depth) {printf("display initialization problem\n");return 1;}
	

	printf("%d\n", depth);
	XMatchVisualInfo(display, screen, depth, DirectColor, &vinfo);
	visual = vinfo.visual;

	if (!visual){printf("Visual Info Initialization Problem\n");return 1;}


	dataSize = height*width*depth/2;
	char* data = (char*)malloc(dataSize);
	
	image = XCreateImage(display, visual, depth, ZPixmap, 0, data, width, height, 32, 0);
	if (!image){printf("image init problem\n"); return 1;}

	pixmap = XCreatePixmap(display, RootWindow(display, screen), width, height, depth);
	if (!pixmap){printf("pixmap init problem\n");return 1;}

	unsigned long baseColor = (255<<16)|(255<<8)|(255);
	values.background = baseColor;
	values.foreground = baseColor;

	gc = XCreateGC(display, pixmap, gcmask, &values);
	if (!gc){printf("gc init problem\n"); return 1;}
	printf("hello");
	//video = popen("ffmpeg -i ~/.start/background/background.mp4 -f image2pipe -vcodec rawvideo -pix_fmt rgb24 -", "r");
	

	//FILE *pipein = popen("ffmpeg -i ~/.start/background/background.mp4 -f image2pipe -vcodec rawvideo -pix_fmt rgb24 -", "r");
	printf("hello\n");
	//int count;
	printf("Oh shit here we go again\n");
	printf("goodbuy\n");

			


		toGo = 0;
		//generateAnImage(image, width, height, pipein);
		//printf("hello\n");
		//count = fread(frame, 1, width*height*3, pipein);
		//printf("hello\n");
		generateAnImage(image, width,height, frame);	
		//if (toGo){
			XPutImage(display, pixmap, gc, image, 0,0,0,0,width,height);
			XSetCloseDownMode(display, RetainTemporary);
			XKillClient(display, AllTemporary);
			XSetWindowBackgroundPixmap(display, RootWindow(display, screen), pixmap);

			XClearWindow(display, RootWindow(display, screen));
			setRootAtoms(pixmap);
		
			XFlush(display);
			XSync(display, False);
		
		//}
		//sleep(1);
	


	XFree(image);
	XFreePixmap(display, pixmap);
	XFreeGC(display, gc);
	printf("1Hello\n");
	sleep(1);
	}
	

	//fflush(pipein);
	//pclose(pipein);


	return 0;
}

void generateAnImage(XImage* image, int w, int h, unsigned int frame[H][W][3]){
	//printf("hello\n");
	unsigned int red = (int)((float)rand()/2147483647.0f*255);
	//unsigned int red = 255;
	printf("red %d\n", red);
	unsigned int green = 0;
	unsigned int blue = 255;
	//unsigned int pixel;
	//printf("Hello\n");
	unsigned int pixel = (blue << 16)|(green << 8)|(red);
	for (int x = 0; x < W; x++) for (int y = 0; y < H; y++){
		//printf("%d %d\n", x, y);
		//pixel = (frame[y][x][2]<<16 | frame[y][x][1]<<8 | frame[y][x][0]);
		//if (XGetPixel(image,x,y) != pixel){toGo = 1;}
		XPutPixel(image, x, y, pixel);
		//printf("you wont seem me\n");
	}

}

int
setRootAtoms(Pixmap pixmap)
{
  Atom atom_root, atom_eroot, type;
  unsigned char *data_root, *data_eroot;
  int format;
  unsigned long length, after;

  atom_root = XInternAtom(display, "_XROOTPMAP_ID", True);
  atom_eroot = XInternAtom(display, "ESETROOT_PMAP_ID", True);

  // doing this to clean up after old background
  if (atom_root != None && atom_eroot != None) {
    XGetWindowProperty(display, RootWindow(display, screen), atom_root, 0L, 1L, False, AnyPropertyType, &type, &format, &length, &after, &data_root);

    if (type == XA_PIXMAP) {
      XGetWindowProperty(display, RootWindow(display, screen), atom_eroot, 0L, 1L, False, AnyPropertyType, &type, &format, &length, &after, &data_eroot);

      if (data_root && data_eroot && type == XA_PIXMAP && *((Pixmap *) data_root) == *((Pixmap *) data_eroot))
        XKillClient(display, *((Pixmap *) data_root));
    }
  }

  atom_root = XInternAtom(display, "_XROOTPMAP_ID", False);
  atom_eroot = XInternAtom(display, "ESETROOT_PMAP_ID", False);

  if (atom_root == None || atom_eroot == None)
    return 0;

  // setting new background atoms
  XChangeProperty(display, RootWindow(display, screen), atom_root, XA_PIXMAP, 32, PropModeReplace, (unsigned char *) &pixmap, 1);
  XChangeProperty(display, RootWindow(display, screen), atom_eroot, XA_PIXMAP, 32, PropModeReplace, (unsigned char *) &pixmap, 1);

  return 1;
}
