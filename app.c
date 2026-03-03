#define  FOUNDATION_IMPL
#include "foundation.h"

#include "opengl.h"

#define Decl(Type, Name) Type Name;
  SELECTED_OPENGL_FUNCTIONS(Decl)
#undef Decl

#include "app.h"

#include "math.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "download/stb_truetype.h"

typedef struct _gl_shader {
  u32 Vsh;
  u32 Fsh;
  u32 Pip;
} gl_shader;

function gl_shader GlMakeShader(const c8 *VertShaderCode, const c8 *FragShaderCode) {
  gl_shader Res;

  Res.Vsh = glCreateShaderProgramv(GL_VERTEX_SHADER, 1, &VertShaderCode);
  Res.Fsh = glCreateShaderProgramv(GL_FRAGMENT_SHADER, 1, &FragShaderCode);

  i32 LinkStatus;
  glGetProgramiv(Res.Vsh, GL_LINK_STATUS, &LinkStatus);
  if (!LinkStatus) {
    c8 ErrorMessage[1024];
    glGetProgramInfoLog(Res.Vsh, sizeof(ErrorMessage), NULL, ErrorMessage);
    OutputDebugStringA(ErrorMessage);
    Assert(!"Failed to create vertex shader!");
  }
  glGetProgramiv(Res.Fsh, GL_LINK_STATUS, &LinkStatus);
  if (!LinkStatus) {
    c8 ErrorMessage[1024];
    glGetProgramInfoLog(Res.Fsh, sizeof(ErrorMessage), NULL, ErrorMessage);
    OutputDebugStringA(ErrorMessage);
    Assert(!"Failed to create fragment shader!");
  }

  glGenProgramPipelines(1, &Res.Pip);
  glUseProgramStages(Res.Pip, GL_VERTEX_SHADER_BIT, Res.Vsh);
  glUseProgramStages(Res.Pip, GL_FRAGMENT_SHADER_BIT, Res.Fsh);

  return Res;
}

typedef struct _gl_buffer {
  u32   Vbo;
  u32   Vao;
  u32   Num;
  void *Vts;
} gl_buffer;

function gl_buffer GlMakeBuffer(void *Vts, u32 VertexSize, u32 Num) {
  gl_buffer Res;
  glCreateBuffers(1, &Res.Vbo);
  glNamedBufferStorage(Res.Vbo, VertexSize*Num, Vts, 0);
  glCreateVertexArrays(1, &Res.Vao);

  Res.Num = Num;
  Res.Vts = Vts;

  return Res;
}

function gl_buffer LoadFontGlyphCurvesIntoGlBuff(c8 *FontPath, u32 Codepoint) {
  typedef struct _curve_vert {
    f32v2 Pos;
    f32   Dir;
  } curve_vert;

  str FontFile = FileOpen(FontPath);
  stbtt_fontinfo FontInfo;
  stbtt_InitFont(&FontInfo, FontFile.Str, 0);
  stbtt_vertex *GlyphVertices = MemRes(1024*sizeof(stbtt_vertex));
  u32 NumberOfVertices = stbtt_GetCodepointShape(&FontInfo, Codepoint, &GlyphVertices);

  curve_vert *CurveVerts = null;

  f32 x = 0, y = 0;
  f32 d1x, d1y;
  f32 d2x, d2y;
  f32 det;
  f32 a = 1000;
  f32 b = -0.8f;
  curve_vert P1;
  curve_vert P2;
  curve_vert PC;
  fornum (i, NumberOfVertices) {
    stbtt_vertex *g = &GlyphVertices[i];
    switch (g->type) {
      case STBTT_vmove:
      case STBTT_vline:
      case STBTT_vcubic:
        x = g->x;
        y = g->y;
        break;
      case STBTT_vcurve:
        P1.Pos.x = x/a + b;
        P1.Pos.y = y/a + b;

        P2.Pos.x = g->x/a + b;
        P2.Pos.y = g->y/a + b;

        PC.Pos.x = g->cx/a + b;
        PC.Pos.y = g->cy/a + b;

        d1x = PC.Pos.x - P1.Pos.x;
        d1y = PC.Pos.y - P1.Pos.y;

        d2x = PC.Pos.x - P2.Pos.x;
        d2y = PC.Pos.y - P2.Pos.y;

        det = d1x*d2y - d2x*d1y;

        P1.Dir = det;
        P2.Dir = det;
        PC.Dir = det;
        
        ArrAdd(CurveVerts, P1);
        ArrAdd(CurveVerts, PC);
        ArrAdd(CurveVerts, P2);

        x = g->x;
        y = g->y;
        break;
    }
  }

  gl_buffer Res = GlMakeBuffer(CurveVerts, sizeof(curve_vert), ArrLen(CurveVerts));
  GLint BuffIdx = 0;
  glVertexArrayVertexBuffer(Res.Vao, BuffIdx, Res.Vbo, 0, sizeof(curve_vert));

  GLint PosArg = 0;
  glVertexArrayAttribFormat(Res.Vao, PosArg, 2, GL_FLOAT, GL_FALSE, offsetof(curve_vert, Pos));
  glVertexArrayAttribBinding(Res.Vao, PosArg, BuffIdx);
  glEnableVertexArrayAttrib(Res.Vao, PosArg);

  GLint DirArg = 1;
  glVertexArrayAttribFormat(Res.Vao, DirArg, 1, GL_FLOAT, GL_FALSE, offsetof(curve_vert, Dir));
  glVertexArrayAttribBinding(Res.Vao, DirArg, BuffIdx);
  glEnableVertexArrayAttrib(Res.Vao, DirArg);

  return Res;
}

