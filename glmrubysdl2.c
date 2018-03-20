#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <mruby.h>
#include <mruby/class.h>
#include <mruby/data.h>
#include <mruby/compile.h>
#include <mruby/variable.h>
#include <mruby/string.h>
#include <mruby/array.h>

SDL_Window *mwindow;
SDL_Renderer *renderer;
SDL_bool fullscrn;
SDL_bool fullscrn_change;

struct Timer
{
  Sint64 now, interval;
} timer;

struct RClass *cImage;

// class work
struct glmrb_image
{
  SDL_Surface *surface;
  SDL_Texture *texture;
  mrb_value width;
  mrb_value height;
};

static void glmrb_image_free(mrb_state *mrb, void *ptr);

static struct mrb_data_type mrb_image_type = {"ImageSurface", glmrb_image_free};

mrb_value mrb_get_backtrace(mrb_state *mrb, mrb_value self);

mrb_value glmrb_image_set_texture(mrb_state *mrb, mrb_value self)
{
  struct glmrb_image *image = DATA_PTR(self);
  if (image->surface == NULL)
  {
    char *m = "Cannot create SDL_Surface.\n";
    SDL_Log(m);
    mrb_raise(mrb, E_RUNTIME_ERROR, m);
  }
  else
  {
    // get image width and height
    image->width = mrb_fixnum_value(image->surface->w);
    image->height = mrb_fixnum_value(image->surface->h);
  }

  // create texture
  image->texture = SDL_CreateTextureFromSurface(renderer,
                                                image->surface);
  if (image->texture == NULL)
  {
    char *m = "Cannot create SDL_Texture.\n";
    SDL_Log(m);
    mrb_raise(mrb, E_RUNTIME_ERROR, m);
  }
  else
  {
    SDL_BlendMode mode;
    mode = SDL_BLENDMODE_BLEND;
    int ret = SDL_SetTextureBlendMode(image->texture, mode);
    if (ret == -1)
    {
      char *m = "Cannot set BLEND MODE.\n";
      SDL_Log(m);
    }
  }

  return self;
}

static void glmrb_get_rgba(mrb_state *mrb, mrb_value vcol,
                           Uint8 *a, Uint8 *r, Uint8 *g, Uint8 *b)
{
  *a = 255;
  *r = 255;
  *g = 255;
  *b = 255;

  if (mrb_array_p(vcol))
  {
    int len = RARRAY_LEN(vcol);
    if (len >= 4)
    {
      *a = (Uint8)mrb_fixnum(mrb_ary_ref(mrb, vcol, 0));
      *r = (Uint8)mrb_fixnum(mrb_ary_ref(mrb, vcol, 1));
      *g = (Uint8)mrb_fixnum(mrb_ary_ref(mrb, vcol, 2));
      *b = (Uint8)mrb_fixnum(mrb_ary_ref(mrb, vcol, 3));
    }
    else if (len == 3)
    {
      *r = (Uint8)mrb_fixnum(mrb_ary_ref(mrb, vcol, 0));
      *g = (Uint8)mrb_fixnum(mrb_ary_ref(mrb, vcol, 1));
      *b = (Uint8)mrb_fixnum(mrb_ary_ref(mrb, vcol, 2));
    }
    else
    {
      SDL_Log("Illegal color array.\n");
      mrb_raise(mrb, E_RUNTIME_ERROR, "Illegal color array.");
    }
  }
  else
  {
    SDL_Log("paramater is not array.\n");
    mrb_raise(mrb, E_RUNTIME_ERROR, "paramater is not array");
  }
}

static mrb_value glmrb_image_fillrect_local(mrb_state *mrb, mrb_value self, mrb_value vcol)
{
  struct glmrb_image *image = DATA_PTR(self);

  // get color
  Uint8 a, r, g, b;
  glmrb_get_rgba(mrb, vcol, &a, &r, &g, &b);

  if (image->surface != NULL)
  {
    // SDL_SetSurfaceBlendMode(image->surface, SDL_BLENDMODE_BLEND);

    // fill surface
    SDL_FillRect(image->surface, NULL,
                 SDL_MapRGBA(image->surface->format, r, g, b, a));
  }
  return glmrb_image_set_texture(mrb, self);
}

