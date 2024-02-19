/*
 * Enum stringizing 
 */
 

#include <lpt/to_string.hpp>

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

    return EXIT_SUCCESS;
}
