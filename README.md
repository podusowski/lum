**This library is highly experimental and incomplete! It's OK to play around with the examples, but please don't use it in any of your projects!**


Tokio's [`loom`](https://docs.rs/loom/latest/loom/) inspired utility to test concurrency in your software.


What does it do?
----------------
There are various thread sanitizers which check whether your structures are synchronized, `lum` check your concurrent algorithms logic. Pretty vague, I know, let's see some examples.

Consider [following code](cases/nosync.cpp). Program has two threads. One (commented as supplementary) writes a value, second one (main thread) reads it. Is this program correct? Well, no. Writes and reads aren't synchronized. In fact, we can even see a data race in both `ThreadSanitizer` and `helgrind`:

``` 
WARNING: ThreadSanitizer: data race (pid=20480)
  Read of size 4 at 0x7ffeaa08cebc by main thread:
    #0 main <null> (nosync+0xd04d3) (BuildId: bdf7a3f9f6d85eec41db285485f6aa4944621de9)

  Previous write of size 4 at 0x7ffeaa08cebc by thread T1:
    #0 main::$_0::operator()() const nosync.cpp (nosync+0xd0af9) (BuildId: bdf7a3f9f6d85eec41db285485f6aa4944621de9)
```

This is pretty easy to fix, right? Just [put some mutexes here and there](cases/sync.cpp). `ThreadSanitizer` doesn't complain anymore, nor is `helgrind`. However, if we run the program couple of times, we'll see that it doesn't give predictable results:

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

The behavior is not deterministic because there is no way to tell which thread will acquire the mutex first. Sometimes it's acquired by the writer and we get `42`, but sometimes the variable id read by the main thread, resulting with still default value of `0`.

`lum` lets you write tests which will use different combinations of whom gets the lock first. In above example, test would run two times (though there is a small caveat here). In the first iteration, "calculating thread" will get the lock first, in the second iteration, the "reading thread" will get it first. Both will happen in a deterministic way.

The [test version](tests/test.cpp) behaves kind of like a Schrödinger's cat, taking both probability paths at once. If we run it, in fact we will see both cases:

```
value: 0
value: 42
```


Limitations and future plans
----------------------------
- Only single multithreading synchronization primitive is supported right now - a mutex. It can't test for example lock-free algorithms, nor ones using `std::conditional_variable` or other primitives.
- ~~`lum` works only with a single mutex.~~
- It does simple permutations of threads that are allowed to take the ownership of the mutexes. This potentially can lead to deadlocks.
- It's VERY intrusive right now. Not only you have to swap `std::mutex` with `lum::mutex`, ~~but it also has additional constructor parameter~~. There are couple of ways to solve it though.


Enabling logging
----------------
To enable `lum`'s internal debug prints, define a `LUM_TRACE` environment variable. Its value doesn't matter.


Cheat sheet
-----------
Building with clang's ThreadSanitizer:
```
cmake -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_CXX_FLAGS="-fsanitize=thread" -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -B build-tsan
cmake --build build-tsan
```
