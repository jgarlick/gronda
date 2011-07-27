#include <math.h>

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

int font = FL_SCREEN;
int font_size = 14;

class MyWindow : public Fl_Window {
  int handle(int);
public:
  MyWindow(int w, int h, const char *t=0L) 
    : Fl_Window( w, h, t ) { }
};

class InputViewport : public Fl_Widget {
	int input_start;
protected:
	void draw() {
		char buf[80];
		
//		sprintf(buf, "%d / %f / %d / %d", fl_height(), fl_width(' '), fl_descent(), y());
		fl_font(font, font_size);
		
//		fl_font(FL_SCREEN, 16);
		fl_color(bg_color);
		fl_rectf(x(), y(), w(), h() );
		fl_color(line_color);
		fl_rect(x(), y(), w(), h() );
		fl_color(text_color);
		fl_draw("Command: ", x() + 3, y() + 4 + fl_height() - fl_descent());
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
		
//		sprintf(buf, "%d / %f / %d / %d", fl_height(), fl_width(' '), fl_descent(), y());
		fl_font(font, font_size);
		
//		fl_font(FL_SCREEN, 16);
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

class EditViewport : public Fl_Widget {
protected:
	int viewport_w, viewport_h;

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
		int y_start;
		
//		sprintf(buf, "%d / %f / %d / %d", fl_height(), fl_width(' '), fl_descent(), y());
//		fl_font(FL_SCREEN, 16);
		
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
		fl_draw((e->cpad->filename ? e->cpad->filename : "(new file)"), x() + 7, y() + 4 + fl_height() - fl_descent());

		/* edit text */
		fl_color(text_color);
		y_start = y() + 6 + (fl_height() * 2 - fl_descent());
		yp = y_start;
		line = LINE_get_line_at (e->cpad, 1 + e->cpad->offset_y);
		if (line == NULL)
			line = e->cpad->line_head;
			
		for (i = 0; i < viewport_h; i++) {
			intab  = 0;
			
			if (line == e->cpad->line_head)
			{
				str = empty_string;
			}
			else if (line == NULL || line->str == NULL)
			{
				str = empty_string;
			}
			else
			{
				offset = get_string_pos (e->cpad->offset_x + 1, line->str->data, &intab);
				if ((size_t)offset < strlen (line->str->data))
					str = line->str->data + offset;
				else
					str = empty_string;
			}
			
			fl_draw(str, x() + 3, yp);
			yp += fl_height();
			
			if (line != e->cpad->line_head)
				line = line->next;
		}
		
		/* cursor */
		fl_color(line_color);
		fl_rectf(x() + 3 + (fl_width(' ') * (e->cpad->curs_x - 1)), y_start + (fl_height() * (e->cpad->curs_y - 2)) + fl_descent(), fl_width(' '), fl_height() );
		fl_color(bg_color);
		sprintf(buf, "%c", pad_get_char_at(e->cpad, e->cpad->curs_x + e->cpad->offset_x, e->cpad->curs_y + e->cpad->offset_y));
		fl_draw(buf, x() + 3 + (fl_width(' ') * (e->cpad->curs_x - 1)), y_start + (fl_height() * (e->cpad->curs_y - 1)));

		
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
};

MyWindow       *window;
EditViewport   *edit_viewport;
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
		printf ("got focus\n");
		return 1;
	}
	if (e == FL_ENTER) {
		printf ("got enter\n");
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
		int mods = Fl::event_state();
	    if (!k)
	      keyname = "0";
	    else if (k < 256) {
	      sprintf(buffer, "%c", k);
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
		if (mods & FL_SHIFT) {
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

	keydef = KEY_find (buffer);

	if (keydef)
	{
		parse ("%s", keydef->def);
	}

	edit_viewport->redraw();
	output_viewport->redraw();
	
	return (1); // eat all keystrokes
}


int main(int argc, char **argv) {
	int font_height;
	int font_width;
	int io_height;

	editor_setup(argc, argv);

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