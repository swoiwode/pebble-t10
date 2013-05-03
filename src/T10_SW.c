// T10_SW.c -- Torgoen T10 analog style for pebble watch.
// Starting with TH10-date.c by Jean-Noel MATTERN.
//
// v0.1 - Getting the code to compile before modifying.
// v0.2 - Edit code to suit my tastes, improve comments (IMHO) and
// remove #if options.  Pulled out the custom date stuff, I am going to
// use the center of the watch for day to week text and M/D below it.
// 05/03/2013 - Scott Woiwode - github\swoiwode

#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"


#define MY_UUID { 0x16, 0x17, 0xE9, 0x55, 0x6A, 0x9B, 0x4C, 0xC8, 0x9F, 0x03, 0xAD, 0xAB, 0x5B, 0x84, 0x4D, 0x8C }
PBL_APP_INFO( MY_UUID,
              "T10_SW", "Scott Woiwode",
              0x00, 0x02,
              RESOURCE_ID_IMAGE_MENU_ICON,
              APP_INFO_WATCH_FACE );

static Window window;
static Layer hand_layer;
static Layer bg_layer;
static PblTm now;
static GFont font_time;
static GFont font_date;
static char buffer[6];

// Dimensions of the watch face
#define W 144 // PEBBLE_SCREEN_WIDTH
#define H 168 // PEBBLE_SCREEN_HEIGHT
#define R ( ( W / 2 ) - 2 )

// Hour hand
static GPath hour_path;
static GPoint hour_points[] = {
    {  -8, -10 },
    { -10, -40 },
    {   0, -60 },
    { +10, -40 },
    {  +8, -10 },
};

// Minute hand
static GPath minute_path;
static GPoint minute_points[] = {
    { -4, -10 },
    { -6, -60 },
    {  0, -76 },
    { +6, -60 },
    { +4, -10 },
};

// Second hand
static GPath second_path;
static GPoint second_points[] = {
    { -2, -10 },
    { -4, -60 },
    {  0, -76 },
    { +4, -60 },
    { +2, -10 },
};

// Hour hand ticks around the circle (slightly shorter)
static GPath hour_tick_path;
static GPoint hour_tick_points[] = {
    { -3, 70 },
    { +3, 70 },
    { +3, 84 },
    { -3, 84 },
};

// Non-hour major ticks around the circle
static GPath major_tick_path;
static GPoint major_tick_points[] = {
    { -3, 60 },
    { +3, 60 },
    { +3, 84 },
    { -3, 84 },
};

// Non-major ticks around the circle; will be drawn as lines
static GPath minor_tick_path;
static GPoint minor_tick_points[] = {
    { 0, 76 },
    { 0, 84 },
};

static void hand_layer_update( Layer * const me, GContext * ctx )
{
	( void ) me;
    
  // Draw the hour hand outline in black and filled with white
  int hour_angle = ( ( now.tm_hour * 60 + now.tm_min ) * TRIG_MAX_ANGLE ) / ( 60 * 24 );    
    
  gpath_rotate_to(&hour_path, hour_angle);
  graphics_context_set_fill_color(ctx, GColorWhite);
  gpath_draw_filled(ctx, &hour_path);
  graphics_context_set_stroke_color(ctx, GColorBlack);
  gpath_draw_outline(ctx, &hour_path);

  // Draw the minute hand outline in black and filled with white
  int minute_angle = ( now.tm_min * TRIG_MAX_ANGLE ) / 60;
    
  gpath_rotate_to(&minute_path, minute_angle);
  graphics_context_set_fill_color(ctx, GColorWhite);
  gpath_draw_filled(ctx, &minute_path);
  graphics_context_set_stroke_color(ctx, GColorBlack);
  gpath_draw_outline(ctx, &minute_path);
    
  // Draw the second hand outline in black and filled with white
  int second_angle = ( now.tm_sec * TRIG_MAX_ANGLE ) / 60;
    
  gpath_rotate_to( &second_path, second_angle );
  graphics_context_set_fill_color( ctx, GColorWhite );
  gpath_draw_filled( ctx, &second_path );
  graphics_context_set_stroke_color( ctx, GColorBlack );
  gpath_draw_outline(ctx, &second_path);

  // Black circle over the hands.  Looks nice.
  graphics_context_set_fill_color( ctx, GColorBlack );
	graphics_fill_circle( ctx, GPoint( W / 2, H / 2 ), 30);

	// Put some date info in the center space.
  graphics_context_set_text_color( ctx, GColorWhite );
	string_format_time( buffer, sizeof( buffer ), "%a", &now );
  graphics_text_draw( ctx,
                      buffer,
                      font_date,
                      GRect( W / 2 - 30, H / 2 - 30, 60, 24 ),
                      GTextOverflowModeTrailingEllipsis,
                      GTextAlignmentCenter,
                      NULL
                     );

	string_format_time( buffer, sizeof( buffer ), "%m/%d", &now );
  graphics_text_draw( ctx,
                      buffer,
                      font_date,
                      GRect( W / 2 - 30 , H / 2 - 8, 60, 24 ),
                      GTextOverflowModeTrailingEllipsis,
                      GTextAlignmentCenter,
                      NULL
                     );
}

