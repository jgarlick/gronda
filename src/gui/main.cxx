/*  Gronda Text Editor
 *  Copyright (C) 2002-2012 James Garlick and Graeme Jefferis
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of version 2 of the GNU General Public License as 
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 *  main.cxx
 *  purpose: GUI front end using FLTK
 *  authors : James Garlick
 */

#include <math.h>
#include <string.h>

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Box.H>
#include <FL/fl_ask.H>

#include <FL/names.h>

/* for mouse position warping */
#include <FL/x.H>
#if defined(__APPLE__)
#include <ApplicationServices/ApplicationServices.h>
#endif

extern "C" {
#include "../include/editor.h"
}

Fl_Color bg_color, line_color, text_color, line_num_color, line_num_line_color, titlebar_color;

int font;
int font_size;

bool cursor_follows_mouse = true;
int show_line_numbers = 1;

int digits(int number) {
	int digits = 1;
	int step = 10;
	while (step <= number) {
		digits++;
		step *= 10;
	}
	return digits;
}

int warp_mouse_pointer (int new_x, int new_y) {
#if defined WIN32
      BOOL result = SetCursorPos(new_x, new_y);
      if (result) return 0; // OK

#elif defined __APPLE__
      CGPoint new_pos;
      CGEventErr err;
      new_pos.x = new_x;
      new_pos.y = new_y;
      err = CGWarpMouseCursorPosition(new_pos);
      if (!err) return 0; // OK

#else // Assume this is Xlib
      Window rootwindow = DefaultRootWindow(fl_display);
      XWarpPointer(fl_display, rootwindow, rootwindow, 0, 0, 0, 0,
new_x, new_y);
      return 0; // Assume OK
#endif

      return -1; // Fail
}

class EditViewport : public Fl_Widget {
public:
	int viewport_w, viewport_h, line_h, line_num_columns;
	
	int pad_start_x, pad_start_y;  // the position in the widget of the start of the pad text

	void set_viewport_size(int W, int H) {
		pad_t *pad;
		
		line_h     = fl_height() + 1;
		viewport_h = floor((H - 10) / line_h) - 1;
		viewport_w = floor((W - (6 + line_num_width())) / fl_width(' '));

		pad_start_x = x() + 3 + line_num_width();

		// 13 = 8 for title bar + 5 spacing below title bar
		pad_start_y = y() + 13 + (fl_height() * 2 - fl_descent());

		pad = e->pad_head;
		while(pad != NULL) {
			pad_set_viewport_size(pad, viewport_w, viewport_h);
			pad = pad->next;
		}
	}
	
protected:
	int line_num_width() {
		line_num_columns = (e->cepad == NULL ? 4 : digits(e->cepad->offset_y + viewport_h));
		if (line_num_columns < 4) line_num_columns = 4;

		return show_line_numbers ? (fl_width(' ') * line_num_columns) + 7 : 0;
	}
	
	void resize(int X, int Y, int W, int H) {
		set_viewport_size(W, H);
	  	Fl_Widget::resize(X,Y,W,H);
	}

