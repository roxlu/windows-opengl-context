/*

  I'm doing some research on creating a shared OpenGL
  context. I'm doing this to figure out why the Filament library
  cause a crash when trying to make their OpenGL context, which
  is shared crashes.
  
  So starting with the most basic windows as described here:
  https://docs.microsoft.com/en-us/windows/win32/learnwin32/your-first-windows-program

*/

/* ----------------------------------------------------------- */

#ifndef UNICODE
#  define UNICODE
#endif 

#include <windows.h>
#include <stdio.h>

/* ----------------------------------------------------------- */

LRESULT CALLBACK window_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

/* ----------------------------------------------------------- */

INT WinMain(HINSTANCE inst, HINSTANCE prevInst, PSTR cmdLine, INT cmdShow) {

  const wchar_t class_name[]  = L"roxlu";
    
  WNDCLASS wc = { };
  wc.lpfnWndProc   = window_proc;
  wc.hInstance     = inst;
  wc.lpszClassName = class_name;

  RegisterClass(&wc);
    
  HWND hwnd = CreateWindowEx(
    0,                              // Optional window styles.
    class_name,                     // Window class
    L"roxlu",                       // Window text
    WS_OVERLAPPEDWINDOW,            // Window style

    // Size and position
    CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

    NULL,       // Parent window    
    NULL,       // Menu
    inst,       // Instance handle
    NULL        // Additional application data
  );

  if (nullptr == hwnd) {
    printf("Error: failed to create the window.\n");
    return 0;
  }

  ShowWindow(hwnd, cmdShow);

  MSG msg = { };
  while (GetMessage(&msg, NULL, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  return 0;
}

/* ----------------------------------------------------------- */

LRESULT CALLBACK window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {

   switch (msg) {
     
     case WM_DESTROY: {
       PostQuitMessage(0);
       return 0;
     }
       
    case WM_PAINT: {
      PAINTSTRUCT ps;
      HDC hdc = BeginPaint(hwnd, &ps);
      FillRect(hdc, &ps.rcPaint, (HBRUSH) (COLOR_WINDOW+2));
      EndPaint(hwnd, &ps);
      return 0;
    }
   }
   
   return DefWindowProc(hwnd, msg, wparam, lparam);
}


/* ----------------------------------------------------------- */
