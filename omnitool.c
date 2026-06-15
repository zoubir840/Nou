#include <furi.h>
#include <furi_hal.h>
#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/submenu.h>
#include <gui/modules/popup.h>

// --- ENUMERATIONS ---
typedef enum {
    OmniViewSubmenuMain,
    OmniViewSubmenuNFC,
    OmniViewSubmenuSubGhz,
    OmniViewSubmenuHidden,
    OmniViewPopupWait,
} OmniView;

typedef enum {
    OmniEventMenuNFC,
    OmniEventMenuSubGhz,
    OmniEventMenuRFID,
    OmniEventMenuInfrared,
    OmniEventMenuHiddenPins,
    OmniEventNFCFuzzer,
    OmniEventNFCMifareDict,
    OmniEventSubGhzRawScan,
    OmniEventSubGhzCC1101Debug,
} OmniEvent;

// --- STRUCTURE DE L'APP ---
typedef struct {
    Gui* gui;
    ViewDispatcher* view_dispatcher;
    Submenu* submenu_main;
    Submenu* submenu_nfc;
    Submenu* submenu_subghz;
    Popup* popup;
} OmniApp;

// --- CALLBACKS ---
static uint32_t omni_navigation_exit_callback(void* context) {
    UNUSED(context);
    return VIEW_NONE; 
}

static uint32_t omni_navigation_back_to_main_callback(void* context) {
    UNUSED(context);
    return OmniViewSubmenuMain;
}

static void omni_submenu_callback(void* context, uint32_t index) {
    OmniApp* app = context;
    
    switch(index) {
        case OmniEventMenuNFC:
            view_dispatcher_switch_to_view(app->view_dispatcher, OmniViewSubmenuNFC);
            break;
        case OmniEventMenuSubGhz:
            view_dispatcher_switch_to_view(app->view_dispatcher, OmniViewSubmenuSubGhz);
            break;
        case OmniEventNFCFuzzer:
            popup_set_header(app->popup, "NFC Actif", 64, 10, AlignCenter, AlignTop);
            popup_set_text(app->popup, "Simulation de trames...", 64, 30, AlignCenter, AlignTop);
            view_dispatcher_switch_to_view(app->view_dispatcher, OmniViewPopupWait);
            break;
        case OmniEventSubGhzCC1101Debug:
            popup_set_header(app->popup, "SubGhz Debug", 64, 10, AlignCenter, AlignTop);
            popup_set_text(app->popup, "Puce CC1101 Isolee", 64, 30, AlignCenter, AlignTop);
            view_dispatcher_switch_to_view(app->view_dispatcher, OmniViewPopupWait);
            break;
        case OmniEventMenuHiddenPins:
            popup_set_header(app->popup, "GPIO Access", 64, 10, AlignCenter, AlignTop);
            popup_set_text(app->popup, "Broches en mode OUTPUT", 64, 30, AlignCenter, AlignTop);
            view_dispatcher_switch_to_view(app->view_dispatcher, OmniViewPopupWait);
            break;
    }
}

// --- ALLOCATION ---
static OmniApp* omni_app_alloc() {
    OmniApp* app = malloc(sizeof(OmniApp));

    app->view_dispatcher = view_dispatcher_alloc();
    app->submenu_main = submenu_alloc();
    app->submenu_nfc = submenu_alloc();
    app->submenu_subghz = submenu_alloc();
    app->popup = popup_alloc();

    submenu_add_item(app->submenu_main, "1. Ultimate NFC", OmniEventMenuNFC, omni_submenu_callback, app);
    submenu_add_item(app->submenu_main, "2. Sub-GHz & CC1101", OmniEventMenuSubGhz, omni_submenu_callback, app);
    submenu_add_item(app->submenu_main, "3. GPIO & Modules", OmniEventMenuHiddenPins, omni_submenu_callback, app);

    submenu_add_item(app->submenu_nfc, "NFC Operations", OmniEventNFCFuzzer, omni_submenu_callback, app);
    submenu_add_item(app->submenu_subghz, "CC1101 Debug Registers", OmniEventSubGhzCC1101Debug, omni_submenu_callback, app);

    view_dispatcher_add_view(app->view_dispatcher, OmniViewSubmenuMain, submenu_get_view(app->submenu_main));
    view_dispatcher_add_view(app->view_dispatcher, OmniViewSubmenuNFC, submenu_get_view(app->submenu_nfc));
    view_dispatcher_add_view(app->view_dispatcher, OmniViewSubmenuSubGhz, submenu_get_view(app->submenu_subghz));
    view_dispatcher_add_view(app->view_dispatcher, OmniViewPopupWait, popup_get_view(app->popup));

    view_dispatcher_set_navigation_event_callback(app->view_dispatcher, omni_navigation_exit_callback);
    view_set_previous_callback(submenu_get_view(app->submenu_nfc), omni_navigation_back_to_main_callback);
    view_set_previous_callback(submenu_get_view(app->submenu_subghz), omni_navigation_back_to_main_callback);
    view_set_previous_callback(popup_get_view(app->popup), omni_navigation_back_to_main_callback);

    return app;
}

// --- LIBERATION MEMOIRE ---
static void omni_app_free(OmniApp* app) {
    view_dispatcher_remove_view(app->view_dispatcher, OmniViewSubmenuMain);
    view_dispatcher_remove_view(app->view_dispatcher, OmniViewSubmenuNFC);
    view_dispatcher_remove_view(app->view_dispatcher, OmniViewSubmenuSubGhz);
    view_dispatcher_remove_view(app->view_dispatcher, OmniViewPopupWait);
    
    submenu_free(app->submenu_main);
    submenu_free(app->submenu_nfc);
    submenu_free(app->submenu_subghz);
    popup_free(app->popup);
    view_dispatcher_free(app->view_dispatcher);
    
    free(app);
}

// --- MAIN ---
int32_t omni_app(void* p) {
    UNUSED(p);
    OmniApp* app = omni_app_alloc();

    app->gui = furi_record_open(RECORD_GUI);
    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);
    
    view_dispatcher_switch_to_view(app->view_dispatcher, OmniViewSubmenuMain);
    view_dispatcher_run(app->view_dispatcher);

    omni_app_free(app);
    furi_record_close(RECORD_GUI);

    return 0;
}
