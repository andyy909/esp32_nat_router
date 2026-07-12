/* ESP32-2432S028R (CYD) 320x240 touch dashboard. */

#include "cyd_display.h"

#if CONFIG_CYD_DISPLAY

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_check.h"
#include "esp_heap_caps.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_touch.h"
#include "esp_lcd_touch_xpt2046.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lvgl.h"

#include "router_config.h"
#include "wifi_config.h"

#define LCD_HOST            SPI2_HOST
#define TOUCH_HOST          SPI3_HOST
#define LCD_HRES            320
#define LCD_VRES            240
#define LCD_DRAW_LINES      20

#define PIN_LCD_MOSI        13
#define PIN_LCD_MISO        12
#define PIN_LCD_CLK         14
#define PIN_LCD_CS          15
#define PIN_LCD_DC          2
#define PIN_LCD_BACKLIGHT   21

#define PIN_TOUCH_MOSI      32
#define PIN_TOUCH_MISO      39
#define PIN_TOUCH_CLK       25
#define PIN_TOUCH_CS        33
#define PIN_TOUCH_IRQ       36

#define TOUCH_X_MIN         200
#define TOUCH_X_MAX         3900
#define TOUCH_Y_MIN         240
#define TOUCH_Y_MAX         3900
#define WIFI_SCAN_MAX       6
#define WIFI_SCAN_FETCH_MAX 20

typedef struct {
    char ssid[33];
    int8_t rssi;
    wifi_auth_mode_t authmode;
} cyd_wifi_entry_t;

extern bool ap_connect;
extern uint16_t connect_count;
extern char *ssid;
extern char *ap_ssid;

static const char *TAG = "cyd";
static esp_lcd_panel_handle_t s_panel;
static esp_lcd_touch_handle_t s_touch;
static lv_disp_drv_t s_disp_drv;
static lv_disp_draw_buf_t s_draw_buf;
static lv_obj_t *s_status_page;
static lv_obj_t *s_access_page;
static lv_obj_t *s_wifi_page;
static lv_obj_t *s_uplink_value;
static lv_obj_t *s_uplink_detail;
static lv_obj_t *s_client_value;
static lv_obj_t *s_rate_value;
static lv_obj_t *s_ap_value;
static lv_obj_t *s_status_dot;
static lv_obj_t *s_switch_internet;
static lv_obj_t *s_switch_clients;
static lv_obj_t *s_switch_private;
static lv_obj_t *s_wifi_scan_panel;
static lv_obj_t *s_wifi_password_panel;
static lv_obj_t *s_wifi_scan_status;
static lv_obj_t *s_wifi_result_list;
static lv_obj_t *s_wifi_selected_label;
static lv_obj_t *s_wifi_password;
static lv_obj_t *s_wifi_keyboard;
static lv_obj_t *s_wifi_nav_buttons[3];
static cyd_wifi_entry_t s_wifi_entries[WIFI_SCAN_MAX];
static char s_selected_ssid[33];
static wifi_auth_mode_t s_selected_authmode;
static volatile bool s_scan_requested;
static volatile bool s_scan_ready;
static volatile bool s_scan_failed;
static uint8_t s_wifi_entry_count;
static uint8_t s_current_page;
static uint64_t s_prev_tx;
static uint64_t s_prev_rx;
static int64_t s_last_touch_log_us;

static int clamp_map(int value, int in_min, int in_max, int out_min, int out_max)
{
    if (value < in_min) value = in_min;
    if (value > in_max) value = in_max;
    return out_min + (value - in_min) * (out_max - out_min) / (in_max - in_min);
}

static void touch_process(esp_lcd_touch_handle_t tp, uint16_t *x, uint16_t *y,
                          uint16_t *strength, uint8_t *point_num, uint8_t max_point_num)
{
    (void)tp;
    (void)strength;
    (void)max_point_num;
    if (*point_num == 0) return;

    uint16_t raw_x = x[0];
    uint16_t raw_y = y[0];
    x[0] = clamp_map(raw_y, TOUCH_Y_MIN, TOUCH_Y_MAX, 0, LCD_HRES - 1);
    y[0] = clamp_map(raw_x, TOUCH_X_MIN, TOUCH_X_MAX, 0, LCD_VRES - 1);
    int64_t now = esp_timer_get_time();
    if (now - s_last_touch_log_us >= 250000) {
        ESP_LOGI(TAG, "Touch raw=%u,%u mapped=%u,%u", raw_x, raw_y, x[0], y[0]);
        s_last_touch_log_us = now;
    }
}

static bool lcd_flush_done(esp_lcd_panel_io_handle_t io,
                           esp_lcd_panel_io_event_data_t *event_data, void *user_ctx)
{
    (void)io;
    (void)event_data;
    (void)user_ctx;
    lv_disp_flush_ready(&s_disp_drv);
    return false;
}

