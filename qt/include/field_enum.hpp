#pragma once

#include <type_traits>

namespace SideAssist {

template <typename T, typename = std::enable_if_t<std::is_enum_v<T>>>
struct FieldEnum {
  using NumericType =
      std::conditional_t<sizeof(T) <= sizeof(uint32_t), uint32_t, uint64_t>;
  static_assert(sizeof(NumericType) >= sizeof(T));

  constexpr FieldEnum() : value((T)0) {}
  constexpr FieldEnum(const FieldEnum&) = default;
  constexpr FieldEnum(FieldEnum&&) = default;
  constexpr FieldEnum& operator=(T v) {
    value = v;
    return *this;
  };
  constexpr FieldEnum& operator=(NumericType v) {
    value = (T)v;
    return *this;
  };
  constexpr FieldEnum(T v) : value(v) {}
  constexpr FieldEnum(NumericType v) : value((T)v) {}
  constexpr FieldEnum operator|(FieldEnum other) const {
    return FieldEnum((NumericType)value | (NumericType)other.value);
  }
  constexpr FieldEnum operator&(FieldEnum other) const {
    return FieldEnum((NumericType)value & (NumericType)other.value);
  }
  constexpr FieldEnum& operator|=(FieldEnum other) {
    value = T((NumericType)value | (NumericType)other.value);
    return *this;
  }
  constexpr FieldEnum& operator&=(FieldEnum other) {
    value = T((NumericType)value & (NumericType)other.value);
    return *this;
  }

  constexpr bool operator==(FieldEnum other) const { return value == other.value; }
  constexpr bool operator!=(FieldEnum other) const { return value != other.value; }

  constexpr bool between(FieldEnum lower, FieldEnum upper) const {
    return (*this & lower) == lower && (*this | upper) == upper;
  }

  // constexpr operator bool() const { return value != 0; }
  constexpr operator NumericType() const { return (NumericType)value; }

  T value;
};

}  // namespace SideAssist
