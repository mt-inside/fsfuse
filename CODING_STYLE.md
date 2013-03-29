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
Tests should use: fail_unless( expr_that_should_be_true, "explanation of what should happen" );
    e.g. fail_unless( in != NULL, "indexnode should be non-null" );
Don't check lifecycle stuff explicity, it's a bit passe. Just do _new() and _delete() in checked fixtures
Don't check property getters and setters - that should all be hidden - tell, don't ask

Integration tests
-----------------

Random Notes on Code
====================
Const should be used whereever possible.
Some types are genuinely const, i.e. none of their members are ever altered after construction. HOWEVER, these do not need to be marked const when returned. We use opaque types to ensure that no-one outside of the class can mutate objects (and the class tries not to).
Objects should never be returned as "const" or "* const".
Arguments should be taken as const and * const where possible.

Everything always returns a copy.
It is the caller's responsibility to free it.

GOAL: add()/set() and other "input" methods should usurp ownership of what they're passed. If the caller wants a copy too they should copy it. That is, ownership goes "in", but copies come "out".
NOW: I think most things take copies on the way in, e.g. indexnode_list_add(), listing and direntry ctors() etc.

All locks should be "external". I.e. everything is single threaded and assumes it's used in e.g. an actor. If you want to use a class in a potentially multi-threaded way it is your responsibility to lock around it (e.g. by making yourself an actor)

Utils and stuff should not be random static init & destroy. They should all be offred as objects which the clases that use them explicity depend on.
