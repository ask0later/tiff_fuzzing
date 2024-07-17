TARGET = target

CC  = /home/ask0later/AFL__/AFL/afl-gcc
CXX = /home/ask0later/AFL__/AFL/afl-g++

PATH_TO_PROGRAM = /home/ask0later/Desktop/afl_test/$(TARGET)

PATH_TO_AFL_INCLUDE = -I./../../AFL__/AFLplusplus/include/

DEFAULT_ENV_VARS = AFL_SKIP_CPUFREQ=1 AFL_I_DONT_CARE_ABOUT_MISSING_CRASHES=1 AFL_MAP_SIZE=65536
MUTAT_ENV_VARS = AFL_CUSTOM_MUTATOR_LIBRARY="/home/ask0later/Desktop/afl_test/custom_mutator/mutator.so"

# only user-defined mutator
# AFL_CUSTOM_MUTATOR_ONLY=1


# *****


# afl-fuzz -i testcase_dir -o findings_dir /home/ask0later/Desktop/alf_test/main @@

# -Q QEMU (only binary file)
# -n blind fuzzer (random input data)
# -d quick & dirty results
# -f mutated data

TESTCASE_DIR = testcase_dir
FINDINGS_DIR = findings_dir

# *****

#default mutator
fuzz: $(TARGET)
	$(DEFAULT_ENV_VARS) afl-fuzz -i $(TESTCASE_DIR) -o $(FINDINGS_DIR) $(PATH_TO_PROGRAM) @@

$(TARGET): 
	echo : where is file.c?

# custom mutator only
cust_fuzz:
	gcc -shared -Wall -fPIC -O3 $(PATH_TO_AFL_INCLUDE) custom_mutator/mutator.c -o custom_mutator/mutator.so
	AFL_CUSTOM_MUTATOR_ONLY=1 $(MUTAT_ENV_VARS) $(DEFAULT_ENV_VARS) afl-fuzz -i $(TESTCASE_DIR) -o $(FINDINGS_DIR) $(PATH_TO_PROGRAM) @@	
 
# custom mutator
cust_and_default_fuzz:
	gcc -shared -Wall -fPIC -O3 $(PATH_TO_AFL_INCLUDE) custom_mutator/mutator.c -o custom_mutator/mutator.so
	$(MUTAT_ENV_VARS) $(DEFAULT_ENV_VARS) afl-fuzz -i $(TESTCASE_DIR) -o $(FINDINGS_DIR) $(PATH_TO_PROGRAM) @@

parallel_fuzz:
	echo please look at the makefile 
#  write these commands in the terminal
#  afl-fuzz -i testcase_dir -o findings_dir -M fuzzer_01 /home/ask0later/Desktop/afl_test/main @@

#  afl-fuzz -i testcase_dir -o findings_dir -S fuzzer_02 /home/ask0later/Desktop/afl_test/main @@

#  afl-fuzz -i testcase_dir -o findings_dir -S fuzzer_03 /home/ask0later/Desktop/afl_test/main @@ 

clean:
	rm -rf *.o $(FINDINGS_DIR) custom_mutator/*.so
	mkdir findings_dir
