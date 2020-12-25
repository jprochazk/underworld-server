#ifndef SERVER_UTIL_DEFER_HPP_
#define SERVER_UTIL_DEFER_HPP_

struct defer_dummy
{};

template<class F>
struct deferred
{
    F f;
    ~deferred() { f(); }
};

template<class F>
deferred<F>
operator*(defer_dummy, F f)
{
    return { f };
}

#define DEFER_MANGLE_VAR_(LINE) zz_defer##LINE
#define DEFER_VARNAME(LINE) DEFER_MANGLE_VAR_(LINE)
#define defer auto DEFER_VARNAME(__LINE__) = defer_dummy{}* [&]()

#endif // SERVER_UTIL_DEFER_HPP_
