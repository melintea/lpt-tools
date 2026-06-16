/*
 * Verify if struct elements are declared by decreasing size
 * C++17
 */
 
#include <source_location>
#include <tuple>
#include <type_traits>
#include <utility>

// Helper to convert a 3-member struct into a tuple via structural binding
template <typename T>
constexpr auto to_tuple(T&& obj) 
{
    auto&& [a, b, c] = std::forward<T>(obj);
    return std::make_tuple(a, b, c);
}

// Constexpr function to verify if tuple elements are sorted by size (largest to smallest)
template <typename Tuple, std::size_t... Is>
constexpr bool is_size_sorted_impl(std::index_sequence<Is...>) 
{
    // Check if sizeof(Element N) >= sizeof(Element N+1) for all adjacent pairs
    return ((sizeof(std::tuple_element_t<Is, Tuple>) >= sizeof(std::tuple_element_t<Is + 1, Tuple>)) && ...);
}

template <typename Tuple>
constexpr bool is_size_sorted() 
{
    constexpr std::size_t Size = std::tuple_size_v<Tuple>;
    if constexpr (Size <= 1) {
        return true;
    } else {
        return is_size_sorted_impl<Tuple>(std::make_index_sequence<Size - 1>{});
    }
}

template <typename Type> struct UnoptimizedTypePrinter;

template <typename Type>
constexpr void is_optimal() 
{
    using TupleType = decltype(to_tuple(std::declval<Type>()));
    constexpr bool isOptimal = is_size_sorted<TupleType>();
    if constexpr ( ! isOptimal) {
        UnoptimizedTypePrinter<Type> err;
        static_assert(isOptimal, "Sort members from largest to smallest");
    }
}


// ==================== TEST CASES ====================

// Valid: Sorted from largest (8 bytes) to smallest (1 byte)
struct OptimizedStruct 
{
    double d; // 8 bytes
    int i;    // 4 bytes
    char c;   // 1 byte
};

// Invalid: Unsorted (4 bytes, then 8 bytes, then 1 byte)
struct BadStruct 
{
    int i;    // 4 bytes
    double d; // 8 bytes
    char c;   // 1 byte
};


int main()
{
    is_optimal<OptimizedStruct>();
    is_optimal<BadStruct>();
    return EXIT_SUCCESS;
}

