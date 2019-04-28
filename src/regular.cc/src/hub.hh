#pragma once

#include "regular.h"

namespace regular {
    template<typename Character>
    struct hub {
        ~hub() = delete;

        using rt=Record<Character>;
        using rlst=record::LinearSome<Character>;
        using rlet=record::LinearEvery<Character>;
        using rkt=record::Kleene<Character>;

        using pt=Pattern<Character>;
        using pot=pattern::Empty<Character>;
        using pst=pattern::Singleton<Character>;
        template<typename Context>
        using psct=pattern::singleton::Closure<Character, Context>;
        using plt=pattern::Linear<Character>;
        using plut=pattern::linear::Union<Character>;
        using plit=pattern::linear::Intersection<Character>;
        using pldt=pattern::linear::Difference<Character>;
        using plct=pattern::linear::Concatenation<Character>;
        using pkt=pattern::KleeneClosure<Character>;
        using ppt=pattern::Placeholder<Character>;
        using pqt=pattern::Collapsed<Character>;
//        using pct=pattern::Custom<Character>;

        static inline std::shared_ptr<pot> po() {
            return std::make_shared<pot>();
        }

        static inline std::shared_ptr<pst> ps(const std::function<bool(const Character &)> &describe) {
            return std::make_shared<pst>(describe);
        }

        template<typename Context>
        static inline std::shared_ptr<psct<Context>>
        ps(Context &&context, const std::function<bool(const Context &, const Character &)> &depict) {
            return std::make_shared<psct<Context>>(std::forward<Context>(context), depict);
        }

        static inline std::shared_ptr<pst> psa() {
            return std::make_shared<pst>([](const Character &) {
                return true;
            });
        }

        static inline std::shared_ptr<psct<Character>> ps(const Character &c0) {
            return std::make_shared<psct<Character>>(Character(c0),
                                                     [](const Character &c0, const Character &c) -> bool {
                                                         return c == c0;
                                                     });
        }

        static std::shared_ptr<psct<std::unordered_map<
                Character,
                nullptr_t
        >>> ps(typename Traits<Character>::String &&s) {
            return std::make_shared<psct<std::unordered_map<
                    Character,
                    nullptr_t
            >>>(({
                std::unordered_map<
                        Character,
                        nullptr_t
                > m;
                for (auto i = s.cbegin(); i != s.cend(); ({
                    m[*i] = nullptr;
                    i++;
                }));
                m;
            }), [&](const std::unordered_map<
                    Character,
                    nullptr_t
            > &m, const Character &c) -> bool {
                return m.find(c) != m.cend();
            });
        }

        static inline std::shared_ptr<psct<std::array<Character, 2>>> ps(const Character &inf, const Character &sup) {
            return std::make_shared<psct<std::array<Character, 2>>>(std::array{inf, sup},
                                                                    [&](const std::array<Character, 2> &interval,
                                                                        const Character &c) -> bool {
                                                                        return interval[0] <= c && c <= interval[1];
                                                                    });
        }

        static std::shared_ptr<psct<std::vector<std::shared_ptr<pst>>>>
        psu(std::vector<std::shared_ptr<pst>> &&vector) {
            return std::make_shared<psct<std::vector<std::shared_ptr<pst>>>>(std::move(vector),
                                                                             [&](const std::vector<std::shared_ptr<pst>> &vector,
                                                                                 const Character &c) -> bool {
                                                                                 for (auto i = vector.cbegin();
                                                                                      i != vector.cend(); ({
                                                                                     if ((*i)->describe(c)) return true;
                                                                                     i++;
                                                                                 }));
                                                                                 return false;
                                                                             });
        }

        static std::shared_ptr<psct<std::vector<std::shared_ptr<pst>>>>
        psi(std::vector<std::shared_ptr<pst>> &&vector) {
            return std::make_shared<psct<std::vector<std::shared_ptr<pst>>>>(std::move(vector),
                                                                             [&](const std::vector<std::shared_ptr<pst>> &vector,
                                                                                 const Character &c) -> bool {
                                                                                 for (auto i = vector.cbegin();
                                                                                      i != vector.cend(); ({
                                                                                     if (!(*i)->describe(c))
                                                                                         return false;
                                                                                     i++;
                                                                                 }));
                                                                                 return true;
                                                                             });
        }

