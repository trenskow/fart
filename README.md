Fart
----

Fart is my own "standard library" for C++.

# What is Fart?

It's a pretty comprehensive library, if I must say myself – and new features are constantly being added, as I need them.

## Features

As of time of writing, Fart supports the following.

* Automatic memory management through retain counting.
  * With support for self-nulling weak references.
* I/O
  * Socket
    * IPv4
    * IPv6
  * File system
* Types
  * String
    * Full Unicode support
    * UTF-8 support
    * UTF-16 support
    * UTF-32 support (internal storage format)
  * Dictionary
  * Array (buffer for memory managed types)
    * Backend for Dictionary
  * Data (buffer for *non*-memory managed types)
    * Backend for both String and Array
    * Uses copy-on-write
  * Couple (Touple)
  * Date
    * Full ISO-8601 support
    * Support for leap year
    * Can convert to and from UTC and local timezone
  * Duration
  * Number
    * Memory managed wrapper for primitive types
* JSON serialization/deserialization
  * Test suites are available through the [fart-json-test](https://github.com/trenskow/fart-json-test) repository.
* Threading
  * Threads
  * Mutexes
  * Semaphores
* A build-in HTTP server

## Why

Because I love the interoperability of C++ – but I f***ng hate the C++ standard library! It's a necessary evil – thereby it's name.

## How to Use

The documentation is non-existing at the moment! I know how it works – and that is sufficient for me.

If you want to use it, then use it at your own risk. I use this in production code – but I wouldn't recommend you to do the same.

Check out the source code, if you want to learn how it works.

# License

See LICENSE.