/* ImageSurface#initialize */
static mrb_value glmrb_image_initialize(mrb_state *mrb, mrb_value self)
{
  struct glmrb_image *image =
      (struct glmrb_image *)mrb_malloc(mrb, sizeof(struct glmrb_image));
  if (image == NULL)
  {
    char *m = "Out of memory. ImageSurface.\n";
    SDL_Log(m);
    mrb_raise(mrb, E_RUNTIME_ERROR, m);
  }
  DATA_TYPE(self) = &mrb_image_type;
  DATA_PTR(self) = image;

  image->width = mrb_nil_value();
  image->height = mrb_nil_value();
  image->surface = NULL;
  image->texture = NULL;

  mrb_iv_set(mrb, self, mrb_intern(mrb, "@filename", 9), mrb_nil_value());

  // get paramater, o:Object [mrb_value], A:array [mrb_value]
  mrb_value vw, vh, vcol;
  mrb_get_args(mrb, "ooA", &vw, &vh, &vcol);

  if (mrb_nil_p(vw) || mrb_nil_p(vh))
    return self;

  int w = mrb_fixnum(vw);
  int h = mrb_fixnum(vh);

  // make surface
  Uint32 rmask, gmask, bmask, amask;

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
  rmask = 0xff000000;
  gmask = 0x00ff0000;
  bmask = 0x0000ff00;
  amask = 0x000000ff;
#else
  rmask = 0x000000ff;
  gmask = 0x0000ff00;
  bmask = 0x00ff0000;
  amask = 0xff000000;
#endif

  image->surface = SDL_CreateRGBSurface(0, w, h, 32,
                                        rmask, gmask, bmask, amask);
  if (image->surface == NULL)
    return self;

  return glmrb_image_fillrect_local(mrb, self, vcol);
}

// ImageSurface#fillrect
static mrb_value glmrb_image_fillrect(mrb_state *mrb, mrb_value self)
{
  mrb_value vcol;
  mrb_get_args(mrb, "A", &vcol);
  return glmrb_image_fillrect_local(mrb, self, vcol);
}

// ImageSurface#load
static mrb_value glmrb_image_load(mrb_state *mrb, mrb_value self)
{
  mrb_value param[3];
  param[0] = mrb_nil_value();
  param[1] = mrb_nil_value();
  param[2] = mrb_ary_new(mrb);
  mrb_value obj = mrb_obj_new(mrb, cImage, 3, param);
  struct glmrb_image *image = DATA_PTR(obj);

  // get paramater, z:String [mrb_value]
  mrb_value fn;
  mrb_get_args(mrb, "S", &fn);

  if (image->texture != NULL)
    SDL_DestroyTexture(image->texture);

  if (image->surface != NULL)
    SDL_FreeSurface(image->surface);

  image->width = mrb_nil_value();
  image->height = mrb_nil_value();
  image->surface = NULL;
  image->texture = NULL;

  // copy filename
  // int len = sizeof(char *) * sizeof(fn);
  // mrb_value str = mrb_str_new(mrb, fn, len);
  mrb_iv_set(mrb, obj, mrb_intern(mrb, "@filename", 9), fn);

  // load image file

  image->surface = IMG_Load(mrb_str_to_cstr(mrb, fn));

  glmrb_image_set_texture(mrb, obj);
  return obj;
}

/* ImageSurface#filename */
static mrb_value glmrb_image_filename(mrb_state *mrb, mrb_value self)
{
  return mrb_iv_get(mrb, self, mrb_intern(mrb, "@filename", 9));
}

