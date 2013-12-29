#include <pebble.h>

// 4f06c0d2-4758-4d5e-9da6-829c599b2219
//#define MY_UUID { 0x4F, 0x06, 0xC0, 0xD2, 0x47, 0x58, 0x4D, 0x5E, 0x9D, 0xA6, 0x82, 0x9C, 0x59, 0x9B, 0x22, 0x19 }
//PBL_APP_INFO(MY_UUID, "Phone Ringer", "Faruq Rasid", 1 /* App version */, 0, RESOURCE_ID_IMAGE_APP_ICON, APP_INFO_STANDARD_APP);

static Window * window;

static ActionBarLayer * actionBarLayer;
static TextLayer * textLayer;
static BitmapLayer * iconLayer;

static AppSync sync;
static uint8_t sync_buffer[32];

static bool is_waiting = false;
static int current_action = 0;
static int current_profile = 2;

static GBitmap * button_image_up;
static GBitmap * button_image_down;
static GBitmap * button_image_select;
static GBitmap * image_icon_vibrate;
static GBitmap * image_icon_volume;
static GBitmap * image_icon_volume_off;

enum {
  CURRENT_ACTION = 0x0,         // TUPLE_INT
  CURRENT_PROFILE = 0x1,		// TUPLE_INT
};

void cycle_backward_profile_mode() {
  if (is_waiting) return;
	
  is_waiting = true;
  text_layer_set_text(textLayer, "Please wait...");

  int profile_to_set = current_profile - 1;
  if (profile_to_set < 0) profile_to_set = 2;
	
  Tuplet update_values[] = {
    TupletInteger(CURRENT_ACTION, 2),
	TupletInteger(CURRENT_PROFILE, profile_to_set),
  };
  app_sync_set(&sync, update_values, ARRAY_LENGTH(update_values));
}

void cycle_forward_profile_mode() {
  if (is_waiting) return;
	
  is_waiting = true;
  text_layer_set_text(textLayer, "Please wait...");

  int profile_to_set = current_profile + 1;
  if (profile_to_set > 2) profile_to_set = 0;
	
  Tuplet update_values[] = {
    TupletInteger(CURRENT_ACTION, 2),
	TupletInteger(CURRENT_PROFILE, profile_to_set),
  };
  app_sync_set(&sync, update_values, ARRAY_LENGTH(update_values));
}

void set_profile_mode(int profile_to_set) {
  if (is_waiting) return;
  if (profile_to_set < 0 || profile_to_set > 2) return;
	
  is_waiting = true;
  text_layer_set_text(textLayer, "Please wait...");
	
  Tuplet update_values[] = {
    TupletInteger(CURRENT_ACTION, 2),
	TupletInteger(CURRENT_PROFILE, profile_to_set),
  };
  app_sync_set(&sync, update_values, ARRAY_LENGTH(update_values));
}

void get_profile_mode() {
  is_waiting = true;
	
  Tuplet update_values[] = {
    TupletInteger(CURRENT_ACTION, 1),
	TupletInteger(CURRENT_PROFILE, current_profile),
  };
  app_sync_set(&sync, update_values, ARRAY_LENGTH(update_values));
}

// Modify these common button handlers

void up_single_click_handler(ClickRecognizerRef recognizer, void *context) {
  //cycle_backward_profile_mode();
  set_profile_mode(2);
}


void down_single_click_handler(ClickRecognizerRef recognizer, void *context) {
  //cycle_forward_profile_mode();
  set_profile_mode(0);
}


void select_single_click_handler(ClickRecognizerRef recognizer, void *context) {
  set_profile_mode(1);
}


void select_long_click_handler(ClickRecognizerRef recognizer, void *context) {

}

// This usually won't need to be modified

void click_config_provider(void *context) {
  //config[BUTTON_ID_SELECT]->click.handler = (ClickHandler) select_single_click_handler;
  window_single_click_subscribe(BUTTON_ID_SELECT, select_single_click_handler);

  //config[BUTTON_ID_SELECT]->long_click.handler = (ClickHandler) select_long_click_handler;
  window_long_click_subscribe(BUTTON_ID_SELECT, 1000, select_long_click_handler, NULL /* No handler on button release */);

  //config[BUTTON_ID_UP]->click.handler = (ClickHandler) up_single_click_handler;
  //config[BUTTON_ID_UP]->click.repeat_interval_ms = 100;
  window_single_repeating_click_subscribe(BUTTON_ID_UP, 100, up_single_click_handler);

  //config[BUTTON_ID_DOWN]->click.handler = (ClickHandler) down_single_click_handler;
  //config[BUTTON_ID_DOWN]->click.repeat_interval_ms = 100;
  window_single_repeating_click_subscribe(BUTTON_ID_DOWN, 100, down_single_click_handler);
}

