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

class CommandView : public Fl_Widget {
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
		fl_draw("Command: ", x() + 3, y() + 2 + fl_height() - fl_descent());
	}
public:
    CommandView(int X, int Y, int W, int H, const char *l=0)
	: Fl_Widget(X,Y,W,H,l) {
		input_start = fl_width("Command: ");
	}
};

class OutputView : public Fl_Widget {
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
		fl_draw(buffer, x() + 3, y() + 2 + fl_height() - fl_descent());
	}
public:
    OutputView(int X, int Y, int W, int H, const char *l=0)
	: Fl_Widget(X,Y,W,H,l) {
	}
};

class EditView : public Fl_Widget {
protected:
	int viewport_w, viewport_h;

	void draw() {
		char buf[80];
		int yp;
		int i;
		line_t *line;
		char *str;
		int offset, intab;
		char empty_string[1] = "";
		
//		sprintf(buf, "%d / %f / %d / %d", fl_height(), fl_width(' '), fl_descent(), y());
//		fl_font(FL_SCREEN, 16);
		
		fl_font(font, font_size);
		fl_color(bg_color);
		fl_rectf(x(), y(), w(), h() );

		fl_color(line_color);
		fl_line(x(), y() + 2, x(), h());
		fl_line(w() - 1, y() + 2, w() - 1, h());

		fl_rectf(x() + 3, y() + 2, w() - 6, fl_height() + 4 );

		fl_color(bg_color);
		fl_draw((e->cpad->filename ? e->cpad->filename : "(new file)"), x() + 7, y() + 4 + fl_height() - fl_descent());

		fl_color(text_color);
//		fl_rect(x(), y(), w(), h() );
		yp = y() + 6 + (fl_height() * 2 - fl_descent());
		line = LINE_get_line_at (e->cpad, 1);
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
			
			fl_draw(str, x() + 4, yp);
			yp += fl_height();
			
			if (line != e->cpad->line_head)
				line = line->next;
		}
/*		fl_draw(buffer, x(), yp);
		yp += fl_height();
		fl_draw("line 2 Command", x(), yp);*/

//		fl_draw("Command: ", x() + 1, y() + fl_height() - fl_descent());
	}
public:
    EditView(int X, int Y, int W, int H, const char *l=0)
	: Fl_Widget(X,Y,W,H,l) {
		viewport_w = W / font_size;
		viewport_h = (H / fl_height())-1;
		pad_set_viewport_size(e->cpad, viewport_w, viewport_h);
	}
};

EditView    *edit_view;
OutputView  *output_view;
CommandView *command_view;

struct keycode_table{int n; const char* text;} table[] = {
  {FL_Escape, "FL_Escape"},
  {FL_BackSpace, "FL_BackSpace"},
  {FL_Tab, "FL_Tab"},
  {FL_Enter, "FL_Enter"},
  {FL_Print, "FL_Print"},
  {FL_Scroll_Lock, "FL_Scroll_Lock"},
  {FL_Pause, "FL_Pause"},
  {FL_Insert, "FL_Insert"},
  {FL_Home, "FL_Home"},
  {FL_Page_Up, "FL_Page_Up"},
  {FL_Delete, "FL_Delete"},
  {FL_End, "FL_End"},
  {FL_Page_Down, "FL_Page_Down"},
  {FL_Left, "FL_Left"},
  {FL_Up, "FL_Up"},
  {FL_Right, "FL_Right"},
  {FL_Down, "FL_Down"},
  {FL_Shift_L, "FL_Shift_L"},
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
	if (e == FL_KEYDOWN || e == FL_KEYUP) {
	    int k = Fl::event_key();
		int mods = Fl::event_state();
	    if (!k)
	      keyname = "0";
	    else if (k < 256) {
	      sprintf(buffer, "'%c'", k);
	    } else if (k > FL_F && k <= FL_F_Last) {
	      sprintf(buffer, "FL_F+%d", k - FL_F);
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
			sprintf(buffer, "Shift + %s", buffer2);
		}
		if (mods & FL_CTRL) {
			strcpy(buffer2, buffer);
			sprintf(buffer, "Ctrl + %s", buffer2);
		}
		if (mods & FL_ALT) {
			strcpy(buffer2, buffer);
			sprintf(buffer, "ALT + %s", buffer2);
		}
		if (mods & FL_META) {
			strcpy(buffer2, buffer);
			sprintf(buffer, "META + %s", buffer2);
		}
		strcpy(buffer2, buffer);
		sprintf(buffer, "%s %s", fl_eventnames[e], buffer2);
	} else if (e == FL_MOVE || e == FL_DRAG || FL_MOUSEWHEEL) {
		sprintf(buffer, "%s (%d, %d)", fl_eventnames[e], Fl::event_x(), Fl::event_y());
	} else if (e == FL_PUSH) {
		sprintf(buffer, "%s %d", fl_eventnames[e], Fl::event_button());
	} else {
		sprintf(buffer, "%s", fl_eventnames[e]);
	}
	edit_view->redraw();
	output_view->redraw();
	
	return (1); // eat all keystrokes
}


int main(int argc, char **argv) {
	int font_height;
	int font_width;
	int io_height;

	editor_setup(argc, argv);

	bg_color    = fl_rgb_color(254, 255, 231);
	line_color = fl_rgb_color(71, 43, 198);
	text_color  = fl_rgb_color(0, 0, 0);

	MyWindow *window = new MyWindow(640,480);

	fl_font(font, font_size);
	font_height = fl_height();
	font_width  = (int)fl_width(' ');
	io_height = font_height + 4;

	edit_view = new EditView(0, 0, 640, 480 - io_height);

	Fl_Group *bottom_section = new Fl_Group(0, 480 - io_height, 640, io_height);
	command_view = new CommandView(0, 480 - io_height, 321, io_height);
	output_view  = new OutputView(319,480 - io_height, 321, io_height);
	bottom_section->end();

	window->resizable(*edit_view);
	window->show();
	Fl::focus(window);
	return(Fl::run());
}