/* ImageSurface#width */
static mrb_value glmrb_image_width(mrb_state *mrb, mrb_value self)
{
  struct glmrb_image *image = DATA_PTR(self);
  return image->width;
}

/* ImageSurface#height */
static mrb_value glmrb_image_height(mrb_state *mrb, mrb_value self)
{
  struct glmrb_image *image = DATA_PTR(self);
  return image->height;
}

/* ImageSurface#draw_to_window */
static mrb_value glmrb_image_draw_to_window(mrb_state *mrb, mrb_value self)
{
  struct glmrb_image *image = DATA_PTR(self);
  SDL_Rect src, dst;
  mrb_value vx, vy;
  int px, py;

  if (mrb_nil_p(image->width) || mrb_nil_p(image->height))
    return self;

  // get paramater, o:Object [mrb_value]
  mrb_get_args(mrb, "oo", &vx, &vy);
  px = (mrb_float_p(vx)) ? (int)mrb_float(vx) : mrb_fixnum(vx);
  py = (mrb_float_p(vy)) ? (int)mrb_float(vy) : mrb_fixnum(vy);

  src.x = 0;
  src.y = 0;
  src.w = mrb_fixnum(image->width);
  src.h = mrb_fixnum(image->height);

  dst.x = px;
  dst.y = py;
  dst.w = src.w;
  dst.h = src.h;

  // SDL_SetTextureAlphaMod(image->texture, 128);
  SDL_RenderCopy(renderer, image->texture, &src, &dst);
  return self;
}

static void glmrb_image_free(mrb_state *mrb, void *ptr)
{
  struct glmrb_image *image = ptr;

  if (image->texture != NULL)
    SDL_DestroyTexture(image->texture);

  if (image->surface != NULL)
    SDL_FreeSurface(image->surface);

  mrb_free(mrb, image);
}

void glmrb_define_class(mrb_state *mrb)
{
  // define class
  cImage = mrb_define_class(mrb, "ImageSurface", mrb->object_class);
  MRB_SET_INSTANCE_TT(cImage, MRB_TT_DATA);

  // define class method
  mrb_define_class_method(mrb, cImage, "load",
                          glmrb_image_load, MRB_ARGS_REQ(1));

  // define instance method
  mrb_define_method(mrb, cImage, "initialize",
                    glmrb_image_initialize, MRB_ARGS_REQ(3));

  mrb_define_method(mrb, cImage, "width",
                    glmrb_image_width, MRB_ARGS_NONE());

  mrb_define_method(mrb, cImage, "height",
                    glmrb_image_height, MRB_ARGS_NONE());

  mrb_define_method(mrb, cImage, "filename",
                    glmrb_image_filename, MRB_ARGS_NONE());

  mrb_define_method(mrb, cImage, "draw_to_window",
                    glmrb_image_draw_to_window, MRB_ARGS_REQ(2));

  mrb_define_method(mrb, cImage, "fillrect",
                    glmrb_image_fillrect, MRB_ARGS_REQ(1));
}

// quit check
int quits(void)
{
  SDL_Event events;
  if (SDL_PollEvent(&events))
  {
    switch (events.type)
    {
    case SDL_QUIT:
      return 0;
      break;
    case SDL_KEYDOWN:
      if (events.key.keysym.sym == SDLK_ESCAPE ||
          events.key.keysym.sym == SDLK_q)
        return 0;
      if (events.key.keysym.sym == SDLK_F11)
        fullscrn_change = SDL_TRUE;
      break;
    default:
      break;
    }
  }
  return 1;
}

// display mruby state error
void mrb_state_error_disp(mrb_state *mrb)
{
  // error
  mrb_value exc = mrb_obj_value(mrb->exc);

  // error information
  mrb_value backtrace = mrb_get_backtrace(mrb, exc);
  mrb_value inspect_bt = mrb_inspect(mrb, backtrace);
  printf("%s\n", mrb_str_to_cstr(mrb, inspect_bt));

  // backtrace
  enum mrb_vtype type = mrb_type(exc);
  mrb_value inspect = mrb_inspect(mrb, exc);
  printf("%d : %s\n", type, mrb_str_to_cstr(mrb, inspect));
}

