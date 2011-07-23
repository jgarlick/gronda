#ifndef __EDITOR_H
#define __EDITOR_H

#include <stdio.h>
#include <stdarg.h>

#ifdef LINUX
#include <stdlib.h>				/* for calloc, free */
#endif

#define ALLOC( x ) ( x* ) calloc( 1, sizeof( x ) )
#define FREE       free

#include "str.h"
#include "keydefs.h"
#include "menu.h"
#include "recovery.h"
#include "command.h"

#define BUFFER_SIZE 1024

typedef enum
{ 
	EDIT_WINDOW,
	COMMAND_WINDOW,
	COMMAND_WINDOW_REQUEST,
	OUTPUT_WINDOW
}
occupied_window_t;

/* editor flags */
#define DEBUG       ( 1 << 0 )	/* display debugging info */
#define INSERT      ( 1 << 1 )	/* insert mode */

/* redraw flags */
#define DIRTY_ALL   ( 1 << 0 )	/* redraw all pad lines                  */
#define TITLE       ( 1 << 1 )	/* redraw pad title and flags            */
#define STATS       ( 1 << 2 )	/* redraw top stats eg curs pos, pad pos */
#define COMMAND     ( 1 << 3 )	/* redraw command window                 */
#define OUTPUT      ( 1 << 4 )	/* redraw output window                  */

/* line_redraw flags */
#define CLEAN 0
#define DIRTY 1

#define DIRTY_LINE( n ) *( e->cpad->line_redraw + n - 1 ) = DIRTY

/* display modifiers */
#define INVERT_ON  (1 << 0)
#define INVERT_OFF (1 << 1)
#define SYNTAX0    (1 << 2)
#define SYNTAX1    (1 << 3)
#define SYNTAX2    (1 << 4)
#define SYNTAX3    (1 << 5)
#define SYNTAX4    (1 << 6)


typedef struct line_s
{
	string_t *str;

	struct line_s *next;
	struct line_s *prev;
}
line_t;

/* pad flags */
#define MODIFIED   ( 1 << 0 )
#define FILE_WRITE ( 1 << 1 )

#define REGION_OFF    (0)
#define REGION_LINEAR (1)
#define REGION_RECT   (2)

typedef struct pad_s
{
	int     curs_x;             /* cursor position within the viewport */
	int     curs_y;

	int     width;              /* viewport width and height */
	int     height;

	char   *filename;
	int     flags;
	int     line_count;			/* total number of lines */
	int     offset_x;			/* x scroll offset       */
	int     offset_y;			/* y scroll offset       */

	int     mark_x;             /* current mark position */
	int     mark_y;

	int     echo;

	int    *line_redraw;		/* redraw for each line */

	line_t *line_head;

	struct pad_s *next;
}
pad_t;

typedef struct buffer_s
{
	char name[255];
	string_t *str;
	struct buffer_s *next;
} buffer_t;

/* command window */
/* TODO: This should be a normal pad (need local key defs first) */
typedef struct input_s
{
	int     offset;
	int     curs_x;

	char    buffer[BUFFER_SIZE];

//       char *current;

}
input_t;

/* TODO: This should be a normal pad (need local key defs first) */
typedef struct output_msg_s
{
	char   *message;

	struct output_msg_s *next;

}
output_msg_t;

/* main editor struct */
typedef struct editor_s
{
	occupied_window_t occupied_window;
	output_msg_t *messages;

	input_t *input;				/* command window */

	buffer_t *buffer_head;      /* paste buffers */

	pad_t  *pad_head;			/* linked list of pads */
	pad_t  *cpad;				/* current pad */

	int     redraw;				/* redraw flags */
	int     flags;

	int    *dm;					/* display modifiers */

	char    key[80];         /* used to store the current keydef executed */
}
editor_t;

#define ADJUST_LEFT  (0)
#define ADJUST_RIGHT (1)

/* command.c */
extern void add_base_commands (void);

/* line.c */
extern line_t *LINE_new_lines (void);
extern void LINE_free_lines (line_t *);
extern void LINE_append (pad_t *, char *);
extern void LINE_insert (pad_t *, int, char *);
extern void LINE_remove (pad_t *, line_t *);
extern line_t *LINE_get_line_at (pad_t *, int);
extern int line_length (line_t *l);
extern void line_grow_pad (pad_t *pad, int ypos);

