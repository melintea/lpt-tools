/*
 *  $Id: $
 *
 *  Copyright 2Michelangelo Altamore.
 *  Released under the MIT license.
 *
 *  Access to private members
 *  @see https://github.com/altamic/privablic
 */

#ifndef INCLUDED_access_hpp_0a96beab_1f4e_4307_b634_ff9181149b2e
#define INCLUDED_access_hpp_0a96beab_1f4e_4307_b634_ff9181149b2e

#pragma once

namespace lpt {

/*
 \code
 
    using namespace lpt;

    //Now, suppose you know the implementation of a class (or a struct) like that:

    class Sheep 
    {
    public:
	Sheep(std::string name_) : name{ std::move(name_) } {}

    private:
	// Data
	std::string name;
	static int TOTAL;

	// Functions
	void baa() { std::cout << name << ": Baa! Baa!\n"; };

	static void FlockCount() 
	{
	  std::cout << "sheperd actually counted " << TOTAL << " sheep\n";
	}
    };

    int Sheep::TOTAL = 42;

    //You only have to map some STUBs according to types of members and/or methods signatures:
    
    //Instance Member

    struct Sheep_name { typedef string (Sheep::*type); };
    template class private_member<Sheep_name, &Sheep::name>;

    //Instance Method

    struct Sheep_baa { typedef void(Sheep::*type)(); };
    template class private_method<Sheep_baa, &Sheep::baa>;

    //Static Instance Member

    struct Sheep_TOTAL { typedef int *type; };
    template class private_member<Sheep_TOTAL, &Sheep::TOTAL>;

    //Static Instance Method

    struct Sheep_FlockCount { typedef void(*type)(); };
    template class private_method<Sheep_FlockCount, &Sheep::FlockCount>;

    //Then, using an instance of Sheep, you can access the private members like this:

    Sheep dolly = Sheep("Dolly");

    // now we have a sheep under our complete control:

    // - change dolly's identity
    dolly.*member<Sheep_name>::value = "Lilly";

    // - make dolly baa
    (&dolly->*func<Sheep_baa>::ptr)();

    // - steal dolly
    int flockCount = *member<Sheep_TOTAL>::value -= 1;

    // - let the sheperd realize it
    (*func<Sheep_FlockCount>::ptr)();

 \endcode
 */


// Generate a static data member of type STUB::type in which to store
// the address of a private member.  It is crucial that STUB does not
// depend on the /value/ of the the stored address in any way so that
// we can access it from ordinary code without directly touching
// private data.
template <class STUB>
struct member
{
  static typename STUB::type value;
}; 
template <class STUB> 
typename STUB::type member<STUB>::value;


// Generate a static data member whose constructor initializes
// member<STUB>::value. This type will only be named in an explicit
// instantiation, where it is legal to pass the address of a private
// member.
template <class STUB, typename STUB::type x>
struct private_member
{
  private_member() { member<STUB>::value = x; }
  static private_member instance;
};
template <class STUB, typename STUB::type x> 
private_member<STUB, x> private_member<STUB, x>::instance;


template<typename STUB>
struct func {
  /* export it ... */
  typedef typename STUB::type type;
  static type ptr;
};

template<typename STUB>
typename func<STUB>::type func<STUB>::ptr;

template<typename STUB, typename STUB::type p>
struct private_method : func<STUB> {
  /* fill it ... */
  struct _private_method {
    _private_method() { func<STUB>::ptr = p; }
  };
  static _private_method private_method_obj;
};

template<typename STUB, typename STUB::type p>
typename private_method<STUB, p>::_private_method private_method<STUB, p>::private_method_obj;

} //namespace lpt


#endif //#define INCLUDED_access_hpp_0a96beab_1f4e_4307_b634_ff9181149b2e
