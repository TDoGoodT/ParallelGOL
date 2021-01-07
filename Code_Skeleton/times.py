from os import system
in_files = {'small', 'mid', 'big'}
exec_file = './GameOfLife'
make = 'make clean && make'
gens = 100
system('rm -rf results.csv')
for fname in in_files:
	print ("Testing " + fname)
	for i in range(1,101):
		print ('running command: "' + exec_file + ' ' + fname + '.txt' + ' '  + str(gens) + ' ' +str(i) + ' n n' + '"')
		system(exec_file + ' ' + fname + '.txt' + ' '  + str(gens) + ' ' +str(i) + ' n n')
