/*
BASIC BMFont example implementation with Kerning, for C++ and OpenGL 2.0

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
--------------------------------------------------------------------------------
These editors can be used to generate BMFonts:
 • http://www.angelcode.com/products/bmfont/ (free, windows)
 • http://glyphdesigner.71squared.com/ (commercial, mac os x)
 • http://www.n4te.com/hiero/hiero.jnlp (free, java, multiplatform)
 • http://slick.cokeandcode.com/demos/hiero.jnlp (free, java, multiplatform)

Some code below based on code snippets from this gamedev posting:

http://www.gamedev.net/topic/330742-quick-tutorial-variable-width-bitmap-fonts/

Although I'm giving this away, I'd appreciate an email with fixes or better code!

aaedev@gmail.com 2012
*/

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GL/wglew.h>
#include "log.h"
#include <iostream>
#include <fstream>
#include <string>
#include <sstream> 
#include <vector>
#include "log.h"
#include "bmfont.h"
#include "gl_basics.h"


#pragma warning (disable : 4996 )


extern  RECT MyWindow;

//Todo: Add buffer overflow checking.

#define MAX_BUFFER 256

char *replace_str(char *str, char *orig, char *rep)
{
  static char buffer[MAX_BUFFER];
  char *p;

  if(!(p = strstr(str, orig)))  // Is 'orig' even in 'str'?
    return str;

  strncpy(buffer, str, p-str); // Copy characters from 'str' start to 'orig' st$
  buffer[p-str] = '\0';

  sprintf(buffer+(p-str), "%s%s", rep, p+strlen(orig));

  return buffer;
}


// This structure holds the vertices for rendering with glDrawArrays
typedef struct 
{
	float x,y;
    float texx,texy;
    GLubyte r,g,b,a;
} vlist;                      

vlist texlst[2048*4];


bool BMFont::ParseFont(char *fontfile )
{
	std::ifstream Stream(fontfile); 
	std::string Line;
	std::string Read, Key, Value;
	std::size_t i;

	bool newalloc=0;
		
	KearningInfo K;
	CharDescriptor C;

	while( !Stream.eof() )
	{
		std::stringstream LineStream;
		std::getline( Stream, Line );
		LineStream << Line;

		//read the line's type
		LineStream >> Read;
		if( Read == "common" )
		{
			//this holds common data
			while( !LineStream.eof() )
			{
				std::stringstream Converter;
				LineStream >> Read;
				i = Read.find( '=' );
				Key = Read.substr( 0, i );
				Value = Read.substr( i + 1 );

				//assign the correct value
				Converter << Value;
				if( Key == "lineHeight" )
				{Converter >> LineHeight;}
				else if( Key == "base" )
				{Converter >> Base;}
				else if( Key == "scaleW" )
				{Converter >> Width;}
				else if( Key == "scaleH" )
				{Converter >> Height;}
				else if( Key == "pages" )
				{Converter >> Pages;}
				else if( Key == "outline" )
				{Converter >> Outline;}
			}
		}
		
		else if( Read == "char" )
		{
			//this is data for a specific char
			int CharID = 0;
	
			while( !LineStream.eof() )
			{
				std::stringstream Converter;
				LineStream >> Read;
				i = Read.find( '=' );
				Key = Read.substr( 0, i );
				Value = Read.substr( i + 1 );

				//assign the correct value
				Converter << Value;
				if( Key == "id" )
				{Converter >> CharID;}
				else if( Key == "x" )
				{	Converter >> C.x;}      
				else if( Key == "y" )
				{	Converter >> C.y;   }      
				else if( Key == "width" )
				{	Converter >> C.Width;  }        
				else if( Key == "height" )
				{	Converter >> C.Height; }         
				else if( Key == "xoffset" )
				{	Converter >> C.XOffset;  }         
				else if( Key == "yoffset" )
				{	Converter >> C.YOffset; }        
				else if( Key == "xadvance" )
				{	Converter >> C.XAdvance; }         
				else if( Key == "page" )
				{	Converter >> C.Page;   }           
			}
			
         	//wrlog("Allocating character here");
			if (newalloc==0)
			{
			Chars.insert(Chars.begin(),CharID,C);
			newalloc=true;
			}
			Chars.insert(Chars.begin()+ CharID,C);
		}
		else if( Read == "kernings" )
		{
			while( !LineStream.eof() )
			{
				std::stringstream Converter;
				LineStream >> Read;
				i = Read.find( '=' );
				Key = Read.substr( 0, i );
				Value = Read.substr( i + 1 );

				//assign the correct value
				Converter << Value;
				if( Key == "count" )
				     {Converter >> KernCount; }
			}
		}

		else if( Read == "kerning" )
		{
			while( !LineStream.eof() )
			{
				std::stringstream Converter;
				LineStream >> Read;
				i = Read.find( '=' );
				Key = Read.substr( 0, i );
				Value = Read.substr( i + 1 );

				//assign the correct value
				Converter << Value;
				if( Key == "first" )
				{Converter >> K.First;}
				
				else if( Key == "second" )
				{Converter >> K.Second;  }
				
				else if( Key == "amount" )
				{Converter >> K.Amount;}
    			
			}
			//wrlog("Done with this pass");
			Kearn.push_back(K);
		}
		
	}

	Stream.close();
	return true;
}


