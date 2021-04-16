#include <fstream>
#include <unistd.h>
#include <semaphore.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <stdlib.h>
#include <iostream>
#include <cstring>
#include <string>
#include <regex>

using namespace std;

char g_sharedMemory[] = "SharedMemoryMDC";
char g_semaphore[] = "SemaphoreMDC";

int main()
{

	unsigned int mode;
	int ShmDescriptor;
	int SHM_SIZE;

	sem_t *SemDescriptor;
	caddr_t MemoryPointer;

	mode = S_IRWXU | S_IRWXG;

	/* Open the shared memory object */

	if ((ShmDescriptor = shm_open(g_sharedMemory, O_CREAT | O_RDWR | O_TRUNC, mode)) == -1)
	{
		perror("shm_open failure");
		exit(0);
	}

	/* Preallocate a shared memory area */

	SHM_SIZE = sysconf(_SC_PAGE_SIZE);

	if (ftruncate(ShmDescriptor, SHM_SIZE) == -1)
	{
		perror("ftruncate failure");
		exit(0);
	}

	if ((MemoryPointer = (caddr_t)mmap((void*)0x7f6d0afcf000, 128, PROT_WRITE | PROT_READ, MAP_SHARED,
									   ShmDescriptor, 0)) == (caddr_t)-1)
	{
		perror("mmap failure");
		exit(0);
	}

	/* while(1)
	{

		char chr[str.length()+1];
	  	strcpy(chr, str.c_str());       
		 memcpy(MemoryPointer, chr, str.length());
		cout<<MemoryPointer;

	}*/
	string filePath = "param.txt";

	std::ifstream fd(filePath, std::ios_base::in);
	while (1)
	{
		sleep(1);
		string line;

		if (!getline(fd, line))
		{
			fd.close();
			fd.open(filePath, std::ios::in);
			continue;
		}
		auto pos = line.find(",,");
		while (pos != std::string::npos)
		{
			line.replace(pos, 1, ",0");
			pos = line.find(",,", pos + 5);
		}
		line = std::regex_replace(line, std::regex(","), std::string(" "));

		char Message[line.length() + 1];

		strcpy(Message, line.c_str());
		/* Create the Semaphore */

		SemDescriptor = sem_open(g_semaphore, O_CREAT, 0777, 0);

		if (SemDescriptor == (void *)-1)
		{
			perror("sem_open failure");
			exit(0);
		}
		memcpy(MemoryPointer, Message, line.length());
		sem_post(SemDescriptor);
		cout << MemoryPointer << endl;
	}
	fd.close();
	//munmap(MemoryPointer, SHM_SIZE);
	sem_close(SemDescriptor);
	sem_unlink(g_semaphore);
	close(ShmDescriptor);
	shm_unlink(g_sharedMemory);
	return 0;
}
