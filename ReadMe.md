
# RND64

### 64-bit multi-threading random data generator


##### RND64 v.0.28

##### Linux and Windows


## Purpose

Generate large files (non-sparse) and large streams of random data (4GB+).


## OS Support

Linux x64

Windows x64


## Usage

    rnd64 [option] <size> <file>
    rnd64 [option] <size> | <program>

#### Options

    -a     (all)             characters 1 to 255        includes control codes
    -f     (fastest)         character zero (48)        fastest generator
    -r     (restrict)        characters 33 to 126       safe for terminal output
    -c     (crypto)          crypto-sourced bytes       Linux: /dev/urandom, Windows: CryptGenRandom
                                                            (much slower byte generation)

    size   1K, 100M, 3G


### Usage Examples

    rnd64.exe or rnd64   (Windows)        display command-line options, as above
    ./rnd64              (Linux)

    rnd64 -a 1k test.txt                  output 1kB of random bytes in the range 1 to 255 to a file called test.txt
    rnd64 -f 1k test.txt                  output 1kB of zeros to test.txt
    rnd64 -r 1k test.txt                  output a restricted range of 7-bit ASCII characters (33 to 126) to test.txt
    rnd -c 1k | ent                       pipe 1kB of crypto bytes to the program *ent* for entropy checking
    rnd -a 1k | nc 192.168.1.20 80        pipe 1kB of random bytes to *netcat* to send to 192.168.1.20 on port 80 (test server response)


###### WARNING: Be careful of the amount of data generated in regards to the available memory of your PC for creating data, and the age and performance of your hard-drive for writing data.  A file dump of more than 1GB can be brutal!


## Build

Compile with GCC x64:

### Linux

    gcc rnd64.c -o rnd64 -lpthread -O3 -Wall -Wextra -Wuninitialized -Wunused -Werror -s

### Windows

    gcc rnd64.c -o rnd64.exe -O3 -Wall -Wextra -Wuninitialized -Wunused -Werror -s 

#### Further Optimisation

    -march=core-avx-i -mtune=core-avx-i        Intel Ivy Bridge
    -march=core-avx2 -mtune=core-avx2          Intel Haswell

    gcc -Q -march=native --help=target         detect current CPU options to use in the above gcc switches


## Other

The switches `-a` and `-c` are dangerous options when both a filename and pipe symbol are omitted. The wide range of output bytes, including control characters, are printed in the terminal, which can cause the terminal to lock or crash (especially on Windows).

On both Linux and Windows, it's more convenient for rnd64 to be available from any directory location via the PATH system variable (rather than copying the executable file to the directory where needed).

#### Linux

Move the rnd64 executable to a location such as */usr/local/bin*  (location must be present in $PATH).

#### Windows

(Windows/Super key + Break) > Advanced tab > Environmental Variables button > click Path line > Edit button > Variable value - append at the end of existing line information: *C:\directory paths\to rnd64.exe\;*


## License

RND64 is released under the [GPL v.3](https://www.gnu.org/licenses/gpl-3.0.html).
