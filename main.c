#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xft/Xft.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <cairo.h>
#include <cairo-xlib.h>
#include <math.h>

int max_len(const char* options[], int num_options)
{
	int max = 0;
	for (int i = 0; i < num_options; i++)
	{
		if (strlen(options[i]) > max)
		{
			max = strlen(options[i]);
		}
	}

	return max;
}


int min_len(const char* options[], int num_options)
{
	int min = INT_MAX;
	for (int i = 0; i < num_options; i++)
	{
		if (strlen(options[i]) < min)
		{
			min = strlen(options[i]);
		}
	}

	return min;
}

int get_clicked_option(int x, int y, int start_y, int spacing, int num_options, int width, int offset)
{
	for (int i = 0; i < num_options; i++)
	{
		int option_y = start_y + i * spacing;
		int text_x = width / 2 - offset;
		int text_y = option_y;

		if (x >= text_x && x <= text_x + 200 && y >= text_y - 20 && y <= text_y + 10)
		{
			return i;
		}
	}

	return -1;
}

void draw_rounded_rectangle(cairo_t* cr, double x, double y, double width, double height, double radius)
{
	double degrees = M_PI / 180.0;
	cairo_new_sub_path(cr);
    cairo_arc(cr, x + width - radius, y + radius, radius, -90 * degrees, 0 * degrees);
    cairo_arc(cr, x + width - radius, y + height - radius, radius, 0 * degrees, 90 * degrees);
    cairo_arc(cr, x + radius, y + height - radius, radius, 90 * degrees, 180 * degrees);
    cairo_arc(cr, x + radius, y + radius, radius, 180 * degrees, 270 * degrees);
    cairo_close_path(cr);
}

void draw_options(Display* display, Window w, XftDraw* xftDraw, XftFont* font,
                  const char* options[], int num_options, int hovered,
                  int width, int start_y, int spacing, int offset,
                  XftColor* color, XftColor* hover_color, XColor* hover_bg, cairo_t* cr)
{
    for (int i = 0; i < num_options; i++)
    {
        int text_x = width / 2 - offset;
        int text_y = start_y + i * spacing;
        int text_width = 2 * offset;
        int text_height = font->ascent + font->descent;
        int padding_x = 20;
        int padding_y = 15;

        if (i == hovered)
        {
            //XSetForeground(display, DefaultGC(display, 0), hover_bg->pixel);
            //XFillRectangle(display, w, DefaultGC(display, 0),
            //    text_x - padding_x,
            //    text_y - font->ascent - padding_y,
            //    text_width + 2 * padding_x,
            //    text_height + 2 * padding_y);

			double rect_x = text_x - padding_x;
			double rect_y = text_y - font->ascent - padding_y;
			double rect_width = text_width +  2 * padding_x;
			double rect_height = text_height + 2 * padding_y;
			double corner_radius = 20.0;


			cairo_set_source_rgb(cr, 
						hover_bg->red / 65535.0,
						hover_bg->green / 65535.0,
						hover_bg->blue / 65535.0);

			draw_rounded_rectangle(cr, rect_x, rect_y, rect_width, rect_height, corner_radius);
			cairo_fill(cr);

        }

        XftColor* draw_color = (i == hovered) ? hover_color : color;
        XftDrawStringUtf8(
			xftDraw, 
			draw_color, 
			font,
            text_x, 
			text_y,
            (XftChar8*)options[i], 
			strlen(options[i]));
    }
}

void execute_command(int clicked, const char* options[])
{
	printf("Clicked: %s\n", options[clicked]);
	printf("%d\n", clicked);
	fflush(stdout);

	if (clicked == 0)
	{
		system("i3-msg exit");
	}
	if (clicked == 1)
	{
		system("systemctl suspend");
	}
	if (clicked == 2)
	{
		system("systemctl reboot");
	}
	if (clicked == 3)
	{
		system("systemctl poweroff");
	}
}


