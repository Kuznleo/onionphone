/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/*

$Log$
Revision 1.15  2004/06/26 03:50:14  markster
Merge source cleanups (bug #1911)

Revision 1.14  2003/02/12 13:59:15  matteo
mer feb 12 14:56:57 CET 2003

Revision 1.1.1.1  2003/02/12 13:59:15  matteo
mer feb 12 14:56:57 CET 2003

Revision 1.2  2000/01/05 08:20:39  markster
Some OSS fixes and a few lpc changes to make it actually work

 * Revision 1.2  1996/08/20  20:40:51  jaf
 * Removed all static local variables that were SAVE'd in the Fortran
 * code, and put them in struct lpc10_encoder_state that is passed as an
 * argument.
 *
 * Removed init function, since all initialization is now done in
 * init_lpc10_encoder_state().
 *
 * Revision 1.1  1996/08/19  22:30:54  jaf
 * Initial revision
 *
 */

#include "hp100.h"
#include "lpc10.h"
#include "prepro.h"

/* Table of constant values */

static int32_t c__1 = 1;

/* ********************************************************************* */

/* 	PREPRO Version 48 */

/* $Log$
 * Revision 1.15  2004/06/26 03:50:14  markster
 * Merge source cleanups (bug #1911)
 *
 * Revision 1.14  2003/02/12 13:59:15  matteo
 * mer feb 12 14:56:57 CET 2003
 *
 * Revision 1.1.1.1  2003/02/12 13:59:15  matteo
 * mer feb 12 14:56:57 CET 2003
 *
 * Revision 1.2  2000/01/05 08:20:39  markster
 * Some OSS fixes and a few lpc changes to make it actually work
 *
 * Revision 1.2  1996/08/20  20:40:51  jaf
 * Removed all static local variables that were SAVE'd in the Fortran
 * code, and put them in struct lpc10_encoder_state that is passed as an
 * argument.
 *
 * Removed init function, since all initialization is now done in
 * init_lpc10_encoder_state().
 *
 * Revision 1.1  1996/08/19  22:30:54  jaf
 * Initial revision
 * */
/* Revision 1.3  1996/03/14  23:22:56  jaf */
/* Added comments about when INITPREPRO should be used. */

/* Revision 1.2  1996/03/14  23:09:27  jaf */
/* Added an entry named INITPREPRO that initializes the local state of */
/* this subroutine, and those it calls (if any). */

/* Revision 1.1  1996/02/07  14:48:54  jaf */
/* Initial revision */

/* ********************************************************************* */

/*    Pre-process input speech: */

/* Inputs: */
/*  LENGTH - Number of SPEECH samples */
/* Input/Output: */
/*  SPEECH(LENGTH) - Speech data. */
/*                   Indices 1 through LENGTH are read and modified. */

/* This subroutine has no local state maintained from one call to the */
/* next, but HP100 does.  If you want to switch to using a new audio */
/* stream for this filter, or reinitialize its state for any other */
/* reason, call the ENTRY INITPREPRO. */

int lpc10_prepro(float *speech, int32_t * length,
		 struct lpc10_encoder_state *st)
{
/*       Arguments */
/*   High Pass Filter at 100 Hz */
	/* Parameter adjustments */
	if (speech) {
		--speech;
	}

	/* Function Body */
	lpc10_hp100(&speech[1], &c__1, length, st);
	return 0;
}				/* lpc10_prepro */
