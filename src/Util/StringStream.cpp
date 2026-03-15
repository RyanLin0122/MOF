#include "Util/StringStream.h"
#include <cstdio>

StringStream::StringStream() {}
StringStream::~StringStream() {}

StringStream& StringStream::operator<<(int value) {
    char tmp[32];
    std::sprintf(tmp, "%d", value);
    m_buf += tmp;
    return *this;
}

const char* StringStream::c_str() const {
    return m_buf.c_str();
}
