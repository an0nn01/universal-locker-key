#include <furi.h>
#include <gui/gui.h>
#include <input/input.h>
#include <notification/notification_messages.h>

typedef enum {
    MainMenu,
    RfidMenu,
    RfMenu
} AppState;

static AppState current_state = MainMenu;
static uint8_t menu_position = 0;
static bool is_running = true;
static FuriString* status_text;

static void draw_main_menu(Canvas* canvas) {
    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2, 12, "UniversalLockerKey");
    canvas_set_font(canvas, FontSecondary);
    const char* items[] = {"[RFID/NFC Attacks]", "[Keypad RF Attacks]"};
    for(int i = 0; i < 2; i++) {
        canvas_draw_str(canvas, 5, 30 + i * 15, items[i]);
        if(i == menu_position) canvas_draw_str(canvas, 2, 30 + i * 15, ">";
    }
}

static void draw_rfid_menu(Canvas* canvas) {
    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2, 12, "RFID/NFC Attacks");
    canvas_set_font(canvas, FontSecondary);
    const char* items[] = {"[Read & Clone Tag]", "[Simulate Saved]", "[Back]"};
    for(int i = 0; i < 3; i++) {
        canvas_draw_str(canvas, 5, 30 + i * 15, items[i]);
        if(i == menu_position) canvas_draw_str(canvas, 2, 30 + i * 15, ">";
    }
}

static void draw_rf_menu(Canvas* canvas) {
    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2, 12, "Keypad RF Attacks");
    canvas_set_font(canvas, FontSecondary);
    const char* items[] = {"[Capture Signal]", "[Emulate Signal]", "[Back]"};
    for(int i = 0; i < 3; i++) {
        canvas_draw_str(canvas, 5, 30 + i * 15, items[i]);
        if(i == menu_position) canvas_draw_str(canvas, 2, 30 + i * 15, ">";
    }
}

static void input_callback(InputEvent* input_event, void* ctx) {
    UNUSED(ctx);
    if(input_event->type == InputTypeShort) {
        switch(input_event->key) {
            case InputKeyBack:
                if(!furi_string_empty(status_text)) {
                    furi_string_reset(status_text);
                } else if(current_state != MainMenu) {
                    current_state = MainMenu;
                    menu_position = 0;
                } else {
                    is_running = false;
                }
                break;
            case InputKeyUp: if(menu_position > 0) menu_position--; break;
            case InputKeyDown:
                if((current_state == MainMenu && menu_position < 1) || (current_state != MainMenu && menu_position < 2)) menu_position++;
                break;
            case InputKeyOk:
                if(current_state == MainMenu) {
                    current_state = (menu_position == 0) ? RfidMenu : RfMenu;
                    menu_position = 0;
                } else if(current_state == RfidMenu) {
                    if(menu_position == 0) furi_string_set(status_text, "Read: Use Native NFC App.");
                    else if(menu_position == 1) furi_string_set(status_text, "Simulate: Feature Pending.");
                    else { current_state = MainMenu; menu_position = 0; }
                } else if(current_state == RfMenu) {
                    if(menu_position == 0) furi_string_set(status_text, "RF Capture: Feature Pending.");
                    else if(menu_position == 1) furi_string_set(status_text, "RF Emulate: Feature Pending.");
                    else { current_state = MainMenu; menu_position = 0; }
                }
                break;
        }
    }
}

static void draw_callback(Canvas* canvas, void* ctx) {
    UNUSED(ctx);
    if(!furi_string_empty(status_text)) {
        canvas_clear(canvas);
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str(canvas, 2, 12, "Status:");
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str(canvas, 2, 30, furi_string_get_cstr(status_text));
        canvas_draw_str(canvas, 2, 50, "Press Back to return.");
    } else {
        switch(current_state) {
            case MainMenu: draw_main_menu(canvas); break;
            case RfidMenu: draw_rfid_menu(canvas); break;
            case RfMenu: draw_rf_menu(canvas); break;
        }
    }
}

int32_t universal_locker_key_app(void* p) {
    UNUSED(p);
    is_running = true;
    current_state = MainMenu;
    menu_position = 0;
    status_text = furi_string_alloc();

    NotificationApp* notification = furi_record_open(RECORD_NOTIFICATION);
    notification_message(notification, &sequence_display_backlight_on);

    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, draw_callback, NULL);
    view_port_input_callback_set(view_port, input_callback, NULL);

    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    while(is_running) furi_delay_ms(50);

    furi_string_free(status_text);
    gui_remove_view_port(gui, view_port);
    view_port_free(view_port);
    furi_record_close(RECORD_GUI);
    furi_record_close(RECORD_NOTIFICATION);

    return 0;
}
