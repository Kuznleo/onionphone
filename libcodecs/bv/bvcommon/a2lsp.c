/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/*****************************************************************************/
/* BroadVoice(R)16 (BV16) Fixed-Point ANSI-C Source Code                     */
/* Revision Date: November 13, 2009                                          */
/* Version 1.1                                                               */
/*****************************************************************************/

/*****************************************************************************/
/* Copyright 2000-2009 Broadcom Corporation                                  */
/*                                                                           */
/* This software is provided under the GNU Lesser General Public License,    */
/* version 2.1, as published by the Free Software Foundation ("LGPL").       */
/* This program is distributed in the hope that it will be useful, but       */
/* WITHOUT ANY SUPPORT OR WARRANTY; without even the implied warranty of     */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the LGPL for     */
/* more details.  A copy of the LGPL is available at                         */
/* http://www.broadcom.com/licenses/LGPLv2.1.php,                            */
/* or by writing to the Free Software Foundation, Inc.,                      */
/* 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.                 */
/*****************************************************************************/

/*****************************************************************************
  a2sp.c : Common Fixed-Point Library: conversion from a's to lsp's

  $Log$
******************************************************************************/

#include <stdint.h>
#include "bvcommon.h"
#include "basop32.h"
#include "mathutil.h"
#include "utility.h"

#define  NAB      (LPCO>>1)+1
#define  NBIS  4		/* number of bisections */

int16_t FNevChebP(int16_t x, int16_t * t_man, int16_t * t_exp, int16_t nd2);

void a2lsp(int16_t pc[],		/* (i) Q12: predictor coefficients */
	   int16_t lsp[],	/* (o) Q15: line spectral pairs    */
	   int16_t old_lsp[])
{				/* (i) Q15: old lsp                */
	int16_t i, j, exp;
	int16_t fa_man[NAB], fa_exp[NAB], fb_man[NAB], fb_exp[NAB];
	int16_t ta_man[NAB], ta_exp[NAB], tb_man[NAB], tb_exp[NAB];
	int16_t *t_man, *t_exp;
	int32_t a0;
	int16_t nd2, nf, ngrd;
	int16_t xroot, xlow, ylow, ind, xhigh, yhigh, xmid, ymid, dx, dy, dxdy,
	    x, sign;

	/* Find normalization for fa and fb */
	/*   fb[0] = fa[0] = 1.0;                             */
	/*   for (i = 1, j = LPCO; i <= (LPCO/2); i++, j--) { */
	/*      fa[i] = pc[i] + pc[j] - fa[i-1];              */
	/*      fb[i] = pc[i] - pc[j] + fb[i-1];              */
	/*   }                                                */
	fa_man[0] = 16384;
	fa_exp[0] = 6;		// fa_man[0] in high 16-bits >> fa_exp[0] = 1.0 in Q24 
	fb_man[0] = 16384;
	fb_exp[0] = 6;		// fb_man[0] in high 16-bits >> fb_exp[0] = 1.0 in Q24
	for (i = 1, j = LPCO; i <= (LPCO / 2); i++, j--) {
		a0 = L_bv_mult0(pc[i], 4096);	// Q24
		a0 = bv_L_mac0(a0, pc[j], 4096);	// Q24
		a0 = L_bv_sub(a0, L_bv_shr(bv_L_deposit_h(fa_man[i - 1]), fa_exp[i - 1]));	// Q24
		fa_exp[i] = bv_norm_l(a0);
		fa_man[i] = intround(L_bv_shl(a0, fa_exp[i]));	// Q(8+fb_exp[i])

		a0 = L_bv_mult0(pc[i], 4096);	// Q24
		a0 = bv_L_msu0(a0, pc[j], 4096);	// Q24
		a0 = L_bv_add(a0, L_bv_shr(bv_L_deposit_h(fb_man[i - 1]), fb_exp[i - 1]));	// Q24
		fb_exp[i] = bv_norm_l(a0);
		fb_man[i] = intround(L_bv_shl(a0, fb_exp[i]));	// Q(8+fb_exp[i])
	}

	nd2 = (LPCO) / 2;

	/* ta[] and tb[] in Q(7+exp)               */
	/* ta[0] = fa[nab-1]; ta[i] = 2.0 * fa[j]; */
	/* tb[0] = fb[nab-1]; tb[i] = 2.0 * fb[j]; */
	ta_man[0] = fa_man[NAB - 1];
	ta_exp[0] = bv_add(fa_exp[NAB - 1], 1);
	tb_man[0] = fb_man[NAB - 1];
	tb_exp[0] = bv_add(fb_exp[NAB - 1], 1);
	for (i = 1, j = NAB - 2; i < NAB; ++i, --j) {
		ta_man[i] = fa_man[j];
		ta_exp[i] = fa_exp[j];
		tb_man[i] = fb_man[j];
		tb_exp[i] = fb_exp[j];
	}

	nf = 0;
	t_man = ta_man;
	t_exp = ta_exp;
	xroot = 0x7fff;
	ngrd = 0;
	xlow = bv_grid[0];	// Q15
	ylow = FNevChebP(xlow, t_man, t_exp, nd2);
	ind = 0;

	/* Root search loop */
	while (ngrd < (Ngrd - 1) && nf < LPCO) {

		ngrd++;
		xhigh = xlow;
		yhigh = ylow;
		xlow = bv_grid[ngrd];
		ylow = FNevChebP(xlow, t_man, t_exp, nd2);

		if (L_bv_mult(ylow, yhigh) <= 0) {

			/* Bisections of the interval containing a sign change */

			dx = xhigh - xlow;
			for (i = 1; i <= NBIS; ++i) {
				dx = bv_shr(dx, 1);
				xmid = bv_add(xlow, dx);
				ymid = FNevChebP(xmid, t_man, t_exp, nd2);
				if (L_bv_mult(ylow, ymid) <= 0) {
					yhigh = ymid;
					xhigh = xmid;
				} else {
					ylow = ymid;
					xlow = xmid;
				}
			}

			/*
			 * Linear interpolation in the bv_subinterval with a sign change
			 * (take care if yhigh=ylow=0)
			 */

			dx = bv_sub(xhigh, xlow);
			dy = bv_sub(ylow, yhigh);
			if (dy != 0) {
				sign = dy;
				dy = bv_abs_s(dy);
				exp = bv_norm_s(dy);
				dy = bv_shl(dy, exp);
				/* The maximum bv_grid distance is 1629 =>                                  */
				/* Maximum dx=1629/2^4=101.8125, i.e. 16384/101.8125=160.92~128 (7 bits) */
				/* However, due to the starting point for the search of a new root,      */
				/* xlow = xroot, 1 more bit of headroom for the division is required.    */
				dxdy = bv_div_s(bv_shl(dx, 6), dy);
				a0 = L_bv_mult(dxdy, ylow);
				a0 = L_bv_shr(a0, bv_sub(6, exp));
				x = intround(a0);
				if (sign < 0)
					x = bv_negate(x);
				xmid = bv_add(xlow, x);
			} else {
				xmid = bv_add(xlow, bv_shr(dx, 1));
			}

			/* acos mapping for New lsp component */
			while ((ind < 63) && (costable[ind] >= xmid))
				ind++;
			ind--;
			a0 = L_bv_mult(bv_sub(xmid, costable[ind]),
				       acosslope[ind]);
			x = intround(L_bv_shl(a0, 4));
			lsp[nf] = bv_add(x, bv_shl(ind, 9));
			++nf;

			/* Start the search for the roots of next polynomial at the estimated
			 * location of the root just found.  We have to catch the case that the
			 * two polynomials have roots at the same place to avoid getting stuck at
			 * that root.
			 */

			if (xmid >= xroot)
				xmid = xlow - dx;
			xroot = xmid;
			if (t_man == ta_man) {
				t_man = tb_man;
				t_exp = tb_exp;
			} else {
				t_man = ta_man;
				t_exp = ta_exp;
			}
			xlow = xmid;
			ylow = FNevChebP(xlow, t_man, t_exp, nd2);

		}
	}

	/* Check if all LSPs are found */
	if (bv_sub(nf, LPCO) < 0) {
		W16copy(lsp, old_lsp, LPCO);
	}

	return;
}

