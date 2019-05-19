# Rax, an ANSI C radix tree implementation

Rax is a radix tree implementation initially written to be used in a specific
place of Redis in order to solve a performance problem, but immediately
converted into a stand alone project to make it reusable for Redis itself, outside the initial intended application, and for other projects as well.

The primary goal was to find a suitable balance between performances
and memory usage, while providing a fully featured implementation of radix trees
that can cope with many different requirements.

During the development of this library, while getting more and more excited
about how practical and applicable radix trees are, I was very surprised to
see how hard it is to write a robust implementation, especially of a fully
featured radix tree with a flexible iterator. A lot of things can go wrong
in node splitting, merging, and various edge cases. For this reason a major
goal of the project is to provide a stable and battle tested implementation
for people to use and in order to share bug fixes. The project relies a lot
on fuzz testing techniques in order to explore not just all the lines of code
the project is composed of, but a large amount of possible states.

Rax is an open source project, released under the BSD two clause license.

Major features:

* Memory conscious:
    + Packed nodes representation.
    + Able to avoid storing a NULL pointer inside the node if the key is set to NULL (there is an `isnull` bit in the node header).
    + Lack of parent node reference. A stack is used instead when needed.
* Fast lookups:
    + Edges are stored as arrays of bytes directly in the parent node, no need to access non useful children while trying to find a match. This translates into less cache misses compared to other implementations.
    + Cache line friendly scanning of the correct child by storing edges as two separated arrays: an array of edge chars and one of edge pointers.
* Complete implementation:
    + Deletion with nodes re-compression as needed.
    + Iterators (including a way to use iterators while the tree is modified).
    + Random walk iteration.
    + Ability to report and resist out of memory: if malloc() returns NULL the API can report an out of memory error and always leave the tree in a consistent state.
* Readable and fixable implementation:
    + All complex parts are commented with algorithms details.
    + Debugging messages can be enabled to understand what the implementation is doing when calling a given function.
    + Ability to print the radix tree nodes representation as ASCII art.
* Portable implementation:
    + Never does unaligned accesses to memory.
    + Written in ANSI C99, no extensions used.
* Extensive code and possible states test coverage using fuzz testing.
    + Testing relies a lot on fuzzing in order to explore non trivial states.
    + Implementation of the dictionary and iterator compared with behavior-equivalent implementations of simple hash tables and sorted arrays, generating random data and checking if the two implementations results match.
    + Out of memory condition tests. The implementation is fuzzed with a special allocator returning `NULL` at random. The resulting radix tree is tested for consistency. Redis, the primary target of this implementation, does not use this feature, but the ability to handle OOM may make this implementation useful where the ability to survive OOMs is needed.
    + Part of Redis: the implementation is stressed significantly in the real world.

The layout of a node is as follows. In the example, a node which represents
a key (so has a data pointer associated), has three children `x`, `y`, `z`.
Every space represents a byte in the diagram.

    +----+---+--------+--------+--------+--------+
    |HDR |xyz| x-ptr  | y-ptr  | z-ptr  |dataptr |
    +----+---+--------+--------+--------+--------+

The header `HDR` is actually a bitfield with the following fields:

    uint32_t iskey:1;     /* Does this node contain a key? */
    uint32_t isnull:1;    /* Associated value is NULL (don't store it). */
    uint32_t iscompr:1;   /* Node is compressed. */
    uint32_t size:29;     /* Number of children, or compressed string len. */

Compressed nodes represent chains of nodes that are not keys and have
exactly a single child, so instead of storing:

    A -> B -> C -> [some other node]

We store a compressed node in the form:

    "ABC" -> [some other node]

The layout of a compressed node is:

    +----+---+--------+
    |HDR |ABC|chld-ptr|
    +----+---+--------+

# Basic API

The basic API is a trivial dictionary where you can add or remove elements.
The only notable difference is that the insert and remove APIs also accept
an optional argument in order to return, by reference, the old value stored
at a key when it is updated (on insert) or removed.

## Creating a radix tree and adding a key

A new radix tree is created with:

    rax *rt = raxNew();

In order to insert a new key, the following function is used:

    int raxInsert(rax *rax, unsigned char *s, size_t len, void *data,
                  void **old);

Example usage:

    raxInsert(rt,(unsigned char*)"mykey",5,some_void_value,NULL);

The function returns 1 if the key was inserted correctly, or 0 if the key
was already in the radix tree: in this case, the value is updated. The
value of 0 is also returned on out of memory, however in that case
`errno` is set to `ENOMEM`.

