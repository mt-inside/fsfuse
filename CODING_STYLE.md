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

"tests.h"

"class_under_test.h"

Modularisation
==============
Public headers go in /src. Module's private headers go in the module dir. These are NOT on the include path. It is NOT acceptable to include headers with a path, this preventing the inclusion of private headers.

Testing
=======
valgrind --track-fds=yes --tool=memcheck --leak-check=full --show-possibly-lost=yes --show-reachable=yes --undef-value-errors=yes --track-origins=yes ./fsfuse

Random Notes on Code
====================
Const should be used whereever possible.
Some types are genuinely const, i.e. none of their members are ever altered after construction. HOWEVER, these do not need to be marked const when returned. We use opaque types to ensure that no-one outside of the class can mutate objects (and the class tries not to).
Objects should never be returned as "const" or "* const".
Arguments should be taken as const and * const where possible.

Everything always returns a copy.
It is the caller's responsibility to free it.

Utils and stuff should not be random static init & destroy. They should all be offred as objects which the clases that use them explicity depend on.