	void draw() {
		char buf[80];
		char line_buffer[512];
		int yp;
		int i;
		line_t *line;
		char *str;
		int line_end, end;
		int offset, intab;
		char empty_string[1] = "";
		pad_t *pad;
		int line_num;

		int start_y = 0;
		int start_x = 0;
		int end_y   = 0;
		int end_x   = 0;

		pad = e->cepad;

		if (pad->echo) {
			get_region (pad->echo, &start_y, &start_x, &end_y, &end_x);

			start_y -= pad->offset_y + 1;
			start_x -= pad->offset_x + 1;
			end_y   -= pad->offset_y + 1;
			end_x   -= pad->offset_x + 1;
		}

		fl_font(font, font_size);

		/* background */
		fl_color(bg_color);
		fl_rectf(x(), y(), w(), h());
		
		/* top bar */
		fl_color(titlebar_color);
		fl_rectf(x() + 2, y() + 2, w() - 4, fl_height() + 6 );
		
		fl_color(FL_WHITE);
		fl_draw((pad->filename ? pad->filename : "pad0001"), x() + 6, y() + 5 + fl_height() - fl_descent());

		sprintf(buf, "%d", pad->offset_y + 1);
		fl_draw(buf, x() + w() - (7 + fl_width(' ') * strlen(buf)), y() + 5 + fl_height() - fl_descent());

		if (pad->offset_x > 0) {
			sprintf(buf, "%d", pad->offset_x + 1);
			fl_draw(buf, x() + w() - (130 + fl_width(' ') * strlen(buf)), y() + 5 + fl_height() - fl_descent());			
		}

		fl_color(bg_color);
		fl_rectf(x() + w() - 112, y() + 5, fl_width(' ') + 8, fl_height());
		fl_color(text_color);
		if (!(pad->flags & FILE_WRITE))
			sprintf(buf, "R");
		else if (e->flags & INSERT)
			sprintf(buf, "I");
		else
			sprintf(buf, "O");
		fl_draw(buf, x() + w() - 108, y() + 5 + fl_height() - fl_descent());

		if (pad->flags & MODIFIED) {
			fl_color(bg_color);
			fl_rectf(x() + w() - 82, y() + 5, fl_width(' ') + 8, fl_height());
			fl_color(text_color);
			fl_draw("M", x() + w() - 78, y() + 5 + fl_height() - fl_descent());
		}

		/* line num decoration */
		if (show_line_numbers) {
			fl_color(line_num_line_color);
			fl_line(line_num_width(), fl_height() + 8, line_num_width(), h());
		}
		
		/* draw the lines of text in the viewport window */
		yp = pad_start_y;
		line_num = 1 + pad->offset_y;
		line = LINE_get_line_at (pad, line_num);
		if (line == NULL)
			line = pad->line_head;
			
		for (i = 0; i < viewport_h; i++) {			
			intab  = 0;
			
			if (line == pad->line_head)
			{
				str = empty_string;
			}
				else if (line == NULL || line->str == NULL)
			{
				str = empty_string;
			}
			else
			{
				offset = get_string_pos (pad->offset_x + 1, line->str->data, &intab);
				if (intab)     // if we are in a tab the offset will be from the start of the tab
					offset++;  // but this is not what we want here
				
				if ((size_t)offset < strlen (line->str->data))
					str = line->str->data + offset;
				else
					str = empty_string;
			}
			
			/* selection highlight background for line */
			fl_color(titlebar_color);
			if (pad->echo == REGION_LINEAR) {
				// get the position that the cursor would be in at the end of the line
				// taking hard tabs into account
				line_end = get_curs_pos(strlen(str), line);
				
				if (i == start_y) {
					if (start_x < line_end) {
						if (start_y == end_y)
							end = (line_end < end_x) ? line_end : end_x;
						else
							end = line_end;
						
						fl_rectf(pad_start_x + (fl_width(' ') * start_x), pad_start_y + (line_h * (start_y - 1)) + fl_descent() + 1, fl_width(' ') * (end - start_x), fl_height());
					}
				} else if (i > start_y && i < end_y) {
					fl_rectf(pad_start_x, pad_start_y + (line_h * (i - 1)) + fl_descent() + 1, fl_width(' ') * line_end, fl_height());
				} else if (i == end_y && end_x > 0) {
					end = (line_end < end_x) ? line_end : end_x;
					fl_rectf(pad_start_x, pad_start_y + (line_h * (i - 1)) + fl_descent() + 1, fl_width(' ') * end, fl_height());
				}
			} else if(pad->echo == REGION_RECT && (i >= start_y && i <= end_y)) {
				fl_rectf(pad_start_x + (fl_width(' ') * start_x), pad_start_y + (line_h * (i - 1)) + fl_descent() + 1, fl_width(' ') * (end_x - start_x), fl_height());
			}

			/* draw line number first */
			if (show_line_numbers && line != pad->line_head) {
//				fl_font(font, font_size - 1);
				fl_color(line_num_color);
				sprintf(line_buffer, "%*d", line_num_columns, line_num);
				fl_draw(line_buffer, 3, yp);
			}

			/* draw line text */
			fl_font(font, font_size);
			fl_color(text_color);
			get_string_for_viewport(str, intab, viewport_w, line_buffer);
			fl_draw(line_buffer, pad_start_x, yp);
			
			yp += line_h;
			
			if (line != pad->line_head) {
				line = line->next;
				line_num++;
			}
		}
		
		/* cursor */
		if (e->occupied_window == EDIT_WINDOW) {
			fl_color(titlebar_color);
			if (pad->echo && !(start_x == end_x && start_y == end_y)) {
				fl_rect(pad_start_x + (fl_width(' ') * (pad->curs_x - 1)), pad_start_y + (line_h * (pad->curs_y - 2)) + fl_descent() + 1, fl_width(' '), fl_height() );
			} else {
				fl_rectf(pad_start_x + (fl_width(' ') * (pad->curs_x - 1)), pad_start_y + (line_h * (pad->curs_y - 2)) + fl_descent() + 1, fl_width(' '), fl_height() );
				fl_color(bg_color);
				sprintf(buf, "%c", pad_get_char_at(pad, pad->curs_x + pad->offset_x, pad->curs_y + pad->offset_y));
				fl_draw(buf, pad_start_x + (fl_width(' ') * (pad->curs_x - 1)), pad_start_y + (line_h * (pad->curs_y - 1)));
			}
		}
	}
public:
    EditViewport(int X, int Y, int W, int H, const char *l=0)
	: Fl_Widget(X,Y,W,H,l) {
		set_viewport_size(W, H);
	}
	