If the associated value `data` is NULL, the node where the key
is stored does not use additional memory to store the NULL value, so
dictionaries composed of just keys are memory efficient if you use
NULL as associated value.

Note that keys are unsigned arrays of chars and you need to specify the
length: Rax is binary safe, so the key can be anything.

The insertion function is also available in a variant that will not
overwrite the existing key value if any:

    int raxTryInsert(rax *rax, unsigned char *s, size_t len, void *data,
                     void **old);

The function is exactly the same as raxInsert(), however if the key
exists the function returns 0 (like raxInsert) without touching the
old value. The old value can be still returned via the 'old' pointer
by reference.

## Key lookup

The lookup function is the following:

    void *raxFind(rax *rax, unsigned char *s, size_t len);

This function returns the special value `raxNotFound` if the key you
are trying to access is not there, so an example usage is the following:

    void *data = raxFind(rax,mykey,mykey_len);
    if (data == raxNotFound) return;
    printf("Key value is %p\n", data);

raxFind() is a read only function so no out of memory conditions are
possible, the function never fails.

## Deleting keys

Deleting the key is as you could imagine it, but with the ability to
return by reference the value associated to the key we are about to
delete:

    int raxRemove(rax *rax, unsigned char *s, size_t len, void **old);

The function returns 1 if the key gets deleted, or 0 if the key was not
there. This function also does not fail for out of memory, however if
there is an out of memory condition while a key is being deleted, the
resulting tree nodes may not get re-compressed even if possible: the radix
tree may be less efficiently encoded in this case.

The `old` argument is optional, if passed will be set to the key associated
value if the function successfully finds and removes the key.

# Iterators

The Rax key space is ordered lexicographically, using the value of the
bytes the keys are composed of in order to decide which key is greater
between two keys. If the prefix is the same, the longer key is considered
to be greater.

Rax iterators allow to seek a given element based on different operators
and then to navigate the key space calling `raxNext()` and `raxPrev()`.

## Basic iterator usage

Iterators are normally declared as local variables allocated on the stack,
and then initialized with the `raxStart` function:

    raxIterator iter;
    raxStart(&iter, rt); // Note that 'rt' is the radix tree pointer.

The function `raxStart` never fails and returns no value.
Once an iterator is initialized, it can be sought (sought is the past tens
of 'seek', which is not 'seeked', in case you wonder) in order to start
the iteration from the specified position. For this goal, the function
`raxSeek` is used:

    int raxSeek(raxIterator *it, unsigned char *ele, size_t len, const char *op);

For instance one may want to seek the first element greater or equal to the
key `"foo"`:

    raxSeek(&iter,">=",(unsigned char*)"foo",3);

The function raxSeek() returns 1 on success, or 0 on failure. Possible failures are:

1. An invalid operator was passed as last argument.
2. An out of memory condition happened while seeking the iterator.

Once the iterator is sought, it is possible to iterate using the function
`raxNext` and `raxPrev` as in the following example:

    while(raxNext(&iter)) {
        printf("Key: %.*s\n", (int)iter.key_len, (char*)iter.key);
    }

The function `raxNext` returns elements starting from the element sought
with `raxSeek`, till the final element of the tree. When there are no more
elements, 0 is returned, otherwise the function returns 1. However the function
may return 0 when an out of memory condition happens as well: while it attempts
to always use the stack, if the tree depth is large or the keys are big the
iterator starts to use heap allocated memory.

The function `raxPrev` works exactly in the same way, but will move towards
the first element of the radix tree instead of moving towards the last
element.

# Releasing iterators

An iterator can be used multiple times, and can be sought again and again
using `raxSeek` without any need to call `raxStart` again. However, when the
iterator is not going to be used again, its memory must be reclaimed
with the following call:

    raxStop(&iter);

Note that even if you do not call `raxStop`, most of the times you'll not
detect any memory leak, but this is just a side effect of how the
Rax implementation works: most of the times it will try to use the stack
allocated data structures. However for deep trees or large keys, heap memory
will be allocated, and failing to call `raxStop` will result into a memory
leak.

## Seek operators

The function `raxSeek` can seek different elements based on the operator.
For instance in the example above we used the following call:

    raxSeek(&iter,">=",(unsigned char*)"foo",3);

In order to seek the first element `>=` to the string `"foo"`. However
other operators are available. The first set are pretty obvious:

