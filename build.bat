..\cw\mwcceppc.exe -i . -I- -i ../k_stdlib -Cpp_exceptions off -enum int -Os -use_lmw_stmw on -fp hard -rostr -sdata 0 -sdata2 0 -c -o main.o main.cpp

..\Kamek\bin\Debug\net6.0\Kamek main.o -static=0x80001B00 -input-dol=main_original.dol -output-dol=main.dol -externals=externals-fsa.txt