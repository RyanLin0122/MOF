#pragma once
#include <string>

// Stub for StringStream used in ground truth (e.g. PickUpItem parameter building).
// Equivalent to a simple integer-to-string output stream.
class StringStream {
public:
    StringStream();
    ~StringStream();

    StringStream& operator<<(int value);
    const char* c_str() const;

private:
    std::string m_buf;
};
