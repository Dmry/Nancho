/*=============================================================================
    Copyright (c) 2016 Paul Fultz II
    C++17 implementation and adaptations by Daniël Emmery
    args.hpp
    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#ifndef ARGS_GUARD_ARGS_HPP
#define ARGS_GUARD_ARGS_HPP

#include <string>
#include <sstream>
#include <unordered_map>
#include <map>
#include <vector>
#include <deque>
#include <initializer_list>
#include <functional>
#include <algorithm>
#include <numeric>

#include <iostream>
#include <iomanip>

#include <cassert>

#define ARGS_RETURNS(...) -> decltype(__VA_ARGS__) { return (__VA_ARGS__); }

namespace args {

template<int N>
struct rank : rank<N-1> {};

template<>
struct rank<0> {};

}

namespace adl_args {

using std::begin;
using std::end;

template<class T>
auto adl_begin(T&& x) ARGS_RETURNS(begin(x));

template<class T>
auto is_container(args::rank<1>, T&& x) -> decltype(
    x.insert(end(x), *begin(x)), std::true_type{}
)
{
    return {};
}

template<class T>
std::false_type is_container(args::rank<0>, T&&)
{
    return {};
}

} // namespace adl_args

namespace args {

template<class T>
struct is_container
: decltype(adl_args::is_container(args::rank<1>{}, std::declval<T>()))
{};

template<>
struct is_container<std::string>
: std::false_type
{};

template<class Range>
using value_of = std::decay_t<decltype(*adl_args::adl_begin(std::declval<Range>()))>;

// Lambda function overloading
template <typename... Ts>
struct overload_set : Ts... {
    using Ts::operator()...;
};

template <typename... T>
overload_set(T...) -> overload_set<T...>;

template<class... Fs>
overload_set<Fs...> overload(Fs... fs) 
{
    return {std::move(fs)...};
}

template<class F>
void each_arg(F) {}

#ifdef __clang__
template<class F, class T, class... Ts>
void each_arg(F f, T&& x, Ts&&... xs)
{
    f(std::forward<T>(x));
    args::each_arg(f, std::forward<Ts>(xs)...);
}
#else
template<class F, class... Ts>
void each_arg(F f, Ts&&... xs)
{
    (void)std::initializer_list<int>{((void)(f(std::forward<Ts>(xs))), 0)...};     
}
#endif

std::vector<std::string> wrap(const std::string& text, unsigned int line_length = 72)
{
    std::vector<std::string> output;
    std::istringstream iss(text.data());

    std::string line;

    do
    {
        std::string word;
        iss >> word;

        if (line.length() + word.length() > line_length)
        {
            output.push_back(line);
            line.clear();
        }
        line += word + " ";
    } while (iss);

    if (!line.empty())
    {
        output.push_back(line);
    }
    return output;
}

template<class Range>
std::string join(Range&& r, std::string delim)
{
    return std::accumulate(std::begin(r), std::end(r), std::string(), [&](const std::string &x, const std::string &y)
    {
        if (x.empty()) return y;
        if (y.empty()) return x;
        return x + delim + y;
    });
}

template<class Predicate>
std::string trim(const std::string &s, Predicate p)
{
   auto wsfront = std::find_if_not(s.begin(), s.end(), p);
   auto wsback = std::find_if_not(s.rbegin(), s.rend(), p).base();
   return (wsback<=wsfront ? std::string() : std::string(wsfront, wsback));
}

template<class Args_Probe_TypeName_>
const std::string& get_type_name()
{
    static std::string name;

    if (name.empty())
    {
#ifdef _MSC_VER
        name = typeid(Args_Probe_TypeName_).name();
        name = name.substr(7);
#else
        const char parameter_name[] = "Args_Probe_TypeName_ =";
        name = __PRETTY_FUNCTION__;

        auto begin = name.find(parameter_name) + sizeof(parameter_name);
        auto length = name.find_first_of("];", begin) - begin;

        name = name.substr(begin, length);
#endif
    }
    
    return name;
}

#define ARGS_GET_CMD_ATTRIBUTE(name, ...) \
namespace detail { \
template<class T> \
auto get_ ## name ## _impl(rank<1>) ARGS_RETURNS(T::name()) \
template<class T> \
auto get_ ## name ## _impl(rank<0>) ARGS_RETURNS(__VA_ARGS__); \
} \
template<class T> \
auto get_ ## name() ARGS_RETURNS(detail::get_ ## name ## _impl<T>(rank<1>{}));

template<class T>
std::string get_command_type_name()
{
    std::string name = args::get_type_name<T>();
    if (auto i = name.find("::"); i != std::string::npos) name = name.substr(i+2);
    return trim(name, [](char c) { return c == '_'; });
}

ARGS_GET_CMD_ATTRIBUTE(name, get_command_type_name<T>());
ARGS_GET_CMD_ATTRIBUTE(help, "");
ARGS_GET_CMD_ATTRIBUTE(options_metavar, "[options...]");

template<class T>
struct value_parser
{
    static T apply(std::string_view x)
    {
        T result;
        std::stringstream ss;
        ss.str(x.data());
        ss >> result;
        return result;
    }
};

template<>
struct value_parser<std::string>
{
    static std::string apply(const std::string& x)
    {
        return x;
    }
};

template<class T, typename std::enable_if<(not is_container<T>{} or std::is_convertible<T, std::string>{}), int>::type = 0>
void write_value_to(T& result, const std::string& x)
{
    result = value_parser<T>::apply(x);
}

template<class T, typename std::enable_if<(is_container<T>{} and not std::is_convertible<T, std::string>{}), int>::type = 0>
void write_value_to(T& result, const std::string& x)
{
    result.insert(result.end(), value_parser<typename T::value_type>::apply(x));
}

void write_value_to(std::nullptr_t, const std::string&)
{
    // Do nothing
}

enum class argument_type
{
    none,
    single,
    multiple
};

template<class T>
argument_type get_argument_type(const T&)
{
    if (std::is_same<T, bool>() or std::is_same<T, std::nullptr_t>()) return argument_type::none;
    else if (is_container<T>()) return argument_type::multiple;
    else return argument_type::single;
}

template<class T>
constexpr std::string type_to_help_impl()
{
    if constexpr (is_container<T>() and not std::is_convertible<T, std::string>()) {
        return args::type_to_help_impl<value_of<T>>() + "...";
    }
    else if constexpr (std::is_same<T, bool>()) return "bool";
    else if constexpr (std::is_convertible<T, std::string>()) return "string";
    else if constexpr (std::is_integral<T>()) return "integer";
    else if constexpr (std::is_floating_point<T>()) return "number";
    else return "argument";
}

template<class T>
std::string type_to_help(const T&)
{
    return "[" + args::type_to_help_impl<T>() + "]";
}

struct argument
{
    argument_type type;
    std::vector<std::string> flags;

    int count = 0;
    bool required = false;
    std::function<void(std::string)> write_value;
    std::vector<std::function<void(const argument&)>> callbacks;
    std::vector<std::function<void(const argument&)>> eager_callbacks;
    std::string help, metavar;

    template<class F>
    void add_callback(F f)
    {
        callbacks.emplace_back(std::move(f));
    }

    template<class F>
    void add_eager_callback(F f)
    {
        eager_callbacks.emplace_back(std::move(f));
    }

    std::string get_flags() const
    {
        std::string result = join(flags, ", ");
        if (type != argument_type::none) result += " " + metavar;
        return result;
    }

    bool write(const std::string& s)
    {
        this->write_value(s);
        count++;
        for(auto&& f:eager_callbacks) f(*this);
        return not eager_callbacks.empty();
    }
};

template<class... Args>
struct subcommand
{
    std::string help;
    std::function<void(std::deque<std::string>, Args...)> run;
};

template<class... Args>
struct context
{
    using subcommand_type = subcommand<Args...>;
    using subcommand_map = std::map<std::string, subcommand_type>;
    std::vector<argument> arguments;
    std::unordered_map<std::string, int> lookup;
    subcommand_map subcommands;
    std::string name;

    bool has_subcommand(const std::string& argv)
    {
        return subcommands.find(argv) != subcommands.end() and argv != name;
    }

    void add(argument arg)
    {
        if (arg.flags.empty()) lookup[""] = arguments.size();
        else for(auto&& name:arg.flags) lookup[name] = arguments.size();
        arguments.emplace_back(std::move(arg));
    }

    template<class T, class... Ts>
    void parse(T&& x, Ts&&... xs)
    {
        argument arg;
        arg.write_value = [&x](const std::string& s) { args::write_value_to(x, s); };
        arg.type = args::get_argument_type(x);
        arg.metavar = args::type_to_help(x);
        args::each_arg(args::overload(
            [&, this](const std::string& name) { arg.flags.push_back(name); },
            [&, this](auto&& attribute) -> decltype(attribute(x, *this, arg), void()) { attribute(x, *this, arg); }
        ), std::forward<Ts>(xs)...);
        this->add(std::move(arg));
    }

    argument& operator[](const std::string& flag)
    {
        if (lookup.find(flag) == lookup.end())
        {
            throw std::runtime_error(name + ": unknown flag: " + flag);
        }
        //else
        return arguments[lookup.at(flag)];
    }

    const argument& operator[](const std::string& flag) const
    {
        if (lookup.find(flag) == lookup.end())
        {
            throw std::runtime_error(name + ": unknown flag: " + flag);
        }
        //else
        return arguments[lookup.at(flag)];
    }

    void show_help_col(const std::string& item, const std::string& help, int width, int total_width) const
    {
        auto txt = args::wrap(help, total_width-width-2);
        assert(!txt.empty());
        std::cout << " " << std::setw(width) << item << " " << txt[0] << std::endl;
        std::for_each(txt.begin()+1, txt.end(), [&](const std::string& line)
        {
            std::cout << " " << std::setw(width) << " " << " " << line << std::endl;
        });
    }

    void show_help(std::string name, std::string description, std::string options_metavar) const
    {
        const int total_width = 80;
        std::vector<std::string> flags;
        int width = 0;
        for(auto&& arg:arguments)
        {
            std::string flag = arg.get_flags();
            width = std::max(width, int(flag.size()));
            flags.push_back(std::move(flag));
        }
        for(auto&& p:subcommands) width = std::max(width, int(p.first.size()));
        std::cout << "Usage: " << name;

        if (subcommands.size() > 0) std::cout << " [command]";

        std::cout << " " << options_metavar;
        if (lookup.count("") > 0) std::cout << " " << (*this)[""].metavar;
        
        std::cout << std::endl; 
        std::cout << std::endl;
        for(auto line:args::wrap(description, total_width-2)) std::cout << "  " << line << std::endl;
        std::cout << std::endl;
        std::cout << "Options: " << std::endl << std::endl;
        // TODO: Switch to different format when width > 40
        for(auto&& arg:arguments)
        {
            this->show_help_col(arg.get_flags(), arg.help, width, total_width);
        }
        if (subcommands.size() > 0)
        {
            std::cout << std::endl;
            std::cout << "Commands: " << std::endl << std::endl;
            for(auto&& p:subcommands)
            {
                this->show_help_col(p.first, p.second.help, width, total_width);
            }
        }
        std::cout << std::endl;
    }

    void post_process()
    {
        for(auto&& arg:arguments)
        {
            for(auto&& f:arg.callbacks) f(arg);
        }
    }
};

template<class F>
auto callback(F f)
{
    return [f](auto&& data, auto& ctx, argument& a)
    {
        a.add_callback([f, &ctx, &data](const argument& arg)
        {
            f(data, ctx, arg);
        });
    };
}

template<class F>
auto eager_callback(F f)
{
    return [f](auto&& data, auto& ctx, argument& a)
    {
        a.add_eager_callback([f, &data, &ctx](const argument& arg)
        {
            f(data, ctx, arg);
        });
    };
}

template<class F>
auto action(F f)
{
    return args::eager_callback(std::bind(f));
}

template<class T>
auto show(T text)
{
    return action([=]{ std::cout << text << std::endl; });
}

auto required()
{
    return [](auto&&, auto&, argument& a)
    {
        a.required = true;
        a.add_callback([](const argument& arg)
        {
            if (arg.required and arg.count == 0)
            {
                throw std::runtime_error("required arg missing: " + arg.get_flags());
            }
        });
    };
}

template<class T>
auto set(T value)
{
    return [value](auto&& data, auto&, argument& a)
    {
        a.type = argument_type::none;
        a.write_value = [&data, value](const std::string&) { data = value; };
    };
}

auto count()
{
    return [](auto&& data, auto&, argument& a)
    {
        a.type = argument_type::none;
        a.add_callback([&data](const argument& arg)
        {
            data = arg.count;
        });
        a.write_value = [](const std::string&) {};
    };
}

#define ARGS_SET_ARG(name) \
template<class T> \
auto name(T&& x) \
{ \
    return [=](auto&&, auto&, argument& a) \
    { \
        a.name = x; \
    }; \
}

ARGS_SET_ARG(help);
ARGS_SET_ARG(metavar); 

template<class T, class F>
auto try_parse(rank<1>, T& x, F f) ARGS_RETURNS(x.parse(f));

template<class T, class F>
void try_parse(rank<0>, T&, F) {}

template<class C, class T>
auto assign_subcommands(rank<1>, C& ctx, T&) -> decltype(T::subcommands, void())
{ ctx.subcommands = T::subcommands(); }

template<class C, class T>
void assign_subcommands(rank<0>, C&, T&) {}

template<class... Ts, class T>
context<T&, Ts...> build_context(T& cmd)
{
    context<T&, Ts...> ctx;
    ctx.name = get_name<T>();
    args::assign_subcommands(rank<1>{}, ctx, cmd);
    ctx.parse(nullptr, "-h", "--help", args::help("Show help"), 
        args::eager_callback([](std::nullptr_t, const auto& c, const argument&)
    {
        c.show_help(get_name<T>(), get_help<T>(), get_options_metavar<T>());
    }));
    args::try_parse(rank<1>{}, cmd, [&](auto&&... xs)
    {
        ctx.parse(std::forward<decltype(xs)>(xs)...);
    });
    return ctx;
}

template<class Iterator>
std::string pop_string(Iterator first, Iterator last)
{
    if (first == last) return std::string();
    else return std::string(first+1, last);
}

std::tuple<std::string, std::string> parse_attached_value(const std::string& s)
{
    assert(s.size() > 0);
    assert(s[0] == '-' && "Not parsing a flag");
    if (s[1] == '-')
    {
        auto it = std::find(s.begin(), s.end(), '=');
        return std::make_tuple(std::string(s.begin(), it), args::pop_string(it, s.end()));
    }
    else if (s.size() > 2)
    {
        return std::make_tuple(s.substr(0, 2), s.substr(2));
    }
    else
    {
        return std::make_tuple(s, std::string());
    }
}

template<class Container>
auto drop(Container c)
{
    c.pop_front();
    return c;
}

template<class T, class... Ts>
auto try_run(rank<2>, T& x, Ts&&... xs) ARGS_RETURNS(x.run(xs...));

template<class T, class... Ts>
auto try_run(rank<1>, T& x, Ts&&...) ARGS_RETURNS(x.run())

template<class T, class... Ts>
void parse(T& cmd, std::deque<std::string> a, Ts&&... xs)
{
    auto ctx = args::build_context<Ts...>(cmd);

    bool capture = false;
    std::string core;
    for(auto&& x:a)
    {
        if (ctx.has_subcommand(x))
        {
            ctx.subcommands[x].run(drop(a), cmd, xs...);
            return;
        } 
        if (x[0] == '-')
        {
            capture = false;
            std::string value;
            std::tie(core, value) = args::parse_attached_value(x);

            if (ctx[core].type == argument_type::none)
            {
                ctx[core].write("");
                if (core == "-h" or core == "--help") return;
            }
            else
            {
                capture = true;
            }
        }
        else if (capture)
        {
            if (ctx[core].write(x)) return;
            capture = ctx[core].type == argument_type::multiple;
        }
        else
        {
            if (core.empty())
            {
                throw std::runtime_error("unknown command: " + x);
            }
            if (ctx[core].type == argument_type::none)
            {
                throw std::runtime_error("flag: " + core + " does not expect an argument.");
            }
            if (ctx[core].type != argument_type::multiple)
            {
                throw std::runtime_error("flag: " + core + " expects only one argument.");
            }
        }


        a.pop_front();
    }
    ctx.post_process();

    args::try_run(rank<2>{}, cmd, xs...);
}

template<class T, class... Ts>
void parse(std::deque<std::string> a, Ts&&... xs)
{
    T cmd = {};
    
    args::parse(cmd, std::move(a), xs...);
}

template<class T>
bool parse(int argc, char const *argv[])
{
    std::deque<std::string> as(argv+1, argv+argc);

    try
    {
        args::parse<T>(as);
    }
    catch(const std::exception& ex)
    {
        std::cerr << "Error: " << ex.what() << std::endl;
        return false;
    }
    
    return true;
}

template<class T, class F>
struct auto_register
{
    static bool auto_register_reg_;
    static bool auto_register_reg_init_()
    {
        F::template apply<T>();
        return true;
    }

    auto_register()
    {
        (void)auto_register_reg_;
    }
};

template<class T, class F>
bool auto_register<T, F>::auto_register_reg_ = auto_register<T, F>::auto_register_reg_init_();

template<class Derived>
struct group
{
    using context_type = context<Derived&>;
    using subcommand_type = typename context_type::subcommand_type;
    using subcommand_map = typename context_type::subcommand_map;

    static subcommand_map& subcommands()
    {
        static subcommand_map subcommands_;
        return subcommands_;
    }
    
    template<class T>
    static void add_command()
    {
        subcommand_type sub;
        sub.run = [](auto a, auto&&... xs)
        {
            args::parse<T>(a, xs...);
        };
        sub.help = get_help<T>();
        subcommands().emplace(get_name<T>(), sub);
    }

    struct auto_register_command
    {
        template<class T>
        static void apply() { add_command<T>(); }
    };

    template<class D>
    struct command : auto_register<D, auto_register_command>
    {};

    void run() {}
};

} // namespace args

#endif