* `==` seek the element exactly equal to the given one.
* `>` seek the element immediately greater than the given one.
* `>=` seek the element equal, or immediately greater than the given one.
* `<` seek the element immediately smaller than the given one.
* `<=` seek the element equal, or immediately smaller than the given one.
* `^` seek the smallest element of the radix tree.
* `$` seek the greatest element of the radix tree.

When the last two operators, `^` or `$` are used, the key and key length
argument passed are completely ignored since they are not relevant.

Note how certain times the seek will be impossible, for example when the
radix tree contains no elements or when we are asking for a seek that is
not possible, like in the following case:

    raxSeek(&iter,">",(unsigned char*)"zzzzz",5);

We may not have any element greater than `"zzzzz"`. In this case, what
happens is that the first call to `raxNext` or `raxPrev` will simply return
zero, so no elements are iterated.

## Iterator stop condition

Sometimes we want to iterate specific ranges, for example from AAA to BBB.
In order to do so, we could seek and get the next element. However we need
to stop once the returned key is greater than BBB. The Rax library offers
the `raxCompare` function in order to avoid you need to code the same string
comparison function again and again based on the exact iteration you are
doing:

    raxIterator iter;
    raxStart(&iter);
    raxSeek(&iter,">=",(unsigned char*)"AAA",3); // Seek the first element
    while(raxNext(&iter)) {
        if (raxCompare(&iter,">",(unsigned char*)"BBB",3)) break;
        printf("Current key: %.*s\n", (int)iter.key_len,(char*)iter.key);
    }
    raxStop(&iter);

The above code shows a complete range iterator just printing the keys
traversed by iterating.

The prototype of the `raxCompare` function is the following:

    int raxCompare(raxIterator *iter, const char *op, unsigned char *key, size_t key_len);

The operators supported are `>`, `>=`, `<`, `<=`, `==`.
The function returns 1 if the current iterator key satisfies the operator
compared to the provided key, otherwise 0 is returned.

## Checking for iterator EOF condition

Sometimes we want to know if the itereator is in EOF state before calling
raxNext() or raxPrev(). The iterator EOF condition happens when there are
no more elements to return via raxNext() or raxPrev() call, because either
raxSeek() failed to seek the requested element, or because EOF was reached
while navigating the tree with raxPrev() and raxNext() calls.

This condition can be tested with the following function that returns 1
if EOF was reached:

    int raxEOF(raxIterator *it);

## Modifying the radix tree while iterating

In order to be efficient, the Rax iterator caches the exact node we are at,
so that at the next iteration step, it can start from where it left.
However an iterator has sufficient state in order to re-seek again
in case the cached node pointers are no longer valid. This problem happens
when we want to modify a radix tree during an iteration. A common pattern
is, for instance, deleting all the elements that match a given condition.

Fortunately there is a very simple way to do this, and the efficiency cost
is only paid as needed, that is, only when the tree is actually modified.
The solution consists of seeking the iterator again, with the current key,
once the tree is modified, like in the following example:

    while(raxNext(&iter,...)) {
        if (raxRemove(rax,...)) {
            raxSeek(&iter,">",iter.key,iter.key_size);
        }
    }

In the above case we are iterating with `raxNext`, so we are going towards
lexicographically greater elements. Every time we remove an element, what we
need to do is to seek it again using the current element and the `>` seek
operator: this way we'll move to the next element with a new state representing
the current radix tree (after the change).

The same idea can be used in different contexts, considering the following:

* Iterators need to be sought again with `raxSeek` every time keys are added or removed while iterating.
* The current iterator key is always valid to access via `iter.key_size` and `iter.key`, even after it was deleted from the radix tree.

## Re-seeking iterators after EOF

After iteration reaches an EOF condition since there are no more elements
to return, because we reached one or the other end of the radix tree, the
EOF condition is permanent, and even iterating in the reverse direction will
not produce any result.

The simplest way to continue the iteration, starting again from the last
element returned by the iterator, is simply to seek itself:

    raxSeek(&iter,iter.key,iter.key_len,"==");

So for example in order to write a command that prints all the elements
of a radix tree from the first to the last, and later again from the last
to the first, reusing the same iterator, it is possible to use the following
approach:

    raxSeek(&iter,"^",NULL,0);
    while(raxNext(&iter,NULL,0,NULL))
        printf("%.*s\n", (int)iter.key_len, (char*)iter.key);

    raxSeek(&iter,"==",iter.key,iter.key_len);
    while(raxPrev(&iter,NULL,0,NULL))
        printf("%.*s\n", (int)iter.key_len, (char*)iter.key);