	void move_cursor(int X, int Y) {
		int new_x, new_y;
		
		new_x = floor((X + 3 - line_num_width()) / fl_width(' '));
		new_y = floor((Y) / line_h) - 1;
	
		if (new_x > 0 && new_x <= viewport_w && new_y > 0 && new_y <= viewport_h) {
			if (e->occupied_window != EDIT_WINDOW) {
				e->occupied_window = EDIT_WINDOW;
				e->cpad = e->cepad;
			}
			e->cpad->curs_x = new_x;
			e->cpad->curs_y = new_y;
		}
	}
	void get_cursor_position(int *pos_x, int *pos_y) {
		*pos_x = pad_start_x + (fl_width(' ') * (e->cepad->curs_x - 1));
		*pos_y = pad_start_y + (line_h * (e->cepad->curs_y - 2)) + fl_descent() + 1;
	}

	int in_edit_window(int X, int Y) {
		return (X > (3 + line_num_width()) && X < w() - 3 && Y > (6 + fl_height()) && Y < (6 + ((viewport_h + 1) * fl_height()) + fl_descent()));
	}
};

EditViewport   *edit_viewport;

void close_window(Fl_Widget* win, void* v) {
	e->cpad = e->cepad;
	cmd_wc(0, 0);
	win->redraw();
}

class MyWindow : public Fl_Double_Window {
	int mouse_x, mouse_y;

	int handle(int);

	void set_mouse_cursor(int X, int Y) {
		if (cursor_follows_mouse && edit_viewport->in_edit_window(X, Y)) {
			fl_cursor(FL_CURSOR_NONE);
		} else {
			fl_cursor(FL_CURSOR_DEFAULT);
		}
	}

public:
	MyWindow(int w, int h, const char *t=0L) 
		: Fl_Double_Window( w, h, t ) { }
};

class InputViewport : public Fl_Widget {
	int viewport_w;
protected:
	void set_viewport_size(int W, int H) {
		viewport_w = floor((W - (fl_width(e->cepad->prompt->data) + 6)) / fl_width(' '));
		pad_set_viewport_size(e->input_pad, viewport_w, 1);
	}

	void resize(int X, int Y, int W, int H) {
		set_viewport_size(W, H);
	  	Fl_Widget::resize(X,Y,W,H);
	}
	