int BMFont::Get_Kerning_Pair(int first, int second)
{
		
	 if (KernCount ) //Only process if there actually is kerning information
	 {
	 //Kearning is checked for every character processed. This is expensive in terms of processing time.
	 	 for (int j = 0; j < KernCount;  j++ )
		 {
			if (Kearn[j].First == first && Kearn[j].Second == second)
			  { 
				 return Kearn[j].Amount;
			      //wrlog("Kerning Pair Found!!!");
				 // wrlog("FIRST: %d SECOND: %d offset %d",first,second,koffset);
			  }
		 }
	 }

return 0;
}


float BMFont::Get_String_Width(const char *string, float scale)
{
  float total=0;
  CharDescriptor  *f;

  for (int i = 0; i != strlen(string); i++)
   { 
	  f=&Chars[string[i]];
      total+=f->XAdvance;
   }

return total*scale;
}


bool  BMFont::LoadFont(char *fontfile)
{
		
	std::ifstream Stream(fontfile);
	if ( !Stream.is_open() )          
	{   
		wrlog("Cannot find font file %s",fontfile);
		return false;         
	}
	Stream.close();
	
	//Ok, we have a file. Can we get the Texture as well?
    char* buf=replace_str( fontfile,".fnt", ".png");

	ftexid = LoadPNG(buf);
    if (!ftexid)
	{   
		wrlog("Cannot find font texture for loading %s",fontfile);
		return false;         
	}
	
	wrlog("Starting to parse font");
	ParseFont(fontfile);
	wrlog("Finished Parsing Font");
	KernCount = (int) Kearn.size();

	return true;
}


void Render_String(int len)
{
   //Draw Text Array, with 2D, two coordinates per vertex.
   glEnableClientState(GL_VERTEX_ARRAY);
   glVertexPointer(2, GL_FLOAT, sizeof(vlist), &texlst[0].x);

   glEnableClientState(GL_TEXTURE_COORD_ARRAY);
   glTexCoordPointer(2, GL_FLOAT, sizeof(vlist), &texlst[0].texx);

   glEnableClientState(GL_COLOR_ARRAY);
   glColorPointer (4, GL_UNSIGNED_BYTE , sizeof(vlist) , &texlst[0].r);

   glDrawArrays(GL_QUADS, 0, len*4);// 4 Coordinates for a Quad. Could use DrawElements here instead! GL_TRIANGLE_STRIP 
 
   //Finished Drawing, disable client states.
   glDisableClientState(GL_TEXTURE_COORD_ARRAY);
   glDisableClientState(GL_VERTEX_ARRAY);
   glDisableClientState(GL_COLOR_ARRAY);
}


