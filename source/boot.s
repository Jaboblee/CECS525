/* boot.s - boot for the CECS 525 Raspberry PI kernel
   by Eugene Rockey Copyright 2015 All Rights Reserved
   debug everything that needs debugging
   Add, remove, modify, preserve in order to fullfill project requirements. */
   
    

.set	Usr32md,	0x10		/* ARM modes */
.set	FIQ32md,	0x11
.set	IRQ32md,	0x12
.set	Svc32md,	0x13
.set	Abt32md,	0x17
.set	Und32md,	0x1b
.set	Sys32md,	0x1f

.set	IrqFiq,		0xC0        /* IRQ and FIQ bits in the CPSR */

.set SYS_Stack, 0x5000          /* Stack addresses for the different modes employed */
.set IRQ_Stack, 0x6000
.set SVC_Stack, 0x7000

.section ".data"                /* variables that can be used between assembly and C */
.global invar, outvar
.set invar, 0
.set outvar, 0

.section ".text"
.global vectors
/***********************************Vectors***************************************/
vectors: 	B	reset
		B	undef
		B	swi
		B	pabt
		B	dabt
		B	na
		B	irq
		B	fiq
/**********************************************************************************/
.org 0x8000
.global reset
/************************************Reset*************************************/
reset:		MOV 	r2, #Svc32md | IrqFiq 	/*Initialize supervisor stack pointer*/
		MSR 	cpsr_c, r2
        	LDR	r13, =SVC_Stack

		MOV 	r2, #IRQ32md	 	/*Initialize irq stack pointer*/
		MSR 	cpsr_c, r2
		LDR	r13, =IRQ_Stack

		MOV	r2, #Sys32md		/*Initialize system stack pointer*/
		MSR	cpsr_c, r2
		LDR	r13, =SYS_Stack

		MRC	p15, 0, r0, c1, c0,2	/*initialize VFP11 math coprocessor*/
		ORR	r0, r0, #0x300000
		ORR	r0, r0, #0xC00000
		MCR	p15, 0, r0, c1, c0, 2
		MOV	r0, #0x40000000
        	FMXR	fpexc, r0		/*enable the VFP11 math coprocessor */

		B	kernel_main		/* run Tiny OS in system mode */
/***********************************Undefined****************************/		
undef:		B	reset
		STMFD	r13!,{r0-r12, r14}
		NOP                         /*How would you handle this exception?*/
        LDMFD	r13!,{r0-r12, pc}^
/**************************************Software Interrupt******************/		
swi:		B	reset
		STMFD	r13!,{r0-r12, r14}
		NOP                         /*How would you handle this exception?*/
        LDMFD	r13!,{r0-r12, pc}^
/************************************Prefetch Abort**************************/
pabt:		B	reset
		SUB	r14, r14, #4
		STMFD	r13!,{r0-r12, r14}
		NOP                         /*How would you handle this exception?*/
        LDMFD	r13!,{r0-r12, pc}^
/*************************************Data Abort*******************************/	
dabt:		B	reset
		SUB	r14, r14, #8
		STMFD	r13!,{r0-r12, r14}
		NOP                         /*How would you handle this exception?*/
        LDMFD	r13!,{r0-r12, pc}^
/**************************************Reserved**********************************/
na:		B	reset                   /*How would you handle this exception?*/
/**************************************Interrupt Request**************************/
irq:		SUB	r14, r14, #4
		STMFD	r13!,{r0-r12, r14}	/*IRQ Handler, first save registers and return address to stack*/
		BL	irq_handler             /*This one is handled*/
		LDMFD	r13!,{r0-r12, pc}^	/*Restore registers from stack and return to application*/
/**************************************Fast Interrupt Request**********************/
fiq:		B	reset
		SUB	r14, r14, #4
		STMFD	r13!,{r0-r7, r14}			
		NOP                         /*How would you handle this exception?*/
		LDMFD	r13!,{r0-r7, pc}^	
/**********************************************************************************/

.global reboot
reboot:
		B	reset

.global enable_arm_irq
enable_arm_irq:
		MRS	r1, cpsr
		BIC	r1, r1, #0x80
		MSR	cpsr_c, r1
		MOV	pc, lr
		
.global disable_arm_irq
disable_arm_irq:
		MRS	r1,cpsr
		ORR	r1, r1, #0x80
		MSR	cpsr_c, r1
		MOV	pc, lr

.global enable_arm_fiq
enable_arm_fiq:
		MRS	r1, cpsr
		BIC	r1, r1, #0x40
		MSR	cpsr_c, r1
		MOV	pc, lr
		
.global disable_arm_fiq
disable_arm_fiq:
		MRS	r1, cpsr
		ORR	r1, r1, #0x40
		MSR	cpsr_c, r1
		MOV	pc, lr

.global vfp11_add
vfp11_add:
		FMSRR	{s1, s2}, r0, r1
		FADDS	s2, s2, s1	
		FMRS	r0, s2
		MOV	pc, lr
		
.global vfp11_sub
vfp11_sub:
		FMSRR	{s1, s2}, r0, r1
		FSUBS	s2, s1, s2	
		FMRS	r0, s2
		MOV	pc, lr

.global vfp11_mul
vfp11_mul:
		FMSRR	{s1, s2}, r0, r1
		FMULS	s2, s2, s1	
		FMRS	r0, s2
		MOV	pc, lr

.global vfp11_div
vfp11_div:
		FMSRR	{s1, s2}, r0, r1
		FDIVS	s2, s1, s2	
		FMRS	r0, s2
		MOV	pc, lr
		
.global vfp11_sqrt
vfp11_sqrt:
		FMSR	s1, r0
		FSQRTS	s2, s1	
		FMRS	r0, s2
		MOV	pc, lr

		
.global addition
.global subtraction
.global multiplication
.global division
.global remaind

addition:
			//.fnstart
			add r0, r0, r1
			//bx lr
			mov pc, lr
			//.fnend	
			
subtraction:
			//.fnstart
			sub r0, r0, r1
			mov pc, lr
			//.fnend
			
			
multiplication:
			//.fnstart
			mul r5, r0, r1
			mov r0, r5
			mov pc, lr
			//.fnend	

division:
			//.fnstart
			mul r5, r0, r1
			cmp r5, #0
			movlt r6, #32
			cmp r0, #0
			neglt r0, r0
			cmp r1, #0
			neglt r1, r1
			mov r5, #0
			mov r4, r0
check1:		cmp r4, r1
			blt check2
			sub r4, r4, r1
			add r5, #1
			bal check1
check2:		cmp r6, #32
			negeq r5, r5
			mov r0, r5
			mov pc, lr
			//.fnend
			
remaind:
			//.fnstart
			cmp r0, #0
			neglt r0, r0
			cmp r1, #0
			neglt r1, r1
			mov r4, r0
check3:		cmp r4, r1
			blt check4
			sub r4, r4, r1
			bal check3
check4:		mov r0, r4
			mov pc, lr
			//.fnend	

			


			

			
					