function gl_buffer LoadFontGlyphInsideIntoGlBuff(c8 *FontPath, u32 Codepoint) {
  str FontFile = FileOpen(FontPath);
  stbtt_fontinfo FontInfo;
  stbtt_InitFont(&FontInfo, FontFile.Str, 0);
  stbtt_vertex *GlyphVertices = MemRes(1024*sizeof(stbtt_vertex));
  u32 NumberOfVertices = stbtt_GetCodepointShape(&FontInfo, Codepoint, &GlyphVertices);

  f32v2 *InsideVerts = null;

  f32 x = 0, y = 0;
  f32 a = 1000;
  f32 b = -0.8f;
  f32 d1x, d1y;
  f32 d2x, d2y;
  f32 det;
  f32v2 P;
  f32v2 P1, P2, PC;
  fornum (i, NumberOfVertices) {
    stbtt_vertex *g = &GlyphVertices[i];
    switch (g->type) {
      case STBTT_vmove:
      case STBTT_vline:
      case STBTT_vcubic:
        P.x = g->x/a + b;
        P.y = g->y/a + b;
        x = g->x;
        y = g->y;
        ArrAdd(InsideVerts, P);
        break;
      case STBTT_vcurve:
        P1.x = x/a + b;
        P1.y = y/a + b;
        P2.x = g->x/a + b;
        P2.y = g->y/a + b;
        PC.x = g->cx/a + b;
        PC.y = g->cy/a + b;

        P.x = g->x/a + b;
        P.y = g->y/a + b;

        d1x = PC.x - P1.x;
        d1y = PC.y - P1.y;

        d2x = PC.x - P2.x;
        d2y = PC.y - P2.y;

        det = d1x*d2y - d2x*d1y;

        if (det > 0) {
          f32v2 C;
          C.x = g->cx/a + b;
          C.y = g->cy/a + b;
          ArrAdd(InsideVerts, C);
        }

        ArrAdd(InsideVerts, P);

        x = g->x;
        y = g->y;
        break;
    }
  }

  gl_buffer Res = GlMakeBuffer(InsideVerts, sizeof(f32v2), ArrLen(InsideVerts));
  GLint BuffIdx = 0;
  glVertexArrayVertexBuffer(Res.Vao, BuffIdx, Res.Vbo, 0, sizeof(f32v2));

  GLint PosArg = 0;
  glVertexArrayAttribFormat(Res.Vao, PosArg, 2, GL_FLOAT, GL_FALSE, 0);
  glVertexArrayAttribBinding(Res.Vao, PosArg, BuffIdx);
  glEnableVertexArrayAttrib(Res.Vao, PosArg);

  return Res;
}

