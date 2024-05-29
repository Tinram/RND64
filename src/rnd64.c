
/**
	* RND64
	* rnd64.c
	*
	* Generate large files (4GB+, non-sparse) and large streams (200GB+) of random data as quickly as possible.
	*
	* @author        Martin Latter
	* @copyright     Martin Latter, April 2014
	* @version       0.42 mt
	* @license       GNU GPL version 3.0 (GPL v3); https://www.gnu.org/licenses/gpl-3.0.html
	* @link          https://github.com/Tinram/RND64.git
	*
	* Compile (GCC x64):
	*                    Linux:      gcc rnd64.c -o rnd64 -lpthread -O3 -Wall -Wextra -Wuninitialized -Wunused -Werror -std=gnu99 -s
	*                    Windows:    gcc rnd64.c -o rnd64.exe -O3 -Wall -Wextra -Wuninitialized -Wunused -Werror -std=c99 -s
	*
	*                    further CPU optimisation examples:
	*                                -mtune=native -march=native                    current CPU
	*                                -march=core-avx2 -mtune=core-avx2              Intel Haswell
	*                                -march=skylake-avx512 -mtune=skylake-avx512    Intel Skylake
*/


#include "rnd64.h"


/* global variables */
FILE* pFile;
char* pFilename = NULL;
pcg32_random_t pcg32_random;


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

	/* create seed for pcg_random() usage */
	seed_pcg_random();

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
	int64_t iNegCheck = 0;

	unsigned int iFIndex = 0;
	uint64_t iTotalBytes = 0;
	uint64_t iThreadBytes = 0;

	char cUnit;
	char sFileSize[iSizeLen];

	clock_t tStart = 0;

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

	/* substring size */
	strncpy(sFileSize, aArgV[2], iSizeLen - 1);
	sFileSize[iSizeLen - 1] = '\0';

	/* check for negative size */
	iNegCheck = strtol(sFileSize, (char**) NULL, 10);

	if (iNegCheck < 0) {
		fprintf(stderr, "\n%s: negative size attempted! Please use a positive number and size suffix of k, m, or g for <size>  e.g. 100k\n\n", pFilename);
		return EXIT_FAILURE;
	}

	/* convert size to unsigned 64-bit */
	iTotalBytes = strtoull(sFileSize, (char**) NULL, 10);

	/* check for zero output */
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

	/* total bytes divided by threads */
	iThreadBytes = iTotalBytes / iNumThreads;

	Params_t params;
	params.bytes = iThreadBytes;

	if (aArgV[3] == NULL) {
		params.filename = NULL;
	}
	else {

		params.filename = aArgV[3];
		pFile = fopen(aArgV[3], "wb");

		if (pFile == NULL) {
			fprintf(stderr, "\n%s: output file cannot be written.\n(check write permissions / filename characters)\n\n", pFilename);
			return EXIT_FAILURE;
		}
	}

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

	/* pass params to thread function */
	for (unsigned int i = 0; i < iNumThreads; i++) {

		#ifdef __linux
			pthread_create(&rThreadID[i], NULL, pFuncs[iFIndex], &params);
		#elif _WIN64
			rThreadID[i] = CreateThread(NULL, 0, pFuncs[iFIndex], &params, 0, &dwThreadID);
		#endif
	}

	/* thread wait and join */
	for (unsigned int i = 0; i < iNumThreads; i++) {

		#ifdef __linux
			pthread_join(rThreadID[i], NULL);
		#elif _WIN64
			WaitForSingleObject(rThreadID[i], INFINITE);
		#endif
	}

	if (params.filename != NULL || STREAM_STATS) { /* file output or STREAM_STATS */

		int iMSec = 0;
		clock_t tDiff = 0;

		if (params.filename != NULL) {
			fclose(pFile);
			printf("\n%s generated\n\nsize: %"PRId64" bytes\n", aArgV[3], iTotalBytes);
		}

		/* timer end */
		tDiff = clock() - tStart;

		/* timer display, by Ben Alpert */
		iMSec = tDiff * 1000 / CLOCKS_PER_SEC;

		if (STREAM_STATS) {
			fprintf(stderr, "time: %d s %d ms\n", iMSec / 1000, iMSec % 1000);
		}
		else {
			printf("time: %d s %d ms\n", iMSec / 1000, iMSec % 1000);
		}

		/* MB/s calculation for size over 50MB */
		if (iTotalBytes > 52428800) {

			if (STREAM_STATS) {
				fprintf(stderr, "MB/s: %0.2f\n", (float) ((iTotalBytes * cMBRECIP * cMBRECIP) / (iMSec * 0.001)));
			}
			else {
				printf("MB/s: %0.2f\n", (float) ((iTotalBytes * cMBRECIP * cMBRECIP) / (iMSec * 0.001)));
			}
		}

		printf("\n");
	}

	return EXIT_SUCCESS;
}


