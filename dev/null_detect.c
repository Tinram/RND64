
/**
	* RND64
	* null_detect.c
	*
	* Stream reader null counter.
	*
	* gcc null_detect.c -o null_detect -Ofast -flto -s
	* rnd64 -f 1m | ./null_detect
	*
	* @author        Martin Latter, 03/01/2019
	* @version       0.02
	* @license       GNU GPL version 3.0 (GPL v3); https://www.gnu.org/licenses/gpl-3.0.html
	* @link          https://github.com/Tinram/RND64.git
*/


#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <time.h>


int main(void)
{
	char c;
	uint64_t iCharCount = 0;
	uint64_t iByteCount = 0;

	clock_t tStart = 0;
	clock_t tDiff = 0;
	int iMSec = 0;

	while ((c = getchar_unlocked()) != EOF)
	{
		if (c == 0)
		{
			iCharCount++;
		}

		iByteCount++;
	}

	fprintf(stderr, "%"PRId64" null bytes found\n", iCharCount);
	fprintf(stderr, "%"PRId64" bytes parsed\n", iByteCount);

	tDiff = clock() - tStart;
	iMSec = tDiff * 1000 / CLOCKS_PER_SEC;
	fprintf(stderr, "time: %d s %d ms\n", iMSec / 1000, iMSec % 1000);

	return EXIT_SUCCESS;
}
