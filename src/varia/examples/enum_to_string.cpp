/*
 * Enum stringizing 
 */
 

#include <lpt/enum_tools.hpp>

#include <cassert>
#include <iostream>

int main()
{
    enum class Color { eRED=-199, eGREEN=0, eYELLOW=5 };
    {
        //auto color = Color::eRED;
        //std::cout << lpt::to_string(color) << std::endl; // 'eRED'
        std::cout << lpt::to_string<Color::eRED>() << std::endl; // 'eRED'
    }
    {
        std::cout << lpt::to_string<Color::eGREEN>() << std::endl; // 'eGREEN'
    }
    {
        //auto color = Color::eYELLOW;
        //std::cout << lpt::to_string(color) << std::endl; // 'eYELLOW'
        std::cout << lpt::to_string<Color::eYELLOW>() << std::endl; // 'eYELLOW'
    }
    {
        if ( true  == lpt::is_valid_enum_value<Color, Color::eRED>() ) {std::cout << "pass\n"; }
        if ( true  == lpt::is_valid_enum_value<Color, -199>() ) {std::cout << "pass\n"; };
        if ( false == lpt::is_valid_enum_value<Color, -200>() ) {std::cout << "pass\n"; };

        auto num(lpt::count<Color, -200, 10>());
        if ( 3     == num ) {std::cout << "pass\n"; } else {std::cout << "fail:" << num << "\n";  };
    }

    {
        //template<>
        //struct enum_reflection<Color> { static constexpr const std::array _vals = { Color::eRED, Color::eGREEN, Color::eYELLOW }; };
    }

    return EXIT_SUCCESS;
}
