/****************************************************************************
 *   Aug 3 12:17:11 2020
 *   Copyright  2020  Dirk Brosswick
 *   Email: dirk.brosswick@googlemail.com
 ****************************************************************************/
 
/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
#include "config.h"
#include <TTGO.h>

#include "example_app.h"
#include "example_app_main.h"

#include "gui/mainbar/app_tile/app_tile.h"
#include "gui/mainbar/main_tile/main_tile.h"
#include "gui/mainbar/mainbar.h"
#include "gui/statusbar.h"

#include <HTTPClient.h>
#include <TFT_eFEX.h>
#include <FS.h>

lv_obj_t *example_app_main_tile = NULL;
lv_style_t example_app_main_style;

lv_task_t * _example_app_task;

LV_IMG_DECLARE(exit_32px);
LV_IMG_DECLARE(setup_32px);
LV_IMG_DECLARE(refresh_32px);
LV_FONT_DECLARE(Ubuntu_72px);

static void exit_example_app_main_event_cb( lv_obj_t * obj, lv_event_t event );
static void enter_example_app_setup_event_cb( lv_obj_t * obj, lv_event_t event );
static void refresh_camera_main_event_cb( lv_obj_t * obj, lv_event_t event );

TTGOClass *ttgo1 = TTGOClass::getWatch();
void loadImg();

void example_app_task( lv_task_t * task );

void example_app_main_setup( uint32_t tile_num ) {

    example_app_main_tile = mainbar_get_tile_obj( tile_num );
    lv_style_copy( &example_app_main_style, mainbar_get_style() );

    lv_obj_t * reload_btn = lv_imgbtn_create( example_app_main_tile, NULL);
    lv_imgbtn_set_src(reload_btn, LV_BTN_STATE_RELEASED, &refresh_32px);
    lv_imgbtn_set_src(reload_btn, LV_BTN_STATE_PRESSED, &refresh_32px);
    lv_imgbtn_set_src(reload_btn, LV_BTN_STATE_CHECKED_RELEASED, &refresh_32px);
    lv_imgbtn_set_src(reload_btn, LV_BTN_STATE_CHECKED_PRESSED, &refresh_32px);
    lv_obj_add_style(reload_btn, LV_IMGBTN_PART_MAIN, &example_app_main_style );
    lv_obj_align(reload_btn, example_app_main_tile, LV_ALIGN_IN_TOP_RIGHT, -10 , 10 );
    lv_obj_set_event_cb( reload_btn, refresh_camera_main_event_cb );

    lv_obj_t * exit_btn = lv_imgbtn_create( example_app_main_tile, NULL);
    lv_imgbtn_set_src(exit_btn, LV_BTN_STATE_RELEASED, &exit_32px);
    lv_imgbtn_set_src(exit_btn, LV_BTN_STATE_PRESSED, &exit_32px);
    lv_imgbtn_set_src(exit_btn, LV_BTN_STATE_CHECKED_RELEASED, &exit_32px);
    lv_imgbtn_set_src(exit_btn, LV_BTN_STATE_CHECKED_PRESSED, &exit_32px);
    lv_obj_add_style(exit_btn, LV_IMGBTN_PART_MAIN, &example_app_main_style );
    lv_obj_align(exit_btn, example_app_main_tile, LV_ALIGN_IN_BOTTOM_LEFT, 10, -10 );
    lv_obj_set_event_cb( exit_btn, exit_example_app_main_event_cb );

    lv_obj_t * setup_btn = lv_imgbtn_create( example_app_main_tile, NULL);
    lv_imgbtn_set_src(setup_btn, LV_BTN_STATE_RELEASED, &setup_32px);
    lv_imgbtn_set_src(setup_btn, LV_BTN_STATE_PRESSED, &setup_32px);
    lv_imgbtn_set_src(setup_btn, LV_BTN_STATE_CHECKED_RELEASED, &setup_32px);
    lv_imgbtn_set_src(setup_btn, LV_BTN_STATE_CHECKED_PRESSED, &setup_32px);
    lv_obj_add_style(setup_btn, LV_IMGBTN_PART_MAIN, &example_app_main_style );
    lv_obj_align(setup_btn, example_app_main_tile, LV_ALIGN_IN_BOTTOM_RIGHT, -10, -10 );
    lv_obj_set_event_cb( setup_btn, enter_example_app_setup_event_cb );

    // uncomment the following block of code to remove the "myapp" label in background
    lv_style_set_text_opa( &example_app_main_style, LV_OBJ_PART_MAIN, LV_OPA_70);
    lv_style_set_text_font( &example_app_main_style, LV_STATE_DEFAULT, &Ubuntu_72px);
    lv_obj_t *app_label = lv_label_create( example_app_main_tile, NULL);
    lv_label_set_text( app_label, "Camera");
    lv_obj_reset_style_list( app_label, LV_OBJ_PART_MAIN );
    lv_obj_add_style( app_label, LV_OBJ_PART_MAIN, &example_app_main_style );
    lv_obj_align( app_label, example_app_main_tile, LV_ALIGN_CENTER, 0, 0);
    
    // create an task that runs every secound
    _example_app_task = lv_task_create( example_app_task, 1000, LV_TASK_PRIO_MID, NULL );
}

static void refresh_camera_main_event_cb( lv_obj_t * obj, lv_event_t event ) {
  switch( event ) {
          case( LV_EVENT_CLICKED ):       loadImg();
          break;
  }
}

static void enter_example_app_setup_event_cb( lv_obj_t * obj, lv_event_t event ) {
    switch( event ) {
        case( LV_EVENT_CLICKED ):       statusbar_hide( true );
                                        mainbar_jump_to_tilenumber( example_app_get_app_setup_tile_num(), LV_ANIM_ON );
                                        break;
    }
}

static void exit_example_app_main_event_cb( lv_obj_t * obj, lv_event_t event ) {
    switch( event ) {
        case( LV_EVENT_CLICKED ):       mainbar_jump_to_maintile( LV_ANIM_OFF );
                                        break;
    }
}

void example_app_task( lv_task_t * task ) {
    // Viene eseguito di continuo
}

void loadImg() {
    HTTPClient http;
    TFT_eFEX fex = TFT_eFEX(ttgo1->tft);
    String url = "http://192.168.0.10:9999/ugo";
    String file_name = "/tmp.jpg";
    fs::File f = SPIFFS.open(file_name, "w");
    if (f) {
      http.begin(url);
      int httpCode = http.GET();
      Serial.printf("Get Camera...");
      if (httpCode > 0) {
        if (httpCode == 200) {
          http.writeToStream(&f);
          f.close();
          //http.end();
          fex.drawJpeg("/tmp.jpg", 0, 0);
        }
      } else {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
      }
    }
}