// myui: a UI for Gaggiuino machines on the Brewdeck core. Started from brewdeck-ui-template (Brewdeck's example UI,
// firmware/src/ui_minimal): one LVGL 9 screen with the link, the machine's live values, the active profile, the
// running shot, and the profile list (a tap selects a profile: a command, with the core's answer shown).
// Every .c / .cpp file in ui/ is compiled into the UI (fonts too); the scripts and .brewdeck/ stay outside it.
//   ./emu.sh            run it in the emulator (a fake machine in a window)
//   ./fw.sh ui-p4       build the firmware for a board (ui = Sunton ESP32-S3, ui-p4 = Guition ESP32-P4)
// Everything runs on the UI task: the core calls bd_ui_init once, then bd_ui_tick every loop pass and bd_ui_event for
// each event. Read state with bd_*_state() / bd_profile_list(); redraw only what changed (bd_seq_*). The API is one
// header, brewdeck_core.h (in the Brewdeck checkout, firmware/src/core); the guide: docs/custom-ui.md there.
#include <stdio.h>
#include "bd_lvgl9.h"           // LVGL 9 on the core's display and touch (include it in one file)

extern "C" const bd_ui_desc BD_UI = {"myui", "0.1", 1};   // name (max 15 chars: also its settings namespace), version, API version it was written for

static const lv_color_t BG = lv_color_hex(0x0A0A0B), CARD = lv_color_hex(0x1A1A1D), TEXT = lv_color_hex(0xF2F2F3),
                        MUTED = lv_color_hex(0x8E8E96), ACCENT = lv_color_hex(0x7CC4FF);
static lv_obj_t *g_link, *g_temp, *g_values, *g_profile, *g_shot, *g_answer, *g_list;

static lv_obj_t *text(lv_obj_t *parent, const lv_font_t *f, lv_color_t c, int x, int y, int w) {
  lv_obj_t *l = lv_label_create(parent);
  lv_obj_set_style_text_font(l, f, 0);
  lv_obj_set_style_text_color(l, c, 0);
  lv_obj_set_pos(l, x, y);
  if (w > 0) lv_obj_set_width(l, w);
  lv_label_set_text(l, "");
  return l;
}

static const char *result_text(bd_result r) {
  switch (r) {
    case BD_OK: return "sent, waiting for the machine";
    case BD_ERR_OFFLINE: return "refused: the machine is offline";
    case BD_ERR_BUSY: return "refused: the machine is busy";
    case BD_ERR_PENDING: return "refused: still waiting for the last one";
    case BD_ERR_UNKNOWN_ID: return "refused: no such profile";
    default: return "refused";
  }
}