static void lvgl_flush(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map)
{
    esp_err_t err = esp_lcd_panel_draw_bitmap(s_panel, area->x1, area->y1,
                                               area->x2 + 1, area->y2 + 1,
                                               color_map);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "LCD flush failed: %s", esp_err_to_name(err));
        lv_disp_flush_ready(drv);
    }
}

static void lvgl_touch_read(lv_indev_drv_t *drv, lv_indev_data_t *data)
{
    (void)drv;
    esp_lcd_touch_point_data_t point = { 0 };
    uint8_t count = 0;

    if (esp_lcd_touch_read_data(s_touch) == ESP_OK &&
        esp_lcd_touch_get_data(s_touch, &point, &count, 1) == ESP_OK && count > 0) {
        data->state = LV_INDEV_STATE_PR;
        data->point.x = point.x;
        data->point.y = point.y;
    } else {
        data->state = LV_INDEV_STATE_REL;
    }
}

static void lvgl_tick(void *arg)
{
    (void)arg;
    lv_tick_inc(2);
}

static lv_obj_t *make_label(lv_obj_t *parent, const char *text, lv_coord_t x,
                            lv_coord_t y, const lv_font_t *font, lv_color_t color)
{
    lv_obj_t *label = lv_label_create(parent);
    lv_label_set_text(label, text);
    lv_obj_set_pos(label, x, y);
    lv_obj_set_style_text_font(label, font, 0);
    lv_obj_set_style_text_color(label, color, 0);
    return label;
}

static lv_obj_t *make_card(lv_obj_t *parent, lv_coord_t x, lv_coord_t y,
                           lv_coord_t width, lv_coord_t height)
{
    lv_obj_t *card = lv_obj_create(parent);
    lv_obj_set_pos(card, x, y);
    lv_obj_set_size(card, width, height);
    lv_obj_set_style_radius(card, 6, 0);
    lv_obj_set_style_bg_color(card, lv_color_hex(0x16202A), 0);
    lv_obj_set_style_border_color(card, lv_color_hex(0x273746), 0);
    lv_obj_set_style_border_width(card, 1, 0);
    lv_obj_set_style_pad_all(card, 8, 0);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);
    return card;
}

static void show_page(uint8_t page)
{
    s_current_page = page;
    if (page == 0) lv_obj_clear_flag(s_status_page, LV_OBJ_FLAG_HIDDEN);
    else lv_obj_add_flag(s_status_page, LV_OBJ_FLAG_HIDDEN);
    if (page == 1) lv_obj_clear_flag(s_access_page, LV_OBJ_FLAG_HIDDEN);
    else lv_obj_add_flag(s_access_page, LV_OBJ_FLAG_HIDDEN);
    if (page == 2) lv_obj_clear_flag(s_wifi_page, LV_OBJ_FLAG_HIDDEN);
    else lv_obj_add_flag(s_wifi_page, LV_OBJ_FLAG_HIDDEN);
}

static void nav_event(lv_event_t *event)
{
    show_page((uint8_t)(intptr_t)lv_event_get_user_data(event));
}

static void add_nav(lv_obj_t *page, uint8_t active_page)
{
    static const char *labels[] = {
        LV_SYMBOL_HOME " Status",
        LV_SYMBOL_SETTINGS " Zugriff",
        LV_SYMBOL_WIFI " WLAN",
    };
    for (uint8_t i = 0; i < 3; i++) {
        lv_obj_t *button = lv_btn_create(page);
        lv_obj_set_pos(button, 5 + i * 105, 204);
        lv_obj_set_size(button, 100, 29);
        lv_obj_set_ext_click_area(button, 8);
        lv_obj_set_style_radius(button, 5, 0);
        lv_obj_set_style_bg_color(button,
            lv_color_hex(i == active_page ? 0x168FA1 : 0x1C2935), 0);
        lv_obj_add_event_cb(button, nav_event, LV_EVENT_CLICKED, (void *)(intptr_t)i);
        if (page == s_wifi_page) s_wifi_nav_buttons[i] = button;
        lv_obj_t *label = lv_label_create(button);
        lv_label_set_text(label, labels[i]);
        lv_obj_set_style_text_font(label, LV_FONT_DEFAULT, 0);
        lv_obj_center(label);
    }
}

static void format_rate(uint64_t bytes, char *buf, size_t len)
{
    if (bytes >= 1024 * 1024) {
        snprintf(buf, len, "%.1f MB/s", bytes / (1024.0 * 1024.0));
    } else if (bytes >= 1024) {
        snprintf(buf, len, "%.0f KB/s", bytes / 1024.0);
    } else {
        snprintf(buf, len, "%" PRIu64 " B/s", bytes);
    }
}

