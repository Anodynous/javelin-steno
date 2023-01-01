//---------------------------------------------------------------------------

#pragma once
#include "bit.h"
#include <string.h>

//---------------------------------------------------------------------------

class BitFieldIterator {
public:
  BitFieldIterator(const size_t *p, const size_t *end) : p(p), end(end) {
    MoveNext();
  }

  bool operator!=(const size_t *end) const { return value != 0 || p != end; }

  void operator++() { MoveNext(); }

  size_t operator*() const {
    return Bit<sizeof(size_t)>::CountTrailingZeros(value) + bitOffset;
  }

private:
  size_t value = 0;
  size_t bitOffset = -8 * sizeof(size_t);
  const size_t *p;
  const size_t *end;

  void MoveNext() {
    // Pull off lowest bit.
    value = value & (value - 1);
    while (value == 0 && p < end) {
      value = *p++;
      bitOffset += 8 * sizeof(size_t);
    }
  }
};

template <size_t N> class BitField {
private:
  static const size_t BITS_PER_WORD = 8 * sizeof(size_t);
  static const size_t NUMBER_OF_WORDS = (N + BITS_PER_WORD - 1) / BITS_PER_WORD;

public:
  bool IsSet(size_t n) const {
    return (data[n / BITS_PER_WORD] & (1ULL << n % BITS_PER_WORD)) != 0;
  }

  bool IsAnySet() const {
    size_t v = data[0];
    for (size_t i = 1; i < NUMBER_OF_WORDS; ++i) {
      v |= data[i];
    }
    return v != 0;
  }

  void Set(size_t n) { data[n / BITS_PER_WORD] |= 1ULL << n % BITS_PER_WORD; }
  void Clear(size_t n) {
    data[n / BITS_PER_WORD] &= ~(1ULL << n % BITS_PER_WORD);
  }

  void ClearAll() { memset(data, 0, sizeof(data)); }

  BitField operator&(const BitField &other) const {
    BitField result;
    for (size_t i = 0; i < NUMBER_OF_WORDS; ++i) {
      result.data[i] = data[i] & other.data[i];
    }
    return result;
  }

  BitField operator|(const BitField &other) const {
    BitField result;
    for (size_t i = 0; i < NUMBER_OF_WORDS; ++i) {
      result.data[i] = data[i] | other.data[i];
    }
    return result;
  }

  BitField operator^(const BitField &other) const {
    BitField result;
    for (size_t i = 0; i < NUMBER_OF_WORDS; ++i) {
      result.data[i] = data[i] ^ other.data[i];
    }
    return result;
  }

  bool operator==(const BitField &other) const {
    return memcmp(data, other.data, sizeof(data)) == 0;
  }
  bool operator!=(const BitField &other) const {
    return memcmp(data, other.data, sizeof(data)) != 0;
  }

  friend BitFieldIterator begin(const BitField &b) {
    return BitFieldIterator(b.data, b.data + NUMBER_OF_WORDS);
  }

  friend const size_t *end(const BitField &b) {
    return b.data + NUMBER_OF_WORDS;
  }

private:
  size_t data[NUMBER_OF_WORDS];
};

//---------------------------------------------------------------------------
