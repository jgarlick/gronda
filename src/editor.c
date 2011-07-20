#include "editor.h"

editor_t *e;

void editor_init ()
{
	e = ALLOC (editor_t);

	if (e == NULL)
	{
		debug ("(main.c) Unable to allocate memory for editor");
		exit (1);
	}

	e->occupied_window = EDIT_WINDOW;

	/* new pad */
	e->pad_head = ALLOC (pad_t);
	if (e->pad_head == NULL)
	{
		debug ("(main.c) Unable to allocate memory for e->pad_head");
		exit (1);
	}
	e->pad_head->line_head = LINE_new_lines ();
	e->pad_head->curs_x = 1;
	e->pad_head->curs_y = 1;
	e->pad_head->flags |= FILE_WRITE;

	display_max_pad_dim (&(e->pad_head->width), &(e->pad_head->height));

	e->cpad = e->pad_head;

	e->input = ALLOC (input_t);
	if (e->input == NULL)
	{
		debug ("(main.c) Unable to allocate memory for e->input");
		exit (1);
	}

	e->input->offset = 0;
	e->input->curs_x = 0;
	e->flags |= INSERT;

	KEY_init ();
}

void
editor_fini ()
{
	/* TODO */
	FREE (e);
}