void BMFont::glPrint(float x, float y, double scale, const char *fmt, ...)
{
	
	float CurX = (float) x;
	float CurY = (float) y;
	float DstX = 0.0;
	float DstY = 0.0;
	int Flen;
	
    float adv = (float) 1.0/Width;                  // Font texture atlas spacing. 
	char	text[512] = "";	                            // Holds Our String
	CharDescriptor  *f;									// Pointer to font.
	                                    
	va_list		ap;										// Pointer To List Of Arguments

	if (fmt == NULL)									// If There's No Text
		return;											// Do Nothing

	va_start(ap, fmt);									// Parses The String For Variables
	vsprintf(text, fmt, ap);						    // And Converts Symbols To Actual Numbers
	va_end(ap);		

	//glRotatef(90,x,y,1.0);
	//This doesn't belong here.
  	use_texture(&ftexid, 0,1);
    
	//Set Text Color, all one color for now. Gradient and shading tba.
	unsigned char *color = (unsigned char*)&fcolor;//fontcolor;
	
	y= y + LineHeight;
    Flen = strlen(text);

	for (int i = 0; i != Flen; ++i)
	{
  
		 f=&Chars[text[i]];

		 CurX = x + f->XOffset;
		 CurY = y - f->YOffset;
		 DstX = CurX + f->Width;
	     DstY = CurY - f->Height;
		 			    
         //0,1 Texture Coord
		 texlst[i*4].texx = adv*f->x;
		 texlst[i*4].texy = (float) 1.0 -(adv * (f->y + f->Height) );
         texlst[i*4].x = (float) scale * CurX;
		 texlst[i*4].y = (float) scale * DstY;
		 texlst[i*4].r = color[0];
		 texlst[i*4].g = color[1];
		 texlst[i*4].b = color[2];
		 texlst[i*4].a = color[3];

		 //1,1 Texture Coord
		 texlst[(i*4)+1].texx = adv*(f->x+f->Width);
		 texlst[(i*4)+1].texy = (float) 1.0 -(adv * (f->y + f->Height) );
         texlst[(i*4)+1].x = (float) scale * DstX;
		 texlst[(i*4)+1].y = (float) scale * DstY;
		 texlst[(i*4)+1].r = color[0];
		 texlst[(i*4)+1].g = color[1];
		 texlst[(i*4)+1].b = color[2];
		 texlst[(i*4)+1].a = color[3];

		 //1,0 Texture Coord
		 texlst[(i*4)+2].texx = adv*(f->x+f->Width);
		 texlst[(i*4)+2].texy = (float) 1.0 -(adv*f->y);
         texlst[(i*4)+2].x = (float) scale * DstX;
		 texlst[(i*4)+2].y = (float) scale * CurY;
		 texlst[(i*4)+2].r = color[0];
		 texlst[(i*4)+2].g = color[1];
		 texlst[(i*4)+2].b = color[2];
		 texlst[(i*4)+2].a = color[3];

		 //0,0 Texture Coord
		 texlst[(i*4)+3].texx = adv*f->x;
		 texlst[(i*4)+3].texy = (float) 1.0 -(adv*f->y);
         texlst[(i*4)+3].x = (float) scale * CurX;
		 texlst[(i*4)+3].y = (float) scale * CurY;
		 texlst[(i*4)+3].r = color[0];
		 texlst[(i*4)+3].g = color[1];
		 texlst[(i*4)+3].b = color[2];
		 texlst[(i*4)+3].a = color[3];

         //Only check kerning if there is greater then 1 character and 
		 //if the check character is 1 less then the end of the string.
		 if (Flen > 1 && i < Flen)
		 { 
			 x += Get_Kerning_Pair(text[i],text[i+1]);
		 }
		  
		 x +=  f->XAdvance;

  }
 	Render_String(strlen(text));
}


void BMFont::glPrintCenter( float y, double scale, const char *string)
{
	int x=0;
	CharDescriptor  *f;		 
	
	int window_width = MyWindow.right;

		int len = strlen(string);

		for (int i = 0; i != len; ++i)
		{
			f=&Chars[string[i]];

			if (len > 1 && i < len)
			 { 
			   x += Get_Kerning_Pair(string[i],string[i+1]);
			 }
			 x +=  f->XAdvance;
		}

	glPrint( (float)(MyWindow.right/2) - (x/2) , y, scale, string);
}


BMFont::~BMFont()
{
	Chars.clear();
	Kearn.clear();
	FreeTexture(ftexid);

}