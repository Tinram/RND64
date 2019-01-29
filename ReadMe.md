
# RND64

### Fast multi-threaded 64-bit data generator for junk files, benchmarks, and stress tests.

#### Linux and Windows

##### RND64 v.0.41


[1]: https://tinram.github.io/images/rnd64.png
![rnd64][1]


## Purpose

Generate large files (over 4GB, non-sparse) and large streams of binary/character data (200GB+) at fast generation rates (~8.5GB/sec i3 desktop, ~4.6GB/sec AWS microinstance; null byte stream output on Linux).

*What's the point of lumps of junk?*  
Uses can be: file hashing, integrity tests, system stress testing, and network speed tests.

A few Windows programs exist to create large files, and there are plenty of shell scripts using `dd`  
I just needed something cross-platform with simple command-line options.


## OS Support

+ Linux x64
+ Windows x64


## Usage

```bash
    rnd64 [option] <size> <file>
    rnd64 [option] <size> | <program>
```

#### Options

    -a     (all)             binary bytes               includes control codes
    -f     (fastest)         null character (0)         fastest generator
    -r     (restrict)        characters 33 to 126       7-bit printable ASCII, safe for terminal output
    -c     (crypto)          crypto-sourced bytes       Linux: /dev/urandom, Windows: CryptGenRandom (slow)

    size   1K, 100M, 8G


### Usage Examples

    rnd64.exe or rnd64   (Windows)        display command-line options, as above
    ./rnd64              (Linux)

    rnd64 -a 1k f.txt                          output 1kB of random binary bytes to the file 'f.txt'
    rnd64 -f 1k f.txt                          output 1kB of null bytes to 'f.txt'
    rnd64 -r 1k f.txt                          output the restricted range of 7-bit ASCII characters (33 to 126) to 'f.txt'
    rnd64 -f 4g | pv > /dev/null               send 4GB of null bytes to /dev/null with 'pv' displaying the throughput rate (Linux)
    rnd64 -c 1k | ent                          pipe 1kB of crypto bytes to the program 'ent'
    rnd64 -a 1k | nc 192.168.1.20 80           pipe 1kB of random bytes to 'netcat' to send to 192.168.1.20 on port 80
    rnd64 -f 100g | pv > /dev/null             stress a system

    nc -lk -p 3000 > /dev/null                 local network speed test (machine receiving, 192.168.1.20)
    rnd64 -f 1g | pv | nc 192.168.1.20 3000    (machine sending)


### Warning!

When using RND64 to generate large files (over 1GB):

+ HDDs: consider the mechanical drive's age and performance,
+ SSDs: consider the potential write wear.

As well as warming the CPU, RND64 is quite capable of exhausting all drive space, finishing off failing HDDs, and reducing SSD lifetimes.


## Executables

Download from [Releases](https://github.com/Tinram/RND64/releases/latest) or directly:

+ Linux: [rnd64](https://github.com/Tinram/RND64/raw/master/bin/rnd64)
+ Windows: [rnd64.exe](https://github.com/Tinram/RND64/raw/master/bin/rnd64.exe)


## Build

```bash
    git clone https://github.com/Tinram/RND64.git
    cd RND64/src
```

### Linux

```bash
    make
```

or full process:

```bash
    make && make install && make clean
```

(Default compiler is GCC. For Clang, just rename the *makefiles*.)

----

#### Manual compilation:

**GCC:**

```bash
    gcc rnd64.c -o rnd64 -lpthread -O3 -Wall -Wextra -Wuninitialized -Wunused -Werror -std=gnu99 -s
```

**Clang:**

```bash
    clang rnd64.c -o rnd64 -lpthread -O3 -Wall -Wextra -Wuninitialized -Wunused -Werror -s
```

##### Further Optimisation

    -mtune=native -march=native                    current CPU
    -flto                                          linker optimize

    -march=core-avx2 -mtune=core-avx2              Intel Haswell
    -march=skylake-avx512 -mtune=skylake-avx512    Intel Skylake

----

### Windows

```bash
    gcc rnd64.c -o rnd64.exe -O3 -Wall -Wextra -Wuninitialized -Wunused -Werror -std=c99 -s
```


## Other

The switches `-a` and `-c` are dangerous options when both a filename and pipe symbol are omitted. The wide range of output bytes, including control characters, are printed in the terminal, which can cause the terminal to lock or crash (especially on Windows).

On both Linux and Windows, it's more convenient for RND64 to be available from any directory location via the *$PATH* system variable.

#### Linux

```bash
    make install
```

Or move the RND64 executable to a location such as */usr/local/bin* (location must be present in *$PATH*).

#### Windows

[Windows/Super key + Break] > Advanced tab > Environmental Variables button > click Path line > Edit button > Variable value &ndash; append at the end of existing line information: *C:\directory\path\to\rnd64.exe\;*


## Speed

### Linux

**RND64 is fast:**

*i3-4170 CPU 3.70GHz, 4.4 kernel*:

        martin@xyz ~ $ rnd64 -f 4g | pv > /dev/null
        4GiB 0:00:00 [8.51GiB/s] [  <=>  ]

`dd` on same machine:

        martin@xyz ~ $ dd if=/dev/zero of=/dev/null bs=4G count=1 iflag=fullblock
        4294967296 bytes (4.3 GB, 4.0 GiB) copied, 0.959431 s, 4.5 GB/s

*AWS Xeon E5-2670 2.50GHz, single core*:

        [ec2-user@ip-172-31-7-109 ~]$ rnd64 -f 4g | pv > /dev/null
        4GiB 0:00:00 [4.61GiB/s] [     <=>     ]

+ Null byte stream generation rates `-f` are decent on Linux (~8GB/sec on vanilla i3-4170), and the [PCG](http://www.pcg-random.org/) random number generator `-a` is pretty fast (~4GB/sec on same CPU) compared to most other RNGs.

**... but not that fast:**

+ File generation rates are slower and subject to a multitude of factors including: OS, OS activity, kernel version, kernel patches, HDD versus SSD drive, SSD interface and underlying SSD technology etc.

In the code, there are faster ways to create files than using C's `fwrite()`, which RND64 uses.

On Linux, `write()` can be up to 4 times faster than `fwrite()` on some machines (using a single-threaded version of RND64, with file descriptor unclosed).  However, `write()` will only transfer a maximum of 2.1GB, even on 64-bit systems [[write(2)](http://man7.org/linux/man-pages/man2/write.2.html)]. `fwrite()` does not have this limitation, and 4GB+ output is what I sought.

Multi-threading has its own speed impacts, such as thread-waiting and data streams being combined.

### Windows

With Windows lacking `pv`, stream output speed is somewhat more difficult to assess.

One way is to provide the stats output on *stderr*.  
This can be enabled by setting a flag in the source.

In *rnd64.h*, set the following macro value to 1:

```c
    #define STREAM_STATS 0 /* Win stream stats */
```

then compile the source as in the *Build > Windows* section.

        C:\rnd64.exe -f 4g > nul
        time: 9 s 938 ms
        MB/s: 412.15


## Credits

+ [Professor Melissa E. O'Neill](https://www.cs.hmc.edu/~oneill/index.html): creator of the fast [PCG](http://www.pcg-random.org/) RNG.
+ [Damir Cohadarevic](https://github.com/cohadar): inspiration, highlighting PCG.
+ [Aleksandr Sergeev](https://github.com/sergeevabc): thorough testing, recommendations.
+ MSDN: Windows crypto.
+ Ben Alpert: microsecond timer.


## License

RND64 is released under the [GPL v.3](https://www.gnu.org/licenses/gpl-3.0.html).