        static std::shared_ptr<psct<std::pair<
                std::vector<std::shared_ptr<pst>>,
                bool
        >>> psd(std::vector<std::shared_ptr<pst>> &&vector, const bool &sign = true) {
            return std::make_shared<psct<std::pair<
                    std::vector<std::shared_ptr<pst>>,
                    bool
            >>>(std::pair{std::move(vector), sign}, [&](const std::pair<
                    std::vector<std::shared_ptr<pst>>,
                    bool
            > &pair, const Character &c) -> bool {
                auto&[l, s]=pair;
                bool b = false;
                for (auto i = l.crbegin(); i != l.crend(); ({
                    b = (*i)->describe(c) && !b;
                    i++;
                }));
                return !(b xor s);
            });
        }

        static inline std::shared_ptr<plut> plu(std::vector<typename plt::Item> &&vector) {
            return std::make_shared<plut>(std::move(vector));
        }

        static inline std::shared_ptr<plit> pli(std::vector<typename plt::Item> &&vector) {
            return std::make_shared<plit>(std::move(vector));
        }

        static inline std::shared_ptr<pldt> pld(std::vector<typename plt::Item> &&vector, const bool &sign = true) {
            return std::make_shared<pldt>(std::move(vector), sign);
        }

        static inline std::shared_ptr<plct> plc(std::vector<typename plt::Item> &&vector) {
            return std::make_shared<plct>(std::move(vector));
        }

        static std::shared_ptr<plct> plcs(const typename Traits<Character>::String &s) {
            std::vector<typename plt::Item> linear(s.size(), nullptr);
            for (auto i = s.cbegin(); i != s.cend(); ({
                linear[i - s.cbegin()] = ps(*i);
                i++;
            }));
            return std::make_shared<plct>(std::move(linear));
        }

        static inline std::shared_ptr<pkt> pk(const std::shared_ptr<pt> &item) {
            return std::make_shared<pkt>(item);
        }

        static inline std::shared_ptr<ppt> pp() {
            return std::make_shared<ppt>();
        }

        static inline std::shared_ptr<pqt> pq(const std::shared_ptr<pt> &p) {
            return std::make_shared<pqt>(p);
        }
    };

    namespace shortcut {
        namespace narrow {
            using rt=hub<char>::rt;
            using rlst=hub<char>::rlst;
            using rlet=hub<char>::rlet;
            using rkt=hub<char>::rkt;

            using pt=hub<char>::pt;
            using pot=hub<char>::pot;
            using pst=hub<char>::pst;
            template<typename Context>
            using psct=hub<char>::psct<Context>;
            using plt=hub<char>::plt;
            using plut=hub<char>::plut;
            using plit=hub<char>::plit;
            using pldt=hub<char>::pldt;
            using plct=hub<char>::plct;
            using pkt=hub<char>::pkt;
            using ppt=hub<char>::ppt;
            using pqt=hub<char>::pqt;

            const auto po = hub<char>::po();

            inline std::shared_ptr<pst> ps(const std::function<bool(const char &)> &f) { return hub<char>::ps(f); }

            template<typename Context>
            inline std::shared_ptr<psct<Context>>
            ps(Context &&c, const std::function<bool(const Context &, const char &)> &f) { return hub<char>::ps(c, f); }

            const auto psa = hub<char>::psa();

            inline std::shared_ptr<psct<char>> ps(const char &c) { return hub<char>::ps(c); }

            inline std::shared_ptr<psct<std::unordered_map<
                    char,
                    nullptr_t
            >>> ps(typename Traits<char>::String &&s) { return hub<char>::ps(std::move(s)); }

            inline std::shared_ptr<psct<std::array<char, 2>>> ps(const char &c, const char &d) {
                return hub<char>::ps(c, d);
            }

            inline std::shared_ptr<psct<std::vector<std::shared_ptr<pst>>>>
            psu(std::vector<std::shared_ptr<pst>> &&l) { return hub<char>::psu(std::move(l)); }

            inline std::shared_ptr<psct<std::vector<std::shared_ptr<pst>>>>
            psi(std::vector<std::shared_ptr<pst>> &&l) { return hub<char>::psi(std::move(l)); }

            inline std::shared_ptr<psct<std::pair<
                    std::vector<std::shared_ptr<pst>>,
                    bool
            >>> psd(std::vector<std::shared_ptr<pst>> &&l, const bool &s = true) {
                return hub<char>::psd(std::move(l), s);
            }

            inline std::shared_ptr<plut> plu(std::vector<typename plt::Item> &&l) {
                return hub<char>::plu(std::move(l));
            }

            inline std::shared_ptr<plit> pli(std::vector<typename plt::Item> &&l) {
                return hub<char>::pli(std::move(l));
            }

            inline std::shared_ptr<pldt> pld(std::vector<typename plt::Item> &&l, const bool &sign = true) {
                return hub<char>::pld(std::move(l), sign);
            }

