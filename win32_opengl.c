#define  FOUNDATION_IMPL
#include "foundation.h"

#include "opengl.h"

#include "app.h"

#include "math.h"

function void *GetAnyGlProc(HMODULE GlDll, PROC (*wglGetProcAddressProc)(LPCSTR), const c8 *Name) {
  void *Proc = (void*)wglGetProcAddressProc(sName);
  if (Proc == 0 || (Proc == (void*)0x1) || (Proc == (void*)0x2) || (Proc == (void*)0x3) || (Proc == (void*)-1))
    Proc = (void *)GetProcAddress(GlDll, Name);
  return Proc;
}

typedef HGLRC wgl_create_context_proc   (HDC);
typedef BOOL  wgl_delete_context_proc   (HGLRC);
typedef BOOL  wgl_make_current_proc     (HDC, HGLRC);
typedef PROC  wgl_get_proc_address_proc (LPCSTR);
wgl_create_context_proc   *wglCreateContextProc  = null;
wgl_delete_context_proc   *wglDeleteContextProc  = null;
wgl_make_current_proc     *wglMakeCurrentProc    = null;
wgl_get_proc_address_proc *wglGetProcAddressProc = null;

static DWORD WINAPI MainThread(LPVOID Param) {
  HWND      Window   = (HWND)Param;
  HINSTANCE Instance = GetModuleHandleW(null);
  HDC       DevCtx   = GetDC(Window);
  HGLRC     GlCtx    = null;

  HMODULE OpenGlDll = LoadLibraryA("opengl32.dll");
  wglCreateContextProc  = (wgl_create_context_proc*)GetProcAddress(OpenGlDll, "wglCreateContext");
  wglDeleteContextProc  = (wgl_delete_context_proc*)GetProcAddress(OpenGlDll, "wglDeleteContext");
  wglMakeCurrentProc    = (wgl_make_current_proc*)GetProcAddress(OpenGlDll, "wglMakeCurrent");
  wglGetProcAddressProc = (wgl_get_proc_address_proc*)GetProcAddress(OpenGlDll, "wglGetProcAddress");

  WNDCLASSW DumbClass = {
    .lpfnWndProc   = DefWindowProcW,
    .hInstance     = Instance,
    .lpszClassName = L"dummy class"
  };
  RegisterClassW(&DumbClass);
  HWND DumbWindow = CreateWindowW(L"dummy class", L"dummy title", WS_TILEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, Instance, 0);
  HDC  DumbDevCtx = GetDC(DumbWindow);
  PIXELFORMATDESCRIPTOR DumbPixDesc = {
    .nSize      = sizeof(DumbPixDesc),
    .nVersion   = 1,
    .dwFlags    = PFD_SUPPORT_OPENGL,
    .iPixelType = PFD_TYPE_RGBA,
    .cColorBits = 24
  };
  INT DumbPixIdx = ChoosePixelFormat(DumbDevCtx, &DumbPixDesc);
  SetPixelFormat(DumbDevCtx, DumbPixIdx, &DumbPixDesc);

  HGLRC DumbGlCtx = wglCreateContextProc(DumbDevCtx);
  wglMakeCurrentProc(DumbDevCtx, DumbGlCtx);

  PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARBProc =
    (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddressProc("wglChoosePixelFormatARB");
  PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARBProc =
    (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddressProc("wglCreateContextAttribsARB");
  PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT =
    (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddressProc("wglSwapIntervalEXT");

  wglMakeCurrentProc(null, null);
  ReleaseDC(DumbWindow, DumbDevCtx);
  wglDeleteContextProc(DumbGlCtx);
  DestroyWindow(DumbWindow);

  INT FormatAttribs[] = {
    WGL_DRAW_TO_WINDOW_ARB, true,
    WGL_ACCELERATION_ARB,   WGL_FULL_ACCELERATION_ARB,
    WGL_SWAP_METHOD_ARB,    WGL_SWAP_EXCHANGE_ARB,
    WGL_SUPPORT_OPENGL_ARB, true,
    WGL_DOUBLE_BUFFER_ARB,  true,
    WGL_PIXEL_TYPE_ARB,     WGL_TYPE_RGBA_ARB,
    WGL_COLOR_BITS_ARB,     32,
    WGL_DEPTH_BITS_ARB,     24,
    WGL_STENCIL_BITS_ARB,   8,
    WGL_SAMPLE_BUFFERS_ARB, true,
    0
  };
  INT  PixelFormatIdx  = 0;
  UINT NumberOfFormats = 0;
  wglChoosePixelFormatARBProc(DevCtx, FormatAttribs, null, 1, &PixelFormatIdx, &NumberOfFormats);
  PIXELFORMATDESCRIPTOR PixelFormatDescriptor = {0};
  SetPixelFormat(DevCtx, PixelFormatIdx, &PixelFormatDescriptor);
  INT ContextAttribs[] = {
    WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
    WGL_CONTEXT_MINOR_VERSION_ARB, 5,
    WGL_CONTEXT_FLAGS_ARB,         WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
    WGL_CONTEXT_PROFILE_MASK_ARB,  WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
    0
  };
  GlCtx = wglCreateContextAttribsARBProc(DevCtx, 0, ContextAttribs);
  wglMakeCurrentProc(DevCtx, GlCtx);

  gl_api Gl;
  #define Assign(Type, Name) \
    Gl.Name = (Type)(GetAnyGlProc(OpenGlDll, wglGetProcAddressProc, strify(Name))); \
    Assert(Gl.Name, "Could not load" strify(Name));
  SELECTED_OPENGL_FUNCTIONS(Assign)
  #undef Assign

  wglSwapIntervalEXT(1);

  ShowWindow(Window, SW_SHOW);

  pool *AppMem = ReservePool();
  void *AppStm = null;

  HMODULE AppDll = LoadLibraryA("app.dll");
  app_start_proc *AppStartProc = (app_start_proc*)GetProcAddress(AppDll, "AppStart");
  app_frame_proc *AppFrameProc = (app_frame_proc*)GetProcAddress(AppDll, "AppFrame");

  AppStartProc(AppMem, &AppStm, &Gl);

  app_in AppIn;

  b32 Running = true;
  f64 t = 0;
  while (Running) {
    u64 T1 = GetUsecs();

    MSG Message;
    while(PeekMessage(&Message, 0, 0, 0, PM_REMOVE)) {  
      switch(Message.message) {
        case WM_CLOSE:
        case WM_DESTROY:
          Running = false;
          break;
      }
    }

    AppIn.Time = t;

    RECT ClientRect;
    GetClientRect(Window, &ClientRect);
    AppIn.WndW = ClientRect.right  - ClientRect.left;
    AppIn.WndH = ClientRect.bottom - ClientRect.top;

    POINT WindowPos = {ClientRect.left, ClientRect.top};
    ClientToScreen(Window, &WindowPos);

    POINT MousePos;
    GetCursorPos(&MousePos);
    AppIn.MseX = MousePos.x - WindowPos.x;
    AppIn.MseY = MousePos.y - WindowPos.y;

    DevCtx = GetDC(Window);
    wglMakeCurrent(DevCtx, GlCtx);

    AppFrameProc(AppMem, &AppStm, &AppIn);

    SwapBuffers(DevCtx);

    u64 T2 = GetUsecs();

    t = ((f64)(T2-T1))/(1000.0*1000.0);
  }

  ReleaseDC(Window, DevCtx);
  DestroyWindow(Window);
  ExitProcess(0);
}

static DWORD MainThreadID = 0;
static LRESULT CALLBACK ActualWndProc(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam) {
  switch (Message) {
    case WM_CLOSE:
    case WM_DESTROY:
    case WM_ENTERSIZEMOVE:
    case WM_SIZE:
      PostThreadMessageW(MainThreadID, Message, WParam, LParam);
      break;
    default:
      return DefWindowProcW(Window, Message, WParam, LParam);
  }
  return 0;
}

int WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, LPSTR CmdLine, int ShowCmd) {
  WNDCLASSEXW ServiceClass = {
    .cbSize        = sizeof(ServiceClass),
    .lpfnWndProc   = DefWindowProcW,
    .hInstance     = Instance,
    .hIcon         = LoadIconA(NULL, IDI_APPLICATION),
    .hCursor       = LoadCursorA(NULL, IDC_ARROW),
    .lpszClassName = L"Hidden Dangerous ServiceClass"
  };
  RegisterClassExW(&ServiceClass);
  HWND ServiceWindow = CreateWindowExW(
    0, ServiceClass.lpszClassName, L"Hidden Dangerous Window", 0,
    CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
    0, 0, Instance, 0
  );

  WNDCLASSW ActualClass = {
    .style         = CS_OWNDC | CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS,
    .lpfnWndProc   = ActualWndProc,
    .hInstance     = Instance,
    .hCursor       = LoadCursor(0, IDC_ARROW),
    .lpszClassName = L"Actual Dangerous Class"
  };
  RegisterClassW(&ActualClass);
  HWND ActualWindow = CreateWindowW(
    L"Actual Dangerous Class", L"title", WS_TILEDWINDOW,
    CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
    0, 0, Instance, 0
  );

  CreateThread(0, 0, MainThread, ActualWindow, 0, &MainThreadID);

  while (true) {
    MSG Message;
    GetMessageW(&Message, 0, 0, 0);
    TranslateMessage(&Message);
    if ((Message.message == WM_CHAR) || (Message.message == WM_KEYDOWN) ||
        (Message.message == WM_QUIT) || (Message.message == WM_SIZE))
      PostThreadMessageW(MainThreadID, Message.message, Message.wParam, Message.lParam);
    else
      DispatchMessageW(&Message);
  }
}