
#include <sys/types.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <stdlib.h>

#include <iostream>
#include <unistd.h>
#include <cstring>

using namespace std;

char g_sharedMemory[] = "SharedMemoryMDC";
char g_semaphore[] = "SemaphoreMDC";

int main()
{
	caddr_t MemoryPointer;
	int ShmDescriptor;
	sem_t *SemDescriptor;
	int SHM_SIZE;

	SHM_SIZE = sysconf(_SC_PAGE_SIZE);				   /* Get Page Size */
	SemDescriptor = sem_open(g_semaphore, 0, 0644, 0); /* Open the Semaphore */

	if ((ShmDescriptor = shm_open(g_sharedMemory, O_RDWR, 0)) == -1) /* Open the shared memory object */
	{
		perror("shm_open failure");
		exit(0);
	}

	if (SemDescriptor == (void *)-1)
	{
		perror("sem_open failure");
		exit(0);
	}

	while (1)  /*Process running */
	{
		sleep(1);
		string str;
		char Message[512];
		if ((MemoryPointer = (caddr_t)mmap((void*)0x7f6d0afcf000, 128, PROT_WRITE | PROT_READ, MAP_SHARED,
										   ShmDescriptor, 0)) == (caddr_t)-1) /*creates a new mapping in the virtual address space of the calling process*/
		{
			perror("mmap failure");
			exit(0);
		}
		if (!sem_wait(SemDescriptor))	/*locks the semaphore pointed to by SemDescriptor - Decrements*/
		{

			memcpy(Message, MemoryPointer, 512);
			sem_post(SemDescriptor);	/*unlocks the semaphore pointed to by SemDescriptor - Increments*/
		}
		cout << Message << endl;
	}

	munmap(MemoryPointer, SHM_SIZE); /*deletes the mappings for the specified address range*/
	close(ShmDescriptor);
	sem_close(SemDescriptor);
	sem_unlink(g_semaphore);

	return 0;
}
