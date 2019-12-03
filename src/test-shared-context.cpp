#include <windows.h>
#include <stdio.h>
#include <gl/gl.h>
#include <gl/glext.h>
#include <gl/wglext.h>

/* ----------------------------------------------------------- */

class GlContext {
public:
  void print(const char* name);

public:
  PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = nullptr;
  PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = nullptr;
  PIXELFORMATDESCRIPTOR fmt = {};
  GlContext* shared = nullptr;
  HWND hwnd = nullptr;
  HGLRC gl = nullptr;
  HDC dc = nullptr;
  int dx = -1;
};

static int create_tmp_context(GlContext& ctx);
static int destroy_tmp_context(GlContext& ctx);
static int create_main_context(GlContext& tmp, GlContext& main);
static int destroy_main_context(GlContext& main);

/* ----------------------------------------------------------- */

int main(int narg, char* arg[]) {

  printf("! Testing with shared context.\n");

  /* Create our temporary context that we need to create our main context. */
  GlContext tmp;
  if (0 != create_tmp_context(tmp)) {
    printf("Failed to create a tmp context. (exiting).\n");
    exit(EXIT_FAILURE);
  }
  tmp.print("tmp");

  /* Create our shared context */
  GlContext shared;
  if (0 != create_main_context(tmp, shared)) {
    printf("Failed to create the shared context. (exiting).\n");
    exit(EXIT_FAILURE);
  }
  shared.print("shared");

  /* Create our main context (which can share with `shared`). */
  GlContext main;
  main.shared = &shared;

  if (0 != create_main_context(tmp, main)) {
    printf("Failed to create a main context. (exiting).\n");
    exit(EXIT_FAILURE);
  }

  main.print("main");

  return EXIT_SUCCESS;
}

/* ------------------------------------------------------------- */