int16_t FNevChebP(int16_t x,	/* (i) Q15: value                     */
		 int16_t * t_man,	/* (i) Q7: mantissa of coefficients */
		 int16_t * t_exp,	/* (i): exponent fo cofficients     */
		 int16_t nd2)
{				/* (i): order                       */
	int16_t i;
	int16_t x2;
	int16_t b_man[NAB], b_exp[NAB];
	int16_t y;
	int32_t a0;

	x2 = x;			// 2x in Q14
	b_man[0] = t_man[nd2];
	b_exp[0] = t_exp[nd2];	// b[0] in Q(7+t_exp)
	a0 = L_bv_mult(x2, b_man[0]);
	a0 = L_bv_shr(a0, bv_sub(b_exp[0], 1));	// t*b[0] in Q23
	a0 = L_bv_add(a0, L_bv_shr(bv_L_deposit_h(t_man[nd2 - 1]), t_exp[nd2 - 1]));	// c[nd2-1] + t*b[0] in Q23
	b_exp[1] = bv_norm_l(a0);
	b_man[1] = intround(L_bv_shl(a0, b_exp[1]));	// b[1] = c[nd2-1] + t * b[0]

	for (i = 2; i < nd2; i++) {
		a0 = L_bv_mult(x2, b_man[i - 1]);
		a0 = L_bv_shr(a0, bv_sub(b_exp[i - 1], 1));	// t*b[i-1] in Q23
		a0 = L_bv_add(a0, L_bv_shr(bv_L_deposit_h(t_man[nd2 - i]), t_exp[nd2 - i]));	// c[nd2-i] + t*b[i-1] in Q23
		a0 = L_bv_sub(a0, L_bv_shr(bv_L_deposit_h(b_man[i - 2]), b_exp[i - 2]));	// c[nd2-i] + t*b[i-1] - b[i-2] in Q23
		b_exp[i] = bv_norm_l(a0);
		b_man[i] = intround(L_bv_shl(a0, b_exp[i]));	// b[i] = c[nd2-i] - b[i-2] + t * b[i-1]
	}

	a0 = L_bv_mult(x, b_man[nd2 - 1]);
	a0 = L_bv_shr(a0, b_exp[nd2 - 1]);	// x*b[nd2-1] in Q23
	a0 = L_bv_add(a0, L_bv_shr(bv_L_deposit_h(t_man[0]), t_exp[0]));	// c[0] + x*b[nd2-1] in Q23
	a0 = L_bv_sub(a0, L_bv_shr(bv_L_deposit_h(b_man[nd2 - 2]), b_exp[nd2 - 2]));	// c[0] + x*b[nd2-1] - b[nd2-2] in Q23

	y = intround(L_bv_shl(a0, 6));	// Q13

	return y;		// Q13
}
