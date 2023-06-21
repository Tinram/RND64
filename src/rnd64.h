
/**
	* RND64
	* rnd64.h
	*
	* Generate large files (4GB+, non-sparse) and large streams (200GB+) of random data as quickly as possible.
	*
	* @author        Martin Latter
	* @copyright     Martin Latter, April 2014
	* @version       0.41 mt
	* @license       GNU GPL version 3.0 (GPL v3); http://www.gnu.org/licenses/gpl.html
	* @link          https://github.com/Tinram/RND64.git
	*
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <inttypes.h>

#ifdef __linux
	#include <pthread.h>
	#include <sys/sysinfo.h>
	#include <unistd.h>
	#define RANDOM_PATH "/dev/urandom"
#elif _WIN64
	#include <windows.h>
	#include <wincrypt.h> /* CryptAcquireContext, CryptGenRandom */
#endif


/* defines */
#define RND64_VERSION "0.41 mt"
#define KB 0x400ULL
#define STREAM_STATS 0 /* Win stream stats */


/* constants */
float const cMBRECIP = 0.000976562;
unsigned int const cBUFFER = 64 * KB; /* optimum 64kB cache size (~L1) on CPUs tested */
unsigned int const cNB = 0;


/* structs */
typedef struct {
	char* filename;
	uint64_t bytes;
} Params_t;

typedef struct {
	uint64_t state;
	uint64_t inc;
} pcg32_random_t;


/* functions */
void seed_pcg_random(void);
inline uint32_t pcg32_random_r(pcg32_random_t* rng);
void menu(char* const pFName);

#ifdef __linux
	void* generateAll(void* st);
	void* generateRestricted(void* st);
	void* generateSingleChar(void* st);
	void* generateCrypto(void* st);
#elif _WIN64
	DWORD WINAPI generateAll(LPVOID st);
	DWORD WINAPI generateRestricted(LPVOID st);
	DWORD WINAPI generateSingleChar(LPVOID st);
	DWORD WINAPI generateCrypto(LPVOID st);
#endif


/* global variables */
extern FILE* pFile;
extern char* pFilename;
extern pcg32_random_t pcg32_random;
