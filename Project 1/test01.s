# Short test case for your project.
#
# Note that this is by no means a comprehensive test!
#

		.text
		addiu	$a0,$0,5
		addiu	$a1,$0,9
		jal	Mystery


Mystery:
		addiu	$v0,$0,0
		jal 	Spooky
Spooky:
		addiu	$v0,$0,0
Loop:
		beq	$a0,$0,Done
		addu	$v0,$v0,$a1
		addiu	$a0,$a0,-1
		j Loop
Done:	
		jr		$ra