// Called once per second.
static void handle_tick( AppContextRef ctx, PebbleTickEvent * const event )
{
    ( void ) ctx;
    
    layer_mark_dirty( &hand_layer );    
    now = *event->tick_time;
}


// ** Draw the initial background image
static void bg_layer_update( Layer * const me, GContext * ctx )
{
    ( void ) me;
    
    graphics_context_set_stroke_color( ctx, GColorWhite );
    graphics_context_set_fill_color( ctx, GColorWhite );
    
    // Draw the outside marks
    for( int min = 0; min < 60; min++ )
    {
        const int angle = ( min * TRIG_MAX_ANGLE ) / 60;
        if( ( min % 15 ) == 0 )
        {
          gpath_rotate_to( &hour_tick_path, angle );
          gpath_draw_filled( ctx, &hour_tick_path );
        } 
        else if( ( min % 5 ) == 0 )
        {
     			gpath_rotate_to( &major_tick_path, angle );
          gpath_draw_filled( ctx, &major_tick_path );
        }
        else
        {
        	gpath_rotate_to( &minor_tick_path, angle );
          gpath_draw_outline( ctx, &minor_tick_path );
        }
    }
    
    // And the large labels
    graphics_context_set_text_color( ctx, GColorWhite );
    graphics_text_draw( ctx,
                        "24",
                        font_time,
                        GRect( W / 2 - 30, 4, 60, 50 ),
                        GTextOverflowModeTrailingEllipsis,
                        GTextAlignmentCenter,
                        NULL
                       );
    
    graphics_text_draw( ctx,
                        "06",
                        font_time,
                        GRect( W / 2, H / 2 - 26, 70, 50 ),
                        GTextOverflowModeTrailingEllipsis,
                        GTextAlignmentRight,
                        NULL
                       );
    
    graphics_text_draw( ctx,
                        "12",
                        font_time,
                        GRect( W / 2 - 30, 110, 60, 50 ),
                        GTextOverflowModeTrailingEllipsis,
                        GTextAlignmentCenter,
                        NULL
                       );
    
    graphics_text_draw( ctx,
                        "18",
                        font_time,
                        GRect( W / 2 - 70, H / 2 - 26, 60, 50 ),
                        GTextOverflowModeTrailingEllipsis,
                        GTextAlignmentLeft,
                        NULL
                       ); 
}

#define GPATH_INIT(PATH, POINTS) ({ \
GPathInfo __info = { sizeof(POINTS) / sizeof(*POINTS), POINTS }; \
gpath_init(PATH, &__info); \
})

static void handle_init( AppContextRef ctx )
{
    ( void ) ctx;
    
    GPATH_INIT( &hour_path, hour_points );
    gpath_move_to( &hour_path, GPoint( W / 2, H / 2 ) );
    
    GPATH_INIT( &minute_path, minute_points );
    gpath_move_to( &minute_path, GPoint( W / 2, H / 2 ) );
    
    GPATH_INIT( &second_path, second_points );
    gpath_move_to( &second_path, GPoint( W / 2, H / 2 ) );
    
    GPATH_INIT(&major_tick_path, major_tick_points);
    gpath_move_to(&major_tick_path, GPoint(W/2,H/2));
    
    GPATH_INIT(&hour_tick_path, hour_tick_points);
    gpath_move_to(&hour_tick_path, GPoint(W/2,H/2));
    
    GPATH_INIT(&minor_tick_path, minor_tick_points);
    gpath_move_to(&minor_tick_path, GPoint(W/2,H/2));
    
    get_time( &now );
    
    window_init(&window, "Main");
    window_stack_push(&window, true);
    window_set_background_color(&window, GColorBlack);
    
    resource_init_current_app( &RESOURCES );
    
    font_time = fonts_load_custom_font(resource_get_handle( RESOURCE_ID_FONT_GILLSANS_40 ) );
    font_date = fonts_load_custom_font(resource_get_handle( RESOURCE_ID_FONT_GILLSANS_20 ) );

    layer_init( &bg_layer, GRect( 0, 0, W, H ) );
    layer_add_child( &window.layer, &bg_layer );
    bg_layer.update_proc = bg_layer_update;
    layer_mark_dirty( &bg_layer );
    
    layer_init( &hand_layer, GRect( 0, 0, W, H ) );
    layer_add_child( &window.layer, &hand_layer );
    hand_layer.update_proc = hand_layer_update;
    layer_mark_dirty( &hand_layer );
}


static void handle_deinit( AppContextRef ctx )
{
    ( void ) ctx;
    
    fonts_unload_custom_font( font_time );
    fonts_unload_custom_font( font_date );
}

void pbl_main( void * const params )
{
    PebbleAppHandlers handlers = {
        .init_handler   = &handle_init,
        .deinit_handler = &handle_deinit,
        .tick_info      = {
            .tick_handler = &handle_tick,
            .tick_units = SECOND_UNIT,
        },
    };
    
    app_event_loop(params, &handlers);
}

