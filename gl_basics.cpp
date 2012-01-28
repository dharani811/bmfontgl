/*
BMFont example implementation with Kerning, for C++ and OpenGL 2.0

This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org/>
*/

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <gl/gl.h>
#include <gl/glu.h>
#include "log.h"
#include "globals.h"
#include "gl_basics.h"
#include "lodepng.h"
#include "glext.h"

HDC hDC;
HGLRC hRC;

#pragma warning (disable : 4996)

//Enable OpenGL

void CreateGLContext()
{
  PIXELFORMATDESCRIPTOR pfd;
  int iFormat;
   // Get the device context (DC)
  hDC = GetDC( win_get_window() );
  // Set the pixel format for the DC
  ZeroMemory( &pfd, sizeof( pfd ) );
  pfd.nSize = sizeof( pfd );
  pfd.nVersion = 1;
  pfd.dwFlags = PFD_DRAW_TO_WINDOW |  PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
  pfd.iPixelType = PFD_TYPE_RGBA;
  pfd.cColorBits = 24;
  pfd.cDepthBits = 16;
  pfd.iLayerType = PFD_MAIN_PLANE;
  iFormat = ChoosePixelFormat( hDC, &pfd );
  SetPixelFormat( hDC, iFormat, &pfd );

  // Create and enable the render context (RC)
  hRC = wglCreateContext( hDC );
  wglMakeCurrent( hDC, hRC );
}

// Disable OpenGL

void DeleteGLContext()
{
  wglMakeCurrent( NULL, NULL );
  wglDeleteContext( hRC );
  ReleaseDC( win_get_window(), hDC );
}


void GlSwap()
{
	SwapBuffers( hDC );
}


void CheckGLVersionSupport()
{
  const char *version;
  int major, minor;

  version = (char *) glGetString(GL_VERSION);
  sscanf_s(version, "%d.%d", &major, &minor, sizeof(version));
  wrlog("OpenGl Version supported %d,%d",major,minor);
 
  if (major < 2)
  {
	  MessageBox(NULL, "This program may not work, your supported opengl version is less then 2.0", "OpenGL version warning",MB_ICONERROR | MB_OK);
  }
}


void FreeTexture( GLuint texture )
{
  glDeleteTextures( 1, &texture );
}


GLuint LoadPNG(char* filename) 
{

	GLuint temptex;

	std::vector< unsigned char > rawImage;
	LodePNG::loadFile( rawImage, filename );

	LodePNG::Decoder decoder;
	std::vector< unsigned char > image;
	decoder.decode( image, rawImage.empty() ? 0 : &rawImage[0],
                (unsigned)rawImage.size() );
	//
	// Flip and invert the PNG image since OpenGL likes to load everything
	// backwards from what is considered normal!
	//

	unsigned char *imagePtr = &image[0];
	int halfTheHeightInPixels = decoder.getHeight() / 2;
	int heightInPixels = decoder.getHeight();

	// Assuming RGBA for 4 components per pixel.
	int numColorComponents = 4;

	// Assuming each color component is an unsigned char.
	int widthInChars = decoder.getWidth() * numColorComponents;

	unsigned char *top = NULL;
	unsigned char *bottom = NULL;
	unsigned char temp = 0;

	for( int h = 0; h < halfTheHeightInPixels; ++h )
	{
		top = imagePtr + h * widthInChars;
		bottom = imagePtr + (heightInPixels - h - 1) * widthInChars;

		for( int w = 0; w < widthInChars; ++w )
		{
			// Swap the chars around.
			temp = *top;
			*top = *bottom;
			*bottom = temp;

			++top;
			++bottom;
		}
	}
	//
	// Create the OpenGL texture and fill it with our PNG image.
	//
	// Allocates one texture handle
	//glHint (GL_GENERATE_MIPMAP_HINT, GL_NICEST);

	glGenTextures( 1, &temptex );

	// Binds this texture handle so we can load the data into it
	glBindTexture( GL_TEXTURE_2D, temptex );

	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST );

	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE); 

	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, decoder.getWidth(),
				  decoder.getHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE,
				  &image[0] );


	rawImage.clear(); 
	
	return temptex;
}


GLvoid ReSizeGLScene(GLsizei width, GLsizei height)             // Resize And Initialize The GL Window
{
    if (height==0)                              // Prevent A Divide By Zero By
    {
       height=1;                           // Making Height Equal One
    }
    glViewport(0, 0, width, height);                    // Reset The Current Viewport
}


