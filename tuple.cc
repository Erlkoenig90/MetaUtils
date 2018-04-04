#include <cstddef>
#include <type_traits>
#include <utility>
#include <iostream>
#include <string>

template <typename T, std::size_t I, bool Inherit>
class TupleElem;

template <typename T, std::size_t I>
class TupleElem<T,I,true> : private T {
	public:
		template<class U, class = typename std::enable_if<std::is_constructible<T, U>::value>::type>
		constexpr TupleElem (U&& x) : T (std::forward<U> (x)) { }

		template<typename X = T, class = typename std::enable_if<std::is_default_constructible<X>::value>::type>
		constexpr TupleElem () {}
		
		constexpr TupleElem (const TupleElem&) = default;
		constexpr TupleElem (TupleElem&&) = default;
		
		constexpr TupleElem& operator = (const TupleElem&) = default;
		constexpr TupleElem& operator = (TupleElem&&) = default;

		constexpr T& get () & { return static_cast<T&> (*this); }
		constexpr const T& get () const & { return static_cast<const T&> (*this); }
		constexpr T&& get () && { return static_cast<T&&> (*this); }
};

template <typename T, std::size_t I>
class TupleElem<T,I,false> {
	private:
		T m_elem;
	public:
		template<class U, class = typename std::enable_if<std::is_constructible<T, U>::value>::type>
		constexpr TupleElem (U&& x) : m_elem (std::forward<U> (x)) { }

		template<typename X = T, class = typename std::enable_if<std::is_default_constructible<X>::value>::type>
		constexpr TupleElem () : m_elem {} {}

		constexpr TupleElem (const TupleElem& src) = default;
		constexpr TupleElem (TupleElem&& src) = default;

		TupleElem& operator = (const TupleElem&) = default;
		TupleElem& operator = (TupleElem&&) = default;

		constexpr T& get () & { return m_elem; }
		constexpr const T& get () const & { return m_elem; }
		constexpr T&& get () && { return static_cast<T&&> (m_elem); }
};

template <typename T, std::size_t I>
using GTupleElem = TupleElem <T, I, std::is_empty<T>::value && !std::is_final<T>::value>;

template <typename... T>
struct TypeList;

template <typename Types, typename Indices>
class TupleI;

template <typename... Types, std::size_t... Indices>
class TupleI<TypeList<Types...>, std::index_sequence<Indices...>> : private GTupleElem<Types, Indices>... {
	private:
		template <std::size_t I, typename T>
		static T getTupleIndex (const GTupleElem<T, I>*);
	public:
		template <std::size_t I>
		using Get = decltype (getTupleIndex<I> (static_cast<TupleI*> (nullptr)));
		
		static constexpr std::size_t size = sizeof...(Types);
		
		template <typename... Args, class = typename std::enable_if<sizeof...(Args) == sizeof...(Types) && (std::is_constructible<Types, Args>::value && ...), int>::type>
		constexpr TupleI (Args&&... args) : GTupleElem<Types, Indices> { std::forward<Args> (args) }... {}

		template<std::size_t C = sizeof...(Types), class = typename std::enable_if<C == 0 || (std::is_default_constructible<Types>::value && ...)>::type>
		constexpr TupleI () {}

		constexpr TupleI (const TupleI&) = default;
		constexpr TupleI (TupleI&& src) = default;
		
		TupleI& operator = (const TupleI&) = default;
		TupleI& operator = (TupleI&&) = default;

		template <std::size_t Index>
		constexpr Get<Index>& get () & { return static_cast<GTupleElem<Get<Index>, Index>&> (*this).get (); }

		template <std::size_t Index>
		constexpr const Get<Index>& get () const& { return static_cast<const GTupleElem<Get<Index>, Index>&> (*this).get (); }

		template <std::size_t Index>
		constexpr Get<Index>&& get () && { return static_cast<GTupleElem<Get<Index>, Index>&&> (*this).get (); }
};

template <typename... T>
using Tuple = TupleI<TypeList<T...>, std::make_index_sequence<sizeof...(T)>>;

template <typename... T>
constexpr Tuple<T...> makeTuple (T&&... args) {
	return { std::forward<T> (args)... };
}

template <typename... T, std::size_t... I>
std::ostream& printTuple (std::ostream& os, const TupleI<TypeList<T...>, std::index_sequence<I...>>& t) {
	(os << ... << t.template get<I> ());
	return os;
}

template <std::size_t... I>
constexpr decltype(auto) makeIntegerTuple (std::index_sequence<I...>) {
	return makeTuple ( I... );
}

template <std::size_t I>
constexpr auto makeIntegerTuple () {
	return makeIntegerTuple (std::make_index_sequence<I> {});
}

int main () {
	constexpr auto t = makeIntegerTuple<1024> ();
	
	printTuple (std::cout, t);
}