## Random element selection

To extract a fair element from a radix tree so that every element is returned
with the same probability is not possible if we require that:

1. The radix tree is not larger than expected (for example augmented with information that allows elements ranking).
2. We want the operation to be fast, at worst logarithmic (so things like reservoir sampling are out since it's O(N)).

However a random walk which is long enough, in trees that are more or less balanced, produces acceptable results, is fast, and eventually returns every possible element, even if not with the right probability.

To perform a random walk, just seek an iterator anywhere and call the
following function:

    int raxRandomWalk(raxIterator *it, size_t steps);

If the number of steps is set to 0, the function will perform a number of
random walk steps between 1 and two times the logarithm in base two of the
number of elements inside the tree, which is often enough to get a decent
result. Otherwise, you may specify the exact number of steps to take.

## Printing trees

For debugging purposes, or educational ones, it is possible to use the
following call in order to get an ASCII art representation of a radix tree
and the nodes it is composed of:

    raxShow(mytree);

However note that this works well enough for trees with a few elements, but
becomes hard to read for very large trees.

The following is an example of the output raxShow() produces after adding
the specified keys and values:

* alligator = (nil)
* alien = 0x1
* baloon = 0x2
* chromodynamic = 0x3
* romane = 0x4
* romanus = 0x5
* romulus = 0x6
* rubens = 0x7
* ruber = 0x8
* rubicon = 0x9
* rubicundus = 0xa
* all = 0xb
* rub = 0xc
* ba = 0xd

```
[abcr]
 `-(a) [l] -> [il]
               `-(i) "en" -> []=0x1
               `-(l) "igator"=0xb -> []=(nil)
 `-(b) [a] -> "loon"=0xd -> []=0x2
 `-(c) "hromodynamic" -> []=0x3
 `-(r) [ou]
        `-(o) [m] -> [au]
                      `-(a) [n] -> [eu]
                                    `-(e) []=0x4
                                    `-(u) [s] -> []=0x5
                      `-(u) "lus" -> []=0x6
        `-(u) [b] -> [ei]=0xc
                      `-(e) [nr]
                             `-(n) [s] -> []=0x7
                             `-(r) []=0x8
                      `-(i) [c] -> [ou]
                                    `-(o) [n] -> []=0x9
                                    `-(u) "ndus" -> []=0xa
```

# Running the Rax tests

To run the tests try:

    $ make
    $ ./rax-test

To run the benchmark:

    $ make
    $ ./rax-test --bench

To test Rax under OOM conditions:

    $ make
    $ ./rax-oom-test

The last one is very verbose currently.

In order to test with Valgrind, just run the tests using it, however
if you want accurate leaks detection, let Valgrind run the *whole* test,
since if you stop it earlier it will detect a lot of false positive memory
leaks. This is due to the fact that Rax put pointers at unaligned addresses
with `memcpy`, so it is not obvious where pointers are stored for Valgrind,
that will detect the leaks. However, at the end of the test, Valgrind will
detect that all the allocations were later freed, and will report that
there are no leaks.

# Debugging Rax

While investigating problems in Rax it is possible to turn debugging messages
on by compiling with the macro `RAX_DEBUG_MSG` enabled. Note that it's a lot
of output, and may make running large tests too slow.

In order to active debugging selectively in a dynamic way, it is possible to
use the function raxSetDebugMsg(0) or raxSetDebugMsg(1) to disable/enable
debugging.

A problem when debugging code doing complex memory operations like a radix
tree implemented the way Rax is implemented, is to understand where the bug
happens (for instance a memory corruption). For that goal it is possible to
use the function raxTouch() that will basically recursively access every
node in the radix tree, itearting every sub child. In combination with
tools like Valgrind, it is possible then to perform the following pattern
in order to narrow down the state causing a give bug:

1. The rax-test is executed using Valgrind, adding a printf() so that for
   the fuzz tester we see what iteration in the loop we are in.
2. After every modification of the radix tree made by the fuzz tester
   in rax-test.c, we add a call to raxTouch().
3. Now as soon as an operation will corrupt the tree, raxTouch() will
   detect it (via Valgrind) immediately. We can add more calls to narrow
   the state.
4. At this point a good idea is to enable Rax debugging messages immediately
   before the moment the tree is corrupted, to see what happens. This can
   be achieved by adding a few "if" statements inside the code, since we
   know the iteration that causes the corruption (because of step 1).

This method was used with success during rafactorings in order to debug the
introduced bugs.
