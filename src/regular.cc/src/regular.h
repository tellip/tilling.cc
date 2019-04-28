#pragma once

#include <array>
#include <codecvt>
#include <functional>
#include <list>
#include <locale>
#include <memory>
#include <string>
#include <tuple>
#include <vector>

#define TYPE(X) typename std::remove_const<decltype(X)>::type

namespace regular {
    template<typename...>
    struct Traits {
        ~Traits() = delete;
    };

    template<>
    struct Traits<char> {
        ~Traits() = delete;

        using String=std::string;

        static inline String string(const std::string &s) {
            return s;
        }
    };

    template<>
    struct Traits<wchar_t> {
        ~Traits() = delete;

        using String=std::wstring;

        static inline String string(const std::string &s) {
            return std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>().from_bytes(s);
        }
    };

    template<typename Character>
    struct Record : std::enable_shared_from_this<Record<Character>> {
        virtual ~Record() = default;

        typename Traits<Character>::String::const_iterator begin, end;

        inline typename Traits<Character>::String string() const {
            return typename Traits<Character>::String(begin, end);
        }

        template<typename Derived>
        std::shared_ptr<Derived> as() const;
    };

    namespace record {
        template<typename Character>
        struct LinearSome : Record<Character> {
            std::size_t index;
            typename Traits<Character>::String key;
            std::shared_ptr<Record<Character>> value;
        };

        template<typename Character>
        struct LinearEvery : Record<Character> {
            std::vector<std::shared_ptr<Record<Character>>> vector;
            std::unordered_map<typename Traits<Character>::String, std::shared_ptr<Record<Character>>> map;
        };

        template<typename Character>
        struct Kleene : Record<Character> {
            std::list<std::shared_ptr<Record<Character>>> list;
        };
    }

    template<typename Character>
    struct Pattern : std::enable_shared_from_this<Pattern<Character>> {
        struct Matched {
            bool success;
            std::shared_ptr<Record<Character>> record;
        };

        virtual typename Pattern<Character>::Matched match(
                const typename Traits<Character>::String::const_iterator &,
                const typename Traits<Character>::String::const_iterator &
        ) const = 0;

        inline typename Pattern<Character>::Matched match(const typename Traits<Character>::String &s) const {
            return match(s.cbegin(), s.cend());
        }

        inline typename Pattern<Character>::Matched adapt(
                const typename Traits<Character>::String::const_iterator &begin,
                const typename Traits<Character>::String::const_iterator &end
        ) const {
            auto[b, r]=match(begin, end);
            return {b && r->end == end, r};
        }

        inline typename Pattern<Character>::Matched adapt(const typename Traits<Character>::String &s) const {
            return adapt(s.cbegin(), s.cend());
        }

        template<typename Derived>
        std::shared_ptr<Derived> as() const;
    };

    namespace pattern {
        template<typename Character>
        struct Empty : Pattern<Character> {
            typename Pattern<Character>::Matched match(
                    const typename Traits<Character>::String::const_iterator &,
                    const typename Traits<Character>::String::const_iterator &
            ) const final;
        };

        template<typename Character>
        struct Singleton : Pattern<Character> {
            const std::function<bool(const Character &)> describe;

            explicit Singleton(const TYPE(describe) &describe) : describe(describe) {}

            typename Pattern<Character>::Matched match(
                    const typename Traits<Character>::String::const_iterator &,
                    const typename Traits<Character>::String::const_iterator &
            ) const final;
        };

        namespace singleton {
            template<typename Character, typename Context>
            struct Closure : Singleton<Character> {
                const Context context;
                const std::function<bool(const Context &, const Character &)> depict;

                Closure(Context &&, const TYPE(depict) &);
            };
        }

        template<typename Character>
        struct Linear : Pattern<Character> {
            struct Item {
                typename Traits<Character>::String key;
                std::shared_ptr<Pattern<Character>> value;

                template<typename Value>
                /*explicit*/ Item(Value &&value) : key(0, 0), value(std::forward<Value>(value)) {}

                Item(TYPE(key) &&key, const TYPE(value) &value) :
                        key(std::move(key)), value(value) {}
            };

            const std::vector<Item> linear;

            explicit Linear(TYPE(linear) &&linear) : linear(std::move(linear)) {}
        };

        namespace linear {
            template<typename Character>
            struct Union : Linear<Character> {
                explicit Union(TYPE(Linear<Character>::linear) &&linear) : Linear<Character>(std::move(linear)) {}

                typename Pattern<Character>::Matched match(
                        const typename Traits<Character>::String::const_iterator &,
                        const typename Traits<Character>::String::const_iterator &
                ) const final;
            };

            template<typename Character>
            struct Intersection : Linear<Character> {
                explicit Intersection(TYPE(Linear<Character>::linear) &&linear) : Linear<Character>(
                        std::move(linear)) {}

                typename Pattern<Character>::Matched match(
                        const typename Traits<Character>::String::const_iterator &,
                        const typename Traits<Character>::String::const_iterator &
                ) const;
            };

            template<typename Character>
            struct Difference : Linear<Character> {
                const bool sign;

                explicit Difference(TYPE(Linear<Character>::linear) &&linear, const TYPE(sign) &sign = true) :
                        Linear<Character>(std::move(linear)), sign(sign) {}

                typename Pattern<Character>::Matched match(
                        const typename Traits<Character>::String::const_iterator &,
                        const typename Traits<Character>::String::const_iterator &
                ) const final;
            };

            template<typename Character>
            struct Concatenation : Linear<Character> {
                explicit Concatenation(TYPE(Linear<Character>::linear) &&linear) : Linear<Character>(
                        std::move(linear)) {}

                typename Pattern<Character>::Matched match(
                        const typename Traits<Character>::String::const_iterator &,
                        const typename Traits<Character>::String::const_iterator &
                ) const;
            };
        }

        template<typename Character>
        struct KleeneClosure : public Pattern<Character> {
            std::shared_ptr<Pattern<Character>> item;

            explicit KleeneClosure(const TYPE(item) &item) : item(item) {}

            typename Pattern<Character>::Matched match(
                    const typename Traits<Character>::String::const_iterator &,
                    const typename Traits<Character>::String::const_iterator &
            ) const final;
        };

        template<typename Character>
        struct Placeholder : Pattern<Character> {
            std::shared_ptr<Pattern<Character>> place;

            typename Pattern<Character>::Matched match(
                    const typename Traits<Character>::String::const_iterator &,
                    const typename Traits<Character>::String::const_iterator &
            ) const final;
        };

        template<typename Character>
        struct Collapsed : Pattern<Character> {
            const std::shared_ptr<Pattern<Character>> core;

            explicit Collapsed(const TYPE(core) &core) : core(core) {};

            typename Pattern<Character>::Matched match(
                    const typename Traits<Character>::String::const_iterator &,
                    const typename Traits<Character>::String::const_iterator &
            ) const final;
        };
    }
}