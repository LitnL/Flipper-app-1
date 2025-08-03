// nrf24_scanner_app.c - Main entry for NRF24 Scanner App

#include <furi.h> #include <furi_hal.h> #include <gui/gui.h> #include <gui/view_port.h> #include <input/input.h> #include <storage/storage.h> #include <notification/notification_messages.h>

#define MAX_CHANNEL 126 #define LOG_PATH "/ext/nrfscan/session1.nrf"

typedef struct { bool activity[MAX_CHANNEL]; bool module_connected; bool should_save; } AppState;

// ---------- NRF24 DRIVER SIMULATION ---------- #define NRF_REG_STATUS 0x07

bool nrf24_is_connected() { // Simulate test register read return true; // TODO: Replace with real SPI test (check != 0xFF) }

bool nrf24_detect_signal(uint8_t ch) { // TODO: Replace with RPD read after setting channel return (ch % 8 == 0); // Simulated signal detection }

// ---------- SAVE UTILITY ---------- void log_event_to_file(uint8_t channel, uint32_t timestamp) { File* f = storage_file_open(LOG_PATH, FSAM_WRITE, FSOM_APPEND); if(f) { char line[32]; snprintf(line, sizeof(line), "%d,%lu\n", channel, timestamp); storage_file_write(f, line, strlen(line)); storage_file_close(f); } }

// ---------- DRAW FUNCTION ---------- void render_callback(Canvas* canvas, void* ctx) { AppState* state = ctx; canvas_clear(canvas);

if(!state->module_connected) {
    canvas_draw_str(canvas, 20, 30, "⚠ Connect NRF24 Module!");
    return;
}

canvas_draw_str(canvas, 5, 10, "NRF24 Signal View");
for(uint8_t ch = 0; ch < MAX_CHANNEL; ch++) {
    if(state->activity[ch]) {
        canvas_draw_box(canvas, 5 + (ch % 32) * 3, 20 + (ch / 32) * 10, 2, 8);
    }
}
canvas_draw_str(canvas, 5, 62, "OK=Save | Right=Replay");

}

// ---------- INPUT ---------- void input_callback(InputEvent* event, void* ctx) { AppState* state = ctx; if(!state->module_connected) { furi_hal_buzzer_beep(); return; }

if(event->type == InputTypeShort) {
    if(event->key == InputKeyOk) {
        state->should_save = true;
    }
}

}

// ---------- MAIN ---------- int32_t nrf24scanner_app(void) { AppState state = {.module_connected = false, .should_save = false};

// GUI
ViewPort* vp = view_port_alloc();
view_port_draw_callback_set(vp, render_callback, &state);
view_port_input_callback_set(vp, input_callback, &state);
Gui* gui = furi_record_open(RECORD_GUI);
gui_add_view_port(gui, vp, GuiLayerFullscreen);

// Check NRF
state.module_connected = nrf24_is_connected();

while(1) {
    if(state.module_connected) {
        for(uint8_t ch = 0; ch < MAX_CHANNEL; ch++) {
            state.activity[ch] = nrf24_detect_signal(ch);
            if(state.should_save && state.activity[ch]) {
                log_event_to_file(ch, furi_get_tick());
            }
        }
        state.should_save = false;
    }
    view_port_update(vp);
    furi_delay_ms(250);
}

// Cleanup (not reached)
view_port_enabled_set(vp, false);
gui_remove_view_port(gui, vp);
view_port_free(vp);
furi_record_close(RECORD_GUI);
return 0;

}

