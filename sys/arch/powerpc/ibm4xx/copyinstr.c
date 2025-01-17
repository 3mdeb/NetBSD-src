/*	$NetBSD: copyinstr.c,v 1.15 2022/09/12 08:02:44 rin Exp $	*/

/*
 * Copyright 2001 Wasabi Systems, Inc.
 * All rights reserved.
 *
 * Written by Eduardo Horvath and Simon Burge for Wasabi Systems, Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed for the NetBSD Project by
 *      Wasabi Systems, Inc.
 * 4. The name of Wasabi Systems, Inc. may not be used to endorse
 *    or promote products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY WASABI SYSTEMS, INC. ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL WASABI SYSTEMS, INC
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/cdefs.h>
__KERNEL_RCSID(0, "$NetBSD: copyinstr.c,v 1.15 2022/09/12 08:02:44 rin Exp $");

#include <sys/param.h>
#include <uvm/uvm_extern.h>
#include <powerpc/ibm4xx/spr.h>
#include <machine/pcb.h>

int
copyinstr(const void *udaddr, void *kaddr, size_t len, size_t *done)
{
	struct pmap *pm = curproc->p_vmspace->vm_map.pmap;
	size_t resid;
	int rv, msr, pid, data, ctx;
	struct faultbuf env;

	if (__predict_false(len == 0)) {
		if (done)
			*done = 0;
		return 0;
	}

	if ((rv = setfault(&env))) {
		curpcb->pcb_onfault = NULL;
		if (done)
			*done = 0;
		return rv;
	}

	if (!(ctx = pm->pm_ctx)) {
		/* No context -- assign it one */
		ctx_alloc(pm);
		ctx = pm->pm_ctx;
	}

	resid = len;
	__asm volatile(
		"mtctr %3;"			/* Set up counter */
		"mfmsr %0;"			/* Save MSR */
		"li %1,0x20;"
		"andc %1,%0,%1; mtmsr %1;"	/* Disable IMMU */
		"isync;"
		MFPID(%1)			/* Save old PID */

		"1: "
		MTPID(%4)			/* Load user ctx */
		"isync;"
		"lbz %2,0(%5); addi %5,%5,1;"	/* Load byte */
		"sync;"
		MTPID(%1)
		"isync;"
		"stb %2,0(%6); dcbst 0,%6; addi %6,%6,1;"
						/* Store kernel byte */
		"or. %2,%2,%2;"
		"sync;"
		"bdnzf 2,1b;"			/* while(ctr-- && !zero) */

		MTPID(%1)			/* Restore PID, MSR */
		"mtmsr %0;"
		"isync;"
		"mfctr %3;"			/* Restore resid */
		: "=&r" (msr), "=&r" (pid), "=&r" (data), "+r" (resid)
		: "r" (ctx), "b" (udaddr), "b" (kaddr));

	curpcb->pcb_onfault = NULL;
	if (done)
		*done = len - resid;
	if (resid == 0 && (char)data != '\0')
		return ENAMETOOLONG;
	else
		return 0;
}