	void draw() {
		char buffer[512];
		line_t *line;
		pad_t  *pad = e->input_pad;
		char *str;
		int offset, intab;
		char *prompt = e->cepad->prompt->data;
		
		fl_font(font, font_size);
		
		fl_color(bg_color);
		fl_rectf(x(), y(), w(), h());
		fl_color(line_color);
//		fl_rect(x(), y(), w(), h());
		fl_line(x(), y(), x() + w(), y());
		
		fl_color(text_color);
		fl_draw(prompt, x() + 3, y() + 4 + fl_height() - fl_descent());

		line = pad->line_head->prev;
		if (line != pad->line_head && line->str && line->str->data) {
			offset = get_string_pos (pad->offset_x + 1, line->str->data, &intab);
			if (intab) offset++;

			if ((size_t)offset < strlen (line->str->data))
				str = line->str->data + offset;
			else
				str = NULL;

			if (str) {
				get_string_for_viewport(str, intab, viewport_w, buffer);
				fl_draw(buffer, x() + 3 + fl_width(prompt), y() + 4 + fl_height() - fl_descent());
			}
		}

		if(e->occupied_window == COMMAND_WINDOW) {
			/* cursor */
			fl_color(titlebar_color);
			fl_rectf(x() + 3 + fl_width(prompt) + (fl_width(' ') * (pad->curs_x - 1)), y() + 4, fl_width(' '), fl_height() );
		}
	}
public:
    InputViewport(int X, int Y, int W, int H, const char *l=0)
	: Fl_Widget(X,Y,W,H,l) {
		set_viewport_size(W, H);
	}

	void get_cursor_position(int *pos_x, int *pos_y) {
		*pos_x = x() + 3 + fl_width(e->cepad->prompt->data) + (fl_width(' ') * (e->input_pad->curs_x - 1));
		*pos_y = y() + 4;
	}
};

class OutputViewport : public Fl_Widget {
protected:
	void draw() {
		char buf[80];
		line_t *line;
		
		fl_font(font, font_size);
		
		fl_color(bg_color);
		fl_rectf(x(), y(), w(), h() );
		fl_color(line_color);
//		fl_rect(x(), y(), w(), h() );
		fl_line(x(), y(), x() + w(), y());
		fl_line(x(), y(), x(), y() + h());
		
		line  = e->output_pad->line_head->prev;
		
		if (line->str && line->str->data) {
			fl_color(text_color);
			fl_draw(line->str->data, x() + 3, y() + 4 + fl_height() - fl_descent());
		}
	}
public:
    OutputViewport(int X, int Y, int W, int H, const char *l=0)
	: Fl_Widget(X,Y,W,H,l) {
	}
};


MyWindow       *window;
OutputViewport *output_viewport;
InputViewport  *input_viewport;

struct keycode_table{int n; const char* text;} table[] = {
  {FL_Escape, "esc"},
  {FL_BackSpace, "bs"},
  {FL_Tab, "tab"},
  {FL_Enter, "enter"},
  {FL_Print, "FL_Print"},
  {FL_Scroll_Lock, "FL_Scroll_Lock"},
  {FL_Pause, "FL_Pause"},
  {FL_Insert, "ins"},
  {FL_Home, "home"},
  {FL_Page_Up, "pgup"},
  {FL_Delete, "del"},
  {FL_End, "end"},
  {FL_Page_Down, "pgdown"},
  {FL_Left, "left"},
  {FL_Up, "up"},
  {FL_Right, "right"},
  {FL_Down, "down"},
  {FL_Caps_Lock, "FL_Caps_Lock"},
//  {FL_Menu, "FL_Menu"},
//  {FL_Help, "FL_Help"},
//  {FL_Num_Lock, "FL_Num_Lock"},
//  {FL_KP_Enter, "FL_KP_Enter"}
};