static int create_tmp_context(GlContext& ctx) {

  int r = 0;
  
  /* Step 1: create a tmp window. */
  ctx.hwnd = CreateWindowA("STATIC", "dummy", 0, 0, 0, 1, 1, NULL, NULL, NULL, NULL);
  if (nullptr == ctx.hwnd) {
    printf("Failed to create our tmp window.\n");
    r = -1;
    goto error;
  }

  /* Step 2: set the pixel format. */
  ctx.fmt.nSize = sizeof(ctx.fmt);
  ctx.fmt.nVersion = 1;
  ctx.fmt.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
  ctx.fmt.iPixelType = PFD_TYPE_RGBA;
  ctx.fmt.cColorBits = 32;
  ctx.fmt.cAlphaBits = 8;
  ctx.fmt.cDepthBits = 24;

  ctx.dc = GetDC(ctx.hwnd);
  if (nullptr == ctx.dc) {
    printf("Failed to get the HDC from our tmp window.\n");
    r = -2;
    goto error;
  }

  ctx.dx = ChoosePixelFormat(ctx.dc, &ctx.fmt);
  if (0 == ctx.dx) {
    printf("Failed to find a pixel format for our tmp hdc.\n");
    r = -3;
    goto error;
  }

  if (FALSE == SetPixelFormat(ctx.dc, ctx.dx, &ctx.fmt)) {
    printf("Failed to set the pixel format on our tmp hdc.\n");
    r = -4;
    goto error;
  }

  /* Step 3. Create temporary GL context. */
  ctx.gl = wglCreateContext(ctx.dc);
  if (nullptr == ctx.gl) {
    printf("Failed to create out temporary GL context.\n");
    r = -5;
    goto error;
  }

  if (FALSE == wglMakeCurrent(ctx.dc, ctx.gl)) {
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
  
  if (nullptr != ctx.hwnd) {
    if (FALSE == DestroyWindow(ctx.hwnd)) {
      printf("Failed to destroy the hwnd.\n");
      r -= 1;
    }
  }

  if (nullptr != ctx.gl) {
    
    if (FALSE == wglMakeCurrent(nullptr, nullptr)) {
      printf("Failed to reset any current OpenGL contexts.\n");
    }
    
    if (FALSE == wglDeleteContext(ctx.gl)) {
      printf("Failed to delete the temporary context.\n");
      r -= 2;
    }
  }

  /* Cleanup the members. */
  ctx.hwnd = nullptr;
  ctx.dc = nullptr;
  ctx.dx = -1;
  ctx.gl = nullptr;
  ctx.shared = nullptr;
  ctx.wglChoosePixelFormatARB = nullptr;
  ctx.wglCreateContextAttribsARB = nullptr;

  memset((char*)&ctx.fmt, 0x00, sizeof(ctx.fmt));
  
  return r;
}

static int create_main_context(GlContext& tmp, GlContext& main) {

  int r = 0;
  UINT fmt_count = 0;
  HGLRC shared_gl = nullptr;

  /* The pixel format attributes that we need. */
  const int pix_attribs[] = {
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

  /* The OpenGL Rendering Context attributes we need. */
  int ctx_attribs[] = {
    WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
    WGL_CONTEXT_MINOR_VERSION_ARB, 1,
    WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
    0
  };

  /* Validate */
  if (nullptr == tmp.gl) {
    printf("Given tmp context is invalid.\n");
    r = -1;
    goto error;
  }

  if (nullptr == tmp.wglChoosePixelFormatARB) {
    printf("Given tmp context has no `wglChoosePixelFormatARB()` set.\n");
    r = -2;
    goto error;
  }

  if (nullptr == tmp.wglCreateContextAttribsARB) {
    printf("Given tmp context has no `wglCreateContextAttribsARB()` set.\n");
    r = -3;
    goto error;
  }
  
  /* Step 1: we still need a HWND for our main context. */
  main.hwnd = CreateWindowA("STATIC", "dummy", 0, 0, 0, 1, 1, NULL, NULL, NULL, NULL);
  if (nullptr == main.hwnd) {
    printf("Failed to create main window (hwnd).\n");
    r = -4;
    goto error;
  }

  /* Step 2: set the pixel format */
  main.dc = GetDC(main.hwnd);
  if (nullptr == main.dc) {
    printf("Failed to get the HDC from our main window.\n");
    r = -5;
    goto error;
  }

  /* Find the best matching pixel format index. */
  if (FALSE == tmp.wglChoosePixelFormatARB(main.dc, pix_attribs, NULL, 1, &main.dx, &fmt_count)) {
    printf("Failed to choose a valid pixel format for our main hdc.\n");
    r = -6;
    goto error;
  }

  /* Now that we have found the index, fill our format descriptor. */
  if (0 == DescribePixelFormat(main.dc, main.dx, sizeof(main.fmt), &main.fmt)) {
    printf("Failed to fill our main pixel format descriptor.\n");
    r = -7;
    goto error;
  }

  if (FALSE == SetPixelFormat(main.dc, main.dx, &main.fmt)) {
    printf("Failed to set the pixel format on our main dc.\n");
    r = -8;
    goto error;
  }

  /* Step 3: create our main context. */
  if (nullptr != main.shared) {
    shared_gl = main.shared->gl;
  }
  
  main.gl = tmp.wglCreateContextAttribsARB(main.dc, shared_gl, ctx_attribs);
  if (nullptr == main.gl) {
    printf("Failed to create our main OpenGL context.\n");
    r = -8;
    goto error;
  }
  
 error:
  
  if (r < 0) {
    printf("Failed to create the main context.\n");
    if (0 != destroy_main_context(main)) {
      printf("After failing to create our main context, we also couldn't clean it up correctly.\n");
    }
  }

  return r;
}

static int destroy_main_context(GlContext& main) {
  return destroy_tmp_context(main);
}

/* ------------------------------------------------------------- */

void GlContext::print(const char* name) {

  if (nullptr == name) {
    printf("Cannot print, pass in a name.\n");
    return;
  }
  
  if (nullptr == gl) {
    printf("Cannot print info, not initialized (gl == nullptr) .\n");
    return;
  }

  if (nullptr == dc) {
    printf("Cannot print into, not initialized (dc == nullptr). \n");
    return;
  }

  wglMakeCurrent(dc, gl);
  printf("%s: GL_VERSION: %s\n", name, glGetString(GL_VERSION));
  printf("%s: GL_VENDOR: %s\n", name, glGetString(GL_VENDOR));
}

/* ------------------------------------------------------------- */