int main()
{
	const char* options[] = { "   Logout", "   Suspend", "   Reboot", "   Shutdown" };
	int num_options = 4;
	int start_y = 100;
	int spacing = 90;

	int x, y = 50;

	int width = 800;
	int height = 600;

	int hovered = -1;

	XEvent event;
	Display* display = XOpenDisplay(NULL);
	if (display == NULL)
	{
		fprintf(stderr, "Cannot open display\n");
		return 1;
	}

	XColor window_bg_color, ext;
	Colormap col_map = DefaultColormap(display, DefaultScreen(display));
	XAllocNamedColor(display, col_map, "#2e2e2e", &window_bg_color, &ext);

	Window w = XCreateSimpleWindow(
		display,
		DefaultRootWindow(display),
		50,50,
		width,height,
		0,
		BlackPixel(display,DefaultScreen(display)),
		window_bg_color.pixel);
		//WhitePixel(display, 0));

	cairo_surface_t* cairo_surface = cairo_xlib_surface_create(
		display, w, DefaultVisual(display, 0), width, height);
	cairo_t* cr = cairo_create(cairo_surface);

	XStoreName(display, w, "Power Menu");

	XClassHint *classHint = XAllocClassHint();
	classHint->res_name = "powermenu";
	classHint->res_class = "PowerMenu";
	XSetClassHint(display, w, classHint);
	XFree(classHint);



	XMapWindow(display, w);
	XSelectInput(display, w, ExposureMask | ButtonPressMask | PointerMotionMask | KeyPressMask);

	XftDraw* xftDraw = NULL;
	XftFont* font = XftFontOpenName(
		display,
		DefaultScreen(display), 
		"FiraCode Nerd Font-26"
	);
	XftColor color;
	Colormap colormap = DefaultColormap(display, DefaultScreen(display));
	XftColorAllocName(display, DefaultVisual(display, DefaultScreen(display)), colormap, "#9eDFFF", &color);

	XftColor hover_color;
	XftColorAllocName(display, DefaultVisual(display, DefaultScreen(display)), colormap, "#9eDFFF", &hover_color);
	
	XColor hover_bg, exact;
	XAllocNamedColor(display, colormap, "#2e2e69", &hover_bg, &exact);





	xftDraw = XftDrawCreate(
		display, 
		w, 
		DefaultVisual(display, DefaultScreen(display)), 
		DefaultColormap(display, DefaultScreen(display)));
	int offset = 28 * max_len(options, num_options) / 2;

	for (;;) {
		XNextEvent(display, &event);
		if (event.type == Expose)
		{
			draw_options(display, w, xftDraw, font, options, num_options, hovered, width, start_y, spacing, offset, &color, &hover_color, &hover_bg, cr);
		}

		else if (event.type == ButtonPress)
		{
			int clicked = get_clicked_option(event.xbutton.x, 
								event.xbutton.y,
								start_y, 
								spacing, 
								num_options, 
								width, 
								offset);

			if (clicked != -1)
			{
				execute_command(clicked, options);
			}
		}

		else if (event.type == MotionNotify)
		{
			int x = event.xmotion.x;
			int y = event.xmotion.y;
			int new_hovered = get_clicked_option(
								x, 
								y,
								start_y, 
								spacing, 
								num_options, 
								width, 
								offset);
			if (new_hovered != hovered)
			{
				hovered = new_hovered;
				XClearWindow(display, w);

				draw_options(display, w, xftDraw, font, options, num_options, hovered, width, start_y, spacing, offset, &color, &hover_color, &hover_bg, cr);

			}
		}
		else if (event.type == KeyPress)
		{
			KeySym key = XLookupKeysym(&event.xkey, 0);
			if (key == XK_Down || key == XK_j)
			{
				hovered = (hovered >= num_options - 1) ? 0 : hovered + 1;
				XClearWindow(display, w);
				draw_options(display, w, xftDraw, font, options, num_options, hovered, width, start_y, spacing, offset, &color, &hover_color, &hover_bg, cr);
				 
			}
			else if (key == XK_Up || key == XK_k)
			{
				hovered = (hovered <= 0) ? num_options - 1 : hovered - 1;
				XClearWindow(display, w);
				draw_options(display, w, xftDraw, font, options, num_options, hovered, width, start_y, spacing, offset, &color, &hover_color, &hover_bg, cr);
			}

			else if (key == XK_Return || key == XK_KP_Enter)
			{
				if (hovered != -1)
				{
					printf("Selected: %s\n", options[hovered]);
					printf("Am I here?");
					fflush(stdout);
					execute_command(hovered, options);
				}
			}
		}
	}
	XftFontClose(display, font);
	XftDrawDestroy(xftDraw);

	return 0;
}