struct app_state {
  u32 CurvesVao;
  gl_shader CurveShader;
  gl_shader InsideShader;
  gl_buffer CurveBuffer;
  gl_buffer InsideBuffer;
  u32 NumCurveVerts;
};

dll_export void AppStart(pool *Mem, void **Stm, gl_api *Gl) {
  #define Assign(Type, Name) Name = Gl->Name;
    SELECTED_OPENGL_FUNCTIONS(Assign)
  #undef Assign

  *Stm = PoolPut(Mem, sizeof(struct app_state));

  struct app_state *Sta = (struct app_state*)*Stm;

  Sta->CurveBuffer  = LoadFontGlyphCurvesIntoGlBuff("download/roboto_mono.ttf", 's');
  Sta->InsideBuffer = LoadFontGlyphInsideIntoGlBuff("download/roboto_mono.ttf", 's');
  Sta->CurveShader = GlMakeShader(
    "#version 450 core                             \n"
    "layout (location=0) in vec2 a_pos;            \n"
    "layout (location=1) in float a_direction;     \n"
    "layout (location=0)                           \n"
    "uniform float AspectRatio;                    \n"
    "out gl_PerVertex { vec4 gl_Position; };       \n"
    "out vec2 uv;                                  \n"
    "out float direction;                          \n"
    "void main() {                                 \n"
    "    vec2 pos = vec2(AspectRatio, 1) * a_pos;  \n"
    "    float v = float(((gl_VertexID+2)%3) == 0);\n"
    "    float w = float(((gl_VertexID+1)%3) == 0);\n"
    "    uv = v*vec2(0.5,0.0) + w*vec2(1.0);       \n"
    "    gl_Position = vec4(pos, 0, 1);            \n"
    "    direction = a_direction;                  \n"
    "}                                             \n",

    "#version 450 core                             \n"
    "in vec2 uv;                                   \n"
    "in float direction;                           \n"
    "layout (location=0)                           \n"
    "out vec4 o_color;                             \n"
    "void main() {                                 \n"
    "    bool Cond = uv.x*uv.x - uv.y < 0;         \n"
    "    if (direction > 0) Cond = !Cond;          \n"
    "    o_color = vec4(1)*float(Cond);            \n"
    "}                                             \n"
  );
  Sta->InsideShader = GlMakeShader(
    "#version 450 core                             \n"
    "layout (location=0) in vec2 a_pos;            \n"
    "layout (location=0)                           \n"
    "uniform float AspectRatio;                    \n"
    "out gl_PerVertex { vec4 gl_Position; };       \n"
    "void main() {                                 \n"
    "    vec2 pos = vec2(AspectRatio, 1) * a_pos;  \n"
    "    gl_Position = vec4(pos, 0, 1);            \n"
    "}                                             \n",

    "#version 450 core                             \n"
    "in vec4 color;                                \n"
    "layout (location=0)                           \n"
    "out vec4 o_color;                             \n"
    "void main() {                                 \n"
    "    o_color = vec4(1.0);                      \n"
    "}                                             \n"
  );

  glEnable(GL_BLEND);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);
}

dll_export void AppFrame(pool *Mem, void **Stm, app_in *In) {
  struct app_state *Sta = (struct app_state*)*Stm;

  glViewport(0, 0, In->WndW, In->WndH);
  glClearColor(0.0f, 0.0f, 0.0f, 1.f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

  i32 AspectRatioUniform = 0;
  glProgramUniform1f(Sta->CurveShader.Vsh, AspectRatioUniform, In->WndH / In->WndW);
  glProgramUniform1f(Sta->InsideShader.Vsh, AspectRatioUniform, In->WndH / In->WndW);

  glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ZERO);
  glBindProgramPipeline(Sta->InsideShader.Pip);
  glBindVertexArray(Sta->InsideBuffer.Vao);
  glDrawArrays(GL_TRIANGLE_FAN, 0, Sta->InsideBuffer.Num);

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glBindProgramPipeline(Sta->CurveShader.Pip);
  glBindVertexArray(Sta->CurveBuffer.Vao);
  glDrawArrays(GL_TRIANGLES, 0, Sta->CurveBuffer.Num);
}