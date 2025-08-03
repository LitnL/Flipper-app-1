#include <furi.h>
#include <gui/gui.h>
#include <input/input.h>
#include <storage/storage.h>
#include <notification/notification_messages.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <furi.h>

int32_t flipper_app_1(void) {
    // Display something or do something here
    return 0;
}
#define LOG_PATH "/ext/nrfscan/session1.nrf"
#define MAX_CHANNELS 126

static bool nrf24_is_connected() {
    return true; // TODO: Real SPI check
}

static bool nrf24_detect_signal(uint8_t ch) {
    return (ch % 8 == 0); // Simulated signal detection
}

typedef struct {
    uint8_t channel;
    bool detected;
} ChannelData;

static void render(Canvas* canvas, ChannelData* data, size_t count, bool connected) {
    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    if(!connected) {
        canvas_draw_str(canvas, 10, 20, "⚠ Connect NRF24 Module!");
        return;
    }
    canvas_draw_str(canvas, 10, 10, "NRF24 Channel Scanner:");
    for(size_t i = 0; i < count && i < 100; i++) {
        if(data[i].detected) {
            canvas_draw_box(canvas, 10 + (i % 20) * 6, 20 + (i / 20) * 10, 5, 5);
        }
    }
}

static void save_to_file(ChannelData* data, size_t count) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(storage);
    if(storage_file_open(file, LOG_PATH, FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
        for(size_t i = 0; i < count; i++) {
            if(data[i].detected) {
                char buf[32];
                snprintf(buf, sizeof(buf), "Ch %d\n", data[i].channel);
                storage_file_write(file, buf, strlen(buf));
            }
        }
        storage_file_close(file);
    }
    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);
}

int32_t nrf24_scanner_app(void* p) {
    Gui* gui = furi_record_open(RECORD_GUI);
    ViewPort* view_port = view_port_alloc();
    bool connected = nrf24_is_connected();

    ChannelData data[MAX_CHANNELS];
    for(uint8_t i = 0; i < MAX_CHANNELS; i++) {
        data[i].channel = i;
        data[i].detected = connected ? nrf24_detect_signal(i) : false;
    }

    view_port_draw_callback_set(view_port, (ViewPortDrawCallback)render, data);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);
    view_port_update(view_port);
    furi
