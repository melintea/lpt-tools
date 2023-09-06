/*
 *  Copyright 2012 Aurelian Melinte. 
 *  Released under GPL 3.0 or later. 
 *
 *  You need a C++0x compiler. 
 *  Needs bfd.h & libbfd.so. Debian: this is package binutils-dev .
 * 
 */
 
 
#include <elf.h>
#include <execinfo.h>
#include <cxxabi.h>
#include <dlfcn.h>

//#define _GNU_SOURCE
#include <signal.h>
#include <bfd.h>
#include <unistd.h>

#include <cassert>
#include <cstring>
#include <cstdlib>

#include <memory>
#include <iostream>
#include <filesystem>
#include <mutex> 
#include <array> 
#include <string> 
#include <map> 
#include <fstream>
#include <sstream>
#include <algorithm>

#include <lpt/nocopy.hpp>
#include <lpt/stackonly.hpp>
#include <lpt/singleton.hpp>

#pragma once 


/*
 *
 */ 
namespace lpt { namespace stack { namespace detail {

/*
 *
 */

typedef void*                address_type;
static const address_type    null_address_type = nullptr;
typedef long                 delta_type;

/*
 *
 */

static inline
int backtrace(address_type* buffer, int size)
{
    return ::backtrace(buffer, size);
}

static inline 
bool ends_with(const std::string& value, const std::string& ending)
{
    if (ending.size() > value.size()) {
        return false;
    }
    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

// Choose between bfd_get_section_vma() of older bfd API and bfd_section_vma()
template <typename BFD, typename ASECTION>
auto bfd_get_section_vma_wrapper(BFD *abfd, const ASECTION *section) -> decltype(bfd_get_section_vma((BFD*)0, nullptr))
{
    return bfd_get_section_vma(abfd, section);
}

template <typename BFD, typename ASECTION>
auto bfd_get_section_vma_wrapper(BFD *abfd, const ASECTION *section) -> decltype(bfd_section_vma((ASECTION*)0))
{
    return bfd_section_vma(section);
}

/*
 *
 */

class null_symbol_info
{
public:

    null_symbol_info(const address_type& addr = null_address_type) noexcept
        : _demangled_func_name(nullptr, std::free)
    {
        resolve(addr);
    }

    null_symbol_info(null_symbol_info const& other) noexcept : null_symbol_info(other._addr) {}
    null_symbol_info& operator=(null_symbol_info other) noexcept
    {
        swap(other);
        return *this;
    }

    null_symbol_info(null_symbol_info&& other) noexcept : null_symbol_info()
    {
        swap(other);
    }
    null_symbol_info& operator=(null_symbol_info&& other) noexcept
    {
        assert(this != &other);
        swap(other);
        return *this;
    }

    void resolve(const address_type& addr) noexcept
    {
        _addr = addr;
        _binary_name = nullptr;
        _func_name = nullptr;
        _demangled_func_name = nullptr;
        _delta = 0;
        _source_file_name = nullptr;
        _line_number = 0;

        _dladdr_ret = false;
    }

    const address_type   addr() const noexcept      { return _addr; }
    const char*  binary_file() const noexcept       { return safe(_binary_name); }
    const char*  raw_function_name() const noexcept { return safe(_func_name); }
    const char*  demangled_function_name() const noexcept
    {
        if (_demangled_func_name) {
            return safe(const_cast<const char*>(_demangled_func_name.get()));
        }
        else {
            return raw_function_name();
        }
    }
    delta_type   delta() const noexcept             { return _delta; }
    const char*  source_file() const noexcept       { return safe(_source_file_name); }
    unsigned int line_number() const noexcept       { return  _line_number; }


    void swap(null_symbol_info& other) noexcept
    {
        std::swap(_addr,                 other._addr);
        std::swap(_binary_name,          other._binary_name);
        std::swap(_func_name,            other._func_name);
        std::swap(_demangled_func_name,  other._demangled_func_name);
        std::swap(_delta,                other._delta);
        std::swap(_source_file_name,     other._source_file_name);
        std::swap(_line_number,          other._line_number);
        std::swap(_dladdr_ret,           other._dladdr_ret);
    }

protected:

    const char* safe(const char* p) const { return (p && *p) ? p : "??"; }

protected:

    address_type  _addr;
    const char*   _binary_name;
    const char*   _func_name;
    std::unique_ptr<char, void(*)(void*)>  _demangled_func_name;
    delta_type    _delta;

    const char*   _source_file_name;
    unsigned int  _line_number;

    bool          _dladdr_ret;
}; //null_symbol_info

inline void swap(null_symbol_info& left, null_symbol_info& right) noexcept
{
    assert(&left != &right);
    left.swap(right);
}


/*
 *
 */

class basic_symbol_info : public null_symbol_info
{
public:

    typedef null_symbol_info  base_type;

    basic_symbol_info(const address_type& addr = nullptr) noexcept : base_type(addr)
    {
        resolve(addr);
    }

    basic_symbol_info(basic_symbol_info const& other) noexcept : basic_symbol_info(other._addr) {}
    basic_symbol_info& operator=(basic_symbol_info other) noexcept
    {
        swap(other);
        return *this;
    }

    basic_symbol_info(basic_symbol_info&& other) noexcept : basic_symbol_info()
    {
        swap(other);
    }
    basic_symbol_info& operator=(basic_symbol_info&& other) noexcept
    {
        assert(this != &other);
        swap(other);
        return *this;
    }

    void resolve(const address_type& addr) noexcept
    {
        _resolve(addr);
    }

    void swap(basic_symbol_info& other) noexcept
    {
        base_type::swap(other);
        std::swap(_info, other._info);
    }

protected:

    void _demangle() noexcept
    {
        int status = 0;
        _demangled_func_name.reset(abi::__cxa_demangle(_func_name, 0, 0, &status));
    }

    void _resolve(const address_type& addr) noexcept
    {
        base_type::resolve(addr);

        if (addr == null_address_type)
            return;

        _dladdr_ret = (::dladdr(_addr, &_info) != 0);
        if (_dladdr_ret)
        {
            _binary_name = _info.dli_fname;
            _func_name   = _info.dli_sname;
            _delta = static_cast<const char *>(_addr) - static_cast<const char *>(_info.dli_saddr);

            _demangle();
        }
    }

protected:

    Dl_info     _info;
}; //basic_symbol_info

inline void swap(basic_symbol_info& left, basic_symbol_info& right) noexcept
{
    assert(&left != &right);
    left.swap(right);
}


/*
 *
 */

namespace bfd {

typedef ::bfd cbfd;
typedef struct _sym_tab
{
    std::shared_ptr< cbfd >        abfd;
    unsigned                       storage_needed{0};
    std::shared_ptr< asymbol * >   syms;
    unsigned                       cSymbols{0};
    asection *                     text{nullptr};
    bool                           dynamic{false};
    std::shared_ptr<asymbol*>      ptr;
    /// Module base address. Linked libraries may be located at arbitrary places within the 
    /// virtual address space of the process. One needs to find where they are located and 
    /// fix the addresses accordingly, before resolving the symbol via libbfd. 
    /// This can be done by reading the /proc/<pid>/maps file, which lists where the loaded 
    /// libraries are located within the address space.
    bfd_vma                        base{0};
} sym_tab_type;

/// Map modules to their base addresses
typedef std::map<std::string, sym_tab_type> sym_map_type;

// Map modules to their base addresses
typedef std::map<std::string, bfd_vma> bfd_vma_map_type;

struct bfd_close_wrapper
{
    bool operator() (cbfd*& abfd)
    {
        bool ret = false;
        if (abfd)
        {
            ret = bfd_close(abfd);
        }
        return ret;
    }
};


class library : public lpt::nocopy
              , public lpt::stackonly
{
public:

    library()
    {
        std::unique_lock<std::mutex> lock(_lib_mutex);
        bfd_init();
    }

    ~library()
    {
        std::unique_lock<std::mutex> lock(_lib_mutex);
        _syms.clear();
        _vmas.clear();
        //No couterparty to bfd_init()
    }

    // This resolver could return stale data with the following scenario:
    // - program loads module dynamically with libdl(3lib) API
    // - check call stack with functions from module on the stack
    // - program unloads module -> stale data in the _vmas map
    // - program loads module again at new base address
    // - check call stack with functions from module on the stack find stale vma
    // Same issue is possible with cached symbol data in _syms.
    // For such situations, use bfd::bfd_lib_type::instance().flush_cached_data();
    void flush_cached_data()
    {
        std::unique_lock<std::mutex> lock(_lib_mutex);
        _syms.clear();
        _vmas.clear();
    }

    void resolve(const address_type&   addr,
                 const char *   binfile,
                 const char **  source_file_name,
                 const char **  func_name,
                 unsigned int * line_number)
    {
        std::unique_lock<std::mutex> lock(_lib_mutex);

        *source_file_name = nullptr;
        *line_number      = 0;
        *func_name        = nullptr;

        if (!binfile || !*binfile) {
            return;
        }

        sym_map_type::iterator itBfd = _syms.find(binfile);
        if (itBfd == _syms.end()) {
            sym_tab_type fbfd = {0};

            fbfd.base = _compute_maps_base(binfile);

            //FIXME: char *find_separate_debug_file (bfd *abfd);
            fbfd.abfd = std::shared_ptr< cbfd >(bfd_openr(binfile, 0),
                                                [] (cbfd* abfd) {
                                                    bool ret = bfd_close(abfd);
                                                    ret = ret; //compiler warning off
                                                });
            if (!fbfd.abfd) {
                return;
            }
#ifdef BFD_DECOMPRESS // Old libbfd?
            fbfd.abfd->flags |= BFD_DECOMPRESS;
#endif

            // Required
            if ( ! bfd_check_format(fbfd.abfd.get(), bfd_object)) {
                return;
            }

            fbfd.text = bfd_get_section_by_name(fbfd.abfd.get(), ".text");
            if ( ! fbfd.text) {
                return;
            }

            fbfd.storage_needed = bfd_get_symtab_upper_bound(fbfd.abfd.get());
            if (0 == fbfd.storage_needed) {
                fbfd.dynamic = true;
                fbfd.storage_needed = bfd_get_dynamic_symtab_upper_bound(fbfd.abfd.get());
            }

            fbfd.syms = std::shared_ptr< asymbol * >((asymbol **) ::malloc(fbfd.storage_needed),
                                                     [] (asymbol ** syms) {
                                                         ::free(syms);
                                                     });

            if (fbfd.dynamic) {
                fbfd.cSymbols = bfd_canonicalize_dynamic_symtab(fbfd.abfd.get(), fbfd.syms.get());
            }
            else {
                fbfd.cSymbols = bfd_canonicalize_symtab(fbfd.abfd.get(), fbfd.syms.get());
            }
            assert(fbfd.cSymbols >= 0);

            _syms[binfile] = fbfd;
            itBfd = _syms.find(binfile);
        }

        if (itBfd != _syms.end()) {
            const sym_tab_type &stab = itBfd->second;

            bfd_vma vma = bfd_get_section_vma_wrapper(stab.abfd.get(), stab.text);
            
            long offset = ((long)addr) - stab.base - vma; //stab.text->vma;
            if (offset > 0) {
                bool found = bfd_find_nearest_line(stab.abfd.get(),
                                                   stab.text,
                                                   stab.syms.get(),
                                                   offset,
                                                   source_file_name,
                                                   func_name,
                                                   line_number);
                if ( ! found) {
                    //std::cerr << "libbfd: could not find " << std::hex << addr;
                    *source_file_name = nullptr;
                    *line_number      = 0;
                    *func_name        = nullptr;
                }
                //trace it
            }
        }
    }

    // Ubuntu 18.04 with PIE-compiled programs
    bool _is_position_independent_executable(std::string const& executable_file_path)
    {
        static bool s_first_time = true;
        static std::string s_executable_file_path;

        if (!s_first_time)
        {
            assert(s_executable_file_path == executable_file_path);
            return _is_pie;
        }

        s_first_time = false;
        s_executable_file_path = executable_file_path;

        std::ifstream executable_file;
        executable_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        executable_file.open(executable_file_path.c_str());

        Elf64_Ehdr elf_header;
        executable_file.read(reinterpret_cast<char*>(&elf_header), sizeof(elf_header));
        // check if it is really an elf file
        if (::memcmp(elf_header.e_ident, ELFMAG, SELFMAG) != 0)
        {
            throw std::runtime_error(
                "Unsupported non-ELF format found for the executable '" + executable_file_path + "'");
        }

        _is_pie = (elf_header.e_type == ET_DYN);
        return _is_pie;
    }

    // Must run under _lib_mutex because of _vmas
    bfd_vma _compute_maps_base(const char * binfile)
    {
        std::string executable_file_path =
            std::filesystem::canonical(binfile);

        bfd_vma_map_type::const_iterator itVma = _vmas.find(executable_file_path);
        if (itVma != _vmas.end())
        {
            return itVma->second;
        }

        pid_t pid = ::getpid();
        std::string procFile = "/proc/" + std::to_string(pid) + "/maps";
        std::ifstream proc_maps_file(procFile.c_str());
        std::string line;

        int line_no = 0;
        while (std::getline(proc_maps_file, line))
        {
            if (ends_with(line, executable_file_path))
            {
                // this is the executable itself and we don't have a position independent executable
                if (line_no == 0 && !_is_position_independent_executable(executable_file_path))
                {
                    _vmas[executable_file_path] = 0;
                    return 0;
                }

                size_t const loc = line.find("-");
                if (loc == std::string::npos)
                {
                    _vmas[executable_file_path] = 0;
                    return 0;
                }

                std::string const base = "0x" + line.substr(0, loc);
                std::stringstream sstream(base);
                sstream.imbue(std::locale::classic());
                bfd_vma base_decimal = 0;
                sstream >> std::hex >> base_decimal;
                _vmas[executable_file_path] = base_decimal;
                return base_decimal;
            }
            line_no++;
        }
        return 0;
    }

private:

    std::mutex       _lib_mutex;
    sym_map_type     _syms;
    bool             _is_pie; // Ubuntu 18.04 with PIE-compiled programs
    bfd_vma_map_type _vmas;
};

typedef lpt::singleton<bfd::library> bfd_lib_type;


class source_resolver
{
public:

    const char*  function_name() const { return _func_name; }
    const char*  source_file() const   { return _source_file_name; }
    unsigned int line_number() const   { return  _line_number; }

    source_resolver(const address_type& addr, const char * binfile)
        : _source_file_name(nullptr)
        , _func_name(nullptr)
        , _line_number(0)
    {
        resolve(addr, binfile);
    }

    friend void swap(source_resolver& left, source_resolver& right)
    {
        std::swap(left._source_file_name, right._source_file_name);
        std::swap(left._func_name,        right._func_name);
        std::swap(left._line_number,      right._line_number);
    }

protected:

    void resolve(const address_type& addr, const char * binfile)
    {
        return bfd::bfd_lib_type::instance().resolve(addr, binfile, &_source_file_name, &_func_name, &_line_number);
    }

protected:

    const char*    _source_file_name;
    const char*    _func_name;
    unsigned int   _line_number;
};

} //namespace bfd

class extended_symbol_info : public basic_symbol_info
{
public:

    typedef basic_symbol_info  base_type;

    extended_symbol_info(const address_type& addr = nullptr) : base_type(addr)
    {
        resolve(addr);
    }

    extended_symbol_info(extended_symbol_info const& other) noexcept : extended_symbol_info(other._addr) {}
    extended_symbol_info& operator=(extended_symbol_info other) noexcept
    {
        swap(other);
        return *this;
    }

    extended_symbol_info(extended_symbol_info&& other) noexcept : extended_symbol_info()
    {
        swap(other);
    }
    extended_symbol_info& operator=(extended_symbol_info&& other) noexcept
    {
        assert(this != &other);
        swap(other);
        return *this;
    }


    void resolve(const address_type& addr)
    {
        base_type::resolve(addr);
        //assert(_binary_name != nullptr);
        _resolve(addr, _binary_name);
    }

    void swap(extended_symbol_info& other) noexcept
    {
        base_type::swap(other);
    }

protected:

    void _resolve(const address_type& addr, const char * binfile)
    {
        //assert(binfile);
        detail::bfd::source_resolver sym(addr, binfile);
        _line_number        = sym.line_number();
        _source_file_name   = sym.source_file();
        if (!_func_name && sym.function_name()) {
            _func_name = sym.function_name();
            _demangle();
        }
    }
}; //extended_symbol_info

inline void swap(extended_symbol_info& left, extended_symbol_info& right) noexcept
{
    assert(&left != &right);
    left.swap(right);
}


}}} //namespace lpt::stack::detail

