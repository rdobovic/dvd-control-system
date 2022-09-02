# DVD control system error decoder

Error decoder is very simple program written in C, used for easier and faster decoding of error codes displayed by DVD control system. To decode error code, start the program and type in all characters displayed after `0x`. Decoder will than display all error flags and their values, those flags with value of `1` are errors that accured.

To compile the program on Linux OS with gcc compiler, navigate to this directory and type

```
gcc -o errordecoder error_decoder.c
```

After that you can run program with
```
./errordecoder
```