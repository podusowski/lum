**This library is highly experimental and incomplete! It's OK to play around with the examples, but please don't use it in any of your projects!**


Tokio's [`loom`](https://docs.rs/loom/latest/loom/) inspired utility to test concurrency in your software.


What does it do?
----------------
There are various thread sanitizers which check whether your structures are synchronized, `lum` check your concurrent algorithms logic. Pretty vague, I know, let's see some examples.

Consider [following code](cases/nosync.cpp). Program has two threads. One (commented as supplementary) writes a value, second one (main thread) reads it. Is this program correct? Well, no. Writes and reads aren't synchronized. In fact, we can even see a data race in ThreadSanitizer:

``` 
WARNING: ThreadSanitizer: data race (pid=20480)
  Read of size 4 at 0x7ffeaa08cebc by main thread:
    #0 main <null> (nosync+0xd04d3) (BuildId: bdf7a3f9f6d85eec41db285485f6aa4944621de9)

  Previous write of size 4 at 0x7ffeaa08cebc by thread T1:
    #0 main::$_0::operator()() const nosync.cpp (nosync+0xd0af9) (BuildId: bdf7a3f9f6d85eec41db285485f6aa4944621de9)
```

This is pretty easy to fix, right? Just [put some mutexes here and there](cases/sync.cpp). ThreadSanitizer doesn't complain anymore, we run the program couple of times to be sure and:

```
$ while true; do ./sync; done
value: 42
value: 42
value: 42
value: 0
value: 42
value: 42
value: 42
value: 42
value: 0
value: 42
value: 42
```

It still doesn't work as expected. The behavior is not deterministic, because there is no way to tell which thread will acquire the mutex first. Sometimes it's acquired by the writer and we get `42`, but sometimes the variable id read by the main thread, resulting with still default value of `0`.

`lum` lets you write tests which will use different combinations of whom gets the lock first. In above example, test would run two times (though there is a small caveat here). In the first iteration, "calculating thread" will get the lock first, in the second iteration, the "reading thread" will get it first. Both will happen in a deterministic way.

The [test version](tests/test.cpp) of the above problem explicitly requires that both possibilities occurs.


Cheat sheet
-----------
Building with clang's ThreadSanitizer:
```
cmake -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_CXX_FLAGS="-fsanitize=thread" -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -B build-tsan
cmake --build build-tsan
```