static void status_update(lv_timer_t *timer)
{
    (void)timer;
    char text[96];
    wifi_ap_record_t info;
    bool connected = ap_connect && esp_wifi_sta_get_ap_info(&info) == ESP_OK;

    lv_label_set_text(s_uplink_value, connected ? "ONLINE" : "OFFLINE");
    lv_obj_set_style_text_color(s_uplink_value,
        lv_color_hex(connected ? 0x55D68B : 0xFF6B6B), 0);
    lv_obj_set_style_bg_color(s_status_dot,
        lv_color_hex(connected ? 0x55D68B : 0xFF6B6B), 0);
    if (connected) {
        snprintf(text, sizeof(text), "%.*s  %d dBm", 24, (const char *)info.ssid, info.rssi);
    } else if (ssid && ssid[0]) {
        snprintf(text, sizeof(text), "%.*s", 28, ssid);
    } else {
        snprintf(text, sizeof(text), "Kein Bezugsnetz");
    }
    lv_label_set_text(s_uplink_detail, text);

    snprintf(text, sizeof(text), "%u", (unsigned)connect_count);
    lv_label_set_text(s_client_value, text);
    snprintf(text, sizeof(text), "%.*s", 22,
             (ap_ssid && ap_ssid[0]) ? ap_ssid : "ESP32_NAT_Router");
    lv_label_set_text(s_ap_value, text);

    uint64_t tx = get_sta_bytes_sent();
    uint64_t rx = get_sta_bytes_received();
    char down[24], up[24];
    format_rate(rx >= s_prev_rx ? rx - s_prev_rx : 0, down, sizeof(down));
    format_rate(tx >= s_prev_tx ? tx - s_prev_tx : 0, up, sizeof(up));
    s_prev_tx = tx;
    s_prev_rx = rx;
    snprintf(text, sizeof(text), "D %s\nU %s", down, up);
    lv_label_set_text(s_rate_value, text);
}

static void save_access(void)
{
    access_internet_enabled = lv_obj_has_state(s_switch_internet, LV_STATE_CHECKED);
    access_clients_enabled = lv_obj_has_state(s_switch_clients, LV_STATE_CHECKED);
    access_private_enabled = lv_obj_has_state(s_switch_private, LV_STATE_CHECKED);
    set_config_param_int("acc_inet", access_internet_enabled);
    set_config_param_int("acc_clients", access_clients_enabled);
    set_config_param_int("acc_private", access_private_enabled);
    ESP_LOGI(TAG, "Touch access mode: internet=%u clients=%u private=%u",
             access_internet_enabled, access_clients_enabled, access_private_enabled);
}

static void switch_event(lv_event_t *event)
{
    (void)event;
    save_access();
}

static void set_switch(lv_obj_t *sw, bool enabled)
{
    if (enabled) lv_obj_add_state(sw, LV_STATE_CHECKED);
    else lv_obj_clear_state(sw, LV_STATE_CHECKED);
}

static void preset_event(lv_event_t *event)
{
    intptr_t mode = (intptr_t)lv_event_get_user_data(event);
    set_switch(s_switch_internet, (mode & 1) != 0);
    set_switch(s_switch_clients, (mode & 2) != 0);
    set_switch(s_switch_private, (mode & 4) != 0);
    save_access();
}

