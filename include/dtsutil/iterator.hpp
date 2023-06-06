#ifndef DTSUTIL_ITERATOR_HPP
#define DTSUTIL_ITERATOR_HPP

#include <cstdint>

namespace dts {

enum class IteratorDecision : std::uint8_t {
    Continue = 0,
    Break,
};

}

#endif //DTSUTIL_ITERATOR_HPP
