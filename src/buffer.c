/**
 * buffer.c
 * Author: Jan Viktorin
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 */

#include "pico.h"
#include "buffer.h"
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

struct buffer {
	char *begin;
	char *end;
};

bool buffer_isend(struct pico *p, char *offset)
{
	assert(offset >= p->buff->begin);
	return offset + 1 >= p->buff->end;
}

bool buffer_init(struct pico *p, char *srcfile)
{
	if(p->buff != NULL)
		return true;

	FILE *f = srcfile == NULL ? stdin : fopen(srcfile, "rb");
	if(f == NULL)
		return error(p, "Can not open the source file");

	size_t cap = 4096;
	size_t len = 0;
	char *source = (char *) malloc(cap);
	if(source == NULL) {
		if(srcfile != NULL)
			fclose(f);
		return error(p, "Memory allocation error");
	}

	while(true) {
		if(len == cap) {
			size_t new_cap = cap * 2;
			char *resized = (char *) realloc(source, new_cap);
			if(resized == NULL) {
				free(source);
				if(srcfile != NULL)
					fclose(f);
				return error(p, "Memory allocation error");
			}

			source = resized;
			cap = new_cap;
		}

		size_t read_count = fread(source + len, 1, cap - len, f);
		len += read_count;

		if(read_count == 0)
			break;
	}

	if(ferror(f)) {
		free(source);
		if(srcfile != NULL)
			fclose(f);
		return error(p, "Can not read the source file");
	}

	if(srcfile != NULL)
		fclose(f);

	if(len == 0) {
		free(source);
		return error(p, "The source file is empty");
	}

	p->buff = (struct buffer *) malloc(sizeof(struct buffer));
	if(p->buff == NULL) {
		free(source);
		return error(p, "Memory allocation error");
	}

	p->buff->begin = source;
	p->buff->end = p->buff->begin + len;
	p->offset = p->buff->begin;
	return true;
}

void buffer_destroy(struct pico *p)
{
	if(p->buff == NULL)
		return;

	free(p->buff->begin);
	free(p->buff);
	p->buff = NULL;
}

