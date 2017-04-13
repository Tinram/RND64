
# RND64

### 64-bit multi-threading random data generator


##### RND64 v.0.35

##### Linux and Windows


## Purpose

Generate large files (non-sparse) and large streams of random data (4GB+) at fast generation rates (~1.5GB/sec stream output on Intel Xeon AWS Linux microinstance, -f option).


## OS Support

+ Linux x64

+ Windows x64


## Usage

    rnd64 [option] <size> <file>
    rnd64 [option] <size> | <program>

#### Options

    -a     (all)             characters 1 to 255        includes control codes
    -f     (fastest)         character zero (48)        fastest generator
    -r     (restrict)        characters 33 to 126       7-bit printable ASCII, safe for terminal output
    -c     (crypto)          crypto-sourced bytes       Linux: /dev/urandom, Windows: CryptGenRandom
                                                            (much slower byte generation)

    size   1K, 100M, 3G


### Usage Examples

    rnd64.exe or rnd64   (Windows)        display command-line options, as above
    ./rnd64              (Linux)

    rnd64 -a 1k test.txt                  output 1kB of random bytes in the range 1 to 255 to a file called test.txt
    rnd64 -f 1k test.txt                  output 1kB of zeros to test.txt
    rnd64 -r 1k test.txt                  output a restricted range of 7-bit ASCII characters (33 to 126) to test.txt
    rnd64 -f 500m | pv > /dev/null        send 500MB of zeros to /dev/null with 'pv' displaying the throughput rate
    rnd64 -c 1k | ent                     pipe 1kB of crypto bytes to 'ent' for entropy checking
    rnd64 -a 1k | nc 192.168.1.20 80      pipe 1kB of random bytes to 'netcat' to send to 192.168.1.20 on port 80


###### WARNING: RND64 depends upon the free memory of the system for data dumping (writing to a file in a single action, not in chunks). For large file dumps (over 1GB) consider first the free memory, and if using a mechanical hard drive, the drive's age and performance. On Linux systems, a large GB value that works on a freshly booted system, may not work on the same busy long-running system, because of caching and application requirements.


## Build

In the directory containing either the clone or the extracted zip files, compile with GCC x64:

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

    -march=core-avx-i -mtune=core-avx-i            Intel Ivy Bridge
    -march=core-avx2 -mtune=core-avx2              Intel Haswell
    -march=skylake-avx512 -mtune=skylake-avx512    Intel Skylake

    gcc -Q -march=native --help=target             detect current CPU options to use in the above GCC switches


## Other

The switches `-a` and `-c` are dangerous options when both a filename and pipe symbol are omitted. The wide range of output bytes, including control characters, are printed in the terminal, which can cause the terminal to lock or crash (especially on Windows).

On both Linux and Windows, it's more convenient for RND64 to be available from any directory location via the PATH system variable (rather than copying the executable file to the directory where needed).

#### Linux

    make install

Or move the RND64 executable to a location such as */usr/local/bin*  (location must be present in $PATH).

#### Windows

[Windows/Super key + Break] > Advanced tab > Environmental Variables button > click Path line > Edit button > Variable value - append at the end of existing line information: *C:\directory\path\to\rnd64.exe\;*


## Speed

RND64 is fast, but not that fast. Stream output rates are decent. For file output, however, there are faster ways to create files than using C's `fwrite()`, which RND64 uses.

On Linux, `write()` can be up to 4 times faster than `fwrite()` on some machines (using a single-threaded version of RND64, with file descriptor unclosed).  However, `write()` will only transfer a maximum of 2.1GB, even on 64-bit systems [[write(2)](http://man7.org/linux/man-pages/man2/write.2.html)]. `fwrite()` does not have this limitation.

Multi-threading has its own speed impacts, with thread-waiting and multiple memory buffers being combined. The single-threaded version of RND64 is slower for data generation but faster for file output.

Chunking output on memory boundaries is another technique used by programmers. So far in my experiments, this has not produced favourable results with multi-threading.


## Credits

+ Aleksandr Sergeev: testing, recommendations.
+ Travis Gockel: Linux free memory report.
+ MSDN: Windows crypto and free system memory.
+ Ben Alpert: microsecond timer.


## License

RND64 is released under the [GPL v.3](https://www.gnu.org/licenses/gpl-3.0.html).
