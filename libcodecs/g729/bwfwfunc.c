/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
   ITU-T G.729 Annex C+ - Reference C code for floating point
                         implementation of G.729 Annex C+
                         (integration of Annexes B, D and E)
                          Version 2.1 of October 1999
*/
/*
 File : BWFWFUNC.C
 */

#include <stdio.h>
#include <math.h>
#include "ld8k.h"
#include "ld8cp.h"
#include "tabld8cp.h"

/* -------------------------------------------------------------------- */
/*                          TST_BWD_DOMINANT                            */
/* -------------------------------------------------------------------- */
/* Test if backward mode is dominant :                                  */
/* Nbre of Backward frames > 4 * Nbre of Forward frames                 */
/* It is used for the choice of the perceptual weighting filter         */
/* Backward dominant : music or stationnary background noise :          */
/*         Quantized filter (30 coef.) + strong weighting               */
/* No backward dominant => Non Stationnary signal (mainly speech) :     */
/*         Unquantized filter (10 coef.) + normal weighting             */
/* -------------------------------------------------------------------- */
static int count_bwd2 = 0, count_fwd2 = 0;

void tst_bwd_dominant(int *bwd_dominant,	/* O   Dominant backward mode indication */
		      int mode)
{				/* I   Selected LPC mode */

	int tmp, count_all;

	if (mode == 0)
		count_fwd2++;
	else
		count_bwd2++;

	count_all = count_bwd2 + count_fwd2;

	if (count_all == 100) {
		count_all = count_all >> 1;
		count_bwd2 = count_bwd2 >> 1;
		count_fwd2 = count_fwd2 >> 1;
	}

	*bwd_dominant = 0;
	if (count_all >= 10) {
		tmp = count_fwd2 << 2;
		if (count_bwd2 > tmp)
			*bwd_dominant = 1;
	}

	return;
}

void int_bwd(float * a_bwd,	/* I/O   LPC backward filter */
	     float * prev_filter,	/* I previous frame filter */
	     float * C_int	/* I/O interpolation coefficient */
    )
{

	int i;
	float tmp1, tmp2;
	float *pa_bwd;
	float C_int_loc;

	pa_bwd = a_bwd + M_BWDP1;

	/* Calculate the interpolated filters  */
	/* ----------------------------------  */
	C_int_loc = *C_int - (float) 0.1;
	if (C_int_loc < 0)
		C_int_loc = 0;

	for (i = 0; i < M_BWDP1; i++) {
		tmp1 = pa_bwd[i] * ((float) 1. - C_int_loc);
		tmp2 = prev_filter[i] * C_int_loc;
		pa_bwd[i] = tmp1 + tmp2;
	}

	for (i = 0; i < M_BWDP1; i++) {
		a_bwd[i] = (float) 0.5 *(pa_bwd[i] + prev_filter[i]);
	}

	*C_int = C_int_loc;
	return;
}

/* ---------------------------------------------------------------------- */
/*                                 ENER_DB                                */
/*                                                                        */
/*                        COMPUTATION OF THE ENERGY                       */
/*                                                                        */
/* ---------------------------------------------------------------------- */
float ener_dB(float * synth, int L)
{

	int i;
	float energy;
	float tmp, edB;
	int32_t k, Ltmp;
	int n;

	for (i = 0, energy = (float) 0.0001; i < L; i++) {
		energy += synth[i] * synth[i];
	}
	edB = (float) log10(energy);
	tmp = edB * INV_LOG2;
	n = (int)tmp;
	if (tmp >= (float) 4.) {
		if (energy > (float) 2147483647.)
			energy = (float) 93.1814;
		else {
			k = (int32_t) energy;
			Ltmp = -(1L << (n - 4));
			k &= Ltmp;
			tmp = (float) k;
			energy = (float) 10. *(float) log10(tmp);
		}
	} else
		energy = (float) 0.005;

	return (energy);
}

/* ----------------------------------------------------------------------- */
/*                                                                         */
/*                              HYBRID WINDOW                              */
/*                              (as in G728)                               */
/*                                                                         */
/*               1st part (0..35) is sine(i*c) where c = 0.047783          */
/*               2nd part (36....) is b*exp((i-35)*log(a))                 */
/*               where b=sine(35*c)                                        */
/*               and a=0.9928337491 is such as a^^40 = 0.75                */
/*               a^^(2*L_FRAME) = 0.75^^4 = 0.31640625                     */
/*                                                                         */
/* ------------------------------------------------------------------------ */

/*------------------------------------------------------------------------------*/
/* Compute the autocorrelation of speech using hybrid window    (as in G.728)   */
/*------------------------------------------------------------------------------*/
void autocorr_hyb_window(float * x,	/* (in)     synthetized speech signal */
			 float * r_bwd,	/* (out)    Autocorrelations    */
			 float * rexp	/* (in/out) */
    )
{
	float tmp;
	int i, n, k;
	float w_s[L_ANA_BWD];

	for (n = L_ANA_BWD_M1, k = 0; n >= 0; n--, k++)
		w_s[n] = x[n] * hw[k];

	for (i = 0; i <= M_BWD; i++) {
		tmp = 0;
		for (n = M_BWD; n < N1; n++)
			tmp += w_s[n] * w_s[n - i];
		rexp[i] = W_FACT * rexp[i] + tmp;
	}

	for (i = 0; i <= M_BWD; i++) {
		r_bwd[i] = rexp[i];
		for (n = N1; n < L_ANA_BWD; n++)
			r_bwd[i] += w_s[n] * w_s[n - i];
	}
	return;

}				/* end of autocorr_hyb_windowing() */

/* ---------------------------------------------------------------------- */
/*                                                                        */
/*                             LAG_WINDOW_BWD                             */
/*                                                                        */
/*      Fixed-point Lag Windowing for backward analysis correlations      */
/*           (values of lag_h_bwd and lag_l_bwd in tab_ld8k.h)            */
/*                                                                        */
/* ---------------------------------------------------------------------- */
void glag_window_bwd(float * r_bwd)
{

	int i;

	for (i = 1; i <= M_BWD; i++) {
		/* !!! la fenetre debute en 0 pour un terme != de 1 */
		r_bwd[i] *= lag_bwd[i - 1];
	}

	if (r_bwd[0] < (float) 1.0)
		r_bwd[0] = (float) 1.0;
	return;
}
