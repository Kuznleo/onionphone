/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/***********************************************************************
Copyright (c) 2006-2010, Skype Limited. All rights reserved. 
Redistribution and use in source and binary forms, with or without 
modification, (subject to the limitations in the disclaimer below) 
are permitted provided that the following conditions are met:
- Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.
- Redistributions in binary form must reproduce the above copyright 
notice, this list of conditions and the following disclaimer in the 
documentation and/or other materials provided with the distribution.
- Neither the name of Skype Limited, nor the names of specific 
contributors, may be used to endorse or promote products derived from 
this software without specific prior written permission.
NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED 
BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND 
CONTRIBUTORS ''AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND 
FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE 
COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF 
USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON 
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
***********************************************************************/

#include "SKP_Silk_main_FIX.h"

void SKP_Silk_LTP_analysis_filter_FIX(int16_t * LTP_res,	/* O:   LTP residual signal of length NB_SUBFR * ( pre_length + subfr_length )  */
				      const int16_t * x,	/* I:   Pointer to input signal with at least max( pitchL ) preceeding samples  */
				      const int16_t LTPCoef_Q14[LTP_ORDER * NB_SUBFR],	/* I:   LTP_ORDER LTP coefficients for each NB_SUBFR subframe                   */
				      const int pitchL[NB_SUBFR],	/* I:   Pitch lag, one for each subframe                                        */
				      const int32_t invGains_Qxx[NB_SUBFR],	/* I:   Inverse quantization gains, one for each subframe                       */
				      const int Qxx,	/* I:   Inverse quantization gains Q domain                                     */
				      const int subfr_length,	/* I:   Length of each subframe                                                 */
				      const int pre_length	/* I:   Length of the preceeding samples starting at &x[0] for each subframe    */
    )
{
	const int16_t *x_ptr, *x_lag_ptr;
	int16_t Btmp_Q14[LTP_ORDER];
	int16_t *LTP_res_ptr;
	int k, i, j;
	int32_t LTP_est;

	x_ptr = x;
	LTP_res_ptr = LTP_res;
	for (k = 0; k < NB_SUBFR; k++) {

		x_lag_ptr = x_ptr - pitchL[k];
		for (i = 0; i < LTP_ORDER; i++) {
			Btmp_Q14[i] = LTPCoef_Q14[k * LTP_ORDER + i];
		}

		/* LTP analysis FIR filter */
		for (i = 0; i < subfr_length + pre_length; i++) {
			LTP_res_ptr[i] = x_ptr[i];

			/* Long-term prediction */
			LTP_est =
			    SKP_SMULBB(x_lag_ptr[LTP_ORDER / 2], Btmp_Q14[0]);
			for (j = 1; j < LTP_ORDER; j++) {
				LTP_est =
				    SKP_SMLABB_ovflw(LTP_est,
						     x_lag_ptr[LTP_ORDER / 2 -
							       j], Btmp_Q14[j]);
			}
			LTP_est = SKP_RSHIFT_ROUND(LTP_est, 14);	// round and -> Q0

			/* Subtract long-term prediction */
			LTP_res_ptr[i] =
			    (int16_t) SKP_SAT16((int32_t) x_ptr[i] - LTP_est);

			/* Scale residual */
			if (Qxx == 16) {
				LTP_res_ptr[i] =
				    SKP_SMULWB(invGains_Qxx[k], LTP_res_ptr[i]);
			} else {
				LTP_res_ptr[i] =
				    (int16_t)
				    SKP_CHECK_FIT16(SKP_RSHIFT64
						    (SKP_SMULL
						     (invGains_Qxx[k],
						      LTP_res_ptr[i]), Qxx));
			}

			x_lag_ptr++;
		}

		/* Update pointers */
		LTP_res_ptr += subfr_length + pre_length;
		x_ptr += subfr_length;
	}
}
