	.text
main:
	jal	swap
	li	$v0,1	# print n1 and n2; should be 27 and 14

	syscall
	li	$v0,11
	li	$a0,' '
	syscall
	li	$v0,1

	syscall
	li	$v0,11
	li	$a0,'\n'
	syscall
	li	$v0,10	# exit
	syscall

swap:	# your code goes here
	
	addi $sp, $sp, -4 #allocate space
	
	lw $t0, 0($a0)
	sw $t0, 0($sp)
	
	lw $t0, 0($a1)
	sw $t0, 0($a0)
	
	lw $t0, 0($sp)
	sw $t0, 0($a1)
	
	addi $sp, $sp, 4
	 
	jr $ra
	
	.data
