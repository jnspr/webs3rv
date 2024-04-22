#ifndef SLICE_hpp
#define SLICE_hpp

#include <vector>
#include <string>
#include <stddef.h>
#include <stdint.h>

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

    /* Constructs a slice from the given std::vector of bytes */
    inline Slice(const std::vector<uint8_t> &vector)
        : _string(reinterpret_cast<const char *>(vector.data()))
        , _length(vector.size())
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

    /* Gets a constant reference to the character at the given index */
    inline const char &operator[](size_t index) const
    {
        return _string[index];
    }

    /* Comparison to other slice */
    bool operator==(Slice other);
    bool operator!=(Slice other);

    /* Removes the slice's start until `delimiter` is reached and populates `outStart` with it,
       the current slice will be the remainder excluding the delimiter */
    bool splitStart(char delimiter, Slice &outStart);

    /* Removes the slice's start until `delimiter` is reached and populates `outStart` with it,
       the current slice will be the remainder excluding the delimiter */
    bool splitStart(Slice delimiter, Slice &outStart);

    /* Removes the slice's end until `delimiter` is reached and populates `outEnd` with it,
       the current slice will be the remainder excluding the delimiter */
    bool splitEnd(char delimiter, Slice &outEnd);

    /* Removes any occurrences of the given character from the start of the slice */
    Slice &stripStart(char character);

    /* Removes any occurrences of the given character from the end of the slice */
    Slice &stripEnd(char character);

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

    /* Consumes the given number of characters from the slice's start */
    void consumeStart(size_t count);

    /* Attempts to consume the given prefix and returns whether it was consumed */
    bool consumeStart(Slice prefix);
private:
    const char *_string;
    size_t      _length;
};

/* Constructs a slice from a constant C-style string */
#define C_SLICE(String) Slice((String), sizeof(String) - 1)

#endif // SLICE_hpp