/**
	* Seed the pcg_random generator.
	*
	* @param   void
	* @return  void
*/

void seed_pcg_random(void) {

	/* set state */
	pcg32_random.state ^= (uint64_t) time(NULL) ^ (uint64_t) &seed_pcg_random;

	/* generate 12 ints for initial state to diverge */
	for (unsigned int i = 0; i < 12; i++) {
		pcg32_random_r(&pcg32_random);
	}
}


/**
	* pcg32_random fast random number generator (minimal PCG32 version)
	* (c) 2014 Professor Melissa E. O'Neill - pcg-random.org
	* Apache License 2.0
	*
	* @param   pcg32_random_t* rng
	* @return  uint32_t
*/

inline uint32_t pcg32_random_r(pcg32_random_t* rng)
{
	uint64_t oldstate = rng->state;

	/* advance internal state */
	rng->state = oldstate * 6364136223846793005ULL + (rng->inc | 1);

	/* calculate output function (XSH RR), uses old state for max ILP */
	uint32_t xorshifted = ((oldstate >> 18u) ^ oldstate) >> 27u;
	uint32_t rot = oldstate >> 59u;

	return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
}


/**
	* Thread function: output random uints using pcg_random.
	*
	* @param   void pointer st, params struct
	* @return  void* / null
*/

#ifdef __linux
	void* generateAll(void* st)
#elif _WIN64
	DWORD WINAPI generateAll(LPVOID st)
#endif

	{
		Params_t* params = (Params_t*) st;
		uint64_t iThreadBytes = params->bytes;

		unsigned int iNumPages = iThreadBytes / cBUFFER;
		unsigned int iNumBytes = cBUFFER / sizeof(unsigned int);
		unsigned int iTailSize = iThreadBytes % cBUFFER;
		uint32_t aBuffer[cBUFFER];

		if (params->filename != NULL) { /* write to file */

			for (unsigned int i = 0; i < iNumPages; i++) {

				for (unsigned int j = 0; j < iNumBytes; j++) {
					aBuffer[j] = pcg32_random_r(&pcg32_random);
				}

				fwrite(aBuffer, 1, cBUFFER, pFile);
			}

			if (iTailSize > 0) {

				for (unsigned int j = 0; j < iNumBytes; j++) {
					aBuffer[j] = pcg32_random_r(&pcg32_random);
				}

				fwrite(aBuffer, 1, iTailSize, pFile);
			}
		}
		else { /* write to stdout */

			for (unsigned int i = 0; i < iNumPages; i++) {

				for (unsigned int j = 0; j < iNumBytes; j++) {
					aBuffer[j] = pcg32_random_r(&pcg32_random);
				}

				fwrite(aBuffer, 1, cBUFFER, stdout);
			}

			if (iTailSize > 0) {

				for (unsigned int j = 0; j < iNumBytes; j++) {
					aBuffer[j] = pcg32_random_r(&pcg32_random);
				}

				fwrite(aBuffer, 1, iTailSize, stdout);
			}
		}

#ifdef __linux
		pthread_exit(NULL);
#elif _WIN64
		return 0;
#endif

	}


/**
	* Thread function: output printable ASCII characters (33 to 127).
	* Not fast, included to generate printable characters for my purposes.
	*
	* @param   void pointer st, params struct
	* @return  void* / null
*/

#ifdef __linux
	void* generateRestricted(void* st)
#elif _WIN64
	DWORD WINAPI generateRestricted(LPVOID st)
