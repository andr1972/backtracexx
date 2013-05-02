A backtrace is a summary of how your program got where it is.
Unfortunately glibc's backtrace() and gdb's (bt) produce an unwind
path instead of true backtrace. This small library uses unwind
information and produces true backtrace with optionally demangled symbols.
it allows you to embed backtracing facility into your application.

Sources available at: http://svn.pld-linux.org/cgi-bin/viewsvn/backtracexx/