static lv_obj_t *access_row(lv_obj_t *parent, const char *title, const char *detail,
                            lv_coord_t y, bool checked)
{
    make_label(parent, title, 12, y, &lv_font_montserrat_16, lv_color_hex(0xEAF4F8));
    make_label(parent, detail, 12, y + 20, LV_FONT_DEFAULT, lv_color_hex(0x8295A5));
    lv_obj_t *sw = lv_switch_create(parent);
    lv_obj_set_pos(sw, 255, y + 4);
    lv_obj_set_size(sw, 48, 24);
    if (checked) lv_obj_add_state(sw, LV_STATE_CHECKED);
    lv_obj_set_style_bg_color(sw, lv_color_hex(0x168FA1), LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_add_event_cb(sw, switch_event, LV_EVENT_VALUE_CHANGED, NULL);
    return sw;
}

static void preset_button(lv_obj_t *parent, const char *text, lv_coord_t x,
                          lv_coord_t y, intptr_t mode)
{
    lv_obj_t *button = lv_btn_create(parent);
    lv_obj_set_pos(button, x, y);
    lv_obj_set_size(button, 72, 31);
    lv_obj_set_style_radius(button, 5, 0);
    lv_obj_set_style_bg_color(button, lv_color_hex(0x223241), 0);
    lv_obj_add_event_cb(button, preset_event, LV_EVENT_CLICKED, (void *)mode);
    lv_obj_t *label = lv_label_create(button);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_font(label, LV_FONT_DEFAULT, 0);
    lv_obj_center(label);
}

static void wifi_restart_timer(lv_timer_t *timer)
{
    (void)timer;
    esp_restart();
}

static void wifi_keyboard_event(lv_event_t *event)
{
    lv_event_code_t code = lv_event_get_code(event);
    if (code == LV_EVENT_CANCEL) {
        lv_obj_add_flag(s_wifi_password_panel, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(s_wifi_scan_panel, LV_OBJ_FLAG_HIDDEN);
        lv_keyboard_set_textarea(s_wifi_keyboard, NULL);
        for (uint8_t i = 0; i < 3; i++) lv_obj_clear_flag(s_wifi_nav_buttons[i], LV_OBJ_FLAG_HIDDEN);
        return;
    }
    if (code != LV_EVENT_READY) return;

    const char *password = lv_textarea_get_text(s_wifi_password);
    if (s_selected_authmode != WIFI_AUTH_OPEN && strlen(password) < 8) {
        lv_label_set_text(s_wifi_selected_label, "Passwort muss mindestens 8 Zeichen haben");
        return;
    }

    esp_err_t err = set_config_param_str("ssid", s_selected_ssid);
    if (err == ESP_OK) err = set_config_param_str("passwd", password);
    if (err == ESP_OK) err = set_config_param_str("ent_username", "");
    if (err == ESP_OK) err = set_config_param_str("ent_identity", "");
    if (err != ESP_OK) {
        lv_label_set_text(s_wifi_selected_label, "Speichern fehlgeschlagen");
        ESP_LOGE(TAG, "Touch WLAN save failed: %s", esp_err_to_name(err));
        return;
    }

    ESP_LOGI(TAG, "Touch WLAN selected: %s", s_selected_ssid);
    lv_keyboard_set_textarea(s_wifi_keyboard, NULL);
    lv_obj_add_flag(s_wifi_keyboard, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(s_wifi_password, LV_OBJ_FLAG_HIDDEN);
    lv_label_set_text(s_wifi_selected_label, "Gespeichert. Router startet neu...");
    lv_timer_t *timer = lv_timer_create(wifi_restart_timer, 1200, NULL);
    lv_timer_set_repeat_count(timer, 1);
}

static void wifi_network_event(lv_event_t *event)
{
    uint8_t index = (uint8_t)(intptr_t)lv_event_get_user_data(event);
    if (index >= s_wifi_entry_count) return;
    if (s_wifi_entries[index].authmode == WIFI_AUTH_WPA2_ENTERPRISE) {
        lv_label_set_text(s_wifi_scan_status, "Enterprise-WLAN bitte in der WebUI einrichten");
        return;
    }

    strlcpy(s_selected_ssid, s_wifi_entries[index].ssid, sizeof(s_selected_ssid));
    s_selected_authmode = s_wifi_entries[index].authmode;
    char title[64];
    snprintf(title, sizeof(title), "%s: %s",
             s_selected_authmode == WIFI_AUTH_OPEN ? "Offenes WLAN" : "Passwort",
             s_selected_ssid);
    lv_label_set_text(s_wifi_selected_label, title);
    lv_textarea_set_text(s_wifi_password, "");
    lv_obj_clear_flag(s_wifi_password, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(s_wifi_keyboard, LV_OBJ_FLAG_HIDDEN);
    lv_keyboard_set_textarea(s_wifi_keyboard, s_wifi_password);
    lv_obj_add_flag(s_wifi_scan_panel, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(s_wifi_password_panel, LV_OBJ_FLAG_HIDDEN);
    for (uint8_t i = 0; i < 3; i++) lv_obj_add_flag(s_wifi_nav_buttons[i], LV_OBJ_FLAG_HIDDEN);
}

static void wifi_render_results(void)
{
    show_page(2);
    lv_obj_clean(s_wifi_result_list);
    if (s_scan_failed) {
        lv_label_set_text(s_wifi_scan_status, "Scan fehlgeschlagen - erneut versuchen");
        return;
    }
    if (s_wifi_entry_count == 0) {
        lv_label_set_text(s_wifi_scan_status, "Keine WLAN-Netze gefunden");
        return;
    }

    char status[48];
    snprintf(status, sizeof(status), "%u Netze gefunden", (unsigned)s_wifi_entry_count);
    lv_label_set_text(s_wifi_scan_status, status);
    for (uint8_t i = 0; i < s_wifi_entry_count; i++) {
        lv_obj_t *button = lv_btn_create(s_wifi_result_list);
        lv_obj_set_pos(button, 0, i * 24);
        lv_obj_set_size(button, 296, 22);
        lv_obj_set_style_radius(button, 4, 0);
        lv_obj_set_style_bg_color(button, lv_color_hex(0x1C2935), 0);
        lv_obj_set_style_pad_hor(button, 7, 0);
        lv_obj_add_event_cb(button, wifi_network_event, LV_EVENT_CLICKED,
                            (void *)(intptr_t)i);
        char row[64];
        snprintf(row, sizeof(row), "%-24.24s %4d dBm %s",
                 s_wifi_entries[i].ssid, s_wifi_entries[i].rssi,
                 s_wifi_entries[i].authmode == WIFI_AUTH_OPEN ? "OFFEN" : "");
        lv_obj_t *label = lv_label_create(button);
        lv_label_set_text(label, row);
        lv_obj_set_style_text_font(label, LV_FONT_DEFAULT, 0);
        lv_obj_align(label, LV_ALIGN_LEFT_MID, 0, 0);
    }
}

static void wifi_scan_ui_timer(lv_timer_t *timer)
{
    (void)timer;
    if (!s_scan_ready) return;
    s_scan_ready = false;
    wifi_render_results();
}

static void wifi_scan_event(lv_event_t *event)
{
    (void)event;
    if (s_scan_requested) return;
    s_scan_failed = false;
    s_scan_ready = false;
    s_scan_requested = true;
    show_page(2);
    lv_label_set_text(s_wifi_scan_status, "Scanne...");
}

static void wifi_scan_task(void *arg)
{
    (void)arg;
    while (true) {
        if (!s_scan_requested) {
            vTaskDelay(pdMS_TO_TICKS(100));
            continue;
        }

        wifi_scan_config_t config = {
            .show_hidden = false,
            .scan_type = WIFI_SCAN_TYPE_ACTIVE,
        };
        esp_err_t err = esp_wifi_scan_start(&config, true);
        s_wifi_entry_count = 0;
        if (err == ESP_OK) {
            uint16_t count = WIFI_SCAN_FETCH_MAX;
            wifi_ap_record_t records[WIFI_SCAN_FETCH_MAX];
            err = esp_wifi_scan_get_ap_records(&count, records);
            if (err == ESP_OK) {
                for (uint16_t i = 0; i < count; i++) {
                    if (records[i].ssid[0] == '\0') continue;
                    bool duplicate = false;
                    for (uint8_t j = 0; j < s_wifi_entry_count; j++) {
                        if (strcmp(s_wifi_entries[j].ssid, (const char *)records[i].ssid) == 0) {
                            duplicate = true;
                            break;
                        }
                    }
                    if (duplicate) continue;

                    uint8_t insert = s_wifi_entry_count;
                    if (insert < WIFI_SCAN_MAX) s_wifi_entry_count++;
                    else if (records[i].rssi <= s_wifi_entries[WIFI_SCAN_MAX - 1].rssi) continue;
                    else insert = WIFI_SCAN_MAX - 1;

                    while (insert > 0 && records[i].rssi > s_wifi_entries[insert - 1].rssi) {
                        if (insert < WIFI_SCAN_MAX) s_wifi_entries[insert] = s_wifi_entries[insert - 1];
                        insert--;
                    }
                    strlcpy(s_wifi_entries[insert].ssid, (const char *)records[i].ssid,
                            sizeof(s_wifi_entries[insert].ssid));
                    s_wifi_entries[insert].rssi = records[i].rssi;
                    s_wifi_entries[insert].authmode = records[i].authmode;
                }
            }
        }
        s_scan_failed = err != ESP_OK;
        s_scan_requested = false;
        s_scan_ready = true;
        ESP_LOGI(TAG, "Touch WLAN scan: %u results (%s)",
                 (unsigned)s_wifi_entry_count, esp_err_to_name(err));
    }
}

static void create_ui(void)
{
    lv_obj_t *screen = lv_scr_act();
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x0B1117), 0);
    lv_obj_set_style_text_color(screen, lv_color_hex(0xEAF4F8), 0);
    lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);

    s_status_page = lv_obj_create(screen);
    lv_obj_set_size(s_status_page, LCD_HRES, LCD_VRES);
    lv_obj_set_style_bg_opa(s_status_page, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(s_status_page, 0, 0);
    lv_obj_set_style_pad_all(s_status_page, 0, 0);
    lv_obj_clear_flag(s_status_page, LV_OBJ_FLAG_SCROLLABLE);

    s_status_dot = lv_obj_create(s_status_page);
    lv_obj_set_pos(s_status_dot, 10, 12);
    lv_obj_set_size(s_status_dot, 10, 10);
    lv_obj_set_style_radius(s_status_dot, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_border_width(s_status_dot, 0, 0);
    make_label(s_status_page, "KLISCHTRONIK", 28, 7, &lv_font_montserrat_20, lv_color_hex(0x52D8E8));
    make_label(s_status_page, "MOD | DARK", 232, 10, LV_FONT_DEFAULT, lv_color_hex(0x8295A5));

    lv_obj_t *uplink = make_card(s_status_page, 8, 37, 304, 58);
    make_label(uplink, "UPLINK", 0, 0, LV_FONT_DEFAULT, lv_color_hex(0x8295A5));
    s_uplink_value = make_label(uplink, "OFFLINE", 0, 16, &lv_font_montserrat_20, lv_color_hex(0xFF6B6B));
    s_uplink_detail = make_label(uplink, "", 117, 20, LV_FONT_DEFAULT, lv_color_hex(0xAFC0CC));
    lv_label_set_long_mode(s_uplink_detail, LV_LABEL_LONG_DOT);
    lv_obj_set_width(s_uplink_detail, 170);

    lv_obj_t *clients = make_card(s_status_page, 8, 103, 94, 92);
    make_label(clients, "CLIENTS", 0, 0, LV_FONT_DEFAULT, lv_color_hex(0x8295A5));
    s_client_value = make_label(clients, "0", 0, 20, &lv_font_montserrat_20, lv_color_hex(0xEAF4F8));
    make_label(clients, "AP", 0, 52, LV_FONT_DEFAULT, lv_color_hex(0x8295A5));
    s_ap_value = make_label(clients, "", 22, 52, LV_FONT_DEFAULT, lv_color_hex(0xAFC0CC));
    lv_label_set_long_mode(s_ap_value, LV_LABEL_LONG_DOT);
    lv_obj_set_width(s_ap_value, 53);

    lv_obj_t *traffic = make_card(s_status_page, 110, 103, 202, 92);
    make_label(traffic, "LIVE TRAFFIC", 0, 0, LV_FONT_DEFAULT, lv_color_hex(0x8295A5));
    s_rate_value = make_label(traffic, "D 0 B/s\nU 0 B/s", 0, 21,
                              &lv_font_montserrat_16, lv_color_hex(0xEAF4F8));
    add_nav(s_status_page, 0);

    s_access_page = lv_obj_create(screen);
    lv_obj_set_size(s_access_page, LCD_HRES, LCD_VRES);
    lv_obj_set_style_bg_opa(s_access_page, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(s_access_page, 0, 0);
    lv_obj_set_style_pad_all(s_access_page, 0, 0);
    lv_obj_clear_flag(s_access_page, LV_OBJ_FLAG_SCROLLABLE);
    make_label(s_access_page, "ZUGRIFFSMODUS", 10, 7, &lv_font_montserrat_20, lv_color_hex(0x52D8E8));
    make_label(s_access_page, "DARK", 272, 10, LV_FONT_DEFAULT, lv_color_hex(0x52D8E8));
    make_label(s_access_page, "Aenderungen gelten sofort", 10, 30, LV_FONT_DEFAULT, lv_color_hex(0x8295A5));

    preset_button(s_access_page, "Internet", 8, 49, 1);
    preset_button(s_access_page, "Geraete", 86, 49, 2);
    preset_button(s_access_page, "Beides", 164, 49, 3);
    preset_button(s_access_page, "Alles", 242, 49, 7);

    s_switch_internet = access_row(s_access_page, "Internet teilen", "Oeffentliche Ziele", 88,
                                    access_internet_enabled);
    s_switch_clients = access_row(s_access_page, "Geraete verbinden", "Clients untereinander", 128,
                                   access_clients_enabled);
    s_switch_private = access_row(s_access_page, "Bezugsnetz", "Private Uplink-Netze", 168,
                                   access_private_enabled);
    add_nav(s_access_page, 1);

    s_wifi_page = lv_obj_create(screen);
    lv_obj_set_size(s_wifi_page, LCD_HRES, LCD_VRES);
    lv_obj_set_style_bg_opa(s_wifi_page, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(s_wifi_page, 0, 0);
    lv_obj_set_style_pad_all(s_wifi_page, 0, 0);
    lv_obj_clear_flag(s_wifi_page, LV_OBJ_FLAG_SCROLLABLE);

    s_wifi_scan_panel = lv_obj_create(s_wifi_page);
    lv_obj_set_size(s_wifi_scan_panel, LCD_HRES, 200);
    lv_obj_set_style_bg_opa(s_wifi_scan_panel, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(s_wifi_scan_panel, 0, 0);
    lv_obj_set_style_pad_all(s_wifi_scan_panel, 0, 0);
    lv_obj_clear_flag(s_wifi_scan_panel, LV_OBJ_FLAG_SCROLLABLE);
    make_label(s_wifi_scan_panel, "BEZUGS-WLAN", 10, 7, &lv_font_montserrat_20,
               lv_color_hex(0x52D8E8));
    lv_obj_t *scan_button = lv_btn_create(s_wifi_scan_panel);
    lv_obj_set_pos(scan_button, 229, 5);
    lv_obj_set_size(scan_button, 82, 30);
    lv_obj_set_style_radius(scan_button, 5, 0);
    lv_obj_set_style_bg_color(scan_button, lv_color_hex(0x168FA1), 0);
    lv_obj_add_event_cb(scan_button, wifi_scan_event, LV_EVENT_CLICKED, NULL);
    lv_obj_t *scan_label = lv_label_create(scan_button);
    lv_label_set_text(scan_label, LV_SYMBOL_REFRESH " Scan");
    lv_obj_center(scan_label);
    s_wifi_scan_status = make_label(s_wifi_scan_panel, "Scan antippen", 10, 39,
                                    LV_FONT_DEFAULT, lv_color_hex(0x8295A5));
    s_wifi_result_list = lv_obj_create(s_wifi_scan_panel);
    lv_obj_set_pos(s_wifi_result_list, 8, 55);
    lv_obj_set_size(s_wifi_result_list, 304, 143);
    lv_obj_set_style_bg_opa(s_wifi_result_list, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(s_wifi_result_list, 0, 0);
    lv_obj_set_style_pad_all(s_wifi_result_list, 0, 0);
    lv_obj_clear_flag(s_wifi_result_list, LV_OBJ_FLAG_SCROLLABLE);

    s_wifi_password_panel = lv_obj_create(s_wifi_page);
    lv_obj_set_size(s_wifi_password_panel, LCD_HRES, LCD_VRES);
    lv_obj_set_style_bg_opa(s_wifi_password_panel, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(s_wifi_password_panel, 0, 0);
    lv_obj_set_style_pad_all(s_wifi_password_panel, 0, 0);
    lv_obj_clear_flag(s_wifi_password_panel, LV_OBJ_FLAG_SCROLLABLE);
    s_wifi_selected_label = make_label(s_wifi_password_panel, "WLAN-Passwort", 9, 2,
                                       LV_FONT_DEFAULT, lv_color_hex(0x52D8E8));
    lv_label_set_long_mode(s_wifi_selected_label, LV_LABEL_LONG_DOT);
    lv_obj_set_width(s_wifi_selected_label, 302);
    s_wifi_password = lv_textarea_create(s_wifi_password_panel);
    lv_obj_set_pos(s_wifi_password, 8, 21);
    lv_obj_set_size(s_wifi_password, 304, 35);
    lv_textarea_set_one_line(s_wifi_password, true);
    lv_textarea_set_password_mode(s_wifi_password, true);
    lv_textarea_set_placeholder_text(s_wifi_password, "Passwort");
    s_wifi_keyboard = lv_keyboard_create(s_wifi_password_panel);
    lv_obj_set_pos(s_wifi_keyboard, 8, 59);
    lv_obj_set_size(s_wifi_keyboard, 304, 171);
    lv_keyboard_set_mode(s_wifi_keyboard, LV_KEYBOARD_MODE_TEXT_LOWER);
    lv_obj_add_event_cb(s_wifi_keyboard, wifi_keyboard_event, LV_EVENT_ALL, NULL);
    lv_obj_add_flag(s_wifi_password_panel, LV_OBJ_FLAG_HIDDEN);
    add_nav(s_wifi_page, 2);

    show_page(0);

    lv_timer_create(status_update, 1000, NULL);
    lv_timer_create(wifi_scan_ui_timer, 250, NULL);
    status_update(NULL);
}

static esp_err_t init_lcd(void)
{
    gpio_config_t backlight = {
        .pin_bit_mask = 1ULL << PIN_LCD_BACKLIGHT,
        .mode = GPIO_MODE_OUTPUT,
    };
    ESP_RETURN_ON_ERROR(gpio_config(&backlight), TAG, "backlight GPIO");
    gpio_set_level(PIN_LCD_BACKLIGHT, 0);

    spi_bus_config_t bus = {
        .sclk_io_num = PIN_LCD_CLK,
        .mosi_io_num = PIN_LCD_MOSI,
        .miso_io_num = PIN_LCD_MISO,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = LCD_HRES * LCD_DRAW_LINES * sizeof(lv_color_t),
    };
    ESP_RETURN_ON_ERROR(spi_bus_initialize(LCD_HOST, &bus, SPI_DMA_CH_AUTO), TAG, "LCD SPI bus");

    esp_lcd_panel_io_handle_t io = NULL;
    esp_lcd_panel_io_spi_config_t io_config = {
        .cs_gpio_num = PIN_LCD_CS,
        .dc_gpio_num = PIN_LCD_DC,
        .spi_mode = 3,
        .pclk_hz = 20 * 1000 * 1000,
        .trans_queue_depth = 10,
        .on_color_trans_done = lcd_flush_done,
        .user_ctx = NULL,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
    };
    ESP_RETURN_ON_ERROR(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_HOST,
                                                 &io_config, &io), TAG, "LCD panel IO");
    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = -1,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_BGR,
        .bits_per_pixel = 16,
    };
    ESP_RETURN_ON_ERROR(esp_lcd_new_panel_st7789(io, &panel_config, &s_panel), TAG, "ST7789");
    ESP_RETURN_ON_ERROR(esp_lcd_panel_reset(s_panel), TAG, "LCD reset");
    ESP_RETURN_ON_ERROR(esp_lcd_panel_init(s_panel), TAG, "LCD init");
    uint8_t lcd_id[3] = { 0 };
    esp_err_t id_err = esp_lcd_panel_io_rx_param(io, 0x04, lcd_id, sizeof(lcd_id));
    ESP_LOGI(TAG, "LCD ID read: %s %02X %02X %02X", esp_err_to_name(id_err),
             lcd_id[0], lcd_id[1], lcd_id[2]);
    ESP_RETURN_ON_ERROR(esp_lcd_panel_invert_color(s_panel, true), TAG, "LCD invert");
    ESP_RETURN_ON_ERROR(esp_lcd_panel_swap_xy(s_panel, true), TAG, "LCD rotate");
    ESP_RETURN_ON_ERROR(esp_lcd_panel_mirror(s_panel, true, false), TAG, "LCD mirror");
    ESP_RETURN_ON_ERROR(esp_lcd_panel_disp_on_off(s_panel, true), TAG, "LCD on");
    gpio_set_level(PIN_LCD_BACKLIGHT, 1);
    return ESP_OK;
}

static esp_err_t init_touch(void)
{
    spi_bus_config_t bus = {
        .sclk_io_num = PIN_TOUCH_CLK,
        .mosi_io_num = PIN_TOUCH_MOSI,
        .miso_io_num = PIN_TOUCH_MISO,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 64,
    };
    ESP_RETURN_ON_ERROR(spi_bus_initialize(TOUCH_HOST, &bus, SPI_DMA_CH_AUTO), TAG, "touch SPI bus");

    esp_lcd_panel_io_handle_t io = NULL;
    esp_lcd_panel_io_spi_config_t io_config =
        ESP_LCD_TOUCH_IO_SPI_XPT2046_CONFIG(PIN_TOUCH_CS);
    ESP_RETURN_ON_ERROR(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)TOUCH_HOST,
                                                 &io_config, &io), TAG, "touch panel IO");
    esp_lcd_touch_config_t touch_config = {
        .x_max = LCD_HRES,
        .y_max = LCD_VRES,
        .rst_gpio_num = -1,
        .int_gpio_num = PIN_TOUCH_IRQ,
        .levels = { .reset = 0, .interrupt = 0 },
        .flags = { .swap_xy = 0, .mirror_x = 0, .mirror_y = 0 },
        .process_coordinates = touch_process,
    };
    return esp_lcd_touch_new_spi_xpt2046(io, &touch_config, &s_touch);
}

static void cyd_task(void *arg)
{
    (void)arg;
    if (init_lcd() != ESP_OK || init_touch() != ESP_OK) {
        ESP_LOGE(TAG, "CYD hardware initialization failed");
        vTaskDelete(NULL);
        return;
    }

    lv_init();
    lv_color_t *buf1 = heap_caps_malloc(LCD_HRES * LCD_DRAW_LINES * sizeof(lv_color_t),
                                        MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
    if (buf1 == NULL) {
        ESP_LOGE(TAG, "Not enough DMA memory for LVGL buffers");
        free(buf1);
        vTaskDelete(NULL);
        return;
    }

    lv_disp_draw_buf_init(&s_draw_buf, buf1, NULL, LCD_HRES * LCD_DRAW_LINES);
    lv_disp_drv_init(&s_disp_drv);
    s_disp_drv.hor_res = LCD_HRES;
    s_disp_drv.ver_res = LCD_VRES;
    s_disp_drv.flush_cb = lvgl_flush;
    s_disp_drv.draw_buf = &s_draw_buf;
    lv_disp_drv_register(&s_disp_drv);

    static lv_indev_drv_t input_drv;
    lv_indev_drv_init(&input_drv);
    input_drv.type = LV_INDEV_TYPE_POINTER;
    input_drv.read_cb = lvgl_touch_read;
    lv_indev_drv_register(&input_drv);

    const esp_timer_create_args_t tick_args = {
        .callback = lvgl_tick,
        .name = "lvgl_tick",
    };
    esp_timer_handle_t tick_timer;
    ESP_ERROR_CHECK(esp_timer_create(&tick_args, &tick_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(tick_timer, 2000));

    create_ui();
    ESP_LOGI(TAG, "CYD2USB dashboard ready (ST7789 mode 3 + XPT2046)");
    while (true) {
        uint32_t delay_ms = lv_timer_handler();
        if (delay_ms < 5) delay_ms = 5;
        if (delay_ms > 20) delay_ms = 20;
        vTaskDelay(pdMS_TO_TICKS(delay_ms));
    }
}

void cyd_display_init(void)
{
    BaseType_t result = xTaskCreatePinnedToCore(cyd_task, "cyd_ui", 7168, NULL, 3, NULL, 1);
    if (result != pdPASS) {
        ESP_LOGE(TAG, "Failed to create CYD display task");
        return;
    }
    result = xTaskCreatePinnedToCore(wifi_scan_task, "cyd_scan", 4096, NULL, 2, NULL, 0);
    if (result != pdPASS) {
        ESP_LOGE(TAG, "Failed to create CYD WLAN scan task");
    }
}

#endif /* CONFIG_CYD_DISPLAY */
