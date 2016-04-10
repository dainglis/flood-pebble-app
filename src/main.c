#include <pebble.h>
#define SIZE 14
#define MAXCOLS 6
#define COLORSCHEME 2
//rgb = 1, inchworm = 2, purpureus = 3

//TODO
//basically, every iteration its going to create a square outwards where all the cells
//in the square are the same color, so the recursion can be directly pushed outwards to
//the edges of the square


/* to reduce recursion:
 * creates an array of all cells in the current "path"
 * changes the color of each cell in the "path"
 * run recursion on each cell and add any new cells to the path (order does not matter)
 */

static const int BOARD_SIZE = SIZE;
static const int MOVES = 25;
static const int PLAY = 0;
static const int WIN = 1;
static const int LOSE = 2;
static const int MENU = 10;

static Window *window;
static TextLayer *text_layer;
static Layer *graphics_layer;
static int active_color;
static int board[SIZE][SIZE];
static int colors[MAXCOLS];
static int moves;
static int game_state;
static bool recur;
static char* move_string;
//static Game *game;

//debug, used for recursion app_log output
static int ree;

/*
 * generates move text on top left of play screen
 */
static void generate_string(int n) {
  char* newstring = malloc(sizeof(char) * 5);
  if (n >= 10) {
    newstring[0] = (n / 10) + '0';
    newstring[1] = (n % 10) + '0';
  } else {
    newstring[0] = ' ';
    newstring[1] = n + '0';
  }
  newstring[2] = '/';
  newstring[3] = (MOVES / 10) + '0';
  newstring[4] = (MOVES % 10) + '0';
  strcpy(move_string, newstring);
  free(newstring);
}

/*
 * checks to see if the whole board is all 1 color
 * if so, the player wins
 */
bool game_won() {
  for (int i = 0; i < 6; i++) {
    if (colors[i] == BOARD_SIZE * BOARD_SIZE) {
      return true;
    }
  }
  return false;
}

/*
 * checks to see if the player has reached the maximum number of moves
 * if so, the player loses
 */
bool game_over() {
  if (moves == MOVES) {
    return true;
  }
  return false;
}

/*
 * run after each move, checks win and loss
 * if neither, the game keeps going
 */
static void set_state() {
  if (game_won()) {
    game_state = WIN;
  } else if (game_over()) {
    game_state = LOSE;
  } else {
    game_state = PLAY;
  }
}

/*
 * initializes the game board
 */
static void init_board() {
  recur = false;
  moves = 0;
  game_state = PLAY;
  for (int i = 0; i < MAXCOLS; i++) {
    colors[i] = 0;
  }
  for (int r = 0; r < BOARD_SIZE; r++) {
    for (int c = 0; c < BOARD_SIZE; c++) {
      int col = rand() % MAXCOLS;
      board[r][c] = col;
      colors[col]++;
    }
  }
  set_state();
}

/*
 * used for the board recursion
 * starting from (0, 0), if a cell connected to the current cell matches the original color of the (0, 0) cell,
 * it is changed to the selected color for that turn
 */
static void set_cell_color(int r, int c, int color, int prev_c) {
  int prev = prev_c;
  board[r][c] = color;
  colors[prev]--;
  colors[color]++;
  
  ree++; //DEBUG
  
  if (r < BOARD_SIZE - 1 && board[r+1][c] == prev) {
    set_cell_color(r + 1, c, color, prev);
  }
  if (r > 0 && board[r-1][c] == prev) {
    set_cell_color(r - 1, c, color, prev);
  }
  if (c < BOARD_SIZE - 1 && board[r][c+1] == prev) {
    set_cell_color(r, c + 1, color, prev);
  }
  if (c > 0 && board[r][c-1] == prev) {
    set_cell_color(r, c - 1, color, prev);
  }
}

