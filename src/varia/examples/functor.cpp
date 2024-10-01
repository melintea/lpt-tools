//==============================================================================
// Poor man's function & task (cdschecker has limited std support 
// and std::function is messing with memory addresses being checked)
//==============================================================================


#include <cassert>
#include <iostream>
#include <utility>
#include <tuple>
#include <type_traits>


// Type checkers: std::is_same<ret_t, void>::value
template<typename T>
struct is_void
{
    static const bool value = false;
};

template<>
struct is_void<void>
{
    static const bool value = true;
};


// callable interface
template<typename RET, typename... ARGS>
struct callable
{
    using ret_t       = RET ;
    using signature_t = ret_t (*)(ARGS...);

    virtual ~callable() {}

    virtual ret_t operator()(ARGS... args) = 0;   
};


// function & lambda closure wrapper
template<typename CLOSURE, typename RET, typename...ARGS>
struct closure: public callable<RET, ARGS...>
{
    using ret_t     = RET;
    using closure_t = CLOSURE;

    const closure_t _closureHandler;

    closure(const CLOSURE& handler)
        : _closureHandler(handler)
    {}

    ret_t operator()(ARGS... args) override
    {
        if constexpr (is_void<ret_t>::value) {
            (void)_closureHandler(std::forward<ARGS>(args)...);
        } else {
            return _closureHandler(std::forward<ARGS>(args)...);
        }
    }
};


// function<> template catch-all
template <typename FUNCTION>
class function
    : public function<decltype(&FUNCTION::operator())>
{
};


// function, lambda, functor...
template <typename RET, typename... ARGS>
class function<RET(*)(ARGS...)>
{

public:

    using ret_t         = RET;
    using signature_t   = ret_t (*)(ARGS...);
    using function_t    = function<signature_t> ;
    
    callable<ret_t, ARGS...>* _callableClosure;

    function(RET(*function)(ARGS...))
        : _callableClosure(new closure<signature_t, ret_t, ARGS...>(function))
    {}

    // Captured lambda specialization
    template<typename CLOSURE>
    function(const CLOSURE& func)
        : _callableClosure(new closure<decltype(func), ret_t, ARGS...>(func))
    {}
    
    ~function()
    {
        delete _callableClosure;
    }

    ret_t operator()(ARGS... args)
    {
        if constexpr (is_void<ret_t>::value) {
            (void)(*_callableClosure)(std::forward<ARGS>(args)...);
        } else {
            return (*_callableClosure)(std::forward<ARGS>(args)...);
        }
    }
};

// Member method
template <typename CLASS, typename RET, typename... ARGS>
class function<RET(CLASS::*)(ARGS...)>
{

public:

    using ret_t         = RET;
    using signature_t   = ret_t (CLASS::*)(ARGS...);
    using function_t    = function<signature_t> ;

    signature_t _methodSignature;

    function(RET(CLASS::*method)(ARGS...))
        : _methodSignature(method)
    {}

    ret_t operator()(CLASS* object, ARGS... args)
    {
        if constexpr (is_void<ret_t>::value) {
            (void)(object->*_methodSignature)(std::forward<ARGS>(args)...);
        } else {
            return (object->*_methodSignature)(std::forward<ARGS>(args)...);
        }
    }
};

// Const member method
template <typename CLASS, typename RET, typename... ARGS>
class function<RET(CLASS::*)(ARGS...) const>
{

public:

    using ret_t         = RET;
    using signature_t   = ret_t (CLASS::*)(ARGS...) const;
    using function_t    = function<signature_t> ;

    signature_t _methodSignature;

    function(RET(CLASS::*method)(ARGS...) const)
        : _methodSignature(method)
    {}

    ret_t operator()(CLASS* object, ARGS... args)
    {
        if constexpr (is_void<ret_t>::value) {
            (void)(object->*_methodSignature)(std::forward<ARGS>(args)...);
        } else {
            return (object->*_methodSignature)(std::forward<ARGS>(args)...);
        }
    }
};

struct task_base
{
    virtual ~task_base() {}
    
    virtual void operator()() = 0; 

    static void run(void* pTask)
    {
        task_base* p(reinterpret_cast<task_base*>(pTask));
        assert(p);
        p->operator()();
    }   
};


// Canned fuction<> + args template catch-all
template <typename FUNCTION>
class task
    : public task<decltype(&FUNCTION::operator())>
{
};

