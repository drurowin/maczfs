/*
 * CDDL HEADER START
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License, Version 1.0 only
 * (the "License").  You may not use this file except in compliance
 * with the License.
 *
 * You can obtain a copy of the license at usr/src/OPENSOLARIS.LICENSE
 * or http://www.opensolaris.org/os/licensing.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 *
 * When distributing Covered Code, include this CDDL HEADER in each
 * file and include the License file at usr/src/OPENSOLARIS.LICENSE.
 * If applicable, add the following below this CDDL HEADER, with the
 * fields enclosed by brackets "[]" replaced with your own identifying
 * information: Portions Copyright [yyyy] [name of copyright owner]
 *
 * CDDL HEADER END
 */

/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*
 * Copyright 2006 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */

#pragma ident	"%Z%%M%	%I%	%E% SMI"	/* SVr4.0 1.1.5.2 */

/*LINTLIBRARY*/

#pragma weak gmatch = _gmatch

#include "gen_synonyms.h"
#include <sys/types.h>
#include <libgen.h>
#include <stdlib.h>
#include <limits.h>
#include <widec.h>
#include "_range.h"

#define	Popwchar(p, c) \
	n = mbtowc(&cl, p, MB_LEN_MAX); \
	c = cl; \
	if (n <= 0) \
		return (0); \
	p += n;

int
gmatch(const char *s, const char *p)
{
	const char	*olds;
	wchar_t		scc, c;
	int 		n;
	wchar_t		cl;

	olds = s;
	n = mbtowc(&cl, s, MB_LEN_MAX);
	if (n <= 0) {
		s++;
		scc = n;
	} else {
		scc = cl;
		s += n;
	}
	n = mbtowc(&cl, p, MB_LEN_MAX);
	if (n < 0)
		return (0);
	if (n == 0)
		return (scc == 0);
	p += n;
	c = cl;

	switch (c) {
	case '[':
		if (scc <= 0)
			return (0);
	{
			int ok;
			wchar_t lc = 0;
			int notflag = 0;

			ok = 0;
			if (*p == '!') {
				notflag = 1;
				p++;
			}
			Popwchar(p, c)
			do
			{
				if (c == '-' && lc && *p != ']') {
					Popwchar(p, c)
					if (c == '\\') {
						Popwchar(p, c)
					}
					if (notflag) {
						if (!multibyte ||
						    valid_range(lc, c)) {
							if (scc < lc || scc > c)
								ok++;
							else
								return (0);
						}
					} else {
						if (!multibyte ||
						    valid_range(lc, c))
							if (lc <= scc &&
							    scc <= c)
								ok++;
					}
				} else if (c == '\\') {
					/* skip to quoted character */
					Popwchar(p, c)
				}
				lc = c;
				if (notflag) {
					if (scc != lc)
						ok++;
					else
						return (0);
				}
				else
				{
					if (scc == lc)
						ok++;
				}
				Popwchar(p, c)
			} while (c != ']');
			return (ok ? gmatch(s, p) : 0);
		}

	case '\\':
		/* skip to quoted character and see if it matches */
		Popwchar(p, c)

	default:
		if (c != scc)
			return (0);
			/*FALLTHRU*/

	case '?':
		return (scc > 0 ? gmatch(s, p) : 0);

	case '*':
		while (*p == '*')
			p++;

		if (*p == 0)
			return (1);
		s = olds;
		while (*s) {
			if (gmatch(s, p))
				return (1);
			n = mbtowc(&cl, s, MB_LEN_MAX);
			if (n < 0)
				/* skip past illegal byte sequence */
				s++;
			else
				s += n;
		}
		return (0);
	}
}
