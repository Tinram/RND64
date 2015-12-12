
/**
	* RND64
	* rnd64.h
	*
	* Generate large files (4GB+) and large streams of random data as quickly as possible.
	*
	* @author       Martin Latter <copysense.co.uk>
	* @copyright     Martin Latter, April 2014
	* @version      0.28 mt
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
	#define RANDOM_PATH "/dev/urandom"
#elif _WIN64
	#include <windows.h>
	#include <wincrypt.h> /* CryptAcquireContext, CryptGenRandom */
#endif


#define RND64_VERSION "0.28 mt"


#ifdef __linux
	void* generateAll(void* buff);
	void* generateRestricted(void* buff);
	void* generateSingleChar(void* buff);
	void* generateCrypto(void* buff);
#elif _WIN64
	DWORD WINAPI generateAll(LPVOID buff);
	DWORD WINAPI generateRestricted(LPVOID buff);
	DWORD WINAPI generateSingleChar(LPVOID buff);
	DWORD WINAPI generateCrypto(LPVOID buff);
#endif


void menu(char* pFilename);


const float cMBRecip = 0.000976562;


extern char* pFilename;
extern uint64_t iBytes;
