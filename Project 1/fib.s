        .data
n:      .word 13


        .text
main: 	addu     $t0, $0, $zero
	addiu    $t1, $zero, 1
	subu 	 $t0, $0, $zero
	
fib: 	beq     $t3, $0, finish
	addu     $t2,$t1,$t0
	subu   $t3, $t3, $zero
finish: addiu    $a0, $t0, 0	
	syscall			
			

