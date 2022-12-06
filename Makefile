mkfile_path := $(abspath $(lastword $(MAKEFILE_LIST)))
current_dir := $(notdir $(patsubst %/,%,$(dir $(mkfile_path))))

compile:
	gcc mib_creator.c -o mib_creator -Wall
run:	
	./mib_creator -i mibTalepFormu.ods
valgrind:
	valgrind --leak-check=full          --show-leak-kinds=all          --track-origins=yes          --verbose          --log-file=valgrind-out.txt          ./mib_creator -i mibTalepFormu.ods
clean:
	rm mib_creator	
	rm *.csv