int MyWindow::handle(int event) {
	char buffer1[100];
	char *buffer;
	keydef_t *keydef;
	int k, c, mods;
	int len;
	buffer_t *paste_buffer;
	int old_x, old_y, curs_x, curs_y;

	if (event == FL_FOCUS) {
		Fl::paste((Fl_Widget &)*this, 1);
		return 1;
	}
	if (event == FL_ENTER) {
		return 1;
	}
	if (event == FL_PASTE) {
		paste_buffer = buffer_find((char *)"clipboard");
		if (paste_buffer) {
			string_truncate (paste_buffer->str, 0);
			string_append(paste_buffer->str, "%s", (char *)Fl::event_text());
		}
		return 1;
	}

	// start 5 characters in so that we can add characters to the front of the string
	buffer = buffer1 + 5;

	buffer[0] = '\0';
	mods = Fl::event_state();
	
	if (event== FL_KEYDOWN || event== FL_KEYUP) {
	    k = Fl::event_key();

		if (k > 0 && k < 256) {
			if ((mods & FL_SHIFT) && !(mods & (FL_CTRL|FL_ALT|FL_META))) { // shift only, no other modifiers
				k = Fl::event_text()[0]; // text for the key, handles shift+standard keypress
			}
			if (k == '\'') {
				sprintf(buffer, "squote");
			} else if (k == '"') {
				sprintf(buffer, "dquote");
			} else {
      			sprintf(buffer, "%c", k);
			}
	    } else if (k > FL_F && k <= FL_F_Last) {
	      sprintf(buffer, "F%d", k - FL_F);
	    } else if (k >= FL_KP && k <= FL_KP_Last) {
	      sprintf(buffer, "FL_KP+'%c'", k-FL_KP);
	    } else if (k >= FL_Button && k <= FL_Button+7) {
	      sprintf(buffer, "FL_Button+%d", k-FL_Button);
	    } else {
	      for (int i = 0; i < int(sizeof(table)/sizeof(*table)); i++)
			if (table[i].n == k) { strcpy(buffer, table[i].text); break;}
		}
	} else if (event== FL_PUSH || event== FL_RELEASE) {
		sprintf(buffer, "M%d", Fl::event_button());
	}

	if (event== FL_MOVE || event== FL_DRAG) {
		mouse_x = Fl::event_x();
		mouse_y = Fl::event_y();
		old_x = e->cpad->curs_x;
		old_y = e->cpad->curs_y;
		if (cursor_follows_mouse)
			edit_viewport->move_cursor(mouse_x, mouse_y);

//		set_mouse_cursor(mouse_x, mouse_y); // use this to hide the mouse pointer all the time when in a pad
		fl_cursor(FL_CURSOR_DEFAULT);

		if(!edit_viewport->in_edit_window(mouse_x, mouse_y) || (e->cpad->curs_x != old_x) || (e->cpad->curs_y != old_y))
			edit_viewport->redraw();
	}

	len = strlen(buffer);
	if (len > 0) {
		// don't add S to regular keys e.g. shift+a, but add them when another modifier key
		// is also pressed e.g. shift+ctrl+a generates ^aS
		if ((mods & FL_SHIFT) && (event== FL_PUSH || event== FL_RELEASE || k > 255 || (mods & (FL_CTRL|FL_ALT|FL_META)))) {
			buffer[len]     = 'S';
			buffer[len + 1] = '\0';
		}
		if (mods & FL_META) {
			buffer--;
			*buffer = '~';
		}
		if (mods & FL_ALT) {
			buffer--;
			*buffer = '*';
		}
		if (mods & FL_CTRL) {
			buffer--;
			*buffer = '^';
		}

		if (event== FL_RELEASE || event== FL_KEYUP) {
			len = strlen(buffer);
			buffer[len]     = 'U';
			buffer[len + 1] = '\0';
		}

//		printf("key find %s\n", buffer);
		keydef = KEY_find (buffer);

		if (keydef) {
			parse ("%s", keydef->def);

			if (cursor_follows_mouse) {
				fl_cursor(FL_CURSOR_NONE); // hide mouse pointer until it is next moved
				curs_x = curs_y = 0;
				if (e->occupied_window == EDIT_WINDOW) {
					edit_viewport->get_cursor_position(&curs_x, &curs_y);
				} else if (e->occupied_window == COMMAND_WINDOW) {
					input_viewport->get_cursor_position(&curs_x, &curs_y);
				}
				// move the mouse pointer to the cursor position
				if(curs_x && curs_y) {
					if(mouse_x < curs_x || mouse_x > (curs_x + fl_width(' ')) || mouse_y < curs_y || mouse_y > (curs_y + fl_height())) {
						mouse_x = curs_x + (fl_width(' ') / 2);
						mouse_y = curs_y + (fl_height() / 2);
						warp_mouse_pointer(x() + mouse_x, y() + mouse_y);
					}
				}
			} else {
				fl_cursor(FL_CURSOR_DEFAULT);
			}
		}

		edit_viewport->redraw();
		input_viewport->redraw();
	}
	output_viewport->redraw();
	
	return (1); // eat all keystrokes
}

extern "C" void cmd_follow (int argc, char *argv[]) {
	if (argc > 1)
	{
		if (strcmp (argv[1], "-on") == 0)
			cursor_follows_mouse = true;
		else if (strcmp (argv[1], "-off") == 0)
			cursor_follows_mouse = false;
	}
	else
		cursor_follows_mouse = !cursor_follows_mouse;
}