/* main.c */
extern editor_t *e;

void    editor_init ();
void    editor_fini ();

void    resize ();

/* signals.c */
void    sig_init (void);
void    sig_handler (int);
void    sig_cleanexit (char *, ...);
void    sig_die (char *str, ...);
void    sig_timer_click (int);
void    sig_interrupt_handler (int);
void    sig_handle_error (int);

/* recovery.c */
int crash_file_check (char *fname);

/* redraw.c */
void    dm_set (int row, int col, int mod);
void    dm_clear_all ();
void	redraw ();

/* buffer.c */
void get_region (int shape, int *start_y, int *start_x, int *end_y, int *end_x);
buffer_t *buffer_create (char *name);
void buffer_cutcopy (buffer_t *buf, int cut, int shape, int start_y, int start_x, int end_y, int end_x);
void buffer_save (buffer_t *buf, char *file_name);
buffer_t *buffer_load (char *file_name);

/* tools.c */
void    debug (char *, ...);
void    output_message (char *, ...);
void    output_message_c (char *, char *, ...);
void    parse (char *, ...);

int     vasprintf (char **, const char *, va_list);

int     parse_commandfile (char *);
void    create_local_config ();
int     get_string_pos (int x, char *str, int *intab);
int     get_curs_pos (int x, line_t *l);
void    cursor_set_pos (pad_t *p, int curs_y, int curs_x, int adjust);
void    pad_modified (pad_t *pad);

/* cmd_buffer.c */
void cmd_dr   (int argc, char *argv[]);
void cmd_echo (int argc, char *argv[]);
void cmd_xc   (int argc, char *argv[]);
void cmd_xd   (int argc, char *argv[]);
void cmd_xp   (int argc, char *argv[]);

/* cmd_command.c */
void cmd_msg  (int argc, char *argv[]);
void cloop    (int argc, char *argv[]);

/* cmd_cursor.c */
void cmd_au   (int argc, char *argv[]);
void cmd_ad   (int argc, char *argv[]);
void cmd_al   (int argc, char *argv[]);
void cmd_ar   (int argc, char *argv[]);

void cmd_tt   (int argc, char *argv[]);
void cmd_tb   (int argc, char *argv[]);
void cmd_tl   (int argc, char *argv[]);
void cmd_tr   (int argc, char *argv[]);

/* cmd_file.c */
void cmd_ce   (int argc, char *argv[]);
void cmd_pw   (int argc, char *argv[]);
void cmd_pn   (int argc, char *argv[]);
void cmd_pwd  (int argc, char *argv[]);
void cmd_ro   (int argc, char *argv[]);

/* cmd_key.c */
void cmd_kd   (int argc, char *argv[]);

/* cmd_pad.c */
void cmd_ph   (int argc, char *argv[]);
void cmd_pp   (int argc, char *argv[]);
void cmd_pt   (int argc, char *argv[]);
void cmd_pv   (int argc, char *argv[]);

/* cmd_system.c */
void cmd_bang (int argc, char *argv[]);

/* cmd_text.c */
void cmd_es   (int argc, char *argv[]);
void cmd_er   (int argc, char *argv[]);
void cmd_en   (int argc, char *argv[]);
void cmd_ee   (int argc, char *argv[]);
void cmd_ed   (int argc, char *argv[]);
void cmd_ei   (int argc, char *argv[]);

/* cmd_window.c */
void cmd_tdm  (int argc, char *argv[]);
void cmd_wc   (int argc, char *argv[]);

/* required display functions */
void    display_init (void);
void    display_fini (void);
void    display_nextevent ();
void    display_max_pad_dim (int *, int *);
void    display_redraw_title (void);
void    display_redraw_stats (void);
void    display_redraw_command (void);
void    display_redraw_output (void);
void    display_redraw_line (int, line_t *);
void    display_redraw_curs (void);
void    display_do_menu (menu_t *);
void    display_finish_menu ();
void    display_beep ();

#endif
