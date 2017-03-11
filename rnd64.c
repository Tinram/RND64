
/**
	* RND64
	* rnd64.c
	*
	* Generate large files (4GB+, non-sparse) and large streams of random data as quickly as possible.
	*
	* @author        Martin Latter <copysense.co.uk>
	* @copyright     Martin Latter, April 2014
	* @version       0.34 mt
	* @license       GNU GPL version 3.0 (GPL v3); https://www.gnu.org/licenses/gpl-3.0.html
	* @link          https://github.com/Tinram/RND64.git
	*
	* Compile (GCC x64):
	*                    Linux:      gcc rnd64.c -o rnd64 -lpthread -O3 -Wall -Wextra -Wuninitialized -Wunused -Werror -std=gnu99 -s
	*                    Windows:    gcc rnd64.c -o rnd64.exe -O3 -Wall -Wextra -Wuninitialized -Wunused -Werror -std=c99 -s
	*
	*                    further CPU optimisation examples:
	*                                -march=core-avx-i -mtune=core-avx-i        Intel Ivy Bridge
	*                                -march=core-avx2 -mtune=core-avx2          Intel Haswell
*/


#include "rnd64.h"


/* global variables for thread functions */
char* pFilename;
uint64_t iBytes;


int main(int iArgCount, char* aArgV[]) {

	/* arguments check */
	if (iArgCount < 3) {

		menu(aArgV[0]);
		return EXIT_FAILURE;
	}

	/* initial variables */
	int iAll = strcmp(aArgV[1], "-a");
	int iSingleChar = strcmp(aArgV[1], "-f");
	int iRestricted = strcmp(aArgV[1], "-r");
	int iCrypto = strcmp(aArgV[1], "-c");
	unsigned int iNumThreads = 1;

	/* set global variable for thread functions */
	pFilename = aArgV[0];

	if (iAll != 0 && iSingleChar != 0 && iRestricted != 0 && iCrypto != 0) {

		menu(pFilename);
		return EXIT_FAILURE;
	}

	/* create seed for rand() usage */
	srand((unsigned int) time(NULL));

	/* main variables */

	/* detect number of CPU threads (logical cores, not physical cores, Intel i3 = 4: 2 cores + 2 threads) */
	#ifdef __linux
		iNumThreads = (unsigned int) get_nprocs();
		pthread_t rThreadID[iNumThreads];
	#elif _WIN64
		DWORD dwThreadID;
		SYSTEM_INFO siSysInfo;
		GetSystemInfo(&siSysInfo);
		iNumThreads = (unsigned int) siSysInfo.dwNumberOfProcessors;
		HANDLE rThreadID[iNumThreads];
	#endif

	int iSizeLen = strlen(aArgV[2]);
	int iMSec;
	unsigned int i;
	unsigned int j;
	unsigned int iFIndex = 0;
	unsigned int iUnavailMB;
	unsigned long long iFreeMemory;
	char cUnit;
	char sFileSize[iSizeLen];
	char* aBuffer[iNumThreads];
	uint64_t iTotalBytes;
	uint64_t iBytesLocal;
	FILE* pFile;
	clock_t tStart, tDiff;

	/* function pointers */
	void (*pFuncs[4]) = {
		generateAll,
		generateSingleChar,
		generateRestricted,
		generateCrypto
	};

	/* get size character */
	cUnit = aArgV[2][iSizeLen - 1];
	cUnit = tolower(cUnit);

	/* check size character */
	if (cUnit != 'k' && cUnit != 'm' && cUnit != 'g') {

		fprintf(stderr, "\n%s: please specify the file/stream size with a suffix of k, m, or g\n\n", pFilename);
		return EXIT_FAILURE;
	}

	/* substring filesize */
	strncpy(sFileSize, aArgV[2], iSizeLen - 1);
	sFileSize[iSizeLen - 1] = '\0';

	/* convert filesize to unsigned 64-bit */
	iTotalBytes = strtoull(sFileSize, (char**) NULL, 10);

	/* check zero output */
	if (iTotalBytes == 0) {

		fprintf(stderr, "\n%s: zero-sized output! Please use a number and size suffix of k, m, or g for <size>  e.g. 100k\n\n", pFilename);
		return EXIT_FAILURE;
	}

	/* convert unit to bytes */
	if (cUnit == 'k') {
		iTotalBytes = 1024 * iTotalBytes;
	}
	else if (cUnit == 'm') {
		iTotalBytes = 1024 * 1024 * iTotalBytes;
	}
	else if (cUnit == 'g') {
		iTotalBytes = 1024 * 1024 * 1024 * iTotalBytes;
	}
	else {

		fprintf(stderr, "\n%s: file size error!\n\n", pFilename);
		return EXIT_FAILURE;
	}

	iFreeMemory = getFreeSystemMemory();

	/* bail out if required memory exceeds, or is likely to dangerously exhaust, free memory */
	if (iTotalBytes > iFreeMemory) {

		iUnavailMB = iTotalBytes / 1024 / 1024;
		fprintf(stderr,"\n%s: insufficient memory to allocate %u MB.\nPlease use a smaller <size>\n\n", pFilename, iUnavailMB);
		return EXIT_FAILURE;
	}

	/* set global variable for thread functions */
	iBytes = iTotalBytes / iNumThreads;

	/* copy global to local */
	iBytesLocal = iBytes;

	/* timer start */
	tStart = clock();

	/* set function pointer index */
	if (iAll == 0) {
		iFIndex = 0;
	}
	else if (iSingleChar == 0) {
		iFIndex = 1;
	}
	else if (iRestricted == 0) {
		iFIndex = 2;
	}
	else if (iCrypto == 0) {
		iFIndex = 3;
	}

	/* allocate buffer and pass to thread function */
	for (i = 0; i < iNumThreads; i++) {

		aBuffer[i] = (char*) malloc(iBytesLocal + 1); /* +1 for 0 */

		if (aBuffer[i] == NULL) {

			iUnavailMB = iBytesLocal / 1024 / 1024;
			fprintf(stderr, "\n%s: insufficient memory to allocate %u MB in thread buffer.\n\n", pFilename, iUnavailMB);

			for (j = 0; j <= i; j++) { /* deallocate up to this i */
				free(aBuffer[j]);
			}

			return EXIT_FAILURE;
		}

		#ifdef __linux
			pthread_create(&rThreadID[i], NULL, pFuncs[iFIndex], aBuffer[i]);
		#elif _WIN64
			rThreadID[i] = CreateThread(NULL, 0, pFuncs[iFIndex], aBuffer[i], 0, &dwThreadID);
		#endif
	}

	/* thread wait - avoid seg fault on larger files */
	for (i = 0; i < iNumThreads; i++) {

		#ifdef __linux
			pthread_join(rThreadID[i], NULL);
		#elif _WIN64
			WaitForSingleObject(rThreadID[i], INFINITE);
		#endif
	}

	/* create stream if no specified [file] argument, then exit */
	if (aArgV[3] == NULL) {

		for (i = 0; i < iNumThreads; i++) {
			fwrite(aBuffer[i], sizeof(char), iBytesLocal, stdout);
		}

		for (i = 0; i < iNumThreads; i++) { /* deallocate */
			free(aBuffer[i]);
		}

		return EXIT_SUCCESS;
	}

	/* write buffer to file */
	pFile = fopen(aArgV[3], "wb");

	if (pFile != NULL) {

		for (i = 0; i < iNumThreads; i++) {
			fwrite(aBuffer[i] , sizeof(char), iBytesLocal, pFile);
		}
	}
	else {

		for (i = 0; i < iNumThreads; i++) { /* deallocate */
			free(aBuffer[i]);
		}

		fprintf(stderr, "\n%s: output file cannot be written.\n(check write permissions / filename characters)\n\n", pFilename);
		return EXIT_FAILURE;
	}

	fclose(pFile);

	/* deallocate */
	for (i = 0; i < iNumThreads; i++) {
		free(aBuffer[i]);
	}

	/* timer end */
	tDiff = clock() - tStart;

	printf("\n%s generated\n\nsize: %"PRId64" bytes\n", aArgV[3], iTotalBytes);

	/* timer display, by Ben Alpert */
	iMSec = tDiff * 1000 / CLOCKS_PER_SEC;
	printf("time: %d s %d ms\n", iMSec / 1000, iMSec % 1000);

	/* MB/s calculation for filesize over 50MB */
	if (iTotalBytes > 52428800) {
		printf("MB/s: %0.2f\n", (float) ((iTotalBytes * cMBRecip * cMBRecip) / (iMSec * 0.001)));
	}

	printf("\n");

	return EXIT_SUCCESS;
}


