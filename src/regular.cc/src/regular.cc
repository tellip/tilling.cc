#include "regular.h"

namespace regular {
    template<typename Character>
    template<typename Derived>
    inline std::shared_ptr<Derived> Record<Character>::as() const {
        return std::dynamic_pointer_cast<Derived>(const_cast<Record<Character> *>(this)->shared_from_this());
    }

    template<typename Character>
    template<typename Derived>
    inline std::shared_ptr<Derived> Pattern<Character>::as() const {
        return std::dynamic_pointer_cast<Derived>(const_cast<Pattern<Character> *>(this)->shared_from_this());
    }

    namespace pattern {
        template<typename Character>
        inline typename Pattern<Character>::Matched Empty<Character>::match(
                const typename Traits<Character>::String::const_iterator &begin,
                const typename Traits<Character>::String::const_iterator &end
        ) const {
            return {true, ({
                auto r = std::make_shared<Record<Character>>();
                r->begin = begin;
                r->end = begin;
                r;
            })};
        }

        template<typename Character>
        typename Pattern<Character>::Matched Singleton<Character>::match(
                const typename Traits<Character>::String::const_iterator &begin,
                const typename Traits<Character>::String::const_iterator &end
        ) const {
            bool success;
            auto end1 = (begin == end ? ({
                success = false;
                begin;
            }) : ({
                success = describe(*begin);
                std::next(begin);
            }));
            return {success, ({
                auto r = std::make_shared<Record<Character>>();
                r->begin = begin;
                r->end = end1;
                r;
            })};
        }

        namespace singleton {
            template<typename Character, typename Context>
            Closure<Character, Context>::Closure(Context &&context, const TYPE(depict) &depict):
                    Singleton<Character>([&](const Character &c) { return this->depict(this->context, c); }),
                    context(std::move(context)), depict(depict) {}
        }

        namespace linear {
            template<typename Character>
            typename Pattern<Character>::Matched Union<Character>::match(
                    const typename Traits<Character>::String::const_iterator &begin,
                    const typename Traits<Character>::String::const_iterator &end
            ) const {
                bool success = false;
                auto end1 = begin;
                typename Traits<Character>::String key(0, 0);
                std::size_t index = 0;
                std::shared_ptr<Record<Character>> record = nullptr;
                for (auto i = this->linear.cbegin(); i < this->linear.cend(); ({
                    auto matched = i->value->match(begin, end);
                    if (matched.success) {
                        success = true;
                        end1 = matched.record->end;
                        index = std::size_t(i - this->linear.cbegin());
                        key = i->key;
                        record = matched.record;
                        i = this->linear.cend();
                    } else i++;
                }));
                return {success, ({
                    auto r = std::make_shared<record::LinearSome<Character>>();
                    r->begin = begin;
                    r->end = end1;
                    r->index = index;
                    r->key = std::move(key);
                    r->value = record;
                    r;
                })};
            }

            template<typename Character>
            typename Pattern<Character>::Matched Intersection<Character>::match(
                    const typename Traits<Character>::String::const_iterator &begin,
                    const typename Traits<Character>::String::const_iterator &end
            ) const {
                bool success = true;
                auto end1 = begin;
                std::unordered_map<typename Traits<Character>::String, std::shared_ptr<Record<Character>>> map;
                std::vector<std::shared_ptr<Record<Character>>> vector(this->linear.size(), nullptr);
                for (auto i = this->linear.cbegin(); i < this->linear.cend(); ({
                    auto matched = i->value->match(begin, end);
                    map[i->key] = matched.record;
                    vector[i - this->linear.cbegin()] = matched.record;
                    if (matched.success) i++;
                    else {
                        success = false;
                        end1 = matched.record->end;
                        i = this->linear.cend();
                    }
                }));
                return {success, ({
                    auto r = std::make_shared<record::LinearEvery<Character>>();
                    r->begin = begin;
                    r->end = end1;
                    r->map = std::move(map);
                    r->vector = std::move(vector);
                    r;
                })};
            }

            template<typename Character>
            typename Pattern<Character>::Matched Difference<Character>::match(
                    const typename Traits<Character>::String::const_iterator &begin,
                    const typename Traits<Character>::String::const_iterator &end
            ) const {
                bool success = false;
                auto end1 = begin;
                std::size_t index = 0;
                typename Traits<Character>::String key(0, 0);
                std::shared_ptr<Record<Character>> record = nullptr;

                for (auto i = this->linear.crbegin(); i < this->linear.crend(); ({
                    auto matched = i->value->match(begin, end);
                    success = matched.success && !success;
                    if (success) {
                        end1 = matched.record->end;
                        index = std::size_t(this->linear.crend() - i - 1);
                        key = i->key;
                        record = matched.record;
                    }
                    i++;
                }));

                return {!(success xor sign), ({
                    auto r = std::make_shared<record::LinearSome<Character>>();
                    r->begin = begin;
                    r->end = end1;
                    r->index = index;
                    r->key = std::move(key);
                    r->value = record;
                    r;
                })};
            }

            template<typename Character>
            typename Pattern<Character>::Matched Concatenation<Character>::match(
                    const typename Traits<Character>::String::const_iterator &begin,
                    const typename Traits<Character>::String::const_iterator &end
            ) const {
                bool success = true;
                auto end1 = begin;
                std::unordered_map<typename Traits<Character>::String, std::shared_ptr<Record<Character>>> map;
                std::vector<std::shared_ptr<Record<Character>>> vector(this->linear.size(), nullptr);
                for (auto i = this->linear.cbegin(); i < this->linear.cend(); ({
                    auto matched = i->value->match(end1, end);
                    end1 = matched.record->end;
                    map[i->key] = matched.record;
                    vector[i - this->linear.cbegin()] = matched.record;
                    if (!matched.success) {
                        success = false;
                        i = this->linear.cend();
                    } else i++;
                }));
                return {success, ({
                    auto r = std::make_shared<record::LinearEvery<Character >>();
                    r->begin = begin;
                    r->end = end1;
                    r->map = std::move(map);
                    r->vector = std::move(vector);
                    r;
                })};
            }
        }

        template<typename Character>
        typename Pattern<Character>::Matched KleeneClosure<Character>::match(
                const typename Traits<Character>::String::const_iterator &begin,
                const typename Traits<Character>::String::const_iterator &end
        ) const {
            std::list<std::shared_ptr<Record<Character>>> list;
            auto i = begin, end1 = end;
            while (({
                auto[success, record] = item->match(i, end);
                success && (record->end > i) ? ({
                    list.emplace_back(record);
                    i = record->end;
                    true;
                }) : ({
                    end1 = i;
                    false;
                });
            }));
            return {true, ({
                auto r = std::make_shared<record::Kleene<Character>>();
                r->begin = begin;
                r->end = end1;
                r->list = std::move(list);
                r;
            })};
        }

        template<typename Character>
        inline typename Pattern<Character>::Matched Placeholder<Character>::match(
                const typename Traits<Character>::String::const_iterator &begin,
                const typename Traits<Character>::String::const_iterator &end
        ) const {
            return place ? place->match(begin, end) : typename Pattern<Character>::Matched{false, nullptr};
        }

        template<typename Character>
        inline typename Pattern<Character>::Matched Collapsed<Character>::match(
                const typename Traits<Character>::String::const_iterator &begin,
                const typename Traits<Character>::String::const_iterator &end
        ) const {
            auto matched = core->match(begin, end);
            return {matched.success, ({
                auto r = std::make_shared<Record<Character>>();
                r->begin = begin;
                r->end = matched.record->end;
                r;
            })};
        }
    }
}

#undef TYPE