extern "C" void cmd_tm (int argc, char *argv[]) {
	edit_viewport->move_cursor(Fl::event_x(), Fl::event_y());
}

extern "C" void cmd_lineno (int argc, char *argv[]) {
	if (argc > 1)
	{
		if (strcmp (argv[1], "-on") == 0)
			show_line_numbers = 1;
		else if (strcmp (argv[1], "-off") == 0)
			show_line_numbers = 0;
	}
	else
		show_line_numbers = !show_line_numbers;
		
	edit_viewport->set_viewport_size(edit_viewport->w(), edit_viewport->h());
}

extern "C" void cmd_icon (int argc, char *argv[]) {
	window->iconize();
}

/* display callbacks */
void display_copy(buffer_t *buf) {
	if (!strcmp(buf->name, "clipboard")) {
		Fl::copy(buf->str->data, strlen(buf->str->data), 1);
	}
}

void display_close () { }

void display_filename(const char *filename) {
	if(window) window->label(filename);
}

void display_beep () { fl_beep(); }

void display_do_menu (menu_t * menu) { }
void display_finish_menu () { }


int main(int argc, char **argv) {
	int font_height;
	int font_width;
	int bottom_bar_height;
	int i,k,t;
	const char *name;

#if defined(__APPLE__)
	font = FL_SCREEN;
	font_size = 14;
#else
	font = FL_COURIER;
	font_size = 14;
#endif

	k = Fl::set_fonts("");
	for (i = 0; i < k; i++) {
		name = Fl::get_font_name((Fl_Font)i,&t);
//		printf("%s\n", name);
		/* use DM font if installed */
/*		if(!strcasecmp(name, "Apollo DM")) {
			font = i;
			font_size = 14;
		}*/
		if(!strcasecmp(name, "Source Code Pro")) {
			font = i;
			font_size = 12;
		}
	}

	editor_setup(argc, argv);
	add_command ("follow", (void (*)())cmd_follow);
	add_command ("tm",     (void (*)())cmd_tm);
	add_command ("lineno", (void (*)())cmd_lineno);
	add_command ("icon",   (void (*)())cmd_icon);

	// Aegis Blue
//	bg_color       = fl_rgb_color(255, 255, 230);
//	titlebar_color = fl_rgb_color(60, 60, 200);
	// Aegis Red
//	bg_color       = fl_rgb_color(255, 240, 180);
//	titlebar_color = fl_rgb_color(210, 20, 0);
	// Aegis Green
//	bg_color       = fl_rgb_color(240, 255, 190);
//	titlebar_color = fl_rgb_color(30, 155, 0);
	// Aegis Brown
//	bg_color       = fl_rgb_color(240, 240, 170);
//	titlebar_color = fl_rgb_color(150, 90, 0);

//	titlebar_color = fl_rgb_color(203, 9, 131); // pink
//	titlebar_color = fl_rgb_color(9, 178, 148); // teal

	bg_color       = fl_rgb_color(248, 245, 235);
	titlebar_color = fl_rgb_color(60, 60, 200);

//	line_color     = fl_rgb_color(150, 150, 150);
	line_color = titlebar_color;

	text_color          = fl_rgb_color(30, 30, 30);
	line_num_color      = fl_rgb_color(140, 140, 140);
	line_num_line_color = fl_rgb_color(210, 210, 210);

	Fl::visual(FL_DOUBLE|FL_INDEX);

	window = new MyWindow(640,480);

	fl_font(font, font_size);
	font_height = fl_height();
	font_width  = (int)fl_width(' ');
	bottom_bar_height = font_height + 7;

	edit_viewport = new EditViewport(0, 0, 640, 480 - bottom_bar_height);

	Fl_Group *bottom_section = new Fl_Group(0, 480 - bottom_bar_height, 640, bottom_bar_height);
	input_viewport  = new InputViewport(0, 480 - bottom_bar_height, 321, bottom_bar_height);
	output_viewport = new OutputViewport(319,480 - bottom_bar_height, 321, bottom_bar_height);
	bottom_section->end();

//	window->border(0); /* remove window manager titles and border */
	window->resizable(*edit_viewport);
	window->callback(close_window);
	display_filename(e->cepad->filename);
	window->show();
	Fl::focus(window);
	return(Fl::run());
}