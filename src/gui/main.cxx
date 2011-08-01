#include <math.h>
#include <string.h>

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Box.H>

#include <FL/names.h>

extern "C" {
#include "../include/editor.h"
}

void display_close ()
{
}

void display_beep ()
{
}

void display_do_menu (menu_t * menu)
{
}

void display_finish_menu ()
{
}

Fl_Color bg_color, line_color, text_color;

char buffer[100];

#if defined(__APPLE__)
int font = FL_SCREEN;
#else
int font = FL_COURIER;
#endif
int font_size = 13;

int mouse_to_cursor = 1;

class EditViewport : public Fl_Widget {
public:
	int viewport_w, viewport_h;
	
protected:
	void set_viewport_size(int W, int H) {
		viewport_w = floor((W - 6) / fl_width(' '));
		viewport_h = floor((H - 6) / fl_height()) - 1;
		pad_set_viewport_size(e->cpad, viewport_w, viewport_h);
	}

	void resize(int X, int Y, int W, int H) {
		set_viewport_size(W, H);
	  	Fl_Widget::resize(X,Y,W,H);
	}

	void draw() {
		char buf[80];
		int yp;
		int i;
		line_t *line;
		char *str;
		int offset, intab;
		char empty_string[1] = "";
		int lines_start_y;
		pad_t *pad;

		int start_y = 0;
		int start_x = 0;
		int end_y   = 0;
		int end_x   = 0;

		pad = e->cepad;
		
		if (pad->echo)
		{
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

		/* lines at sides */
		fl_color(line_color);
		fl_line(x(), y() + 2, x(), h());
		fl_line(w() - 1, y() + 2, w() - 1, h());

		/* top bar */
		fl_rectf(x() + 3, y() + 2, w() - 6, fl_height() + 4 );

		fl_color(bg_color);
		fl_draw((pad->filename ? pad->filename : "(new file)"), x() + 7, y() + 4 + fl_height() - fl_descent());

		lines_start_y = y() + 6 + (fl_height() * 2 - fl_descent());

		/* highlight background */
		if (pad->echo == REGION_RECT)
		{
			fl_color(line_color);
			fl_rectf(x() + 3 + (fl_width(' ') * start_x), lines_start_y + (fl_height() * (start_y - 1)) + fl_descent(), fl_width(' ') * (end_x - start_x), fl_height() * ((end_y - start_y) + 1));
		}
		
		/* edit text */
		fl_color(text_color);
		yp = lines_start_y;
		line = LINE_get_line_at (pad, 1 + pad->offset_y);
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
				if ((size_t)offset < strlen (line->str->data))
					str = line->str->data + offset;
				else
					str = empty_string;
			}
			
			fl_draw(str, x() + 3, yp);
			yp += fl_height();
			
			if (line != pad->line_head)
				line = line->next;
		}
		
		/* cursor */
		if (e->occupied_window == EDIT_WINDOW) {
			fl_color(line_color);
			if (pad->echo == REGION_RECT && start_x != end_x && start_y != end_y) {
				fl_rect(x() + 3 + (fl_width(' ') * (pad->curs_x - 1)), lines_start_y + (fl_height() * (pad->curs_y - 2)) + fl_descent(), fl_width(' '), fl_height() );			
			} else {
				fl_rectf(x() + 3 + (fl_width(' ') * (pad->curs_x - 1)), lines_start_y + (fl_height() * (pad->curs_y - 2)) + fl_descent(), fl_width(' '), fl_height() );
				fl_color(bg_color);
				sprintf(buf, "%c", pad_get_char_at(pad, pad->curs_x + pad->offset_x, pad->curs_y + pad->offset_y));
				fl_draw(buf, x() + 3 + (fl_width(' ') * (pad->curs_x - 1)), lines_start_y + (fl_height() * (pad->curs_y - 1)));
			}
		}
		
/*		fl_draw(buffer, x(), yp);
		yp += fl_height();
		fl_draw("line 2 Command", x(), yp);*/

//		fl_draw("Command: ", x() + 1, y() + fl_height() - fl_descent());
	}
