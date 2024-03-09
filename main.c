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
int screen=0;			//screen nummer
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

void generateAnImage(XImage*,int,int,unsigned char [H][W][3]); //a function which generates
int setRootAtoms(Pixmap);
unsigned char frame[H][W][3] = {0};


int main(){
	display = XOpenDisplay(NULL);			//initializing display
	width = DisplayWidth(display, screen);		//finding screen width. I know I difined it on the beginning of the file. I don't care
	height = DisplayHeight(display, screen);	//you may guess
	depth = DefaultDepth(display, screen);		//how many bits can code one color the most displays 


	if (!display || !width || !height || !depth) {printf("display initialization problem\n");return 1;}

	XMatchVisualInfo(display, screen, depth, DirectColor, &vinfo); 	//This function just writes some useless (in my opinion) information about visuals to vinfo
	visual = vinfo.visual;						//Write part of these fucking important information to visual

	if (!visual){printf("Visual Info Initialization Problem\n");return 1;}

	dataSize = height*width*depth;					//how many bits are allocated for data part of image. It should be divided by 8. I have no clue, why, but it doesn't work that way (I mean there is not enought allocated memory, but it should be enought, so. Whatever)
	char* data = (char*)malloc(dataSize);				//allocate that amount of memory
	
	image = XCreateImage(display, visual, depth, ZPixmap, 0, data, width, height, 32, 0);	//create image template (it is changable, so I will write it into pixmap later after changes)
	if (!image){printf("image init problem\n"); return 1;}

	pixmap = XCreatePixmap(display, RootWindow(display, screen), width, height, depth);	//create pixmap (I didn't find anything about how to change it directly)
	if (!pixmap){printf("pixmap init problem\n");return 1;}

	unsigned long baseColor = (255<<16)|(255<<8)|(255);					//It is for sending image to pixmap, but exact function of it isn't clear for me
	values.background = baseColor;
	values.foreground = baseColor;

	gc = XCreateGC(display, pixmap, gcmask, &values);					//create some shit, which I described in previous comment
	if (!gc){printf("gc init problem\n"); return 1;}
	
	rereadTheFile:
	FILE *pipein = popen("ffmpeg -i ~/.start/background/background.mp4 -f image2pipe -vcodec rawvideo -pix_fmt rgb24 -", "r"); 	//open file with this commant (I feel like my comments are really useful)
	int count;															//the number of items readfrom file



	while (1){
		count = fread(frame, 1, width*height*3, pipein);		//so i read content of pipein into frame
		if (count != W*H*3) {fflush(pipein);pclose(pipein);goto rereadTheFile;};	//check if the file ended already
		generateAnImage(image, width,height, frame);			//I put infos from frame into image
		XPutImage(display, pixmap, gc, image, 0,0,0,0,width,height);	//I put image into pixmap

			
				
		XKillClient(display, AllTemporary);				//I'm not sure. I don't understand this borind documentation. It just ends connection with xserver?
		XSetCloseDownMode(display, RetainTemporary);			//I understand it even less

		setRootAtoms(pixmap);						//I just stole it from hsetroot, there is one little fix, which cost me around 5 hours


		XSetWindowBackgroundPixmap(display, RootWindow(display, screen), pixmap);	//guess
		XClearWindow(display, RootWindow(display, screen));				//guess
		
		XFlush(display);								//something like apply changes
		XSync(display, False);								//something like the same
			
		//sleep(0.05);									//here must be implemented normal timer, but it is slow enought to work even so
	}
	

	//freeing the memory (I know I have and infinit loop before, I just don't care)
	XFree(image);
	XFreePixmap(display, pixmap);
	XFreeGC(display, gc);

	//also would must have been done if there were no infinite loop before. (After writing this sentence I feel like a native english speaker, and after writing this program I feel like native c speaker . It's sad, that both are just feelings)


	return 0;
}

void generateAnImage(XImage* image, int w, int h, unsigned char frame[H][W][3]){
	unsigned int pixel;							//pixel color it is coded easy. you write a number as a binary, shift it something to the left, and at the end its like 255 255 255 in one number in binary
	for (int x = 0; x < W; x++) for (int y = 0; y < H; y++){
		pixel = (frame[y][x][0]<<16 | frame[y][x][1]<<8 | frame[y][x][2]);	// the shift I described earlier
		XPutPixel(image, x, y, pixel);						//guess
	}

}

int
setRootAtoms(Pixmap pixmap)						//I stole it from hsetroot
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

      if (data_root && data_eroot && type == XA_PIXMAP && *((Pixmap *) data_root) == *((Pixmap *) data_eroot)){}
        //XKillClient(display, *((Pixmap *) data_root));		//this one comment cost me a few hours
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