#endif

	{
		Params_t* params = (Params_t*) st;
		uint64_t iThreadBytes = params->bytes;

		unsigned int iNumPages = iThreadBytes / cBUFFER;
		unsigned int iNumBytes = cBUFFER / sizeof(char); /* sizeof(char) slow; hack: sizeof(unsigned int) faster but corrupted buffer */
		unsigned int iTailSize = iThreadBytes % cBUFFER;
		char aBuffer[cBUFFER];

		#ifdef __linux
			unsigned int iSeed = 0;
			iSeed = rand();
		#elif _WIN64
			static unsigned int iSeed = 3142;
			iSeed += (((unsigned int) time(NULL)) * 0.2);
			srand(iSeed);
		#endif

		if (params->filename != NULL) { /* write to file */

			for (unsigned int i = 0; i < iNumPages; i++) {

				for (unsigned int j = 0; j < iNumBytes; j++) {

					#ifdef __linux
						aBuffer[j] = (rand_r(&iSeed) % 94) + 33;
					#elif _WIN64
						aBuffer[j] = (rand() % 94) + 33;
					#endif
				}

				fwrite(aBuffer, 1, cBUFFER, pFile);
			}

			if (iTailSize > 0) {

				for (unsigned int j = 0; j < iNumBytes; j++) {

					#ifdef __linux
						aBuffer[j] = (rand_r(&iSeed) % 94) + 33;
					#elif _WIN64
						aBuffer[j] = (rand() % 94) + 33;
					#endif
				}

				fwrite(aBuffer, 1, iTailSize, pFile);
			}
		}
		else { /* write to stdout */

			for (unsigned int i = 0; i < iNumPages; i++) {

				for (unsigned int j = 0; j < iNumBytes; j++) {

					#ifdef __linux
						aBuffer[j] = (rand_r(&iSeed) % 94) + 33;
					#elif _WIN64
						aBuffer[j] = (rand() % 94) + 33;
					#endif
				}

				fwrite(aBuffer, 1, cBUFFER, stdout);
			}

			if (iTailSize > 0) {

				for (unsigned int j = 0; j < iNumBytes; j++) {

					#ifdef __linux
						aBuffer[j] = (rand_r(&iSeed) % 94) + 33;
					#elif _WIN64
						aBuffer[j] = (rand() % 94) + 33;
					#endif
				}

				fwrite(aBuffer, 1, iTailSize, stdout);
			}
		}

#ifdef __linux
		pthread_exit(NULL);
#elif _WIN64
		return 0;
#endif

	}


/**
	* Thread function: output null character.
	*
	* @param   void pointer st, params struct
	* @return  void* / null
*/

#ifdef __linux
	void* generateSingleChar(void* st)
#elif _WIN64
	DWORD WINAPI generateSingleChar(LPVOID st)
#endif

	{
		Params_t* params = (Params_t*) st;
		uint64_t iThreadBytes = params->bytes;

		unsigned int iNumPages = iThreadBytes / cBUFFER;
		unsigned int iNumBytes = cBUFFER / sizeof(unsigned int);
		unsigned int iTailSize = iThreadBytes % cBUFFER;
		unsigned int aBuffer[cBUFFER]; /* slightly faster (just) in using array on stack rather than heap (malloc); int_fast8_t tried */

		if (params->filename != NULL) { /* write to file */

			for (unsigned int i = 0; i < iNumPages; i++) {
				memset(aBuffer, cNB, iNumBytes);
				fwrite(aBuffer, 1, cBUFFER, pFile);
			}

			if (iTailSize > 0) {
				memset(aBuffer, cNB, iNumBytes);
				fwrite(aBuffer, 1, iTailSize, pFile);
			}
		}
		else { /* write to stdout */

			for (unsigned int i = 0; i < iNumPages; i++) {
				memset(aBuffer, cNB, iNumBytes);
				fwrite(aBuffer, 1, cBUFFER, stdout);
			}

			if (iTailSize > 0) {
				memset(aBuffer, cNB, iNumBytes);
				fwrite(aBuffer, 1, iTailSize, stdout);
			}
		}

#ifdef __linux
		pthread_exit(NULL);
#elif _WIN64
		return 0;
#endif

	}


/**
	* Thread function: output crypto-generated bytes.
	*
	* @param   void pointer st, params struct
	* @return  void* / null
*/