public:
    EditViewport(int X, int Y, int W, int H, const char *l=0)
	: Fl_Widget(X,Y,W,H,l) {
		set_viewport_size(W, H);
	}
	
	void move_cursor(int X, int Y) {
		int new_x, new_y;
		
		if (mouse_to_cursor) {
			new_x = floor((X + 3) / fl_width(' '));
			new_y = floor((Y + 6) / fl_height()) - 1;
		
			if (new_x > 0 && new_x <= viewport_w && new_y > 0 && new_y <= viewport_h) {
				if (e->occupied_window != EDIT_WINDOW) {
					e->occupied_window = EDIT_WINDOW;
					e->cpad = e->cepad;
				}
				e->cpad->curs_x = new_x;
				e->cpad->curs_y = new_y;
			}
		}
	}

	int in_edit_window(int X, int Y) {
		return (X > 3 && X < w() - 3 && Y > (6 + fl_height()) && Y < (6 + ((viewport_h + 1) * fl_height()) + fl_descent()));
	}
};

EditViewport   *edit_viewport;


class MyWindow : public Fl_Window {
	int handle(int);

	void set_mouse_cursor(int X, int Y) {
		if (mouse_to_cursor && edit_viewport->in_edit_window(X, Y)) {
			fl_cursor(FL_CURSOR_NONE);
		} else {
			fl_cursor(FL_CURSOR_DEFAULT);
		}
	}

public:
	MyWindow(int w, int h, const char *t=0L) 
		: Fl_Window( w, h, t ) { }
};

class InputViewport : public Fl_Widget {
	int input_start;
protected:
	void draw() {
		char buf[80];
		line_t *line;
		pad_t  *pad = e->input_pad;
		char *str;
		int offset, intab;
		
		fl_font(font, font_size);
		
		fl_color(bg_color);
		fl_rectf(x(), y(), w(), h() );
		fl_color(line_color);
		fl_rect(x(), y(), w(), h() );
		fl_color(text_color);
		fl_draw("Command: ", x() + 3, y() + 4 + fl_height() - fl_descent());

		line = pad->line_head->prev;
		if (line != pad->line_head && line->str && line->str->data) {
			offset = get_string_pos (pad->offset_x + 1, line->str->data, &intab);
			if ((size_t)offset < strlen (line->str->data))
				str = line->str->data + offset;
			else
				str = NULL;

			if (str)
				fl_draw(str, x() + 3 + (fl_width(' ') * 9), y() + 4 + fl_height() - fl_descent());
		}

		if(e->occupied_window == COMMAND_WINDOW) {
			/* cursor */
			fl_color(line_color);
			fl_rectf(x() + 3 + (fl_width(' ') * 9) + (fl_width(' ') * (pad->curs_x - 1)), y() + 4, fl_width(' '), fl_height() );
		}
	}
public:
    InputViewport(int X, int Y, int W, int H, const char *l=0)
	: Fl_Widget(X,Y,W,H,l) {
		input_start = fl_width("Command: ");
	}
};