// ---- profile list: one row per profile, a tap selects it (only for now: save = false) -------------------------------
static void on_row(lv_event_t *e) {
  const int id = (int)(intptr_t)lv_event_get_user_data(e);
  char b[96];
  snprintf(b, sizeof b, "Select profile %d: %s", id, result_text(bd_select_profile(id, false)));
  lv_label_set_text(g_answer, b);
}
static void build_list(void) {
  lv_obj_clean(g_list);
  const bd_profiles *l = bd_profile_list();
  for (int i = 0; i < l->count; i++) {
    lv_obj_t *row = lv_obj_create(g_list);
    lv_obj_remove_style_all(row);
    lv_obj_set_size(row, LV_PCT(100), 48);
    lv_obj_set_style_radius(row, 12, 0);
    lv_obj_set_style_bg_opa(row, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(row, l->p[i].active ? lv_color_hex(0x10293A) : CARD, 0);
    lv_obj_set_style_bg_color(row, lv_color_hex(0x2A2A2F), LV_STATE_PRESSED);
    lv_obj_add_flag(row, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(row, on_row, LV_EVENT_CLICKED, (void *)(intptr_t)l->p[i].id);
    lv_obj_t *t = text(row, &lv_font_montserrat_14, l->p[i].active ? ACCENT : TEXT, 14, 0, 320);
    lv_label_set_long_mode(t, LV_LABEL_LONG_MODE_DOTS);
    lv_obj_set_height(t, lv_font_get_line_height(&lv_font_montserrat_14));   // one line, "..." when too long
    lv_obj_align(t, LV_ALIGN_LEFT_MID, 14, 0);
    lv_label_set_text(t, l->p[i].name);
  }
  if (!l->count) {
    lv_obj_t *t = text(g_list, &lv_font_montserrat_14, MUTED, 0, 0, 0);
    lv_obj_set_width(t, LV_PCT(100));               // wrap inside the list
    lv_label_set_long_mode(t, LV_LABEL_LONG_MODE_WRAP);
    lv_label_set_text(t, "No profiles yet: they come once the machine answers.");
  }
}

// ---- what the core calls -------------------------------------------------------------------------------------------
void bd_ui_init(void) {
  bd_lvgl9_setup();
  lv_obj_t *scr = lv_screen_active();
  lv_obj_set_style_bg_color(scr, BG, 0);
  lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);
  lv_obj_remove_flag(scr, LV_OBJ_FLAG_SCROLLABLE);
  lv_label_set_text(text(scr, &lv_font_montserrat_20, TEXT, 24, 20, 0), "My UI");
  g_link = text(scr, &lv_font_montserrat_14, MUTED, 24, 50, 360);
  g_temp = text(scr, &lv_font_montserrat_48, TEXT, 24, 84, 360);
  g_values = text(scr, &lv_font_montserrat_14, MUTED, 24, 150, 360);
  lv_obj_set_style_text_line_space(g_values, 4, 0);
  g_profile = text(scr, &lv_font_montserrat_20, TEXT, 24, 250, 360);
  lv_label_set_long_mode(g_profile, LV_LABEL_LONG_MODE_DOTS);
  lv_obj_set_height(g_profile, lv_font_get_line_height(&lv_font_montserrat_20));
#if LV_USE_OBJ_NAME   // tags: names the shared UI checks find objects by (./checks.sh); they change nothing on screen
  lv_obj_set_name_static(g_temp, "value.boiler_temp");
  lv_obj_set_name_static(g_profile, "value.profile");
#endif
  g_shot = text(scr, &lv_font_montserrat_14, ACCENT, 24, 290, 360);
  g_answer = text(scr, &lv_font_montserrat_14, MUTED, 24, 330, 360);
  lv_label_set_text(text(scr, &lv_font_montserrat_14, MUTED, 416, 20, 360), "Profiles (tap to select)");
  g_list = lv_obj_create(scr);
  lv_obj_remove_style_all(g_list);
  lv_obj_set_pos(g_list, 416, 50);
  lv_obj_set_size(g_list, 360, 360);
  lv_obj_set_flex_flow(g_list, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_style_pad_row(g_list, 6, 0);
  lv_label_set_text(text(scr, &lv_font_montserrat_14, MUTED, 24, 440, 752),
                    "A UI on the Brewdeck core. Not made by or affiliated with the Gaggiuino project.");
  build_list();
}

void bd_ui_tick(uint32_t now) {
  static uint32_t seen_m = ~0u, seen_p = ~0u;
  if (bd_seq_machine() != seen_m) {                 // machine state changed: refresh the left column
    seen_m = bd_seq_machine();
    const bd_machine *m = bd_machine_state();
    static const char *LINK[] = {"Connecting...", "Machine not found", "Online"};
    lv_label_set_text(g_link, m->link <= BD_LINK_ONLINE ? LINK[m->link] : "?");   // later versions may add states
    char b[160];
    if (m->link == BD_LINK_ONLINE) {
      snprintf(b, sizeof b, "%.1f\xC2\xB0" "C", m->temperature);
      lv_label_set_text(g_temp, b);
      snprintf(b, sizeof b, "Target %.0f\xC2\xB0" "C\nPressure %.1f bar\nWater %d %%\nScale %s\nMode %s", m->target_temp,
               m->pressure, m->water_level, m->scale_present ? "connected" : "not connected", m->mode);
      lv_label_set_text(g_values, b);
    } else {
      lv_label_set_text(g_temp, "--");
      lv_label_set_text(g_values, "");
    }
    const char *name = m->running.name[0] ? m->running.name : m->profile_name;
    snprintf(b, sizeof b, "%s", name[0] ? name : "No profile yet");
    lv_label_set_text(g_profile, b);
  }
  if (bd_seq_profiles() != seen_p) { seen_p = bd_seq_profiles(); build_list(); }   // list or active profile changed
  bd_lvgl9_tick(now);                               // LVGL draws what changed, through bd_display_flush
}

void bd_ui_event(const bd_event *e) {
  char b[96];
  switch (e->type) {
    case BD_EV_SHOT_START: lv_label_set_text(g_shot, "Shot started"); break;
    case BD_EV_SHOT_POINT:                          // ~7 per second while a shot runs
      snprintf(b, sizeof b, "Shot %.1f s, %.1f g, %.1f bar", e->u.point->t / 10.0, e->u.point->w / 10.0,
               e->u.point->p / 10.0);
      lv_label_set_text(g_shot, b);
      break;
    case BD_EV_SHOT_END: {
      const bd_shot *s = bd_live_shot();
      if (!s) break;
      snprintf(b, sizeof b, "Shot done: %.1f g in %.1f s", s->n ? s->pt[s->n - 1].w / 10.0 : 0.0, s->dur10 / 10.0);
      lv_label_set_text(g_shot, b);
      break;
    }
    case BD_EV_COMMAND:                             // the machine's answer to bd_select_profile
      snprintf(b, sizeof b, "Machine: %s", e->u.command.answer == BD_ANSWER_OK ? "done" : "did not take it");
      lv_label_set_text(g_answer, b);
      break;
    default: break;                                 // notices, Wi-Fi, stored shots: not used here
  }
}

void bd_ui_wake(void) {}                            // no dimming or screen-off in this example