/**
	* Thread function: populate buffer with all ASCII characters.
	*
	* @param   void pointer buff, buffer
	* @return  void* / null
*/

#ifdef __linux

	void* generateAll(void* buff) {

		uint64_t i = 0;
		uint64_t iBytesLocal = iBytes;
		char* pBuffer = (char*) buff;
		unsigned int iSeed = 0;

		/* rand_r() seed for each thread (time() unsuitable) */
		iSeed = rand();

		for (i = 0; i < iBytesLocal; i++) {
			pBuffer[i] = (rand_r(&iSeed) % 254) + 1; /* avoid 0 */
		}

		pBuffer[iBytesLocal] = '\0';
		pthread_exit(NULL);
	}

#elif _WIN64

	DWORD WINAPI generateAll(LPVOID buff) {

		uint64_t i = 0;
		uint64_t iBytesLocal = iBytes;
		char* pBuffer = (char*) buff;
		static unsigned int iSeed = 3142;

		/* srand() seed munger for each thread (time(), GetTickCount(), and rand() unsuitable) */
		iSeed += (((unsigned int) time(NULL)) * 0.2);
		srand(iSeed);

		for (i = 0; i < iBytesLocal; i++) {
			pBuffer[i] = (rand() % 254) + 1; /* avoid 0 */
		}

		pBuffer[iBytesLocal] = '\0';
		return NULL;
	}

