#ifndef __DISPLAY_H
#define __DISPLAY_H

#ifndef TIOCGSIZE
#define TIOCGSIZE TIOCGWINSZ
#define ttysize winsize
#define ts_lines ws_row
#define ts_cols  ws_col
#endif

void next_event (void);

extern struct ttysize dimensions;
extern int lineno;

#endif