            inline std::shared_ptr<plct> plc(std::vector<typename plt::Item> &&l) {
                return hub<char>::plc(std::move(l));
            }

            inline std::shared_ptr<plct> plcs(const typename Traits<char>::String &s) { return hub<char>::plcs(s); }

            inline std::shared_ptr<pkt> pk(const std::shared_ptr<pt> &p) { return hub<char>::pk(p); }

            inline std::shared_ptr<ppt> pp() { return hub<char>::pp(); }

            inline std::shared_ptr<pqt> pq(const std::shared_ptr<pt> &p) { return hub<char>::pq(p); }
        }
        namespace wide {
            using wrt=hub<wchar_t>::rt;
            using wrlst=hub<wchar_t>::rlst;
            using wrlet=hub<wchar_t>::rlet;
            using wrkt=hub<wchar_t>::rkt;

            using wpt=hub<wchar_t>::pt;
            using wpot=hub<wchar_t>::pot;
            using wpst=hub<wchar_t>::pst;
            template<typename Context>
            using wpsct=hub<wchar_t>::psct<Context>;
            using wplt=hub<wchar_t>::plt;
            using wplut=hub<wchar_t>::plut;
            using wplit=hub<wchar_t>::plit;
            using wpldt=hub<wchar_t>::pldt;
            using wplct=hub<wchar_t>::plct;
            using wpkt=hub<wchar_t>::pkt;
            using wppt=hub<wchar_t>::ppt;
            using wpqt=hub<wchar_t>::pqt;

            const auto wpo = hub<wchar_t>::po();

            inline std::shared_ptr<wpst> wps(const std::function<bool(const wchar_t &)> &f) {
                return hub<wchar_t>::ps(f);
            }

            template<typename Context>
            inline std::shared_ptr<wpsct<Context>>
            wps(Context &&c, const std::function<bool(const Context &, const wchar_t &)> &f) {
                return hub<wchar_t>::ps(c, f);
            }

            const auto wpsa = hub<wchar_t>::psa();

            inline std::shared_ptr<wpsct<wchar_t>> wps(const wchar_t &c) { return hub<wchar_t>::ps(c); }

            inline std::shared_ptr<wpsct<std::unordered_map<
                    wchar_t,
                    nullptr_t
            >>> wps(typename Traits<wchar_t>::String &&s) { return hub<wchar_t>::ps(std::move(s)); }

            inline std::shared_ptr<wpsct<std::array<wchar_t, 2>>>
            wps(const wchar_t &c, const wchar_t &d) { return hub<wchar_t>::ps(c, d); }

            inline std::shared_ptr<wpsct<std::vector<std::shared_ptr<wpst>>>>
            wpsu(std::vector<std::shared_ptr<wpst>> &&l) { return hub<wchar_t>::psu(std::move(l)); }

            inline std::shared_ptr<wpsct<std::vector<std::shared_ptr<wpst>>>>
            wpsi(std::vector<std::shared_ptr<wpst>> &&l) { return hub<wchar_t>::psi(std::move(l)); }

            inline std::shared_ptr<wpsct<std::pair<
                    std::vector<std::shared_ptr<wpst>>,
                    bool
            >>> wpsd(std::vector<std::shared_ptr<wpst>> &&l, const bool &s = true) {
                return hub<wchar_t>::psd(std::move(l), s);
            }

            inline std::shared_ptr<wplut> wplu(std::vector<typename wplt::Item> &&l) {
                return hub<wchar_t>::plu(std::move(l));
            }

            inline std::shared_ptr<wplit> wpli(std::vector<typename wplt::Item> &&l) {
                return hub<wchar_t>::pli(std::move(l));
            }

            inline std::shared_ptr<wpldt> wpld(std::vector<typename wplt::Item> &&l, const bool &sign = true) {
                return hub<wchar_t>::pld(std::move(l), sign);
            }

            inline std::shared_ptr<wplct> wplc(std::vector<typename wplt::Item> &&l) {
                return hub<wchar_t>::plc(std::move(l));
            }

            inline std::shared_ptr<wplct> wplcs(const typename Traits<wchar_t>::String &s) {
                return hub<wchar_t>::plcs(s);
            }

            inline std::shared_ptr<wpkt> wpk(const std::shared_ptr<wpt> &p) { return hub<wchar_t>::pk(p); }

            inline std::shared_ptr<wppt> wpp() { return hub<wchar_t>::pp(); }

            inline std::shared_ptr<wpqt> wpq(const std::shared_ptr<wpt> &p) { return hub<wchar_t>::pq(p); }
        }
    }
}