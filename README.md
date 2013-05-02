A backtrace is a summary of how your program got where it is.
Unfortunately glibc's backtrace() and gdb's (bt) produce an unwind
path instead of true backtrace. This small library uses an unwind
informations to produce true backtrace with optionally demangled symbols
and allows you to embed backtracing facility into your application.
