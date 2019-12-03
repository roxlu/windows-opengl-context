/*

  BARE BONE OPENGL CONTEXT 
  =========================

  I'm doing some research on creating a shared OpenGL
  context. I'm doing this to figure out why the Filament library
  crashes when it uses a shared context. 
  
  I'm starting with the most basic way to create a Window and a
  OpenGL 4.1 context, as described [here][0]. Another good
  resource that describes how to create an OpenGL context can [be
  found here][1].

  OUTCOME / RESULT:
  ==================

  After diving into this I think I know enough to continue
  testing with GLFW and Filament. Have a look at the `SUMMARY`
  section in the code (inside main()) below. I'll describe some
  relevant things in the paragraphs below.

  Device Contexts
  ----------------

  As Device Contexts (DCs) are very common when reading up about
  the Windows API and the OpenGL Windows API (WGL) I think it
  would be good to paste some snippets here. As a reference see
  [this page on Device Contexts][5].

  [This article starts with a pretty clear description][5] :

     A device context is a Windows data structure containing
     information about the drawing attributes of a device such as
     a display or a printer. All drawing calls are made through a
     device-context object, which encapsulates the Windows APIs
     for drawing lines, shapes, and text. Device contexts allow
     device-independent drawing in Windows. Device contexts can
     be used to draw to the screen, to the printer, or to a
     metafile.

  [Another article][6] shares a bit more info:

     A device context is a structure that defines a set of graphic
     objects and their associated attributes, as well as the
     graphic modes that affect output. The graphic objects include
     a pen for line drawing, a brush for painting and filling, a
     bitmap for copying or scrolling parts of the screen, a
     palette for defining the set of available colors, a region
     for clipping and other operations, and a path for painting
     and drawing operations.

  [In turn, this article][7] describes what graphic objects are:
  
     The pen, brush, bitmap, palette, region, and path associated
     with a DC are referred to as its graphic objects. The
     following attributes are associated with each of these
     objects.  

   [Device Context Types][8]

     There are four types of DCs: display, printer, memory (or
     compatible), and information. Each type serves a specific
     purpose, as described in the following table.

     Display Device Context:

     An application obtains a display DC by calling the
     BeginPaint, GetDC, or GetDCEx functions and identifying the
     window in which the corresponding output will
     appear. Typically, an application obtains a display DC only
     when it must draw in the client area. However, one may
     obtain a window device context by calling the GetWindowDC
     function. When the application is finished drawing, it must
     release the DC by calling the EndPaint or ReleaseDC
     function.

     There are five types of DCs for video displays:
     
     - Class
     - Common
     - Private
     - Window
     - Parent

     Private Display Device Context:
     -------------------------------- 

     Private device contexts are display DCs that, unlike common
     device contexts, retain any changes to the default data even
     after an application releases them. Private device contexts
     are used in applications that perform numerous drawing
     operations such as computer-aided design (CAD) applications,
     desktop-publishing applications, drawing and painting
     applications, and so on. Private device contexts are not
     part of the system cache and therefore need not be released
     after use. The system automatically removes a private device
     context after the last window of that class has been
     destroyed.

     An application creates a private device context by first
     specifying the CS_OWNDC window-class style when it
     initializes the style member of the WNDCLASS structure and
     calls the RegisterClass function. (For more information
     about window classes, see Window Classes.)

     After creating a window with the CS_OWNDC style, an
     application can call the GetDC, GetDCEx, or BeginPaint
     function once to obtain a handle identifying a private
     device context. The application can continue using this
     handle (and the associated DC) until it deletes the window
     created with this class. Any changes to graphic objects and
     their attributes, or graphic modes are retained by the
     system until the window is deleted.
 
     The CS_OWNDC style:

     - See [this forum thread][2] 
     - See [this Microsoft Devblog article][3] 
     - See above, `Private Display Device Context`

  Pixel Formats:
  ---------------

  Before you can create a OpenGL Rendering Context, you have
  create a window that fits the needs of the GL Rendering
  Context. This includes things like if you want a depth buffer,
  single or double buffer, stencil, etc. To do this you have to
  find the pixel format that is suitable for your needs.

  The pixel format for a window is identified by an index. Before
  you can search for the correct pixel format you should make use
  of the `WGL_ARB_pixel_format` extension. This extension allows
  you to select a pixel format that's not provided through the
  windows default features; You can read up on why the OpenGL
  functions that Windows provides are so limited, but in short it
  has to do that Windows basically didn't want to keep their
  OpenGL support up-to-date. Luckily OpenGL supports extensions.
  An extension can be loaded at runtime; meaning that the
  functions that the extension supports are dynamically loaded;
  much like a dll plugin.

  The `wglChoosePixeFormatARB()` provides a big list with
  attributes that can be associated with a context. To be able to
  use this function you have to create a dummy context first.
  This is the typical crazy OpenGL dance we have to do.

  The first argument that we pass into this function is a HDC.
  This is the device context of the window. You can use
  `GetDC(HWND hdwnd)` to retrieve this device context. This HDC
  can then be used to draw into the client area of the window.

  Filament
  ---------

  - Filament creates a PIXELFORMATDESCRIPTOR with the `PFD_MAIN_PLANE`
    [the documentation][11] tells use that this field is not used anymore;
    Why is filament using this flag?  From my understanding is that this
    describes what framebuffer is used; e.g. do we share our buffer
    with the Windows framebuffer or an overlay buffer ...

  - IMPORTANT: [I think this][12] is a super important message, especially 
    when dealing with share GL contexts and multiple threads: 
    

  REFERENCES: 

    [0]: https://docs.microsoft.com/en-us/windows/win32/learnwin32/your-first-windows-program 
    [1]: https://mariuszbartosik.com/opengl-4-x-initialization-in-windows-without-a-framework/
    [2]: https://community.khronos.org/t/about-cs-owndc/29018/14 "Thread about CS_OWND" style."
    [3]: https://devblogs.microsoft.com/oldnewthing/20060601-06/?p=31003 "Background info on CS_OWN"
    [4]: https://docs.microsoft.com/en-us/windows/win32/gdi/display-device-contexts "Display Device Contexts"
    [5]: https://docs.microsoft.com/en-us/cpp/mfc/device-contexts?view=vs-2019 "Device Contexts"
    [6]: https://docs.microsoft.com/en-us/windows/win32/gdi/device-contexts "Device Contexts"
    [7]: https://docs.microsoft.com/en-us/windows/win32/gdi/graphic-objects "Graphics Objects"
    [8]: https://docs.microsoft.com/en-us/windows/win32/gdi/device-context-types "Device Context Types"
    [9]: https://docs.microsoft.com/en-us/windows/win32/gdi/display-device-contexts "Display Device Context"
    [10]: https://gist.github.com/roxlu/427580c63be3c4f3d44c448d60a411f0 "This file, with the minimal that is needed to create a simple window"
    [11]: https://docs.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-pixelformatdescriptor "PixelFormatDescriptor reference"
    [12]: https://docs.microsoft.com/en-us/windows/win32/opengl/multithread-opengl-drawing-strategies "OpenGL and Multi Threading"
*/

