#include "basics.h"
#include "gdraw/ggdkdrawP.h"
#include "gdraw/gtkbridge.h"

#ifdef FONTFORGE_CAN_USE_GTK_BRIDGE

void gtkb_AddWindow(GWindow gw) {
	GGDKWindow ggdkw = (GGDKWindow) gw;
	GdkWindow *gdkw = (GdkWindow *) ggdkw->w;
	GGDKDisplay *gdisp = ggdkw->display;
	//printf("Pointers: %p %p\n", ggw, gdkw);
	printf("Pointers: %p\n", gdkw);
	_gtkb_AddWindow(gdisp->gtkb_state, gdkw);
}

#endif // FONTFORGE_CAN_USE_GTK_BRIDGE
