#ifndef APP_H
#define APP_H

#undef FOUDNATION_IMPL
#include "foundation.h"

#undef OPENGL_IMPL
#include "opengl.h"

typedef struct _input {
  f32 WndW;
  f32 WndH;
  f32 MseX;
  f32 MseY;
  f64 Time;
} app_in;

typedef void app_start_proc(pool*, void**, gl_api*);
typedef void app_frame_proc(pool*, void**, app_in*);

#endif