/* ----------------------------------------------------------- */

#ifndef UNICODE
#  define UNICODE
#endif 

#include <windows.h>
#include <stdio.h>
#include <gl/gl.h>
#include <gl/glext.h>
#include <gl/wglext.h>

/* ----------------------------------------------------------- */

LRESULT CALLBACK window_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

/* ----------------------------------------------------------- */

int main(int narg, char* arg[]) {

  /* 
     ... we have to create a temporary window so that we can use
     it's HDC to create a dummy OpenGL Context. This `dummy`
     OpenGL context is then used to retrieve the necessary
     INITIAL extensions that we need to create our real OpenGL
     context that supports the features that we need; these
     features are normally requested with `wglChoosePixelFormat`
     but because this `wglChoosePixelFormat` function is ancient
     and Windows hasn't updated their OpenGL API support we have
     to query an extension that allows us to use the
     `wglChoosePixelFormatARB()`. This last function provides
     everything we need to setup a OpenGL Rendering Context.
  */

  printf("- Register our dummy class.\n");

  HINSTANCE inst = GetModuleHandle(nullptr);
  
  /* Register our dummy class */
  const wchar_t* dummy_class_name = L"dummy-class";
  WNDCLASSEX dummy_class = {};
  dummy_class.cbSize = sizeof(dummy_class);
  dummy_class.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
  dummy_class.lpfnWndProc = window_proc;
  dummy_class.hInstance = inst;
  dummy_class.hCursor = LoadCursor(NULL, IDC_ARROW);
  dummy_class.lpszClassName = dummy_class_name;

  ATOM dummy_atom = RegisterClassEx(&dummy_class);
  if (0 == dummy_atom) {
    printf("Failed to register the dummy class. (exiting).\n");
    exit(EXIT_FAILURE);
  }

  /* Create a window for the class that we just registered. */
  HWND dummy_hwnd = CreateWindow(
    dummy_class_name,
    L"window title for dummy", 
    WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 
    0, 0, 1, 1,                       
    nullptr,
    nullptr,                 
    inst,
    nullptr
  );       

  if (nullptr == dummy_hwnd) {
    printf("Failed to create our dummy window. (exiting).\n");
    exit(EXIT_FAILURE);
  }

  /* Get the device context for the client area in the window. */
  HDC dummy_hdc = GetDC(dummy_hwnd);
  if (nullptr == dummy_hdc) {
    printf("Failed to get the Device Context to our dummy window. (exiting).\n");
    exit(EXIT_FAILURE);
  }

  printf("- We got a dummy HDC\n");

  /* 
     Now that we have a device context, we have to create a pixel
     format description that we can pass into
     `ChoosePixelFormat()`. Once we have a pixel format we can
     create our dummy rendering context. There are many more
     fields but here we only set the fields that are required for
     `ChoosePixelFormat()`.

     In `filament` the `dwLayerMask` is set to PFD_MAIN_PLANE. 
     [This reference][11] tells us that this field is not used anymore.
     
  */
  PIXELFORMATDESCRIPTOR dummy_pix_fmt = {};
  dummy_pix_fmt.nSize = sizeof(dummy_pix_fmt);
  dummy_pix_fmt.nVersion = 1;
  dummy_pix_fmt.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER; /* Q: what flags should we use here; as this is a dummy only should be bother with e.g. duoble buffering?  A: See `ChoosePixelFormat()` that lists the requirements. */
  dummy_pix_fmt.iPixelType = PFD_TYPE_RGBA;
  dummy_pix_fmt.cColorBits = 32;
  dummy_pix_fmt.cAlphaBits = 8;
  dummy_pix_fmt.cDepthBits = 24;

  int dummy_pix_fmt_dx = ChoosePixelFormat(dummy_hdc, &dummy_pix_fmt);
  if (0 == dummy_pix_fmt_dx) {
    printf("Failed to select find a pixel format that matches our dummy pixel format descriptor. (exiting). \n");
    exit(EXIT_FAILURE);
  }

  printf("- We found a dummy pixel format: %d\n", dummy_pix_fmt);

  if (FALSE == SetPixelFormat(dummy_hdc, dummy_pix_fmt_dx, &dummy_pix_fmt)) {
    printf("Failed to set the pixel format for our dummy device context. (exiting).\n");
    exit(EXIT_FAILURE);
  }

  printf("- We've set the pixel format on our dummy device context.\n");

  /*
    Now that the dummy device context know about the pixel format to use, we 
    can create a dummy OpenGL context that uses this pixel format. Once we have
    a dummy OpenGL context, we retrieve the extension that gives us
    `wglChoosePixelFormatARB()`
   */
  HGLRC dummy_gl_ctx = wglCreateContext(dummy_hdc);
  if (0 == dummy_gl_ctx) {
    printf("Failed to create our dummy opengl context. (exiting).\n");
    exit(EXIT_FAILURE);
  }

  printf("- We've created our dummy OpenGL context.\n");

  if (FALSE == wglMakeCurrent(dummy_hdc, dummy_gl_ctx)) {
    printf("Failed to make our dummy OpenGL context current. (exiting).\n");
    exit(EXIT_FAILURE);
  }

  printf("- We've made our dummy OpenGL context current.\n");

  /* ------------------------------------------------------------------------------- */
  /*          C R E A T E   " M O D E R N "  O P E N G L  C O N T E X T              */
  /* ------------------------------------------------------------------------------- */
  
  /* 
     Again, create a window. We can reuse our registered windows
     `class` (see above, where we defined the WNDCLASSEX and set
     a pointer to our window procedure). We have to create a new
     window because you can only assign a pixel format to a
     window once.
  */
  
  printf("- We've got a handle to `wglChoosePixelFormatARB()`\n");

  HWND main_hwnd = CreateWindowEx(
    0,                              /* Optional window styles. */
    dummy_class_name,               /* Window class */
    L"roxlu",                       /* Window title */
    WS_OVERLAPPEDWINDOW,            /* Window style */

    /* Size and position */
    CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

    nullptr,       /* Parent window */
    nullptr,       /* Menu */
    inst,          /* Instance handle */
    nullptr        /* Additional application data */
  );

  if (nullptr == main_hwnd) {
    printf("`main_hwnd` is nullptr. Failed to create our main window. (exiting).\n");
    exit(EXIT_FAILURE);
  }

  HDC main_hdc = GetDC(main_hwnd);
  if (nullptr == main_hdc) {
    printf("Failed to get the DC for our `main_hwnd`. (exiting).\n");
    exit(EXIT_FAILURE);
  }
  
  /* 
     Finally ... we can use our dummy OpenGL context, to retrieve
     the `wglChoosePixelForamtARB()~ function that gives us a way
     to create a OpenGL context that supports more modern
     features (e.g. like multi sampling).
   */
  PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = nullptr;
  wglChoosePixelFormatARB = reinterpret_cast<PFNWGLCHOOSEPIXELFORMATARBPROC>(wglGetProcAddress("wglChoosePixelFormatARB"));
  if (wglChoosePixelFormatARB == nullptr) {
    printf("Failed to get the `wglChoosePixelFormatARB()` function. (exiting).\n");
    exit(EXIT_FAILURE);
  }

  /* Now we can use a pixel format with more features. */
  const int main_pix_attribs[] = {
    WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
    WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
    WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
    WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
    WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
    WGL_COLOR_BITS_ARB, 32,
    WGL_ALPHA_BITS_ARB, 8,
    WGL_DEPTH_BITS_ARB, 24,
    WGL_STENCIL_BITS_ARB, 8,
    WGL_SAMPLE_BUFFERS_ARB, GL_TRUE,
    WGL_SAMPLES_ARB, 4,
    0
  };
 
  int main_pix_fmt_dx = 0;
  UINT main_pix_fmt_count = 0;
  if (FALSE == wglChoosePixelFormatARB(main_hdc, main_pix_attribs, NULL, 1, &main_pix_fmt_dx, &main_pix_fmt_count)) {
    printf("Failed to choose a valid pixel format for our main hdc. (exiting).\n");
    exit(EXIT_FAILURE);
  }

  /* 
     Now that we found a valid pixel format index, we have to
     fill a PIXELFORMATDESCRIPTOR to tell our HDC what pixel
     format it should use. To create our PIXELFORMATDESCRIPTOR we
     use `DescribePixelFormat()`.
  */
  PIXELFORMATDESCRIPTOR main_pix_fmt = {};
  if (0 == DescribePixelFormat(main_hdc, main_pix_fmt_dx, sizeof(main_pix_fmt), &main_pix_fmt)) {
    printf("Failed to fill our main pixel format descriptor. (exiting).\n");
    exit(EXIT_FAILURE);
  }

  if (FALSE == SetPixelFormat(main_hdc, main_pix_fmt_dx, &main_pix_fmt)) {
    printf("Failed to set our main pixel format. (exiting).\n");
    exit(EXIT_FAILURE);
  }

  /* 
     Nice, we have our main HDC and setup the pixel format. Now,
     it's finally time to create our main GL context.
  */
  PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = nullptr;
  wglCreateContextAttribsARB = reinterpret_cast<PFNWGLCREATECONTEXTATTRIBSARBPROC>(wglGetProcAddress("wglCreateContextAttribsARB"));
  if (wglCreateContextAttribsARB == nullptr) {
    printf("wglGetProcAddress() failed. (exiting).\n");
    exit(EXIT_FAILURE);
  }

  int main_ctx_attribs[] = {
    WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
    WGL_CONTEXT_MINOR_VERSION_ARB, 1,
    WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
    0
  };
  
  HGLRC main_ctx = wglCreateContextAttribsARB(main_hdc, 0, main_ctx_attribs);
  if (nullptr == main_ctx) {
    printf("Failed to create our main OpenGL context. (exiting).\n");
    exit(EXIT_FAILURE);
  }
    
  if (FALSE == wglMakeCurrent(main_hdc, main_ctx)) {
    printf("Failed to make our main OpenGL context current. (exiting).\n");
    exit(EXIT_FAILURE);
  }

  printf("- Found pixel format for our main hwnd and GL context: %d\n", main_pix_fmt_dx);
  printf("- Max pixel formats found: %d\n", main_pix_fmt_count);
  printf("- GL_VERSION: %s\n", glGetString(GL_VERSION));
  printf("- GL_VENDOR: %s\n", glGetString(GL_VENDOR));
  
  /* ------------------------------------------------------------- */
  /*

    SUMMARY
    ========

    At this point I had a good understanding of how you create a context
    and why/how you need to create a dummy context. I had a chat with derhass
    on ##OpenGL who explained (my own wordings/understanding) the points below.

    - To be able to create a shared context you have to be sure
      that the GL implementation is compatible.
 
    - A context is compatible when theh IDC dll is the same; e.g.
      you can share between contexts that are created by the same
      GL driver (nvidia, intel, etc). 

    - To create a context that is compatible with another one, you
      can make sure that the pixel format is the same. When you have 
      a HWND you can query the pixel format (... I have to check this..)

   - To check if a context is compatible the GL_VENDOR and GL_RENDERER 
     need to be the same.

   */
  /* ------------------------------------------------------------- */

  ShowWindow(main_hwnd, SW_SHOW);

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

     case WM_KEYDOWN: {
       if (VK_ESCAPE == wparam) {
         PostQuitMessage(0);
       }
       return 0;
     }
     
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