// Canned (function || lambda || functor) + args for a thread
template <typename RET, typename... ARGS>
class task<RET(*)(ARGS...)> 
    : public task_base
{

public:

    using ret_t         = RET;
    using signature_t   = ret_t (*)(ARGS...);
    using function_t    = function<signature_t> ;
    
    callable<ret_t, ARGS...>* _callableClosure;
    std::tuple<ARGS...>       _args;

    task(RET(*func)(ARGS...), ARGS&&... args)
        : _callableClosure(new closure<signature_t, ret_t, ARGS...>(func))
    , _args(std::forward<ARGS>(args)...)
    {}
    
    // Captured lambda specialization
    template<typename CLOSURE>
    task(const CLOSURE& func)
        : _callableClosure(new closure<decltype(func), ret_t, ARGS...>(func))
    {}
    
    ~task()
    {
        delete _callableClosure;
    }

    ret_t operator()()
    {
        if constexpr (is_void<ret_t>::value == true) {
            (void)std::apply(*_callableClosure, std::move(_args));
        } else {
            return std::apply<ret_t>(*_callableClosure, std::move(_args));
        }
    }
    
    /*
    void run(void* pTask)
    {
        task* p(reinterpret_cast<task*>(pTask));
        assert(p);
        p->operator()();
    }
    */
};



template <typename FUNC, typename ... ARGS>
auto /*task<RET(*)(ARGS...)>*/
create_task(FUNC&& func, ARGS&& ... args)// -> decltype(func(args...))
{
    using ret_t         = decltype(func(args...));
    using signature_t   = ret_t (*)(ARGS...);
    using function_t    = function<signature_t> ;
    
    return task<signature_t>(std::forward<FUNC>(func), std::forward<ARGS>(args)...);
}



//==============================================================================
// 
//==============================================================================


class Foo
{

public:

    int bar(int a, int b)
    {
        return a + b;
    }
};



int somefunction(int a, int b)
{
    return a + b;
}

int someTask1(int x)
{
    std::cout << "x=" << x << std::endl;
    return x;
}

void someTask2(int x, int* py)
{
    std::cout << "someTask2 x=" << x << std::endl;
    std::cout << "someTask2 py = 0x" << py << '\n';
}


int main(int argc, char** argv)
{
    { // function
        int  a  = 15;
        int* pa = &a;
        int  b  = 1;
    
        // Lambda capturing scope
        a = 101;
        function<void(*)()> fn0([&]() {
                a = 10;
        });
        fn0();
        std::cout << a << std::endl; // 10
        assert(a == 10);

        // Lambda without capturing
        function<int(*)(int)> fn1([] (int b) -> int {
            return b;
        });
        auto r1(fn1(2));
        std::cout << r1 << std::endl; // 2
        assert(r1 == 2);

        // Lambda capturing variable
        function<int(*)(int)> fn2([a] (int c) -> int {
            return a + c;
        });
        auto r2(fn2(-7));
        std::cout << r2 << std::endl; // 3
        assert(r2 == 3);

        // Lambda capturing scope
        function<int(*)(int)> fn3([&] (int c) -> int {
            return a + c;
        });
        auto r3(fn3(-5));
        std::cout << r3 << std::endl; // 5
        assert(r3 == 5);

        // Arguments by reference
        function<void(*)(int&, int)> fn4([] (int& d, int f) {
            d = d + f;
        });
        fn4(a/*&d*/, -3); // Void call
        std::cout << a << std::endl; // 7
        assert(a == 7);

        // Top level function reference
        function<int(*)(int, int)> fn6(somefunction);
        auto r6(fn6(a, 4));
        std::cout << r6 << std::endl; // 11
        assert(r6 == 11);

        // Member method
        Foo* foo = new Foo();
        function<int(Foo::*)(int,int)> fn7(&Foo::bar);
        auto r7(fn7(foo, a, 8));
        std::cout << r7 << std::endl; // 15
        assert(r7 == 15);
    }
    
    std::cout << "==== task \n";
    { // task
        int  a  = 15;
        int* pa = &a;
        int  b  = 1;
        std::cout << "&a = 0x" << pa << '\n';

        // Lambda 
        auto /*task<void(*)()>*/ task1 = create_task([&]() {
            a += 5;
            std::cout << "&a = 0x" << pa << '\n';
            assert(pa == &a);
        });
        task1();
        std::cout << a << std::endl; // 20
        assert(a == 20);

        // Top level function reference
        a = 5;
        auto /*task<void(*)(int, int*)>*/ task2 = create_task(someTask2, 42, &a);
        task2();
        //decltype(task2)::run(&task2);
        task_base::run((void*)&task2);
        task_base* pTask = static_cast<task_base*>(&task2);
        pTask->run(&task2);
        std::cout << a << std::endl; // 5
        assert(a == 5);

#if 0
        // not yet supported
        auto /*task<int(*)(int)>*/ task3 = create_task(someTask1, 42);
        task3();
        std::cout << a << std::endl; // 5
        assert(a == 5);
#endif
    }
}


