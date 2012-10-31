/*
 
 This file is part of FFTS -- The Fastest Fourier Transform in the South
  
 Copyright (c) 2012, Anthony M. Blake <amb@anthonix.com>
 Copyright (c) 2012, The University of Waikato 
 
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
 	* Redistributions of source code must retain the above copyright
 		notice, this list of conditions and the following disclaimer.
 	* Redistributions in binary form must reproduce the above copyright
 		notice, this list of conditions and the following disclaimer in the
 		documentation and/or other materials provided with the distribution.
 	* Neither the name of the organization nor the
	  names of its contributors may be used to endorse or promote products
 		derived from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL ANTHONY M. BLAKE BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#include "ffts_nd.h"


void ffts_free_nd(ffts_plan_t *p) {
	free(p->Ns);
	free(p->Ms);

	int i;
	for(i=0;i<p->rank;i++) {
		ffts_free(p->plans[i]);
	}

	free(p->plans);
	free(p->buf);
	free(p);
}

void ffts_transpose(void *d, int n1, int n2) {
	uint64_t *v = (uint64_t *)d;
	
	int i, j;
	for (i = 0; i<n2; i++) {
		for (j = i+1; j<n1; j++) {
  		uint64_t temp = v[j*n2 + i];
  		v[j*n2 + i] = v[i*n1 + j];
  		v[i*n1 + j] = temp;
//		uint64_t temp = v[i*n2 + j];
//		v[i*n2 + j] = v[j*n1 + i];
//		v[j*n1 + i] = temp;
		}
	}

}

void ffts_execute_nd(ffts_plan_t *p, const void *  in, void *  out) {

	printf("Exe ND\n");
	uint64_t *din = in;
	uint64_t *buf0, *buf1;
	
	if(p->rank & 1) {
		buf1 = out;
		buf0 = p->buf;
	}else{
		buf1 = p->buf;
		buf0 = out;
	}
	
	size_t i,j;
	for(i=0;i<p->Ms[0];i++) {
		ffts_execute(p->plans[0], din + (i * p->Ns[0]), buf1 + (i * p->Ns[0]));	
	}
	ffts_transpose(buf1, p->Ms[0], p->Ns[0]);	

	for(i=1;i<p->rank;i++) {
		printf("t %zu\n", i);	
		for(j=0;j<p->Ms[i];j++) { 
			ffts_execute(p->plans[i], buf1 + (j * p->Ns[i]), buf0 + (j * p->Ns[i]));	
		}
		ffts_transpose(buf0, p->Ms[i], p->Ns[i]);	

		void *b = buf0;
		buf0 = buf1;
		buf1 = b;
	}
}

ffts_plan_t *ffts_init_nd(int rank, size_t *Ns, int sign) {
	size_t vol = 1;

	ffts_plan_t *p = malloc(sizeof(ffts_plan_t));

	p->transform = &ffts_execute_nd;
	p->destroy = &ffts_free_nd;

	p->rank = rank;
	p->Ns = malloc(sizeof(size_t) * rank);
	p->Ms = malloc(sizeof(size_t) * rank);
	p->plans = malloc(sizeof(ffts_plan_t **) * rank);
	printf("rank = %d\n", rank);	
	int i;
	for(i=0;i<rank;i++) {
		p->Ns[i] = Ns[i];
		printf("N %zu\n", p->Ns[i]);
		vol *= Ns[i];	
	}
	printf("VOL %zu\n", vol);
	p->buf = malloc(sizeof(float) * 2 * vol);

	for(i=0;i<rank;i++) {
		p->Ms[i] = vol / p->Ns[i];
		printf("M N %zu %zu\n", p->Ms[i], p->Ns[i]);
		p->plans[i] = ffts_init_1d(p->Ns[i], sign); 
	}

	return p;
}


ffts_plan_t *ffts_init_2d(size_t N1, size_t N2, int sign) {
	size_t Ns[2];
	Ns[0] = N1;
	Ns[1] = N2;
	return ffts_init_nd(2, Ns, sign);
}
