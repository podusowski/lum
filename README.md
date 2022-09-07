Tokio's [`loom`](https://docs.rs/loom/latest/loom/) inspired utility to test concurrency in your software.


What does it do?
----------------
There are various thread sanitizers which check whether your structures are synchronized, `lum` check your concurrent algorithms logic. Pretty vague, I know, let's see some examples.

Consider [following program](cases/nosync.cpp). Program has two threads. One (commented as supplementary) writes a value, second one (main thread) reads it. Is this program correct? Well, no. Writes and reads aren't synchronized. In fact, we can even see a data rate in ThreadSanitizer:

``` 
WARNING: ThreadSanitizer: data race (pid=20480)
  Read of size 4 at 0x7ffeaa08cebc by main thread:
    #0 main <null> (nosync+0xd04d3) (BuildId: bdf7a3f9f6d85eec41db285485f6aa4944621de9)

  Previous write of size 4 at 0x7ffeaa08cebc by thread T1:
    #0 main::$_0::operator()() const nosync.cpp (nosync+0xd0af9) (BuildId: bdf7a3f9f6d85eec41db285485f6aa4944621de9)
```


Cheat sheet
-----------
Building with clang's ThreadSanitizer:
```
cmake -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_CXX_FLAGS="-fsanitize=thread" -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -B build-tsan
cmake --build build-tsan
```
