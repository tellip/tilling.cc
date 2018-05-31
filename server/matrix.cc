#include "matrix.hh"

namespace wm {
	namespace matrix {
		Space::Space(Display *const &d) :
				display(d),
				normal_pixel(_colorPixel(d, config::normal_color)),
				focus_pixel(_colorPixel(d, config::normal_color)),
				xia_protocols(XInternAtom(d, "WM_PROTOCOLS", False)),
				xia_delete_window(XInternAtom(d, "WM_DELETE_WINDOW", False)) {
			if (xia_protocols == None || xia_delete_window == None) error("XInternAtom");

			_display_width = (unsigned int) XDisplayWidth(d, XDefaultScreen(d));
			_display_height = (unsigned int) XDisplayHeight(d, XDefaultScreen(d));
			_display_hv = HV(_display_width > _display_height);

			_root = _view = _active = NULL;
		}

		unsigned long Space::_colorPixel(Display *const &display, const char *const &cc) {
			auto cm = DefaultColormap(display, XDefaultScreen(display));
			XColor x_color;
			if (XAllocNamedColor(display, cm, cc, &x_color, &x_color) == 0) error("XAllocNamedColor");
			return x_color.pixel;
		}
	}
}