#ifdef __linux

	void* generateCrypto(void* st) {

		Params_t* params = (Params_t*) st;
		uint64_t iThreadBytes = params->bytes;

		unsigned int iNumPages = iThreadBytes / cBUFFER;
		unsigned int iNumBytes = cBUFFER / sizeof(char);
		unsigned int iTailSize = iThreadBytes % cBUFFER;
		unsigned int aBuffer[cBUFFER];
		FILE* pUrand;

		if ((pUrand = fopen(RANDOM_PATH, "r")) == NULL) {
			fprintf(stderr, "\n%s: secure data generation unavailable.\n\n", pFilename);
		}
		else if (params->filename != NULL) { /* write to file */

			for (unsigned int i = 0; i < iNumPages; i++) {

				if (fread(aBuffer, 1, iNumBytes, pUrand) != iNumBytes) {
					fprintf(stderr, "\n%s: insufficient crypto random bytes available.\n\n", pFilename);
					goto exit;
				}

				fwrite(aBuffer, 1, cBUFFER, pFile);
			}

			if (iTailSize > 0) {

				if (fread(aBuffer, 1, iNumBytes, pUrand) != iNumBytes) {
					fprintf(stderr, "\n%s: insufficient crypto random bytes available.\n\n", pFilename);
					goto exit;
				}

				fwrite(aBuffer, 1, iTailSize, pFile);
			}
		}
		else { /* write to stdout */

			for (unsigned int i = 0; i < iNumPages; i++) {

				if (fread(aBuffer, 1, iNumBytes, pUrand) != iNumBytes) {
					fprintf(stderr, "\n%s: insufficient crypto random bytes available.\n\n", pFilename);
					goto exit;
				}

				fwrite(aBuffer, 1, cBUFFER, stdout);
			}

			if (iTailSize > 0) {

				if (fread(aBuffer, 1, iNumBytes, pUrand) != iNumBytes) {
					fprintf(stderr, "\n%s: insufficient crypto random bytes available.\n\n", pFilename);
					goto exit;
				}

				fwrite(aBuffer, 1, iTailSize, stdout);
			}
		}

		exit:

			fclose(pUrand);

			pthread_exit(NULL);
	}

#elif _WIN64

	DWORD WINAPI generateCrypto(LPVOID st) {

		Params_t* params = (Params_t*) st;
		uint64_t iThreadBytes = params->bytes;

		unsigned int iNumPages = iThreadBytes / cBUFFER;
		unsigned int iNumBytes = cBUFFER / sizeof(char);
		unsigned int iTailSize = iThreadBytes % cBUFFER;
		HCRYPTPROV rCryptHandle = 0;
		char aBuffer[cBUFFER];
		char cS;

		if (CryptAcquireContext(&rCryptHandle, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT) == FALSE) {
			fprintf(stderr, "\n%s: secure data generation unavailable (CryptAcquireContext failed).\n\n", pFilename);
		}
		else {

			if (params->filename != NULL) { /* write to file */

				for (unsigned int i = 0; i < iNumPages; i++) {

					for (unsigned int j = 0; j < iNumBytes; j++) {
						CryptGenRandom(rCryptHandle, 1, (BYTE*) &cS);
						aBuffer[j] = cS;
					}

					fwrite(aBuffer, 1, cBUFFER, pFile);
				}

				if (iTailSize > 0) {

					for (unsigned int j = 0; j < iNumBytes; j++) {
						CryptGenRandom(rCryptHandle, 1, (BYTE*) &cS);
						aBuffer[j] = cS;
					}

					fwrite(aBuffer, 1, iTailSize, pFile);
				}
			}
			else { /* write to stdout */

				for (unsigned int i = 0; i < iNumPages; i++) {

					for (unsigned int j = 0; j < iNumBytes; j++) {
						CryptGenRandom(rCryptHandle, 1, (BYTE*) &cS);
						aBuffer[j] = cS;
					}

					fwrite(aBuffer, 1, cBUFFER, stdout);
				}

				if (iTailSize > 0) {

					for (unsigned int j = 0; j < iNumBytes; j++) {
						CryptGenRandom(rCryptHandle, 1, (BYTE*) &cS);
						aBuffer[j] = cS;
					}

					fwrite(aBuffer, 1, iTailSize, stdout);
				}
			}

			if (CryptReleaseContext(rCryptHandle, 0)) {
				rCryptHandle = 0;
			}

		}

		return 0;
	}

#endif


/**
	* Display menu.
	*
	* @param   char* pFName, name from aArgV[0]
	* @return  void
*/

void menu(char* const pFName) {

	printf("\nRND64 v.%s\nby Tinram", RND64_VERSION);
	printf("\n\nUsage:\n");
	printf("\t\t%s [option] <size> [file]", pFName);
	printf("\n\t\t%s [option] <size> | <prog>", pFName);
	printf("\n\nOptions:");
	printf("\n\t\t-a\t chars 0-255    (all)");
	printf("\n\t\t-f\t single char    (fastest)");
	printf("\n\t\t-r\t chars 33-126   (restrict)");
	printf("\n\t\t-c\t crypto bytes");
	printf("\n\n\t\tsize\t 1K, 100M, 8G\n\n");
}
