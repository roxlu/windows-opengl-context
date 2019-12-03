#include <windows.h>
#include <stdio.h>
#include <gl/gl.h>
#include <gl/glext.h>
#include <gl/wglext.h>

/* ----------------------------------------------------------- */

LRESULT CALLBACK window_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

/* ----------------------------------------------------------- */

struct GlContext {
  HWND tmp_hwnd = nullptr;
  HDC tmp_dc = nullptr;
  int tmp_dx = -1;
  PIXELFORMATDESCRIPTOR tmp_fmt = {};
  HGLRC tmp_gl = nullptr;
  PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = nullptr;
  PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = nullptr;
};

static int create_tmp_context(GlContext& ctx);
static int destroy_tmp_context(GlContext& ctx);

/* ----------------------------------------------------------- */

int main(int narg, char* arg[]) {

  printf("! Testing with shared context between threads.\n");

  
  printf("! Created a temporary GL context: %s\n", glGetString(GL_VERSION));


  

  return EXIT_SUCCESS;
}

/* ------------------------------------------------------------- */

static int create_tmp_context(GlContext& ctx) {

  int r = 0;
  
  /* Step 1: create a tmp window. */
  ctx.tmp_hwnd = CreateWindowA("STATIC", "dummy", 0, 0, 0, 1, 1, NULL, NULL, NULL, NULL);
  if (nullptr == ctx.tmp_hwnd) {
    printf("Failed to create our tmp window.\n");
    r = -1;
    goto error;
  }

  /* Step 2: set the pixel format. */
  ctx.tmp_fmt.nSize = sizeof(ctx.tmp_fmt);
  ctx.tmp_fmt.nVersion = 1;
  ctx.tmp_fmt.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
  ctx.tmp_fmt.iPixelType = PFD_TYPE_RGBA;
  ctx.tmp_fmt.cColorBits = 32;
  ctx.tmp_fmt.cAlphaBits = 8;
  ctx.tmp_fmt.cDepthBits = 24;

  ctx.tmp_dc = GetDC(ctx.tmp_hwnd);
  if (nullptr == ctx.tmp_dc) {
    printf("Failed to get the HDC from our tmp window.\n");
    r = -2;
    goto error;
  }

  ctx.tmp_dx = ChoosePixelFormat(ctx.tmp_dc, &ctx.tmp_fmt);
  if (0 == ctx.tmp_dx) {
    printf("Failed to find a pixel format for our tmp hdc.\n");
    r = -3;
    goto error;
  }

  if (FALSE == SetPixelFormat(ctx.tmp_dc, ctx.tmp_dx, &ctx.tmp_fmt)) {
    printf("Failed to set the pixel format on our tmp hdc.\n");
    r = -4;
    goto error;
  }

  /* Step 3. Create temporary GL context. */
  ctx.tmp_gl = wglCreateContext(ctx.tmp_dc);
  if (nullptr == ctx.tmp_gl) {
    printf("Failed to create out temporary GL context.\n");
    r = -5;
    goto error;
  }

  if (FALSE == wglMakeCurrent(ctx.tmp_dc, ctx.tmp_gl)) {
    printf("Failed to make our temporary GL context current.\n");
    r = -6;
    goto error;
  }

  /* Step 4. Get the extension we need for a more feature-rich context. */
  ctx.wglChoosePixelFormatARB = reinterpret_cast<PFNWGLCHOOSEPIXELFORMATARBPROC>(wglGetProcAddress("wglChoosePixelFormatARB"));
  if (nullptr == ctx.wglChoosePixelFormatARB) {
    printf("Failed to get the `wglChoosePixelFormatARB()` function.\n");
    r = -7;
    goto error;
  }
  
  ctx.wglCreateContextAttribsARB = reinterpret_cast<PFNWGLCREATECONTEXTATTRIBSARBPROC>(wglGetProcAddress("wglCreateContextAttribsARB"));
  if (nullptr == ctx.wglCreateContextAttribsARB) {
    printf("wglGetProcAddress() failed.\n");
    r = -8;
    goto error;
  }

 error:

  if (r < 0) {
    if (0 != destroy_tmp_context(ctx)) {
      printf("After failing to create a tmp context ... we also failed to deallocate some temporaries :(\n");
    }
  }

  return r;
}

static int destroy_tmp_context(GlContext& ctx) {

  int r = 0;
  
  if (nullptr != ctx.tmp_hwnd) {
    if (FALSE == DestroyWindow(ctx.tmp_hwnd)) {
      printf("Failed to destroy the tmp_hwnd.\n");
      r -= 1;
    }
  }

  if (nullptr != ctx.tmp_gl) {
    
    if (FALSE == wglMakeCurrent(nullptr, nullptr)) {
      printf("Failed to reset any current OpenGL contexts.\n");
    }
    
    if (FALSE == wglDeleteContext(ctx.tmp_gl)) {
      printf("Failed to delete the temporary context.\n");
      r -= 2;
    }
  }

  /* Cleanup the members. */
  ctx.tmp_hwnd = nullptr;
  ctx.tmp_dc = nullptr;
  ctx.tmp_dx = -1;
  ctx.tmp_gl = nullptr;
  ctx.wglChoosePixelFormatARB = nullptr;
  ctx.wglCreateContextAttribsARB = nullptr;

  memset((char*)&ctx.tmp_fmt, 0x00, sizeof(ctx.tmp_fmt));
  
  return r;
}

/* ------------------------------------------------------------- */