class OutputViewport : public Fl_Widget {
protected:
	void draw() {
		char buf[80];
		
		fl_font(font, font_size);
		
		fl_color(bg_color);
		fl_rectf(x(), y(), w(), h() );
		fl_color(line_color);
		fl_rect(x(), y(), w(), h() );
		fl_color(text_color);
		fl_draw(buffer, x() + 3, y() + 4 + fl_height() - fl_descent());
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
  {FL_Shift_L, ""},
  {FL_Shift_R, "FL_Shift_R"},
  {FL_Control_L, "FL_Control_L"},
  {FL_Control_R, "FL_Control_R"},
  {FL_Caps_Lock, "FL_Caps_Lock"},
  {FL_Alt_L, "FL_Alt_L"},
  {FL_Alt_R, "FL_Alt_R"},
  {FL_Meta_L, "FL_Meta_L"},
  {FL_Meta_R, "FL_Meta_R"},
  {FL_Menu, "FL_Menu"},
  {FL_Help, "FL_Help"},
  {FL_Num_Lock, "FL_Num_Lock"},
  {FL_KP_Enter, "FL_KP_Enter"}
};

int MyWindow::handle(int e) {
	char buffer2[100];
	char eventname[100];
    const char *keyname = buffer;
	keydef_t *keydef;

	if (e == FL_FOCUS) {
		return 1;
	}
	if (e == FL_ENTER) {
		return 1;
	}
	if (e == FL_KEYDOWN) {
		strcpy(eventname, "keydown");
	} else if (e == FL_KEYUP) {
		strcpy(eventname, "keyup");
	} else {
		strcpy(eventname, "other");
//		eventname[0] = '\0';
	}
	if (e == FL_KEYDOWN/* || e == FL_KEYUP*/) {
	    int k = Fl::event_key();
		int c = Fl::event_text()[0]; // text for the key, handles shift+standard keypress
		int mods = Fl::event_state();

	    if (!k)
	      keyname = "0";
	    else if (k < 256) {
			if (k == '\'') {
				sprintf(buffer, "squote");
			} else if (k == '"') {
				sprintf(buffer, "dquote");
			} else {
	      		sprintf(buffer, "%c", c);
			}
	    } else if (k > FL_F && k <= FL_F_Last) {
	      sprintf(buffer, "F%d", k - FL_F);
	    } else if (k >= FL_KP && k <= FL_KP_Last) {
	      sprintf(buffer, "FL_KP+'%c'", k-FL_KP);
	    } else if (k >= FL_Button && k <= FL_Button+7) {
	      sprintf(buffer, "FL_Button+%d", k-FL_Button);
	    } else {
	      sprintf(buffer, "0x%04x", k);
	      for (int i = 0; i < int(sizeof(table)/sizeof(*table)); i++)
			if (table[i].n == k) {keyname = table[i].text; break;}
		  if (strlen(keyname) > 0) {
			strcpy(buffer, keyname);
		}
	    }
		if ((mods & FL_SHIFT) && k > 255) {
			strcpy(buffer2, buffer);
			sprintf(buffer, "%sS", buffer2);
		}
		if (mods & FL_CTRL) {
			strcpy(buffer2, buffer);
			sprintf(buffer, "^%s", buffer2);
		}
		if (mods & FL_ALT) {
			strcpy(buffer2, buffer);
			sprintf(buffer, "*%s", buffer2);
		}
		if (mods & FL_META) {
			strcpy(buffer2, buffer);
			sprintf(buffer, "~%s", buffer2);
		}
/*		if (e == FL_KEYUP) {
			strcpy(buffer2, buffer);
			sprintf(buffer, "%sU", buffer2);
		}*/
//		strcpy(buffer2, buffer);
//		sprintf(buffer, "%s %s", fl_eventnames[e], buffer2);
	} else if (e == FL_MOVE || e == FL_DRAG || e == FL_MOUSEWHEEL) {
		sprintf(buffer, "%s (%d, %d)", fl_eventnames[e], Fl::event_x(), Fl::event_y());
	} else if (e == FL_PUSH) {
		sprintf(buffer, "%s %d", fl_eventnames[e], Fl::event_button());
	} else {
		sprintf(buffer, "%s", fl_eventnames[e]);
	}

	if (e == FL_MOVE) {
		edit_viewport->move_cursor(Fl::event_x(), Fl::event_y());
		set_mouse_cursor(Fl::event_x(), Fl::event_y());
	}

	keydef = KEY_find (buffer);

	if (keydef)
	{
		parse ("%s", keydef->def);
	}

	edit_viewport->redraw();
	input_viewport->redraw();
	output_viewport->redraw();
	
	return (1); // eat all keystrokes
}

extern "C" void cmd_mouse (int argc, char *argv[])
{
	if (argc > 1)
	{
		if (strcmp (argv[1], "-on") == 0)
		{
			mouse_to_cursor = 1;
		}
		else if (strcmp (argv[1], "-off") == 0)
		{
			mouse_to_cursor = 0;
		}
	}
	else
		mouse_to_cursor = !mouse_to_cursor;
}

int main(int argc, char **argv) {
	int font_height;
	int font_width;
	int io_height;

	editor_setup(argc, argv);
	add_command ("mouse",    (void (*)())cmd_mouse);

	bg_color    = fl_rgb_color(254, 255, 231);
	line_color  = fl_rgb_color(71, 43, 198);
	text_color  = fl_rgb_color(0, 0, 0);

	window = new MyWindow(640,480);

	fl_font(font, font_size);
	font_height = fl_height();
	font_width  = (int)fl_width(' ');
	io_height = font_height + 8;

	edit_viewport = new EditViewport(0, 0, 640, 480 - io_height);

	Fl_Group *bottom_section = new Fl_Group(0, 480 - io_height, 640, io_height);
	input_viewport  = new InputViewport(0, 480 - io_height, 321, io_height);
	output_viewport = new OutputViewport(319,480 - io_height, 321, io_height);
	bottom_section->end();

//	window->border(0); /* remove window manager titles and border */
	window->resizable(*edit_viewport);
	window->show();
	Fl::focus(window);
	return(Fl::run());
}