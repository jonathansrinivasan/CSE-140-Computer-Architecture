.data 
str0: .asciiz "\nWelcome to BobCat Candy, home to the famous BobCat Bars!"
str1: .asciiz "\nPlease enter the price of a Bobcat Bar: "
str2: .asciiz "\nPlease enter the number of wrappers needed to exchange for a new bar: "
str3: .asciiz "\nHow much money do you have: "
str4: .asciiz "\nGood let me run the number!"
str5: .asciiz "\nYou first buy this many bars: "
str6: .asciiz "\nThen you buy this many bars: "
str7: .asciiz "\nYou will buy a maximum of this many bars: "
# Declare any necessary data here



.text

main:
		#This is the main program.
		#It first asks user to enter the price of each BobCat Bar.
		#It then asks user to enter the number of bar wrappers needed to exchange for a new bar.
		#It then asks user to enter how much money he/she has.
		#It then calls maxBars function to perform calculation of the maximum BobCat Bars the user will receive based on the information entered. 
		#It then prints out a statement about the maximum BobCat Bars the user will receive.
		
		addi $sp, $sp -4	# Feel free to change the increment if you need for space.
		sw $ra, 0($sp)
		# Implement your main here
		li $v0, 4
		la $a0, str0
		syscall
		
		
		li $v0, 4
		la $a0, str1
		syscall
		
		li $v0, 5
		syscall
		
		move $t0, $v0 #a0 = price
		
		li $v0, 4
		la $a0, str2
		syscall
		
		li $v0, 5
		syscall
		
		move $t1, $v0 #a1 = wrappers
		
		li $v0, 4
		la $a0, str3
		syscall
		
		li $v0, 5
		syscall
		
		move $t2, $v0 #a2 = price
		
		li $v0, 4
		la $a0, str4
		syscall
		
		#Load arguments for maxBars
		add $a0, $t0, $zero
		add $a1, $t1, $zero
		add $a2, $t2, $zero
		

		jal maxBars 	# Call maxBars to calculate the maximum number of BobCat Bars

		# Print out final statement here
		li $v0, 4
		la $a0, str7
		syscall
		
		li $v0, 1
		move $a0, $v1
		syscall


		j end			# Jump to end of program



maxBars:
		# This function calculates the maximum number of BobCat Bars.
		# It takes in 3 arguments ($a0, $a1, $a0) as n, price, and money. It returns the maximum number of bars
		addi $sp, $sp, -8 #Create space to save $s0
		sw $ra, 0($sp)
		
		div $a2, $a0 #bars = money/price
		mflo $s0
		
		add $s1, $s0, $zero
		add $s2, $a1, $zero #save n as global variable
		
		#Print the first buy
		li $v0, 4
		la $a0, str5
		syscall
		
		li $v0, 1
		move $a0, $s0
		syscall
		
		sw $s0, 4($sp)
		jal newBars 	# Call a helper function to keep track of the number of bars.
		
		lw $s0, 4($sp)
		lw $ra, 0($sp)
		
		add $v1, $s0, $v1 #get total bars
		addi $sp, $sp, 8
		jr $ra
		# End of maxBars

newBars:
		# This function calculates the number of BobCat Bars a user will receive based on n.
		# It takes in 2 arguments ($a0, $a1) as number of bars so far and n.
		addi $sp, $sp, -8
		sw $ra, 0($sp)
		
		add $t0, $s0, $zero
		add $t1, $s1, $zero
		
		div $t1, $s2
		mflo $t2 # Quotient -- newBars
		mfhi $t1 # Remainder -- wrappers
		
		#Print intermediate buys
		li $v0, 4
		la $a0, str6
		syscall
		
		li $v0, 1
		move $a0, $t2
		syscall
		
		
		add $s0, $t2, $zero #For callback
		add $s1, $t1, $t2 #callback
		
		sw $t2, 4($sp) #save bars if callback
		add $v0, $t2, $zero #get t2 recursively
		
		bne $t2, $zero, newBars
		
		lw $t2, 4($sp) #load back t2
		lw $ra, 0($sp)
		add $v1, $t2, $v1 # total new bars += bars recursive
		
		addi $sp, $sp, 8
		
		jr $ra
		# End of newBars

end: 
		# Terminating the program
		lw $ra, 0($sp)
		addi $sp, $sp 4
		li $v0, 10 
		syscall
