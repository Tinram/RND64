
# RND64

### Multi-threaded 64-bit random data generator.

#### Linux and Windows

##### RND64 v.0.40


[1]: https://tinram.github.io/images/rnd64.png
![rnd64][1]


## Purpose

Generate large files (4GB+, non-sparse) and large streams of random data (200GB+) at fast generation rates (~8GB/sec stream output, Linux, Core i3 CPU, `-f` option).

*What's the point of such large lumps of junk?*

Some uses are: file hashing, integrity tests, benchmarking, and network speed tests.

A few Windows programs exist to create large files, and there are plenty of shell scripts using `dd` etc. I just needed something cross-platform with simple command-line options.


## OS Support

+ Linux x64

+ Windows x64


## Usage

    rnd64 [option] <size> <file>
    rnd64 [option] <size> | <program>

#### Options

    -a     (all)             binary characters          includes control codes
    -f     (fastest)         character zero (48)        fastest generator
    -r     (restrict)        characters 33 to 126       7-bit printable ASCII, safe for terminal output
    -c     (crypto)          crypto-sourced bytes       Linux: /dev/urandom, Windows: CryptGenRandom
                                                            (much slower byte generation)

    size   1K, 100M, 8G


### Usage Examples

    rnd64.exe or rnd64   (Windows)        display command-line options, as above
    ./rnd64              (Linux)

    rnd64 -a 1k f.txt                     output 1kB of random bytes to the file 'f.txt'
    rnd64 -f 1k f.txt                     output 1kB of zeros to 'f.txt'
    rnd64 -r 1k f.txt                     output a restricted range of 7-bit ASCII characters (33 to 126) to 'f.txt'
    rnd64 -f 4g | pv > /dev/null          send 4GB of zeros to /dev/null with 'pv' displaying the throughput rate (Linux)
    rnd64 -c 1k | ent                     pipe 1kB of crypto bytes to the program 'ent'
    rnd64 -a 1k | nc 192.168.1.20 80      pipe 1kB of random bytes to 'netcat' to send to 192.168.1.20 on port 80
    rnd64 -f 100g | pv > /dev/null        stress your system


###### WARNING: When dumping large files to disk (over 1GB) &ndash; for HDDs consider the mechanical drive's age and performance; for SSDs consider the write wear. As well as warming the CPU, RND64 is quite capable of exhausting all hard drive space and reducing SSD lifetimes.


## Build

In the directory containing the cloned repo / extracted zip files, compile with GCC:

(or Clang, just rename the makefiles)

### Linux

    make

or full process:

    make && make install && make clean

compile manually:

**GCC:**

    gcc rnd64.c -o rnd64 -lpthread -O3 -Wall -Wextra -Wuninitialized -Wunused -Werror -std=gnu99 -s

**Clang:**

    clang rnd64.c -o rnd64 -lpthread -O3 -Wall -Wextra -Wuninitialized -Wunused -Werror -s

### Windows

    gcc rnd64.c -o rnd64.exe -O3 -Wall -Wextra -Wuninitialized -Wunused -Werror -std=c99 -s

#### Further Optimisation

    -mtune=native -march=native                    current CPU
    gcc -Q -march=native --help=target             show -march/-mtune native CPU parameters

    -march=core-avx2 -mtune=core-avx2              Intel Haswell
    -march=skylake-avx512 -mtune=skylake-avx512    Intel Skylake
    -flto


## Other

The switches `-a` and `-c` are dangerous options when both a filename and pipe symbol are omitted. The wide range of output bytes, including control characters, are printed in the terminal, which can cause the terminal to lock or crash (especially on Windows).

On both Linux and Windows, it's more convenient for RND64 to be available from any directory location via the *$PATH* system variable.

#### Linux

    make install

Or move the RND64 executable to a location such as */usr/local/bin*  (location must be present in *$PATH*).

#### Windows

[Windows/Super key + Break] > Advanced tab > Environmental Variables button > click Path line > Edit button > Variable value - append at the end of existing line information: *C:\directory\path\to\rnd64.exe\;*


## Speed

RND64 is fast, but not that fast:

+ Zero stream generation rates `-f` are decent on Linux (~8GB/sec, Core i3 Haswell 3.4GHz desktop CPU), and the pcg32_random generator `-a` is fast (~4GB/sec on same machine) compared to most other random byte generators.

+ File generation rates are slower and subject to a multitude of factors including: OS, OS activity, kernel patches, HDD versus SSD drive, SSD interface and underlying SSD technology etc.

In the code, there are faster ways to create files than using C's `fwrite()`, which RND64 uses.

On Linux, `write()` can be up to 4 times faster than `fwrite()` on some machines (using a single-threaded version of RND64, with file descriptor unclosed).  However, `write()` will only transfer a maximum of 2.1GB, even on 64-bit systems [[write(2)](http://man7.org/linux/man-pages/man2/write.2.html)]. `fwrite()` does not have this limitation, and 4GB+ output is what I sought.

Multi-threading has its own speed impacts, with thread-waiting and multiple memory buffers being combined.


## Credits

+ Professor Melissa E. O'Neill: creating the fast pcg_random number generator.
+ Damir Cohadarevic: inspiration, highlighting pcg_random.
+ Aleksandr Sergeev: testing, recommendations.
+ MSDN: Windows crypto and free system memory.
+ Ben Alpert: microsecond timer.


## License

RND64 is released under the [GPL v.3](https://www.gnu.org/licenses/gpl-3.0.html).
