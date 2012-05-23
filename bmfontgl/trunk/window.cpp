
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

 BMFont *Snap;
 BMFont *Times;
 BMFont *Lucida;

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
  
  LogOpen("glfont.log");
  // enable OpenGL for the window
  CreateGLContext();
  ShowWindow(hWnd,SW_SHOW);                                         // Show The Window
  SetForegroundWindow(hWnd);									    // Slightly Higher Priority
  SetFocus(hWnd);													// Sets Keyboard Focus To The Window
  ReSizeGLScene(WinWidth, WinHeight);										    // Set Up Our Perspective GL Screen
  
  WRLOG("Starting Program");
 
  CheckGLVersionSupport();

  //Fill in the Window Rect;
  GetClientRect(hWnd, &MyWindow);

  ViewOrtho(WinWidth,WinHeight);
 

  Snap = new BMFont;
  Times = new BMFont;
  Lucida = new BMFont;


  if (!Lucida->LoadFont("lucida.fnt"))
  {
   MessageBox(NULL, "Error, font file not found, exiting", "File Not Found",MB_ICONERROR | MB_OK);
   Quit = TRUE;
  }
  LOG_DEBUG("Font Loaded Sucessfully");
  
  if (!Snap->LoadFont("snap.fnt"))
  {
   MessageBox(NULL, "Error, font file not found, exiting", "File Not Found",MB_ICONERROR | MB_OK);
   Quit = TRUE;
  }
  LOG_DEBUG("Font Loaded Sucessfully");

   if (!Times->LoadFont("times.fnt"))
  {
   MessageBox(NULL, "Error, font file not found, exiting", "File Not Found",MB_ICONERROR | MB_OK);
   Quit = TRUE;
  }
  LOG_DEBUG("Font Loaded Sucessfully");
  
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

      glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
      glClear( GL_COLOR_BUFFER_BIT );
      
      // setup texture mapping
      glEnable( GL_TEXTURE_2D );
      
      glLoadIdentity();	
	
	  Times->SetColor(250,251,252,255);
	  Times->PrintCenter(280,"A different font, centered. Kerning: To Ti");
	  
	  Lucida->SetColor(250,250,55,254);
	  Lucida->PrintCenter(240,"This is another font.");
	  
	  Snap->SetColor(255,255,255,255);
	  Snap->Print(2, Snap->GetHeight()*4, "This is standard printing.");
	  Snap->SetScale(1.35f);
      Snap->Print(2, Snap->GetHeight()/4, "Scaling makes it Big!");
	  Snap->SetScale(0.7f);
	  Snap->Print(2, Snap->GetHeight()*4, "Or it can make it smaller!");
	  Snap->SetScale(1.0f);
	  Snap->SetColor(255,255,255,255);
	  Snap->PrintCenter(320, "Centered printing: To Ti");
	
	  GlSwap();
     }

  }

  // ********** Cleanup and exit gracefully. **********
 
  // shutdown OpenGL
  DeleteGLContext();
  LogClose();

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
      PostQuitMessage( 0 );  return 0;

  case VK_F1: {SnapShot();return 0;}

  case VK_F2: {Times->UseKerning(false);Snap->UseKerning(false); Lucida->UseKerning(false); LOG_DEBUG("Times kerning disabled");return 0;}

  case VK_F3: {Times->UseKerning(true);Snap->UseKerning(true); Lucida->UseKerning(true);LOG_DEBUG("Times kerning enabled");return 0;}

    }
    return 0;

  default:
    return DefWindowProc( hWnd, message, wParam, lParam );

  }

}

