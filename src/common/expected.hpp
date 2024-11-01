#pragma once
#include <variant>

namespace jp {
template <typename T, typename E> class [[nodiscard]] expected {
    expected(expected &&) = default;
    expected &operator=(expected &&) = default;
    expected(expected const &) = delete;
    expected &operator=(expected const &) = delete;

  public:
    using value_type = T;
    using error_type = E;

    template <typename... Args> expected(Args &&...args) : m_expected_var(std::forward<Args>(args)...) {}

    auto value() -> T & { return std::get<T>(m_expected_var); }

    auto value() const -> const T & { return std::get<T>(m_expected_var); }

    auto error() -> E & { return std::get<E>(m_expected_var); }

    auto error() const -> const E & { return std::get<E>(m_expected_var); }

    auto has_value() const -> bool { return std::holds_alternative<T>(m_expected_var); }

    auto has_error() const -> bool { return std::holds_alternative<E>(m_expected_var); }

    auto consume_value() -> T { return std::move(std::get<T>(m_expected_var)); }

    auto consume_error() -> E { return std::move(std::get<E>(m_expected_var)); }

    auto value_or(T &&default_value) -> T {
        if (has_value()) {
            return value();
        }
        return std::move(default_value);
    }

    auto error_or(E &&default_error) -> E {
        if (has_error()) {
            return error();
        }
        return std::move(default_error);
    }

    auto swap(expected &other) -> void { m_expected_var.swap(other.m_expected_var); }

  private:
    std::variant<T, error_type> m_expected_var;
};

} // namespace jp
