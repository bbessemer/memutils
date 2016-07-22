# memutils
A tiny, public-domain library of memory and string utilities.

The recommended method of using memutils is simply to drop this file and its
header into your project. The header automatically redefines memcpy, strlen,
strcpy, and strdup as their memutils counterparts. It does not redefine
malloc or realloc. You may do this yourself if you wish.

If you're looking for documentation, the header is heavily commented, but
that's about it.