void ViewOrtho(int width, int height)							
{
	glViewport(0,0,width,height);             // Set Up An Ortho View	 
	glMatrixMode(GL_PROJECTION);			  // Select Projection
	glLoadIdentity();						  // Reset The Matrix
	glOrtho( 0, width , 0, height, -1, 1 );	  // Select Ortho 2D Mode (640x480)
	glMatrixMode(GL_MODELVIEW);				  // Select Modelview Matrix
	glLoadIdentity();						  // Reset The Matrix
}



void rect(int xmin,int xmax, int ymin , int ymax)
{
    glBegin( GL_QUADS );
    glTexCoord2i(0,1);glVertex2i(xmin,ymin);
    glTexCoord2i(0,0);glVertex2i(xmin,ymax); 
    glTexCoord2i(1,0);glVertex2i(xmax,ymax);
    glTexCoord2i(1,1);glVertex2i(xmax,ymin);
	glEnd();
}


void use_texture(GLuint *texture, GLboolean linear, GLboolean mipmapping)
{
   GLenum filter = linear ? GL_LINEAR : GL_NEAREST;
   glBindTexture(GL_TEXTURE_2D, *texture); 
   glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   
   if (mipmapping)
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
   else 
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (GLfloat) filter);
}


void SnapShot()
{
  BYTE* bmpBuffer;
  BYTE* copybuffer;
  FILE *filePtr;
  time_t tim;
  struct tm tm; 
  char buf[15]; 
  char temppath[80];

  int windowWidth=640+1;
  int windowHeight=480+1;
  int i=0;
  BITMAPFILEHEADER bitmapFileHeader;
  BITMAPINFOHEADER bitmapInfoHeader;
///////////////////////////////////

  bitmapFileHeader.bfType = 0x4D42; //"BM"
  bitmapFileHeader.bfSize = windowWidth*windowHeight*3;
  bitmapFileHeader.bfReserved1 = 0;
  bitmapFileHeader.bfReserved2 = 0;
  bitmapFileHeader.bfOffBits =
  sizeof(bitmapFileHeader) + sizeof(bitmapInfoHeader);

  bitmapInfoHeader.biSize = sizeof(bitmapInfoHeader);
  bitmapInfoHeader.biWidth = windowWidth-1;
  bitmapInfoHeader.biHeight = windowHeight-1;
  bitmapInfoHeader.biPlanes = 1;
  bitmapInfoHeader.biBitCount = 24;
  bitmapInfoHeader.biCompression = BI_RGB;
  bitmapInfoHeader.biSizeImage = 0;
  bitmapInfoHeader.biXPelsPerMeter = 0; // ?
  bitmapInfoHeader.biYPelsPerMeter = 0; // ?
  bitmapInfoHeader.biClrUsed = 0;
  bitmapInfoHeader.biClrImportant = 0;

//Get time/date stamp
  tim = time(0); 
  tm = *localtime(&tim); 
  strftime(buf, sizeof buf, "%Y%m%d%H%M%S", &tm); 
  puts(buf); 
  //////
  bmpBuffer = (BYTE*)malloc(windowWidth*windowHeight*3);
  copybuffer = (BYTE*)malloc (sizeof(bitmapFileHeader) + sizeof(bitmapInfoHeader)+ (windowWidth*windowHeight*3));

  if (!bmpBuffer)
  return;

  glReadPixels((GLint)0, (GLint)0,(GLint)windowWidth-1, (GLint)windowHeight-1,GL_BGR, GL_UNSIGNED_BYTE, bmpBuffer);
    //Make quick path
  
  strcat(temppath,"SS");
  strcat(temppath,buf);
  strcat(temppath,".bmp");
  strcat(temppath,"\0");
  wrlog("Saving Snapshot: %s",temppath);      

  filePtr = fopen(temppath, "wb");
  if (!filePtr)
  return;

  fwrite(&bitmapFileHeader, sizeof(bitmapFileHeader), 1, filePtr);
  fwrite(&bitmapInfoHeader, sizeof(bitmapInfoHeader), 1, filePtr);
  fwrite(bmpBuffer, windowWidth*windowHeight*3, 1, filePtr);
  fclose(filePtr);
  free(bmpBuffer);
}