// TODO: Error handling
static void sync_error_callback(DictionaryResult dict_error, AppMessageResult app_message_error, void *context) {
  (void) dict_error;
  (void) app_message_error;
  (void) context;
	
  is_waiting = false;
  text_layer_set_text(textLayer, "Timeout!");
}

static void sync_tuple_changed_callback(const uint32_t key, const Tuple* new_tuple, const Tuple* old_tuple, void* context) {
  (void) old_tuple;

  is_waiting = false;
	
  switch (key) {
  	case CURRENT_ACTION:
	    current_action = new_tuple->value->int32;
    	break;
	case CURRENT_PROFILE:
	  	current_profile = new_tuple->value->int32;
	  
	  	switch (current_profile) {
			case -1:
				text_layer_set_text(textLayer, "Please wait...");
				break;
			case 0:
				text_layer_set_text(textLayer, "Silent Mode");
				bitmap_layer_set_bitmap(iconLayer, image_icon_volume_off);
				break;
			case 1:
				text_layer_set_text(textLayer, "Vibrate Mode");
				bitmap_layer_set_bitmap(iconLayer, image_icon_vibrate);
				break;
			case 2:
				text_layer_set_text(textLayer, "Normal Mode");
				bitmap_layer_set_bitmap(iconLayer, image_icon_volume);
				break;
	    }
	  	
	  	break;
  	default:
    	return;
  }
}

// Standard app initialisation

void handle_init() {
  button_image_up = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_ICON_VOLUME_SMALL);
  button_image_select = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_ICON_VIBRATE_SMALL);
  button_image_down = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_ICON_VOLUME_OFF_SMALL);
  image_icon_volume = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_ICON_VOLUME);
  image_icon_vibrate = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_ICON_VIBRATE);
  image_icon_volume_off = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_ICON_VOLUME_OFF);
	
  //window_init(&window, "Phone Ringer");
  window = window_create();
  Layer * window_layer = window_get_root_layer(window);
	
  actionBarLayer = action_bar_layer_create();
  action_bar_layer_add_to_window(actionBarLayer, window);
  action_bar_layer_set_click_config_provider(actionBarLayer, (ClickConfigProvider) click_config_provider);
	
  action_bar_layer_set_icon(actionBarLayer, BUTTON_ID_UP, button_image_up);
  action_bar_layer_set_icon(actionBarLayer, BUTTON_ID_SELECT, button_image_select);
  action_bar_layer_set_icon(actionBarLayer, BUTTON_ID_DOWN, button_image_down);
	
  iconLayer = bitmap_layer_create(GRect(0, 0, 144 - 20, 120));
  bitmap_layer_set_bitmap(iconLayer, image_icon_volume);
  bitmap_layer_set_alignment(iconLayer, GAlignCenter);
  layer_add_child(window_layer, bitmap_layer_get_layer(iconLayer));

  textLayer = text_layer_create(GRect(0, 120, 144 - 20, 48));
  text_layer_set_text_alignment(textLayer, GTextAlignmentCenter);
  text_layer_set_font(textLayer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  layer_add_child(window_layer, text_layer_get_layer(textLayer));
	
  window_stack_push(window, true /* Animated */);
	
  Tuplet initial_values[] = {
    TupletInteger(CURRENT_ACTION, current_action),
	TupletInteger(CURRENT_PROFILE, -1),
  };
	
  app_message_open(64, 64);
  app_sync_init(&sync, sync_buffer, sizeof(sync_buffer), initial_values, ARRAY_LENGTH(initial_values),
                sync_tuple_changed_callback, sync_error_callback, NULL);
	
  get_profile_mode();
}

void handle_deinit() {
  gbitmap_destroy(button_image_up);
  gbitmap_destroy(button_image_down);
  gbitmap_destroy(button_image_select);
  gbitmap_destroy(image_icon_volume);
  gbitmap_destroy(image_icon_vibrate);
  gbitmap_destroy(image_icon_volume_off);
	
  app_sync_deinit(&sync);
  action_bar_layer_destroy(actionBarLayer);
  bitmap_layer_destroy(iconLayer);
  text_layer_destroy(textLayer);
  window_destroy(window);
}

int main(void) {
  /*PebbleAppHandlers handlers = {
    .init_handler = &handle_init,
    .deinit_handler = &handle_deinit,
    .messaging_info = {
      .buffer_sizes = {
        .inbound = 64,
        .outbound = 64,
      }
    }
  };
  app_event_loop(params, &handlers);*/
	
  handle_init();
  app_event_loop();
  handle_deinit();
}