#endif


/**
	* Thread function: populate buffer with restricted ASCII character range (33 to 127).
	*
	* @param   void pointer buff, buffer
	* @return  void* / null
*/

#ifdef __linux

	void* generateRestricted(void* buff) {

		uint64_t i = 0;
		uint64_t iBytesLocal = iBytes;
		char* pBuffer = (char*) buff;
		unsigned int iSeed = 0;

		iSeed = rand();

		for (i = 0; i < iBytesLocal; i++) {
			pBuffer[i] = (rand_r(&iSeed) % 94) + 33;
		}

		pBuffer[iBytesLocal] = '\0';
		pthread_exit(NULL);
	}

#elif _WIN64

	DWORD WINAPI generateRestricted(LPVOID buff) {

		uint64_t i = 0;
		uint64_t iBytesLocal = iBytes;
		char* pBuffer = (char*) buff;
		static unsigned int iSeed = 3142;

		iSeed += (((unsigned int) time(NULL)) * 0.2);
		srand(iSeed);

		for (i = 0; i < iBytesLocal; i++) {
			pBuffer[i] = (rand() % 94) + 33;
		}

		pBuffer[iBytesLocal] = '\0';
		return NULL;
	}

#endif


/**
	* Thread function: populate buffer with single character (0).
	*
	* @param   void pointer buff, buffer
	* @return  void* / null
*/

#ifdef __linux

	void* generateSingleChar(void* buff) {

		uint64_t iBytesLocal = iBytes;
		char* pBuffer = (char*) buff;

		memset(pBuffer, 48, iBytesLocal);
		pBuffer[iBytesLocal] = '\0';
		pthread_exit(NULL);
	}

#elif _WIN64

	DWORD WINAPI generateSingleChar(LPVOID buff) {

		uint64_t iBytesLocal = iBytes;
		char* pBuffer = (char*) buff;

		memset(pBuffer, 48, iBytesLocal);
		pBuffer[iBytesLocal] = '\0';
		return NULL;
	}

