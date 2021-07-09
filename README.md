FART
----

Fart (because I'm without imagination) is my own "standard library" for C++.

# What is FART?

It's a pretty comprehensive library, if I must say myself – and new features are constantly being added, as I need them.

## Features

As of time of writing, Fart supports the following.

* Automatic memory management through retain couting.
  * With support for self-nulling weak references.
* I/O
  * Socket
    * IPv4
    * IPv6
  * File system
* Types
  * Data (buffer for *non*-memory managed types)
  * Arrays (buffer for memory managed types)
  * Strings
    * Full Unicode support
    * UTF-8 support
    * UTF-16 support
    * UTF-32 support (internal storage format)
  * Dictionaries
  * Couples (Touples)
  * Dates
  * Durations
  * Numbers
* JSON serialization/deserialization
* Threading
  * Threads
  * Mutexes
  * Semaphores
* A build-in HTTP server

## Why

Because I love the interoperability of C++ – but I f***ng hate the C++ standard library!

## How to Use

The documentation is non-existing at the moment! I know how it works – and that is sufficient for me.

If you want to use it, then use it at your own risk. I use this in production code – but I wouldn't recommend you to do the same.

Check out the source code, if you want to learn how it works.

# License

See LICENSE.

