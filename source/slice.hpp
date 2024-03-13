#ifndef SLICE_hpp
#define SLICE_hpp

#include <string>
#include <stddef.h>

/* Slice of unowned string memory */
class Slice
{
public:
    /* Constructs an empty slice */
    inline Slice()
        : _string(NULL)
        , _length(0)
    {
    }

    /* Constructs a slice from the given memory */
    inline Slice(const char *string, size_t length)
        : _string(string)
        , _length(length)
    {
    }

    /* Constructs a slice from the given std::string */
    inline Slice(const std::string &string)
        : _string(string.c_str())
        , _length(string.length())
    {
    }

    /* Returns whether the slice is empty or not */
    inline bool isEmpty() const
    {
        return _string == NULL || _length == 0;
    }

    /* Gets the slice's length */
    inline size_t getLength() const
    {
        return _length;
    }

    /* Gets the character at the given index */
    inline char operator[](size_t index) const
    {
        return _string[index];
    }

    /* Comparison to other slice */
    bool operator==(Slice other);
    bool operator!=(Slice other);

    /* Removes the slice's start until `delimiter` is reached and populates `outSlice` with it,
       the current slice will be the remainder excluding the delimiter */
    bool splitStart(char delimiter, Slice &outStart);

    /* Removes the slice's start until `delimiter` is reached and populates `outSlice` with it,
       the current slice will be the remainder excluding the delimiter */
    bool splitStart(Slice delimiter, Slice &outStart);

    /* Removes the slice's end until `delimiter` is reached and populates `outSlice` with it,
       the current slice will be the remainder including the delimiter */
    bool splitEnd(char delimiter, Slice &outEnd);

    /* Removes any occurrences of the given character from the start of the slice */
    Slice &stripStart(char character);

    /* If available, removes the given prefix from the slice and returns true */
    bool removePrefix(Slice prefix);

    /* Writes a slice's content into an output stream */
    friend std::ostream &operator<<(std::ostream &stream, const Slice &slice);

    /* Gets if the slice starts with the given prefix */
    bool startsWith(Slice prefix) const;

    /* Gets if the slice ends with the given suffix */
    bool endsWith(Slice prefix) const;

    /* Returns a new slice with the given number of characters removed from the start,
       the size is clamped by the available character count */
    Slice cut(size_t amount) const;

    /* Converts the slice to a string */
    inline std::string toString() const
    {
        return std::string(_string, _length);
    }

    /* Removes leading and trailing double-quotes from the slice */
    void removeDoubleQuotes();
    
private:
    const char *_string;
    size_t      _length;
};

/* Constructs a slice from a constant C-style string */
#define C_SLICE(String) Slice((String), sizeof(String) - 1)

#endif // SLICE_hpp
