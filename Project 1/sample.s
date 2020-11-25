# Short test case for your project.
#
# Note that this is by no means a comprehensive test!
#

		.text
		addu	$a0,$0,$zero
		addiu	$a1,$0,9
		subu	$a2, $s1, $s6
		sll	$a3, $s1, 4
		srl	$s7, $a3, 8
		and	$v1, $v1, $v1
		andi	$a0, $a1, 10
		or	$a1, $a1, $a1
		ori	$s1, $s2, 5
		lui 	$s3, 7
		slt	$s4, $s5, $s6
		beq	$v1, $v1, Mystery
		bne	$v1, $v0, Spooky
		j 	Mystery 
		jal	Spooky
		jr 	$a1
		sw	$t0, 4($sp)
		lw	$t0, 4($sp)
		addi	$0,$0,0 #unsupported instruction, terminate
		

Mystery:
		addiu	$v0,$0,0
Spooky:
		addiu	$v0,$0,0
Loop:
		beq	$a0,$0,Done
		addu	$v0,$v0,$v1
		addiu	$a0,$a0,-1
Done:	
		addiu	$v0,$0, 1
		add 	$zero, $zero, $zero