#endif


/**
	* Thread function: populate buffer with crypto-generated bytes.
	*
	* @param   void pointer, buff, buffer
	* @return  void* / null
*/

#ifdef __linux

	void* generateCrypto(void* buff) {

		uint64_t iBytesLocal = iBytes;
		char* pBuffer = (char*) buff;
		FILE* pUrand;

		if ((pUrand = fopen(RANDOM_PATH, "r")) == NULL) {

			free(pBuffer); /* deallocate */
			fprintf(stderr, "\n%s: secure data generation unavailable.\n\n", pFilename);
		}

		if (fread(pBuffer, 1, iBytesLocal, pUrand) != iBytesLocal) {

			fclose(pUrand);
			free(pBuffer); /* deallocate */
			fprintf(stderr, "\n%s: insufficient crypto random bytes available.\n\n", pFilename);
		}
		else { /* buffer populated, close stream */

			fclose(pUrand);
			pBuffer[iBytesLocal] = '\0';
		}

		pthread_exit(NULL);
	}

#elif _WIN64

	DWORD WINAPI generateCrypto(LPVOID buff) {

		uint64_t i = 0;
		uint64_t iBytesLocal = iBytes;
		unsigned int iErrFlag = 0;
		char cS;
		char* pBuffer = (char*) buff;
		HCRYPTPROV rCryptHandle = 0;

		if (CryptAcquireContext(&rCryptHandle, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT) == FALSE) {

			free(pBuffer); /* deallocate */
			iErrFlag = 1;
			fprintf(stderr, "\n%s: secure data generation unavailable (CryptAcquireContext failed).\n\n", pFilename);
		}

		for (i = 0; i < iBytesLocal; i++) {

			CryptGenRandom(rCryptHandle, 1, (BYTE*) &cS);
			pBuffer[i] = cS;
		}

		if ( ! iErrFlag) {

			pBuffer[iBytesLocal] = '\0';

			if (CryptReleaseContext(rCryptHandle, 0)) {
				rCryptHandle = 0;
			}
		}

		return NULL;
	}

#endif


/**
	* Attempt to find free memory available for allocation.
	* Windows reporting through MEMORYSTATUSEX seems accurate.
	* Linux is trickier: what is reported as free in conjunction with considerable buffering and caching.
	*
	* @return  integer
*/

#ifdef __linux

unsigned long long getFreeSystemMemory() {

	/**
		* Derived from an example by Travis Gockel.
		* Linux apparently provides only total and free (unbuffered) memory values reliably.
		* Found that 0.5GB margin on kernel 4.4 with 6GB RAM is not quite enough (e.g 5g okay, 5300m locks system).
	*/

	long iPages;
	long iPageSize;

	iPages = sysconf(_SC_PHYS_PAGES);
	iPageSize = sysconf(_SC_PAGE_SIZE);

	return (iPages * iPageSize) - cSafetyChunk; /* assumes system has at least 1GB total memory */
}

#elif _WIN64

unsigned long long getFreeSystemMemory() {

	/* from MSDN */

	MEMORYSTATUSEX stMemState;

	stMemState.dwLength = sizeof(stMemState);
	GlobalMemoryStatusEx(&stMemState);

	return stMemState.ullAvailPhys;
}

#endif


/**
	* Display menu.
	*
	* @param   char* pFilename, name from aArgV[0]
	* @return  void
*/

void menu(char* pFilename) {

	printf("\nRND64 v.%s\ncopysense.co.uk", RND64_VERSION);
	printf("\n\nUsage:\n");
	printf("\t\t%s [option] <size> [file]", pFilename);
	printf("\n\t\t%s [option] <size> | <prog>", pFilename);
	printf("\n\nOptions:");
	printf("\n\t\t-a\t chars 1-255    (all)");
	printf("\n\t\t-f\t single char    (fastest)");
	printf("\n\t\t-r\t chars 33-126   (restrict)");
	printf("\n\t\t-c\t crypto bytes");
	printf("\n\n\t\tsize\t 1K, 100M, 3G\n\n");
}
