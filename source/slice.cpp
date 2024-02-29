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
   the current slice will be the remainder excluding the delimiter */
bool Slice::splitStart(char delimiter, Slice &outStart)
{
    const char *position = (const char *)memchr(_string, delimiter, _length);
    if (position == NULL)
        return false;

    size_t index = position - _string;

    outStart._string = _string;
    outStart._length = index;

    _string += index + 1;
    _length -= index + 1;

    return true;
}


/* Removes the slice's start until `delimiter` is reached and populates `outSlice` with it,
    the current slice will be the remainder excluding the delimiter */
bool Slice::splitStart(Slice delimiter, Slice &outStart)
{
    const char *position = (const char *)memmem(_string, _length, delimiter._string, delimiter._length);
    if (position == NULL)
        return false;

    size_t index = position - _string;

    outStart._string = _string;
    outStart._length = index;

    _string += index + delimiter._length;
    _length -= index + delimiter._length;

    return true;
}

/* Removes the slice's end until `delimiter` is reached and populates `outSlice` with it,
    the current slice will be the remainder including the delimiter */
bool Slice::splitEnd(char delimiter, Slice &outEnd)
{
    const char *position = (const char *)memrchr(_string, delimiter, _length);
    if (position == NULL)
        return false;

    size_t index = position - _string + 1; // +1 to include the delimiter in the output slice

    outEnd._string = _string + index;
    outEnd._length = _length - index;

    _length = index;

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

/* Gets if the slice starts with the given prefix */
bool Slice::startsWith(Slice prefix) const
{
    if (_length < prefix._length)
        return false;
    return memcmp(_string, prefix._string, prefix._length) == 0;
}

/* Returns a new slice with the given number of characters removed from the start,
   the size is clamped by the available character count */
Slice Slice::cut(size_t amount) const
{
    if (amount > _length)
        amount = _length;
    return Slice(_string + amount, _length - amount);
}

/* Removes 1 instance of the given character of the beginning and the end of the slice, used for quotes mainly */
void Slice::removequotes(char character)
{
    if (_length >= 2 && _string[0] == character && _string[_length -1] == character)
    {
        _string++;
        _length -= 2;
    }

}