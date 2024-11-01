#pragma once
#include <variant>

namespace jp {
template <typename T, typename E> class [[nodiscard]] expected {
    static_assert(!std::is_same_v<T, E>, "T and E cannot be the same type");

  public:
    expected(T value) : m_expected_var(std::move(value)) {}
    expected(E error) : m_expected_var(std::move(error)) {}

    expected(expected &&) = default;
    expected &operator=(expected &&) = default;
    expected(expected const &) = delete;
    expected &operator=(expected const &) = delete;
    ~expected() = default;

    using value_type = T;
    using error_type = E;

    [[nodiscard]] auto value() -> T & { return std::get<T>(m_expected_var); }
    [[nodiscard]] auto value() const -> const T & { return std::get<T>(m_expected_var); }
    [[nodiscard]] auto error() -> E & { return std::get<E>(m_expected_var); }
    [[nodiscard]] auto error() const -> const E & { return std::get<E>(m_expected_var); }
    [[nodiscard]] auto has_value() const -> bool { return std::holds_alternative<T>(m_expected_var); }
    [[nodiscard]] auto has_error() const -> bool { return std::holds_alternative<E>(m_expected_var); }
    [[nodiscard]] auto consume_value() -> T { return std::move(std::get<T>(m_expected_var)); }
    [[nodiscard]] auto consume_error() -> E { return std::move(std::get<E>(m_expected_var)); }
    [[nodiscard]] auto value_or(T &&default_value) -> T {
        if (has_value()) {
            return value();
        }
        return std::move(default_value);
    }
    [[nodiscard]] auto error_or(E &&default_error) -> E {
        if (has_error()) {
            return error();
        }
        return std::move(default_error);
    }
    auto swap(expected &other) -> void { m_expected_var.swap(other.m_expected_var); }
    [[nodiscard]] auto operator==(const expected &rhs) const -> bool {
        if (has_value() && rhs.has_value()) {
            return value() == rhs.value();
        }
        if (has_error() && rhs.has_error()) {
            return error() == rhs.error();
        }
        return false;
    }

  private:
    std::variant<T, error_type> m_expected_var;
};

} // namespace jp