// SDL quit and work free
void glmrb_sdl_quit(void)
{
  if (renderer != NULL)
    SDL_DestroyRenderer(renderer);

  if (mwindow != NULL)
    SDL_DestroyWindow(mwindow);

  SDL_Quit();
}

int main(int argc, char *argv[])
{
  FILE *fp;
  mrb_value mrb_obj;

  fullscrn = SDL_FALSE;
  fullscrn_change = SDL_FALSE;

  mwindow = NULL;
  renderer = NULL;

  // mruby state open
  mrb_state *mrb = mrb_open();
  if (mrb == 0)
  {
    SDL_Log("Cannot open mruby state.\n");
    return -2;
  }

  // define class
  glmrb_define_class(mrb);

  // SDL2 init
  if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
  {
    SDL_Log("Failure SDL_INIT()\n");
    mrb_close(mrb);
    return -1;
  }

  // create window
  mwindow = SDL_CreateWindow("glmrubysdl2",
                             SDL_WINDOWPOS_UNDEFINED,
                             SDL_WINDOWPOS_UNDEFINED,
                             640, 480, 0);

  if (mwindow == NULL)
  {
    SDL_Log("Cannot create window.\n");
    glmrb_sdl_quit();
    mrb_raise(mrb, E_RUNTIME_ERROR, "Cannot create window.");
    return -1;
  }

  // create renderer
  renderer = SDL_CreateRenderer(mwindow, -1, SDL_RENDERER_ACCELERATED);
  if (renderer == NULL)
  {
    SDL_Log("Cannot create renderer.\n");
    glmrb_sdl_quit();
    mrb_raise(mrb, E_RUNTIME_ERROR, "cannot create renderer.");
    return -1;
  }

  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
  SDL_RenderSetLogicalSize(renderer, 640, 480);

  // load and run mruby script. execute after create renderer
  fp = fopen("main.rb", "r");
  if (fp == NULL)
  {
    SDL_Log("Cannot load main.rb.\n");
    mrb_raise(mrb, E_RUNTIME_ERROR, "Cannot load main.rb");
    return -2;
  }
  mrb_obj = mrb_load_file(mrb, fp);
  fclose(fp);

  if (mrb->exc != 0 && (mrb_nil_p(mrb_obj) || mrb_undef_p(mrb_obj)))
  {
    // error
    mrb_state_error_disp(mrb);
    mrb->exc = 0;

    mrb_close(mrb);
    glmrb_sdl_quit();
    return -2;
  }

  timer.interval = (Sint64)SDL_GetPerformanceFrequency() / 60;
  timer.now = 0;

  int framecounter = 0;

  // main loop
  while (quits() != 0)
  {
    // fullscreen change
    if (fullscrn_change)
    {
      fullscrn_change = SDL_FALSE;
      fullscrn = (fullscrn == SDL_TRUE) ? SDL_FALSE : SDL_TRUE;
      if (fullscrn)
        SDL_SetWindowFullscreen(mwindow, SDL_WINDOW_FULLSCREEN_DESKTOP);
      else
        SDL_SetWindowFullscreen(mwindow, 0);
    }

    // update
    mrb_funcall(mrb, mrb_obj, "update", 0);

    // clear with background color
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // draw
    mrb_funcall(mrb, mrb_obj, "draw", 0);

    SDL_RenderPresent(renderer);

    // timer wait
    while (((Sint64)SDL_GetPerformanceCounter() - timer.now) < timer.interval)
      SDL_Delay(0);
    timer.now = SDL_GetPerformanceCounter();

    framecounter++;
  }

  mrb_close(mrb);
  glmrb_sdl_quit();

  return 0;
}
