#ifndef __DISPLAY_H
#define __DISPLAY_H

#ifndef TIOCGSIZE
#define TIOCGSIZE TIOCGWINSZ
#define ttysize winsize
#define ts_lines ws_row
#define ts_cols  ws_col
#endif

/* main.c */
void resize (void);

/* cmd_terminal.c */
void cmd_delay (int argc, char *argv[]);
void cmd_lineno (int argc, char *argv[]);
void cmd_showtabs (int argc, char *argv[]);

/* display.c */
void next_event (void);

extern struct ttysize dimensions;
extern int lineno;
extern int showtabs;

/* redraw.c */
void	redraw (void);
void    redraw_title (void);
void    redraw_stats (void);
void    redraw_command (void);
void    redraw_output (void);
void    redraw_line (int, line_t *);
void    redraw_curs (void);

void 	apply_dm (int c_dm, int *colour, int *invert);
void    dm_set (int row, int col, int mod);
void    dm_clear_all ();


#endif
