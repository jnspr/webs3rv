#include "slice.hpp"

#include <ostream>
#include <string.h>

/* Comparison to other slice */
bool Slice::operator==(Slice other)
{
    if (_length != other._length)
        return false;
    return memcmp(_string, other._string, _length) == 0;
}

/* Removes the slice's start until `delimiter` is reached and populates `outSlice` with it,
   the current slice will be the remainder including the delimiter */
bool Slice::splitStart(char delimiter, Slice &outStart)
{
    const char *position = (const char *)memchr(_string, delimiter, _length);
    if (position == NULL)
        return false;

    size_t index = position - _string;

    outStart._string = _string;
    outStart._length = index;

    _string += index;
    _length -= index;

    return true;
}

/* Removes any occurrences of the given character from the start of the slice */
void Slice::stripStart(char character)
{
    while (_length > 0 && _string[0] == character)
    {
        _string++;
        _length--;
    }
}

/* If available, removes the given prefix from the slice and returns true */
bool Slice::removePrefix(Slice prefix)
{
    if (_length < prefix._length)
        return false;
    if (memcmp(_string, prefix._string, prefix._length) != 0)
        return false;

    _string += prefix._length;
    _length -= prefix._length;

    return true;
}

/* Writes a slice's content into an output stream */
std::ostream &operator<<(std::ostream &stream, const Slice &slice)
{
    return stream.write(slice._string, slice._length);
}
