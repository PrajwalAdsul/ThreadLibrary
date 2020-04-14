![ThreadLibraryLogo](https://github.com/PrajwalAdsul/ThreadLibrary/blob/master/ThreadLibraryLogo.png)
### Thread Library Implementation in C language 
Akhil Potula (111703073), Prajwal Adsul (111703046)

Tried library in all the 3 modes
1) one-one
  For each user thread there is one kernel thread
2) many - one
  For all user theads there is one kernel thread.
  Implemented using setjmp.h
3) many - many
  For all user threads there are fixed number of kernel threads

Find each library code in respective libraries folder

For running test files, go to each library folder and then to testfiles folder
select the test file you want to test and type following commands
### cc -c test.c
### cc -c threads.c
### cc test.o threads.o -o program
### ./program

All instructed functions are done in all 3 modes. thread_kill and thread_exit not done in many-many mode.
