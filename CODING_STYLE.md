Headers
=======

Header Order - Source Files
---------------------------
    "common.h"

    <sys.h>

    "myclass.h"
    "my_superclass.h"
    "my_super_or_friendclass_internal.h"

    "other_modules.h" (few as possible)

Header Order - Tests
--------------------
    "common.h"

    <sys.h>

    <check.h>
    "tests.h"
    "other_test_headers.h" (e.g. stubs)

    "class_under_test.h"
    "other_production_classes.h" (few as possible)

Modularisation
==============
Public headers go in /src. Module's private headers go in the module dir. These are NOT on the include path. It is NOT acceptable to include headers with a path, this preventing the inclusion of private headers.

Testing
=======

Valgrind
--------
    valgrind --track-fds=yes --tool=memcheck --leak-check=full --show-possibly-lost=yes --show-reachable=yes --undef-value-errors=yes --track-origins=yes ./fsfuse

Unit tests
----------
We use check (http://check.sourceforge.com)
Tests should use: fail_unless( expr_that_should_be_true, "explanation of what should happen" ), e.g.:

    fail_unless( in != NULL, "indexnode should be non-null" );

Don't check lifecycle stuff explicity, it's a bit passe. Just do _new() and _delete() in checked fixtures
Don't check property getters and setters - that should all be hidden - tell, don't ask

Integration tests
-----------------

Random Notes on Code
====================

Const-correctness
-----------------
We use opaque data types wherever possible (i.e. other modules only see typedef struct _foo *foo).
This means that such types don't need to be passed round const all the time, as only the files that have access to the struct definition can meaningfully change them.
Complex types are returned non-const from ctors and hence don't need storing as const by the client.

Nothing should be returned const because fsfuse's convention is that everything returns a copy, thus the caller can do whatever it wants with the value it gets given.
This applies to pointer const-ness too.
Obvious exceptions are things like string_buffer_peek().

Getters and other methods that just read the data should take their first argument ("this") as const to indicate that they don't update the state of the object.
This is like "void Class::Method() const" in C++.
Methods that change the state of the object (discouraged) obviously take the object non-const.

Headers shouldn't declare arguments 2+ const because the convention is that you are giving up ownership of anything you pass in, so the method can do whatever it wants.
Headers shouldn't declare functions as taking const pointers either because it's pointless - the pointer is passed by value so any change made to it by the method won't be reflected in the calling code.
Implementation function signatures can specify const data or pointers if they wish to keep themselves honest (this is valid C). This point it that these are implementation details and the implementation should be free to change without affecting the contract (which is that the code /could/ modify anything it likes).

Ownership
---------
Everything always returns a copy. Non-refc'd objects (inc char* strings) are physically coped, refc'd classes are posted.
It is the caller's responsibility to free()/delete() them.

Functions take ownership of any arguments passed into them.
Thus if the caller wishes to retain a copy for itself it must strdup()/post()/etc before calling the function.
To maintain this convention, if a method does not retain the object it's been given it should free() it.

The exception is when the first argument is a "this" pointer.
e.g. in:

    static_utility( char *a, char *b );
    
both a and b must be usurped.
however in:

    foo_method( foo_t *foo, char *a );
    
only a must be usurped; foo need not be copied to be passed in (indeed a lot of classes aren't ref-counted / copyable).

This has the nice effect that a lot of functions, those that just get objects from one place and pass them onto another, don't need to do any copying or deleting.

Alas I think currently that most things take copies on the way in, e.g. indexnode_list_add(), listing and direntry ctors() etc.

Other
-----
All locks should be "external". I.e. everything is single threaded and assumes it's used in e.g. an actor. If you want to use a class in a potentially multi-threaded way it is your responsibility to lock around it (e.g. by making yourself an actor)

Utils and stuff should not be random static init & destroy. They should all be offered as objects which the classes that use them explicitly depend on.
Indeed, "utils" is a code smell. A collection of static functions that I can't name the function of.

The (.Net) "tryget" style is preferred over the use of NULL returns. This is because
a) They can be marginally easier to write
b) They scale to more-than-one return value trivially (as values are returned in out parameters rather than by return)
c) They also work for non-pointer types
d) They allow for more than a Unit-typed description of the error (i.e. the whole range of int can be used)
I.e. rather than:

    foo_t *foo = get_foo( );
    if( foo )
    {
        ...

or:

    foo_t *foo;
    if( ( foo = get_foo( ) ) )
    {
        ...

the preferred style is:

    foo_t *foo;
    if( tryget_foo( &foo ) )
    {
        ...