static void graphics_layer_update_proc(Layer *layer, GContext *ctx) {
  #if COLORSCHEME == 1
    GColor game_colors[6] = {GColorRed, GColorChromeYellow, GColorYellow, GColorGreen, GColorPictonBlue, GColorPurple};
  #elif COLORSCHEME == 2
    GColor game_colors[6] = {GColorInchworm, GColorCadetBlue, GColorDarkGray, GColorSunsetOrange, GColorRajah, GColorIcterine};
  #elif COLORSCHEME == 3
    GColor game_colors[6] = {GColorSunsetOrange, GColorChromeYellow, GColorMayGreen, GColorVividCerulean, GColorPurpureus, GColorDarkGray};
  #endif
  int offset_x = 2; //old 12 left and right border size
  int offset_y = 26; //old 36 top border size such that bottom border is 2px
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, GRect(0, 0, 144, 168), 0, GCornerNone);
  
  /* Draws game board */
  for (int r = 0; r < BOARD_SIZE; r++) {
    for (int c = 0; c < BOARD_SIZE; c++) {
      graphics_context_set_fill_color(ctx, game_colors[board[r][c]]);
      graphics_fill_rect(ctx, GRect(offset_x + (c * 10), offset_y + (r * 10), 10, 10), 0, GCornerNone);
    }
  }
  
  /* Draws color selector */
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, GRect(12 + (active_color * 10), 8, 10, 10), 0, GCornerNone);
  for (int i = 0; i < MAXCOLS; i++) {
    graphics_context_set_fill_color(ctx, game_colors[i]);
    graphics_fill_rect(ctx, GRect(13 + (i * 10), 9, 8, 8), 0, GCornerNone);
  }
  
  if (game_state == WIN) {
    text_layer_set_text(text_layer, "Winner");
    active_color = 0;
  } else if (game_state == LOSE) {
    text_layer_set_text(text_layer, "Game over");
    active_color = 0;
  } else {
    if (moves == 0) {
      text_layer_set_text(text_layer, "Flood");
    } else {
      generate_string(moves);
      text_layer_set_text(text_layer, move_string);
    }
  }
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (game_state == PLAY) {
    if (!recur) {
      ree = 0; //DEBUG
      recur = true;
      int prev;
      prev = board[0][0];
      set_cell_color(0, 0, active_color, prev);
      APP_LOG(APP_LOG_LEVEL_DEBUG, "Recursions: %d", ree); //DEBUG
      moves++;
      set_state();
    }
  } else if (game_state == MENU) {
    //select will not affect the game when in menu
  } else {
    init_board();
  }
  layer_mark_dirty(graphics_layer);
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (game_state == MENU) {
    //
  } else {
    recur = false;
    if (active_color > 0) {
      active_color--;
    } else {
      active_color = MAXCOLS - 1;
    }
    layer_mark_dirty(graphics_layer);
  }
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (game_state == MENU) {
    //
  } else {
    recur = false;
    if (active_color < MAXCOLS - 1) {
      active_color++;
    } else {
      active_color = 0;
    }
    layer_mark_dirty(graphics_layer);
  }
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  //GRect bounds = layer_get_bounds(window_layer);
  
  move_string = malloc(sizeof(char) * 6);
  init_board();

  graphics_layer = layer_create(GRect(0, 0, 144, 168));
  layer_set_update_proc(graphics_layer, graphics_layer_update_proc);
  
  text_layer = text_layer_create(GRect(80, 3, 64, 23));
  text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
  text_layer_set_font(text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text(text_layer, "Flood");
  
  layer_add_child(window_layer, graphics_layer);
  layer_add_child(window_layer, text_layer_get_layer(text_layer));
}

static void window_unload(Window *window) {
  text_layer_destroy(text_layer);
  layer_destroy(graphics_layer);
  free(move_string);
}

static void init(void) {
  window = window_create();
  window_set_click_config_provider(window, click_config_provider);
  window_set_window_handlers(window, (WindowHandlers) {
	.load = window_load,
    .unload = window_unload,
  });
  const bool animated = true;
  window_stack_push(window, animated);
}

static void deinit(void) {
  window_destroy(window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}