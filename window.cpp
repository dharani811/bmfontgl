
#include <windows.h>
#include <gl/gl.h>
#include <gl/glu.h>
#include "gl_basics.h"
#include "log.h"
#include "globals.h"
#include "bmfont.h"

#pragma warning (disable : 4996)

HWND hWnd;
RECT MyWindow;

int WinWidth=640;
int WinHeight=480;

#pragma comment (lib, "opengl32.lib")  
#pragma comment (lib, "glu32.lib")  


LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );

// Simple, generic window init //

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    LPSTR lpCmdLine, int iCmdShow )
{
  WNDCLASS wc;
  
  MSG msg;
  bool Quit = FALSE;
  DWORD       dwExStyle;                      // Window Extended Style
  DWORD       dwStyle;                        // Window Style

  // register window class
  wc.style = CS_OWNDC;
  wc.lpfnWndProc = WndProc;
  wc.cbClsExtra = 0;
  wc.cbWndExtra = 0;
  wc.hInstance = hInstance;
  wc.hIcon = LoadIcon( NULL, IDI_APPLICATION );
  wc.hCursor = LoadCursor( NULL, IDC_ARROW );
  wc.hbrBackground = (HBRUSH)GetStockObject( BLACK_BRUSH );
  wc.lpszMenuName = NULL;
  wc.lpszClassName = "BMFontGL";
  RegisterClass( &wc );

  dwExStyle=  WS_EX_APPWINDOW;   // Window Extended Style    
  dwStyle= WS_OVERLAPPEDWINDOW | WS_THICKFRAME;                    // Windows Style
  RECT WindowRect;                                                 // Grabs Rectangle Upper Left / Lower Right Values
  WindowRect.left=(long)0;                                         // Set Left Value To 0
  WindowRect.right=(long)WinWidth;                                 // Set Right Value To Requested Width
  WindowRect.top=(long)0;                                          // Set Top Value To 0
  WindowRect.bottom=(long)WinHeight;                               // Set Bottom Value To Requested Height
  AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);      // Adjust Window To True Requested Size

   
  // Create The Window
	if (!(hWnd=CreateWindowEx(	dwExStyle,							// Extended Style For The Window
								"BMFontGL",							// Class Name
								"OpenGL BMFont Sample Implementation",						// Window Title
								dwStyle |							// Defined Window Style
								WS_CLIPSIBLINGS |					// Required Window Style
								WS_CLIPCHILDREN,					// Required Window Style
								CW_USEDEFAULT, 0,   				// Window Position
								WindowRect.right-WindowRect.left,	// Calculate Window Width
								WindowRect.bottom-WindowRect.top,	// Calculate Window Height
								NULL,								// No Parent Window
								NULL,								// No Menu
								hInstance,							// Instance
								NULL)))								// Dont Pass Anything To WM_CREATE
	{
										// Reset The Display
		MessageBox(NULL,"Window Creation Error.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}
  

  //********** Program Initializations *************

  open_log("glfont.log");
  // enable OpenGL for the window
  CreateGLContext();
  ShowWindow(hWnd,SW_SHOW);                                         // Show The Window
  SetForegroundWindow(hWnd);									    // Slightly Higher Priority
  SetFocus(hWnd);													// Sets Keyboard Focus To The Window
  ReSizeGLScene(WinWidth, WinHeight);										    // Set Up Our Perspective GL Screen
  
  wrlog("Starting Program");
 
  CheckGLVersionSupport();

  //Fill in the Window Rect;
  GetClientRect(hWnd, &MyWindow);

  ViewOrtho(WinWidth,WinHeight);
  BMFont Handel;
  BMFont Times;
  BMFont Lucida;

  if (!Lucida.LoadFont("lucida.fnt"))
  {
   MessageBox(NULL, "Error, font file not found, exiting", "File Not Found",MB_ICONERROR | MB_OK);
   Quit = TRUE;
  }
  wrlog("Font Loaded Sucessfully");
  
  if (!Handel.LoadFont("handel.fnt"))
  {
   MessageBox(NULL, "Error, font file not found, exiting", "File Not Found",MB_ICONERROR | MB_OK);
   Quit = TRUE;
  }
  wrlog("Font Loaded Sucessfully");

   if (!Times.LoadFont("times.fnt"))
  {
   MessageBox(NULL, "Error, font file not found, exiting", "File Not Found",MB_ICONERROR | MB_OK);
   Quit = TRUE;
  }
  wrlog("Font Loaded Sucessfully");
  
  // ********** Program Main Loop **********

  while ( !Quit ) {

    // check for messages
    if ( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) ) {

      // handle or dispatch messages
      if ( msg.message == WM_QUIT ) {
        Quit = TRUE;
      } else {
        TranslateMessage( &msg );
        DispatchMessage ( &msg );
      }

    } else {

  // ********** OpenGL drawing code, very simple, just to show of the font. **********

      glClearColor( 0.7f, 0.7f, 0.7f, 1.0f );
      glClear( GL_COLOR_BUFFER_BIT );
      
      // setup texture mapping
      glEnable( GL_TEXTURE_2D );
      glEnable(GL_BLEND);
      glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	  glLoadIdentity();	
	
	  Times.SetColor(250,251,252,255);
	  Times.glPrintCenter(280,1.0,"This is a different font, centered.");
	  
	  Lucida.SetColor(250,250,55,254);
	  Lucida.glPrintCenter(240,1.0,"This is another font, centered.");
	  
	  Handel.SetColor(155,155,255,255);
	  Handel.glPrint(2, 30*4, 1.0, "This is standard printing $%&!.");
      Handel.glPrint(2, 30*1, 1.4, "Scaling makes it Bigger!");
	  Handel.glPrint(2, 30/2, .7, "Or Smaller!");
	
	  Handel.SetColor(255,155,155,255);
	  Handel.glPrintCenter(320, 1.0, "This is centered printing with kerning: To Ti");
	
	  GlSwap();
     }

  }

  // ********** Cleanup and exit gracefully. **********
 
  // shutdown OpenGL
  DeleteGLContext();
  close_log();

  // destroy the window 
  DestroyWindow( hWnd );

  return msg.wParam;
}

HWND win_get_window(void)
{
   return hWnd;
}

// Window Proc ///////////////////////////////////////////////////////

LRESULT CALLBACK WndProc( HWND hWnd, UINT message,
                          WPARAM wParam, LPARAM lParam )
{

  switch ( message ) {

  case WM_CREATE:
    return 0;

  case WM_CLOSE:
    PostQuitMessage( 0 );
    return 0;

  case WM_DESTROY:
    return 0;

  case WM_KEYDOWN:
    switch ( wParam ) {

  case VK_ESCAPE:
      PostQuitMessage( 0 );
      return 0;
  case VK_F1: {SnapShot();}

    }
    return 0;

  default:
    return DefWindowProc( hWnd, message, wParam, lParam );

  }

}

