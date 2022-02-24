// https://github.com/kunitoki/LuaBridge
// Copyright 2020, Lucio Asnaghi
// SPDX-License-Identifier: MIT

#pragma once

#include <array>
#include <cassert>
#include <exception>
#include <functional>
#include <iostream>
#include <list>
#include <map>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <typeinfo>
#include <unordered_map>
#include <utility>
#include <vector>

// Begin File: Source/LuaBridge/detail/Config.h

// https://github.com/kunitoki/LuaBridge
// Copyright 2020, Lucio Asnaghi
// Copyright 2020, Dmitry Tarakanov
// Copyright 2019, George Tokmaji
// SPDX-License-Identifier: MIT


#if !(__cplusplus >= 201703L || (defined(_MSC_VER) && _HAS_CXX17))
#error LuaBridge 3.0 requires a compliant C++17 compiler, or C++17 has not been enabled !
#endif

// End File: Source/LuaBridge/detail/Config.h

// Begin File: Source/LuaBridge/detail/LuaHelpers.h

// https://github.com/kunitoki/LuaBridge
// Copyright 2020, Lucio Asnaghi
// Copyright 2012, Vinnie Falco <vinnie.falco@gmail.com>
// Copyright 2007, Nathan Reed
// SPDX-License-Identifier: MIT




namespace luabridge {

// These are for Lua versions prior to 5.2.0.

#if LUA_VERSION_NUM < 502
inline int lua_absindex(lua_State* L, int idx)
{
    if (idx > LUA_REGISTRYINDEX && idx < 0)
        return lua_gettop(L) + idx + 1;
    else
        return idx;
}

inline void lua_rawgetp(lua_State* L, int idx, void const* p)
{
    idx = lua_absindex(L, idx);
    lua_pushlightuserdata(L, const_cast<void*>(p));
    lua_rawget(L, idx);
}

inline void lua_rawsetp(lua_State* L, int idx, void const* p)
{
    idx = lua_absindex(L, idx);
    lua_pushlightuserdata(L, const_cast<void*>(p));
    // put key behind value
    lua_insert(L, -2);
    lua_rawset(L, idx);
}

#define LUA_OPEQ 1
#define LUA_OPLT 2
#define LUA_OPLE 3

inline int lua_compare(lua_State* L, int idx1, int idx2, int op)
{
    switch (op)
    {
    case LUA_OPEQ:
        return lua_equal(L, idx1, idx2);
        break;

    case LUA_OPLT:
        return lua_lessthan(L, idx1, idx2);
        break;

    case LUA_OPLE:
        return lua_equal(L, idx1, idx2) || lua_lessthan(L, idx1, idx2);
        break;

    default:
        return 0;
    };
}

inline int get_length(lua_State* L, int idx)
{
    return int(lua_objlen(L, idx));
}

#else
inline int get_length(lua_State* L, int idx)
{
    lua_len(L, idx);
    int len = int(luaL_checknumber(L, -1));
    lua_pop(L, 1);
    return len;
}

#endif

#ifndef LUA_OK
#define LUABRIDGE_LUA_OK 0
#else
#define LUABRIDGE_LUA_OK LUA_OK
#endif

/**
 * @brief Get a table value, bypassing metamethods.
 */
inline void rawgetfield(lua_State* L, int index, char const* key)
{
    assert(lua_istable(L, index));
    index = lua_absindex(L, index);
    lua_pushstring(L, key);
    lua_rawget(L, index);
}

/**
 * @brief Set a table value, bypassing metamethods.
 */
inline void rawsetfield(lua_State* L, int index, char const* key)
{
    assert(lua_istable(L, index));
    index = lua_absindex(L, index);
    lua_pushstring(L, key);
    lua_insert(L, -2);
    lua_rawset(L, index);
}

/**
 * @brief Returns true if the value is a full userdata (not light).
 */
inline bool isfulluserdata(lua_State* L, int index)
{
    return lua_isuserdata(L, index) && !lua_islightuserdata(L, index);
}

/**
 * @brief Test lua_State objects for global equality.
 *
 * This can determine if two different lua_State objects really point
 * to the same global state, such as when using coroutines.
 * 
 * @note This is used for assertions.
 */
inline bool equalstates(lua_State* L1, lua_State* L2)
{
    return lua_topointer(L1, LUA_REGISTRYINDEX) == lua_topointer(L2, LUA_REGISTRYINDEX);
}

/**
 * @brief Return an aligned pointer of type T.
 */
template <class T>
T* align(void* ptr) noexcept
{
    auto address = reinterpret_cast<size_t>(ptr);

    auto offset = address % alignof(T);
    auto aligned_address = (offset == 0) ? address : (address + alignof(T) - offset);

    return reinterpret_cast<T*>(aligned_address);
}

/**
 * @brief Return the space needed to align the type T on an unaligned address.
 */
template <class T>
size_t maximum_space_needed_to_align() noexcept
{
    return sizeof(T) + alignof(T) - 1;
}

/**
 * @brief Allocate lua userdata taking into account alignment.
 *
 * Using this instead of lua_newuserdata directly prevents alignment warnings on 64bits platforms.
 */
template <class T, class... Args>
void* lua_newuserdata_aligned(lua_State* L, Args&&... args)
{
    void* pointer = lua_newuserdata(L, maximum_space_needed_to_align<T>());
    T* aligned = align<T>(pointer);
    new (aligned) T(std::forward<Args>(args)...);
    return pointer;
}

} // namespace luabridge

// End File: Source/LuaBridge/detail/LuaHelpers.h

// Begin File: Source/LuaBridge/detail/ClassInfo.h

// https://github.com/kunitoki/LuaBridge
// Copyright 2020, Dmitry Tarakanov
// Copyright 2012, Vinnie Falco <vinnie.falco@gmail.com>
// SPDX-License-Identifier: MIT



namespace luabridge {
namespace detail {

/**
 * A unique key for a type name in a metatable.
 */
inline const void* getTypeKey()
{
#ifdef _NDEBUG
    static char value;
    return &value;
#else
    return reinterpret_cast<void*>(0x71);
#endif
}

/**
 * The key of a const table in another metatable.
 */
inline const void* getConstKey()
{
#ifdef _NDEBUG
    static char value;
    return &value;
#else
    return reinterpret_cast<void*>(0xc07);
#endif
}

/**
 * The key of a class table in another metatable.
 */
inline const void* getClassKey()
{
#ifdef _NDEBUG
    static char value;
    return &value;
#else
    return reinterpret_cast<void*>(0xc1a);
#endif
}

/**
 * The key of a propget table in another metatable.
 */
inline const void* getPropgetKey()
{
#ifdef _NDEBUG
    static char value;
    return &value;
#else
    return reinterpret_cast<void*>(0x6e7);
#endif
}

/**
 * The key of a propset table in another metatable.
 */
inline const void* getPropsetKey()
{
#ifdef _NDEBUG
    static char value;
    return &value;
#else
    return reinterpret_cast<void*>(0x5e7);
#endif
}

/**
 * The key of a static table in another metatable.
 */
inline const void* getStaticKey()
{
#ifdef _NDEBUG
    static char value;
    return &value;
#else
    return reinterpret_cast<void*>(0x57a);
#endif
}

/**
 * The key of a parent table in another metatable.
 */
inline const void* getParentKey()
{
#ifdef _NDEBUG
    static char value;
    return &value;
#else
    return reinterpret_cast<void*>(0xdad);
#endif
}

/**
    Get the key for the static table in the Lua registry.
    The static table holds the static data members, static properties, and
    static member functions for a class.
*/
template<class T>
void const* getStaticRegistryKey()
{
    static char value;
    return &value;
}

/** Get the key for the class table in the Lua registry.
    The class table holds the data members, properties, and member functions
    of a class. Read-only data and properties, and const member functions are
    also placed here (to save a lookup in the const table).
*/
template<class T>
void const* getClassRegistryKey()
{
    static char value;
    return &value;
}

/** Get the key for the const table in the Lua registry.
    The const table holds read-only data members and properties, and const
    member functions of a class.
*/
template<class T>
void const* getConstRegistryKey()
{
    static char value;
    return &value;
}

} // namespace detail

} // namespace luabridge

// End File: Source/LuaBridge/detail/ClassInfo.h

// Begin File: Source/LuaBridge/detail/TypeTraits.h

// https://github.com/kunitoki/LuaBridge
// Copyright 2020, Lucio Asnaghi
// Copyright 2019, Dmitry Tarakanov
// Copyright 2012, Vinnie Falco <vinnie.falco@gmail.com>
// SPDX-License-Identifier: MIT




namespace luabridge {

//------------------------------------------------------------------------------
/**
    Container traits.

    Unspecialized ContainerTraits has the isNotContainer typedef for SFINAE.
    All user defined containers must supply an appropriate specialization for
    ContinerTraits (without the typedef isNotContainer). The containers that
    come with LuaBridge also come with the appropriate ContainerTraits
    specialization. See the corresponding declaration for details.

    A specialization of ContainerTraits for some generic type ContainerType
    looks like this:

        template <class T>
        struct ContainerTraits <ContainerType <T>>
        {
          typedef typename T Type;

          static T* get (ContainerType <T> const& c)
          {
            return c.get (); // Implementation-dependent on ContainerType
          }
        };
*/
template<class T>
struct ContainerTraits
{
    typedef bool isNotContainer;
    typedef T Type;
};

namespace detail {

//------------------------------------------------------------------------------
/**
    Type traits.

    Specializations return information about a type.
*/
struct TypeTraits
{
    /** Determine if type T is a container.

        To be considered a container, there must be a specialization of
        ContainerTraits with the required fields.
    */
    template<typename T>
    class isContainer
    {
    private:
        typedef char yes[1]; // sizeof (yes) == 1
        typedef char no[2]; // sizeof (no)  == 2

        template<typename C>
        static no& test(typename C::isNotContainer*);

        template<typename>
        static yes& test(...);

    public:
        static const bool value = sizeof(test<ContainerTraits<T>>(0)) == sizeof(yes);
    };

    /** Determine if T is const qualified.
     */
    /** @{ */
    template<class T>
    struct isConst
    {
        static bool const value = false;
    };

    template<class T>
    struct isConst<T const>
    {
        static bool const value = true;
    };
    /** @} */

    /** Remove the const qualifier from T.
     */
    /** @{ */
    template<class T>
    struct removeConst
    {
        typedef T Type;
    };

    template<class T>
    struct removeConst<T const>
    {
        typedef T Type;
    };
    /**@}*/
};

} // namespace detail

} // namespace luabridge

// End File: Source/LuaBridge/detail/TypeTraits.h

// Begin File: Source/LuaBridge/detail/Userdata.h

// https://github.com/kunitoki/LuaBridge
// Copyright 2020, Lucio Asnaghi
// Copyright 2019, Dmitry Tarakanov
// Copyright 2012, Vinnie Falco <vinnie.falco@gmail.com>
// SPDX-License-Identifier: MIT




namespace luabridge {
namespace detail {

//==============================================================================
/**
  Return the identity pointer for our lightuserdata tokens.

  Because of Lua's dynamic typing and our improvised system of imposing C++
  class structure, there is the possibility that executing scripts may
  knowingly or unknowingly cause invalid data to get passed to the C functions
  created by LuaBridge. In particular, our security model addresses the
  following:
    1. Scripts cannot create a userdata (ignoring the debug lib).
    2. Scripts cannot create a lightuserdata (ignoring the debug lib).
    3. Scripts cannot set the metatable on a userdata.
*/

/**
  Interface to a class pointer retrievable from a userdata.
*/
class Userdata
{
protected:
    void* m_p = nullptr; // subclasses must set this

    Userdata() {}

    //--------------------------------------------------------------------------
    /**
      Get an untyped pointer to the contained class.
    */
    void* getPointer() { return m_p; }

private:
    //--------------------------------------------------------------------------
    /**
      Validate and retrieve a Userdata on the stack.

      The Userdata must exactly match the corresponding class table or
      const table, or else a Lua error is raised. This is used for the
      __gc metamethod.
    */
    static Userdata* getExactClass(lua_State* L, int index, void const* /*classKey*/)
    {
        return static_cast<Userdata*>(lua_touserdata(L, lua_absindex(L, index)));
    }

    //--------------------------------------------------------------------------
    /**
      Validate and retrieve a Userdata on the stack.

      The Userdata must be derived from or the same as the given base class,
      identified by the key. If canBeConst is false, generates an error if
      the resulting Userdata represents to a const object. We do the type check
      first so that the error message is informative.
    */
    static Userdata* getClass(lua_State* L,
                              int index,
                              void const* registryConstKey,
                              void const* registryClassKey,
                              bool canBeConst)
    {
        index = lua_absindex(L, index);

        lua_getmetatable(L, index); // Stack: object metatable (ot) | nil
        if (!lua_istable(L, -1))
        {
            lua_rawgetp(L, LUA_REGISTRYINDEX, registryClassKey); // Stack: registry metatable (rt) | nil
            return throwBadArg(L, index);
        }

        lua_rawgetp(L, -1, getConstKey()); // Stack: ot | nil, const table (co) | nil
        assert(lua_istable(L, -1) || lua_isnil(L, -1));

        // If const table is NOT present, object is const. Use non-const registry table
        // if object cannot be const, so constness validation is done automatically.
        // E.g. nonConstFn (constObj)
        // -> canBeConst = false, isConst = true
        // -> 'Class' registry table, 'const Class' object table
        // -> 'expected Class, got const Class'
        bool isConst = lua_isnil(L, -1); // Stack: ot | nil, nil, rt
        if (isConst && canBeConst)
        {
            lua_rawgetp(L, LUA_REGISTRYINDEX, registryConstKey); // Stack: ot, nil, rt
        }
        else
        {
            lua_rawgetp(L, LUA_REGISTRYINDEX, registryClassKey); // Stack: ot, co, rt
        }

        lua_insert(L, -3); // Stack: rt, ot, co | nil
        lua_pop(L, 1); // Stack: rt, ot

        for (;;)
        {
            if (lua_rawequal(L, -1, -2)) // Stack: rt, ot
            {
                lua_pop(L, 2); // Stack: -
                return static_cast<Userdata*>(lua_touserdata(L, index));
            }

            // Replace current metatable with it's base class.
            lua_rawgetp(L, -1, getParentKey()); // Stack: rt, ot, parent ot (pot) | nil

            if (lua_isnil(L, -1)) // Stack: rt, ot, nil
            {
                // Drop the object metatable because it may be some parent metatable
                lua_pop(L, 2); // Stack: rt
                return throwBadArg(L, index);
            }

            lua_remove(L, -2); // Stack: rt, pot
        }

        // no return
    }

    static bool isInstance(lua_State* L, int index, void const* registryClassKey)
    {
        index = lua_absindex(L, index);

        int result = lua_getmetatable(L, index); // Stack: object metatable (ot) | nothing
        if (result == 0)
            return false; // Nothing was pushed on the stack

        if (!lua_istable(L, -1))
        {
            lua_pop(L, 1); // Stack: -
            return false;
        }

        lua_rawgetp(L, LUA_REGISTRYINDEX, registryClassKey); // Stack: ot, rt
        lua_insert(L, -2); // Stack: rt, ot

        for (;;)
        {
            if (lua_rawequal(L, -1, -2)) // Stack: rt, ot
            {
                lua_pop(L, 2); // Stack: -
                return true;
            }

            // Replace current metatable with it's base class.
            lua_rawgetp(L, -1, getParentKey()); // Stack: rt, ot, parent ot (pot) | nil

            if (lua_isnil(L, -1)) // Stack: rt, ot, nil
            {
                lua_pop(L, 3); // Stack: -
                return false;
            }

            lua_remove(L, -2); // Stack: rt, pot
        }
    }

    static Userdata* throwBadArg(lua_State* L, int index)
    {
        assert(lua_istable(L, -1) || lua_isnil(L, -1)); // Stack: rt | nil

        const char* expected = 0;
        if (lua_isnil(L, -1)) // Stack: nil
        {
            expected = "unregistered class";
        }
        else
        {
            lua_rawgetp(L, -1, getTypeKey()); // Stack: rt, registry type
            expected = lua_tostring(L, -1);
        }

        const char* got = 0;
        if (lua_isuserdata(L, index))
        {
            lua_getmetatable(L, index); // Stack: ..., ot | nil
            if (lua_istable(L, -1)) // Stack: ..., ot
            {
                lua_rawgetp(L, -1, getTypeKey()); // Stack: ..., ot, object type | nil
                if (lua_isstring(L, -1))
                {
                    got = lua_tostring(L, -1);
                }
            }
        }

        if (!got)
        {
            got = lua_typename(L, lua_type(L, index));
        }

        luaL_argerror(L, index, lua_pushfstring(L, "%s expected, got %s", expected, got));
        return 0;
    }

public:
    virtual ~Userdata() {}

    //--------------------------------------------------------------------------
    /**
      Returns the Userdata* if the class on the Lua stack matches.
      If the class does not match, a Lua error is raised.

      @tparam T     A registered user class.
      @param  L     A Lua state.
      @param  index The index of an item on the Lua stack.
      @returns A userdata pointer if the class matches.
    */
    template<class T>
    static Userdata* getExact(lua_State* L, int index)
    {
        return getExactClass(L, index, detail::getClassRegistryKey<T>());
    }

    //--------------------------------------------------------------------------
    /**
      Get a pointer to the class from the Lua stack.
      If the object is not the class or a subclass, or it violates the
      const-ness, a Lua error is raised.

      @tparam T          A registered user class.
      @param  L          A Lua state.
      @param  index      The index of an item on the Lua stack.
      @param  canBeConst TBD
      @returns A pointer if the class and constness match.
    */
    template<class T>
    static T* get(lua_State* L, int index, bool canBeConst)
    {
        if (lua_isnil(L, index))
        {
            luaL_error(L, "argument %d is nil", index - 1);
            return 0;
        }

        return static_cast<T*>(getClass(L,
                                        index,
                                        detail::getConstRegistryKey<T>(),
                                        detail::getClassRegistryKey<T>(),
                                        canBeConst)
                                   ->getPointer());
    }

    template<class T>
    static bool isInstance(lua_State* L, int index)
    {
        return isInstance(L, index, detail::getClassRegistryKey<T>());
    }
};

//----------------------------------------------------------------------------
/**
  Wraps a class object stored in a Lua userdata.

  The lifetime of the object is managed by Lua. The object is constructed
  inside the userdata using placement new.
*/
template<class T>
class UserdataValue : public Userdata
{
private:
    UserdataValue<T>(UserdataValue<T> const&);
    UserdataValue<T> operator=(UserdataValue<T> const&);

    char m_storage[sizeof(T)];

private:
    /**
      Used for placement construction.
    */
    UserdataValue() { m_p = 0; }

    ~UserdataValue()
    {
        if (getPointer() != 0)
        {
            getObject()->~T();
        }
    }

public:
    /**
      Push a T via placement new.

      The caller is responsible for calling placement new using the
      returned uninitialized storage.

      @param L A Lua state.
      @returns An object referring to the newly created userdata value.
    */
    static UserdataValue<T>* place(lua_State* const L)
    {
        UserdataValue<T>* const ud = new (lua_newuserdata(L, sizeof(UserdataValue<T>))) UserdataValue<T>();

        lua_rawgetp(L, LUA_REGISTRYINDEX, detail::getClassRegistryKey<T>());

        if (!lua_istable(L, -1))
            throw std::logic_error("The class is not registered in LuaBridge");

        lua_setmetatable(L, -2);

        return ud;
    }

    /**
      Push T via copy construction from U.

      @tparam U A container type.
      @param  L A Lua state.
      @param  u A container object reference.
    */
    template<class U>
    static inline void push(lua_State* const L, U const& u)
    {
        UserdataValue<T>* ud = place(L);
        new (ud->getObject()) U(u);
        ud->commit();
    }

    /**
      Confirm object construction.
    */
    void commit()
    {
        m_p = getObject();
    }

    T* getObject()
    {
        // If this fails to compile it means you forgot to provide
        // a Container specialization for your container!
        return reinterpret_cast<T*>(&m_storage[0]);
    }
};

//----------------------------------------------------------------------------
/**
  Wraps a pointer to a class object inside a Lua userdata.

  The lifetime of the object is managed by C++.
*/
class UserdataPtr : public Userdata
{
private:
    UserdataPtr(UserdataPtr const&);
    UserdataPtr operator=(UserdataPtr const&);

private:
    /** Push a pointer to object using metatable key.
     */
    static void push(lua_State* L, const void* p, void const* const key)
    {
        new (lua_newuserdata(L, sizeof(UserdataPtr))) UserdataPtr(const_cast<void*>(p));
        lua_rawgetp(L, LUA_REGISTRYINDEX, key);

        if (!lua_istable(L, -1))
        {
            lua_pop(L, 1); // possibly: a nil
            throw std::logic_error("The class is not registered in LuaBridge");
        }

        lua_setmetatable(L, -2);
    }

    explicit UserdataPtr(void* const p)
    {
        // Can't construct with a null pointer!
        assert(p != nullptr);

        m_p = p;
    }

public:
    /** Push non-const pointer to object.

      @tparam T A user registered class.
      @param  L A Lua state.
      @param  p A pointer to the user class instance.
    */
    template<class T>
    static void push(lua_State* const L, T* const p)
    {
        if (p)
            push(L, p, getClassRegistryKey<T>());
        else
            lua_pushnil(L);
    }

    /** Push const pointer to object.

      @tparam T A user registered class.
      @param  L A Lua state.
      @param  p A pointer to the user class instance.
    */
    template<class T>
    static void push(lua_State* const L, T const* const p)
    {
        if (p)
            push(L, p, getConstRegistryKey<T>());
        else
            lua_pushnil(L);
    }
};

//============================================================================
/**
  Wraps a container that references a class object.

  The template argument C is the container type, ContainerTraits must be
  specialized on C or else a compile error will result.
*/
template<class C>
class UserdataShared : public Userdata
{
private:
    UserdataShared(UserdataShared<C> const&);
    UserdataShared<C>& operator=(UserdataShared<C> const&);

    typedef typename TypeTraits::removeConst<typename ContainerTraits<C>::Type>::Type T;

    C m_c;

private:
    ~UserdataShared() {}

public:
    /**
      Construct from a container to the class or a derived class.

      @tparam U A container type.
      @param  u A container object reference.
    */
    template<class U>
    explicit UserdataShared(U const& u) : m_c(u)
    {
        m_p = const_cast<void*>(reinterpret_cast<void const*>((ContainerTraits<C>::get(m_c))));
    }

    /**
      Construct from a pointer to the class or a derived class.

      @tparam U A container type.
      @param  u A container object pointer.
    */
    template<class U>
    explicit UserdataShared(U* u) : m_c(u)
    {
        m_p = const_cast<void*>(reinterpret_cast<void const*>((ContainerTraits<C>::get(m_c))));
    }
};

//----------------------------------------------------------------------------
//
// SFINAE helpers.
//

// non-const objects
template<class C, bool makeObjectConst>
struct UserdataSharedHelper
{
    typedef typename TypeTraits::removeConst<typename ContainerTraits<C>::Type>::Type T;

    static void push(lua_State* L, C const& c)
    {
        if (ContainerTraits<C>::get(c) != 0)
        {
            new (lua_newuserdata(L, sizeof(UserdataShared<C>))) UserdataShared<C>(c);
            lua_rawgetp(L, LUA_REGISTRYINDEX, getClassRegistryKey<T>());
            // If this goes off it means the class T is unregistered!
            assert(lua_istable(L, -1));
            lua_setmetatable(L, -2);
        }
        else
        {
            lua_pushnil(L);
        }
    }

    static void push(lua_State* L, T* const t)
    {
        if (t)
        {
            new (lua_newuserdata(L, sizeof(UserdataShared<C>))) UserdataShared<C>(t);
            lua_rawgetp(L, LUA_REGISTRYINDEX, getClassRegistryKey<T>());
            // If this goes off it means the class T is unregistered!
            assert(lua_istable(L, -1));
            lua_setmetatable(L, -2);
        }
        else
        {
            lua_pushnil(L);
        }
    }
};

// const objects
template<class C>
struct UserdataSharedHelper<C, true>
{
    typedef typename TypeTraits::removeConst<typename ContainerTraits<C>::Type>::Type T;

    static void push(lua_State* L, C const& c)
    {
        if (ContainerTraits<C>::get(c) != 0)
        {
            new (lua_newuserdata(L, sizeof(UserdataShared<C>))) UserdataShared<C>(c);
            lua_rawgetp(L, LUA_REGISTRYINDEX, getConstRegistryKey<T>());
            // If this goes off it means the class T is unregistered!
            assert(lua_istable(L, -1));
            lua_setmetatable(L, -2);
        }
        else
        {
            lua_pushnil(L);
        }
    }

    static void push(lua_State* L, T* const t)
    {
        if (t)
        {
            new (lua_newuserdata(L, sizeof(UserdataShared<C>))) UserdataShared<C>(t);
            lua_rawgetp(L, LUA_REGISTRYINDEX, getConstRegistryKey<T>());
            // If this goes off it means the class T is unregistered!
            assert(lua_istable(L, -1));
            lua_setmetatable(L, -2);
        }
        else
        {
            lua_pushnil(L);
        }
    }
};

/**
  Pass by container.

  The container controls the object lifetime. Typically this will be a
  lifetime shared by C++ and Lua using a reference count. Because of type
  erasure, containers like std::shared_ptr will not work. Containers must
  either be of the intrusive variety, or in the style of the RefCountedPtr
  type provided by LuaBridge (that uses a global hash table).
*/
template<class C, bool byContainer>
struct StackHelper
{
    static void push(lua_State* L, C const& c)
    {
        UserdataSharedHelper<C, TypeTraits::isConst<typename ContainerTraits<C>::Type>::value>::
            push(L, c);
    }

    typedef typename TypeTraits::removeConst<typename ContainerTraits<C>::Type>::Type T;

    static C get(lua_State* L, int index) { return Userdata::get<T>(L, index, true); }
};

/**
  Pass by value.

  Lifetime is managed by Lua. A C++ function which accesses a pointer or
  reference to an object outside the activation record in which it was
  retrieved may result in undefined behavior if Lua garbage collected it.
*/
template<class T>
struct StackHelper<T, false>
{
    static inline void push(lua_State* L, T const& t) { UserdataValue<T>::push(L, t); }

    static inline T const& get(lua_State* L, int index)
    {
        return *Userdata::get<T>(L, index, true);
    }
};

//------------------------------------------------------------------------------
/**
  Lua stack conversions for pointers and references to class objects.

  Lifetime is managed by C++. Lua code which remembers a reference to the
  value may result in undefined behavior if C++ destroys the object. The
  handling of the const and volatile qualifiers happens in UserdataPtr.
*/

template<class C, bool byContainer>
struct RefStackHelper
{
    typedef C return_type;

    static inline void push(lua_State* L, C const& t)
    {
        UserdataSharedHelper<C, TypeTraits::isConst<typename ContainerTraits<C>::Type>::value>::
            push(L, t);
    }

    typedef typename TypeTraits::removeConst<typename ContainerTraits<C>::Type>::Type T;

    static return_type get(lua_State* L, int index) { return Userdata::get<T>(L, index, true); }
};

template<class T>
struct RefStackHelper<T, false>
{
    typedef T& return_type;

    static void push(lua_State* L, T const& t) { UserdataPtr::push(L, &t); }

    static return_type get(lua_State* L, int index)
    {
        T* t = Userdata::get<T>(L, index, true);

        if (!t)
            luaL_error(L, "nil passed to reference");

        return *t;
    }
};

/**
 * Voider class template. Used to force a comiler to instantiate
 * an otherwise probably unused template parameter type T.
 * See the C++20 std::void_t <> for details.
 */
template<class T>
struct Void
{
    typedef void Type;
};

/**
 * Trait class that selects whether to return a user registered
 * class object by value or by reference.
 */

template<class T, class Enabler = void>
struct UserdataGetter
{
    typedef T* ReturnType;

    static ReturnType get(lua_State* L, int index) { return Userdata::get<T>(L, index, false); }
};

template<class T>
struct UserdataGetter<T, typename Void<T (*)()>::Type>
{
    typedef T ReturnType;

    static ReturnType get(lua_State* L, int index)
    {
        return StackHelper<T, TypeTraits::isContainer<T>::value>::get(L, index);
    }
};

} // namespace detail

//==============================================================================

/**
  Lua stack conversions for class objects passed by value.
*/
template<class T>
struct Stack
{
    typedef void IsUserdata;

    typedef detail::UserdataGetter<T> Getter;
    typedef typename Getter::ReturnType ReturnType;

    static void push(lua_State* L, T const& value)
    {
        using namespace detail;
        StackHelper<T, TypeTraits::isContainer<T>::value>::push(L, value);
    }

    static ReturnType get(lua_State* L, int index) { return Getter::get(L, index); }

    static bool isInstance(lua_State* L, int index)
    {
        return detail::Userdata::isInstance<T>(L, index);
    }
};

namespace detail {

/**
 * Trait class indicating whether the parameter type must be
 * a user registered class. The trait checks the existence of
 * member type Stack::IsUserdata specialization for detection.
 */
template<class T, class Enable = void>
struct IsUserdata
{
    static const bool value = false;
};

template<class T>
struct IsUserdata<T, typename Void<typename Stack<T>::IsUserdata>::Type>
{
    static const bool value = true;
};

/**
 * Trait class that selects a specific push/get implemenation.
 */
template<class T, bool isUserdata>
struct StackOpSelector;

// pointer
template<class T>
struct StackOpSelector<T*, true>
{
    typedef T* ReturnType;

    static void push(lua_State* L, T* value) { UserdataPtr::push(L, value); }

    static T* get(lua_State* L, int index) { return Userdata::get<T>(L, index, false); }

    static bool isInstance(lua_State* L, int index) { return Userdata::isInstance<T>(L, index); }
};

// pointer to const
template<class T>
struct StackOpSelector<const T*, true>
{
    typedef const T* ReturnType;

    static void push(lua_State* L, const T* value) { UserdataPtr::push(L, value); }

    static const T* get(lua_State* L, int index) { return Userdata::get<T>(L, index, true); }

    static bool isInstance(lua_State* L, int index) { return Userdata::isInstance<T>(L, index); }
};

// reference
template<class T>
struct StackOpSelector<T&, true>
{
    typedef RefStackHelper<T, TypeTraits::isContainer<T>::value> Helper;
    typedef typename Helper::return_type ReturnType;

    static void push(lua_State* L, T& value) { UserdataPtr::push(L, &value); }

    static ReturnType get(lua_State* L, int index) { return Helper::get(L, index); }

    static bool isInstance(lua_State* L, int index) { return Userdata::isInstance<T>(L, index); }
};

// reference to const
template<class T>
struct StackOpSelector<const T&, true>
{
    typedef RefStackHelper<T, TypeTraits::isContainer<T>::value> Helper;
    typedef typename Helper::return_type ReturnType;

    static void push(lua_State* L, const T& value) { Helper::push(L, value); }

    static ReturnType get(lua_State* L, int index) { return Helper::get(L, index); }

    static bool isInstance(lua_State* L, int index) { return Userdata::isInstance<T>(L, index); }
};

} // namespace detail

} // namespace luabridge

// End File: Source/LuaBridge/detail/Userdata.h

// Begin File: Source/LuaBridge/detail/Stack.h

// https://github.com/kunitoki/LuaBridge
// Copyright 2020, Lucio Asnaghi
// Copyright 2019, Dmitry Tarakanov
// Copyright 2012, Vinnie Falco <vinnie.falco@gmail.com>
// Copyright 2007, Nathan Reed
// SPDX-License-Identifier: MIT




namespace luabridge {

/// Lua stack traits for C++ types.
///
/// @tparam T A C++ type.
///
template<class T>
struct Stack;

template<>
struct Stack<void>
{
    static void push(lua_State*) {}
};

//------------------------------------------------------------------------------
/**
    Receive the lua_State* as an argument.
*/
template<>
struct Stack<lua_State*>
{
    static lua_State* get(lua_State* L, int) { return L; }
};

//------------------------------------------------------------------------------
/**
    Stack specialization for a lua_CFunction.
*/
template<>
struct Stack<lua_CFunction>
{
    static void push(lua_State* L, lua_CFunction f) { lua_pushcfunction(L, f); }

    static lua_CFunction get(lua_State* L, int index) { return lua_tocfunction(L, index); }

    static bool isInstance(lua_State* L, int index) { return lua_iscfunction(L, index); }
};

//------------------------------------------------------------------------------
/**
    Stack specialization for `int`.
*/
template<>
struct Stack<int>
{
    static void push(lua_State* L, int value)
    {
        lua_pushinteger(L, static_cast<lua_Integer>(value));
    }

    static int get(lua_State* L, int index)
    {
        return static_cast<int>(luaL_checkinteger(L, index));
    }

    static bool isInstance(lua_State* L, int index) { return lua_type(L, index) == LUA_TNUMBER; }
};

//------------------------------------------------------------------------------
/**
    Stack specialization for `unsigned int`.
*/
template<>
struct Stack<unsigned int>
{
    static void push(lua_State* L, unsigned int value)
    {
        lua_pushinteger(L, static_cast<lua_Integer>(value));
    }

    static unsigned int get(lua_State* L, int index)
    {
        return static_cast<unsigned int>(luaL_checkinteger(L, index));
    }

    static bool isInstance(lua_State* L, int index) { return lua_type(L, index) == LUA_TNUMBER; }
};

//------------------------------------------------------------------------------
/**
    Stack specialization for `unsigned char`.
*/
template<>
struct Stack<unsigned char>
{
    static void push(lua_State* L, unsigned char value)
    {
        lua_pushinteger(L, static_cast<lua_Integer>(value));
    }

    static unsigned char get(lua_State* L, int index)
    {
        return static_cast<unsigned char>(luaL_checkinteger(L, index));
    }

    static bool isInstance(lua_State* L, int index) { return lua_type(L, index) == LUA_TNUMBER; }
};

//------------------------------------------------------------------------------
/**
    Stack specialization for `short`.
*/
template<>
struct Stack<short>
{
    static void push(lua_State* L, short value)
    {
        lua_pushinteger(L, static_cast<lua_Integer>(value));
    }

    static short get(lua_State* L, int index)
    {
        return static_cast<short>(luaL_checkinteger(L, index));
    }

    static bool isInstance(lua_State* L, int index) { return lua_type(L, index) == LUA_TNUMBER; }
};

//------------------------------------------------------------------------------
/**
    Stack specialization for `unsigned short`.
*/
template<>
struct Stack<unsigned short>
{
    static void push(lua_State* L, unsigned short value)
    {
        lua_pushinteger(L, static_cast<lua_Integer>(value));
    }

    static unsigned short get(lua_State* L, int index)
    {
        return static_cast<unsigned short>(luaL_checkinteger(L, index));
    }

    static bool isInstance(lua_State* L, int index) { return lua_type(L, index) == LUA_TNUMBER; }
};

//------------------------------------------------------------------------------
/**
    Stack specialization for `long`.
*/
template<>
struct Stack<long>
{
    static void push(lua_State* L, long value)
    {
        lua_pushinteger(L, static_cast<lua_Integer>(value));
    }

    static long get(lua_State* L, int index)
    {
        return static_cast<long>(luaL_checkinteger(L, index));
    }

    static bool isInstance(lua_State* L, int index) { return lua_type(L, index) == LUA_TNUMBER; }
};

//------------------------------------------------------------------------------
/**
    Stack specialization for `unsigned long`.
*/
template<>
struct Stack<unsigned long>
{
    static void push(lua_State* L, unsigned long value)
    {
        lua_pushinteger(L, static_cast<lua_Integer>(value));
    }

    static unsigned long get(lua_State* L, int index)
    {
        return static_cast<unsigned long>(luaL_checkinteger(L, index));
    }

    static bool isInstance(lua_State* L, int index) { return lua_type(L, index) == LUA_TNUMBER; }
};

//------------------------------------------------------------------------------
/**
 * Stack specialization for `long long`.
 */
template<>
struct Stack<long long>
{
    static void push(lua_State* L, long long value)
    {
        lua_pushinteger(L, static_cast<lua_Integer>(value));
    }

    static long long get(lua_State* L, int index)
    {
        return static_cast<long long>(luaL_checkinteger(L, index));
    }

    static bool isInstance(lua_State* L, int index) { return lua_type(L, index) == LUA_TNUMBER; }
};

//------------------------------------------------------------------------------
/**
 * Stack specialization for `unsigned long long`.
 */
template<>
struct Stack<unsigned long long>
{
    static void push(lua_State* L, unsigned long long value)
    {
        lua_pushinteger(L, static_cast<lua_Integer>(value));
    }
    static unsigned long long get(lua_State* L, int index)
    {
        return static_cast<unsigned long long>(luaL_checkinteger(L, index));
    }

    static bool isInstance(lua_State* L, int index) { return lua_type(L, index) == LUA_TNUMBER; }
};

//------------------------------------------------------------------------------
/**
    Stack specialization for `float`.
*/
template<>
struct Stack<float>
{
    static void push(lua_State* L, float value)
    {
        lua_pushnumber(L, static_cast<lua_Number>(value));
    }

    static float get(lua_State* L, int index)
    {
        return static_cast<float>(luaL_checknumber(L, index));
    }

    static bool isInstance(lua_State* L, int index) { return lua_type(L, index) == LUA_TNUMBER; }
};

//------------------------------------------------------------------------------
/**
    Stack specialization for `double`.
*/
template<>
struct Stack<double>
{
    static void push(lua_State* L, double value)
    {
        lua_pushnumber(L, static_cast<lua_Number>(value));
    }

    static double get(lua_State* L, int index)
    {
        return static_cast<double>(luaL_checknumber(L, index));
    }

    static bool isInstance(lua_State* L, int index) { return lua_type(L, index) == LUA_TNUMBER; }
};

//------------------------------------------------------------------------------
/**
    Stack specialization for `bool`.
*/
template<>
struct Stack<bool>
{
    static void push(lua_State* L, bool value) { lua_pushboolean(L, value ? 1 : 0); }

    static bool get(lua_State* L, int index) { return lua_toboolean(L, index) ? true : false; }

    static bool isInstance(lua_State* L, int index) { return lua_isboolean(L, index); }
};

//------------------------------------------------------------------------------
/**
    Stack specialization for `char`.
*/
template<>
struct Stack<char>
{
    static void push(lua_State* L, char value) { lua_pushlstring(L, &value, 1); }

    static char get(lua_State* L, int index) { return luaL_checkstring(L, index)[0]; }

    static bool isInstance(lua_State* L, int index) { return lua_type(L, index) == LUA_TSTRING; }
};

//------------------------------------------------------------------------------
/**
    Stack specialization for `const char*`.
*/
template<>
struct Stack<char const*>
{
    static void push(lua_State* L, char const* str)
    {
        if (str != 0)
            lua_pushstring(L, str);
        else
            lua_pushnil(L);
    }

    static char const* get(lua_State* L, int index)
    {
        return lua_isnil(L, index) ? 0 : luaL_checkstring(L, index);
    }

    static bool isInstance(lua_State* L, int index)
    {
        return lua_isnil(L, index) || lua_type(L, index) == LUA_TSTRING;
    }
};

//------------------------------------------------------------------------------
/**
    Stack specialization for `std::string`.
*/
template<>
struct Stack<std::string>
{
    static void push(lua_State* L, std::string const& str)
    {
        lua_pushlstring(L, str.data(), str.size());
    }

    static std::string get(lua_State* L, int index)
    {
        size_t len;
        if (lua_type(L, index) == LUA_TSTRING)
        {
            const char* str = lua_tolstring(L, index, &len);
            return std::string(str, len);
        }

        // Lua reference manual:
        // If the value is a number, then lua_tolstring also changes the actual value in the stack
        // to a string. (This change confuses lua_next when lua_tolstring is applied to keys during
        // a table traversal.)
        lua_pushvalue(L, index);
        const char* str = lua_tolstring(L, -1, &len);
        std::string string(str, len);
        lua_pop(L, 1); // Pop the temporary string
        return string;
    }

    static bool isInstance(lua_State* L, int index) { return lua_type(L, index) == LUA_TSTRING; }
};

namespace detail {

template<class T>
struct StackOpSelector<T&, false>
{
    typedef T ReturnType;

    static void push(lua_State* L, T& value) { Stack<T>::push(L, value); }

    static ReturnType get(lua_State* L, int index) { return Stack<T>::get(L, index); }

    static bool isInstance(lua_State* L, int index) { return Stack<T>::isInstance(L, index); }
};

template<class T>
struct StackOpSelector<const T&, false>
{
    typedef T ReturnType;

    static void push(lua_State* L, const T& value) { Stack<T>::push(L, value); }

    static ReturnType get(lua_State* L, int index) { return Stack<T>::get(L, index); }

    static bool isInstance(lua_State* L, int index) { return Stack<T>::isInstance(L, index); }
};

template<class T>
struct StackOpSelector<T*, false>
{
    typedef T ReturnType;

    static void push(lua_State* L, T* value) { Stack<T>::push(L, *value); }

    static ReturnType get(lua_State* L, int index) { return Stack<T>::get(L, index); }

    static bool isInstance(lua_State* L, int index) { return Stack<T>::isInstance(L, index); }
};

template<class T>
struct StackOpSelector<const T*, false>
{
    typedef T ReturnType;

    static void push(lua_State* L, const T* value) { Stack<T>::push(L, *value); }

    static ReturnType get(lua_State* L, int index) { return Stack<T>::get(L, index); }

    static bool isInstance(lua_State* L, int index) { return Stack<T>::isInstance(L, index); }
};

} // namespace detail

template<class T>
struct Stack<T&>
{
    typedef detail::StackOpSelector<T&, detail::IsUserdata<T>::value> Helper;
    typedef typename Helper::ReturnType ReturnType;

    static void push(lua_State* L, T& value) { Helper::push(L, value); }

    static ReturnType get(lua_State* L, int index) { return Helper::get(L, index); }
};

template<class T>
struct Stack<const T&>
{
    typedef detail::StackOpSelector<const T&, detail::IsUserdata<T>::value> Helper;
    typedef typename Helper::ReturnType ReturnType;

    static void push(lua_State* L, const T& value) { Helper::push(L, value); }

    static ReturnType get(lua_State* L, int index) { return Helper::get(L, index); }
};

template<class T>
struct Stack<T*>
{
    typedef detail::StackOpSelector<T*, detail::IsUserdata<T>::value> Helper;
    typedef typename Helper::ReturnType ReturnType;

    static void push(lua_State* L, T* value) { Helper::push(L, value); }

    static ReturnType get(lua_State* L, int index) { return Helper::get(L, index); }
};

template<class T>
struct Stack<const T*>
{
    typedef detail::StackOpSelector<const T*, detail::IsUserdata<T>::value> Helper;
    typedef typename Helper::ReturnType ReturnType;

    static void push(lua_State* L, const T* value) { Helper::push(L, value); }

    static ReturnType get(lua_State* L, int index) { return Helper::get(L, index); }
};

//------------------------------------------------------------------------------
/**
 * Push an object onto the Lua stack.
 */
template<class T>
inline void push(lua_State* L, T t)
{
    Stack<T>::push(L, t);
}

//------------------------------------------------------------------------------
/**
 * Check whether an object on the Lua stack is of type T.
 */
template<class T>
inline bool isInstance(lua_State* L, int index)
{
    return Stack<T>::isInstance(L, index);
}

} // namespace luabridge

// End File: Source/LuaBridge/detail/Stack.h

// Begin File: Source/LuaBridge/detail/Dump.h

// https://github.com/kunitoki/LuaBridge
// Copyright 2020, Lucio Asnaghi
// Copyright 2019, Dmitry Tarakanov
// Copyright 2012, Vinnie Falco <vinnie.falco@gmail.com>
// Copyright 2007, Nathan Reed
// SPDX-License-Identifier: MIT




namespace luabridge {
namespace debug {

inline void putIndent(std::ostream& stream, unsigned level)
{
    for (unsigned i = 0; i < level; ++i)
    {
        stream << "  ";
    }
}

inline void dumpTable(lua_State* L, int index, std::ostream& stream, unsigned level);

inline void dumpValue(lua_State* L, int index, std::ostream& stream, unsigned level = 0)
{
    const int type = lua_type(L, index);
    switch (type)
    {
    case LUA_TNIL:
        stream << "nil";
        break;

    case LUA_TBOOLEAN:
        stream << (lua_toboolean(L, index) ? "true" : "false");
        break;

    case LUA_TNUMBER:
        stream << lua_tonumber(L, index);
        break;

    case LUA_TSTRING:
        stream << '"' << lua_tostring(L, index) << '"';
        break;

    case LUA_TFUNCTION:
        if (lua_iscfunction(L, index))
        {
            stream << "cfunction@" << lua_topointer(L, index);
        }
        else
        {
            stream << "function@" << lua_topointer(L, index);
        }
        break;

    case LUA_TTHREAD:
        stream << "thread@" << lua_tothread(L, index);
        break;

    case LUA_TLIGHTUSERDATA:
        stream << "lightuserdata@" << lua_touserdata(L, index);
        break;

    case LUA_TTABLE:
        dumpTable(L, index, stream, level);
        break;

    case LUA_TUSERDATA:
        stream << "userdata@" << lua_touserdata(L, index);
        break;

    default:
        stream << lua_typename(L, type);
        ;
        break;
    }
}

inline void dumpTable(lua_State* L, int index, std::ostream& stream, unsigned level)
{
    stream << "table@" << lua_topointer(L, index);

    if (level > 0)
    {
        return;
    }

    index = lua_absindex(L, index);
    stream << " {";
    lua_pushnil(L); // Initial key
    while (lua_next(L, index))
    {
        stream << "\n";
        putIndent(stream, level + 1);
        dumpValue(L, -2, stream, level + 1); // Key
        stream << ": ";
        dumpValue(L, -1, stream, level + 1); // Value
        lua_pop(L, 1); // Value
    }
    putIndent(stream, level);
    stream << "\n}";
}

inline void dumpState(lua_State* L, std::ostream& stream = std::cerr)
{
    int top = lua_gettop(L);
    for (int i = 1; i <= top; ++i)
    {
        stream << "stack #" << i << ": ";
        dumpValue(L, i, stream, 0);
        stream << "\n";
    }
}

} // namespace debug

} // namespace luabridge

// End File: Source/LuaBridge/detail/Dump.h

// Begin File: Source/LuaBridge/Map.h

// https://github.com/kunitoki/LuaBridge
// Copyright 2020, Lucio Asnaghi
// Copyright 2018, Dmitry Tarakanov
// SPDX-License-Identifier: MIT




namespace luabridge {

template<class K, class V>
struct Stack<std::map<K, V>>
{
    typedef std::map<K, V> Map;

    static void push(lua_State* L, const Map& map)
    {
        lua_createtable(L, 0, static_cast<int>(map.size()));
        typedef typename Map::const_iterator ConstIter;
        for (ConstIter i = map.begin(); i != map.end(); ++i)
        {
            Stack<K>::push(L, i->first);
            Stack<V>::push(L, i->second);
            lua_settable(L, -3);
        }
    }

    static Map get(lua_State* L, int index)
    {
        if (!lua_istable(L, index))
        {
            luaL_error(L, "#%d argument must be a table", index);
        }

        Map map;
        int const absindex = lua_absindex(L, index);
        lua_pushnil(L);
        while (lua_next(L, absindex) != 0)
        {
            map.emplace(Stack<K>::get(L, -2), Stack<V>::get(L, -1));
            lua_pop(L, 1);
        }
        return map;
    }

    static bool isInstance(lua_State* L, int index) { return lua_istable(L, index); }
};

} // namespace luabridge

// End File: Source/LuaBridge/Map.h

// Begin File: Source/LuaBridge/UnorderedMap.h

// https://github.com/kunitoki/LuaBridge
// Copyright 2020, Lucio Asnaghi
// Copyright 2019, Dmitry Tarakanov
// SPDX-License-Identifier: MIT




namespace luabridge {

template<class K, class V>
struct Stack<std::unordered_map<K, V>>
{
    typedef std::unordered_map<K, V> Map;

    static void push(lua_State* L, const Map& map)
    {
        lua_createtable(L, 0, static_cast<int>(map.size()));
        typedef typename Map::const_iterator ConstIter;
        for (ConstIter i = map.begin(); i != map.end(); ++i)
        {
            Stack<K>::push(L, i->first);
            Stack<V>::push(L, i->second);
            lua_settable(L, -3);
        }
    }

    static Map get(lua_State* L, int index)
    {
        if (!lua_istable(L, index))
        {
            luaL_error(L, "#%d argument must be a table", index);
        }

        Map map;
        int const absindex = lua_absindex(L, index);
        lua_pushnil(L);
        while (lua_next(L, absindex) != 0)
        {
            map.emplace(Stack<K>::get(L, -2), Stack<V>::get(L, -1));
            lua_pop(L, 1);
        }
        return map;
    }

    static bool isInstance(lua_State* L, int index) { return lua_istable(L, index); }
};

} // namespace luabridge

// End File: Source/LuaBridge/UnorderedMap.h

// Begin File: Source/LuaBridge/RefCountedObject.h

// https://github.com/kunitoki/LuaBridge
// Copyright 2020, Lucio Asnaghi
// Copyright 2012, Vinnie Falco <vinnie.falco@gmail.com>
// Copyright 2004-11 by Raw Material Software Ltd.
// SPDX-License-Identifier: MIT

//==============================================================================
/*
  This is a derivative work used by permission from part of
  JUCE, available at http://www.rawaterialsoftware.com

  License: The MIT License (http://www.opensource.org/licenses/mit-license.php)

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.

  This file incorporates work covered by the following copyright and
  permission notice:

    This file is part of the JUCE library - "Jules' Utility Class Extensions"
    Copyright 2004-11 by Raw Material Software Ltd.
*/
//==============================================================================




namespace luabridge {

//==============================================================================
/**
  Adds reference-counting to an object.

  To add reference-counting to a class, derive it from this class, and
  use the RefCountedObjectPtr class to point to it.

  e.g. @code
  class MyClass : public RefCountedObjectType
  {
      void foo();

      // This is a neat way of declaring a typedef for a pointer class,
      // rather than typing out the full templated name each time..
      typedef RefCountedObjectPtr<MyClass> Ptr;
  };

  MyClass::Ptr p = new MyClass();
  MyClass::Ptr p2 = p;
  p = 0;
  p2->foo();
  @endcode

  Once a new RefCountedObjectType has been assigned to a pointer, be
  careful not to delete the object manually.
*/
template<class CounterType>
class RefCountedObjectType
{
public:
    //==============================================================================
    /** Increments the object's reference count.

        This is done automatically by the smart pointer, but is public just
        in case it's needed for nefarious purposes.
    */
    inline void incReferenceCount() const { ++refCount; }

    /** Decreases the object's reference count.

        If the count gets to zero, the object will be deleted.
    */
    inline void decReferenceCount() const
    {
        assert(getReferenceCount() > 0);

        if (--refCount == 0)
            delete this;
    }

    /** Returns the object's current reference count.
     * @returns The reference count.
     */
    inline int getReferenceCount() const { return static_cast<int>(refCount); }

protected:
    //==============================================================================
    /** Creates the reference-counted object (with an initial ref count of zero). */
    RefCountedObjectType() : refCount() {}

    /** Destructor. */
    virtual ~RefCountedObjectType()
    {
        // it's dangerous to delete an object that's still referenced by something else!
        assert(getReferenceCount() == 0);
    }

private:
    //==============================================================================
    CounterType mutable refCount;
};

//==============================================================================

/** Non thread-safe reference counted object.

    This creates a RefCountedObjectType that uses a non-atomic integer
    as the counter.
*/
typedef RefCountedObjectType<int> RefCountedObject;

//==============================================================================
/**
    A smart-pointer class which points to a reference-counted object.

    The template parameter specifies the class of the object you want to point
    to - the easiest way to make a class reference-countable is to simply make
    it inherit from RefCountedObjectType, but if you need to, you could roll
    your own reference-countable class by implementing a pair of methods called
    incReferenceCount() and decReferenceCount().

    When using this class, you'll probably want to create a typedef to
    abbreviate the full templated name - e.g.

    @code

    typedef RefCountedObjectPtr <MyClass> MyClassPtr;

    @endcode
*/
template<class ReferenceCountedObjectClass>
class RefCountedObjectPtr
{
public:
    /** The class being referenced by this pointer. */
    typedef ReferenceCountedObjectClass ReferencedType;

    //==============================================================================
    /** Creates a pointer to a null object. */
    inline RefCountedObjectPtr() : referencedObject(0) {}

    /** Creates a pointer to an object.
        This will increment the object's reference-count if it is non-null.

        @param refCountedObject A reference counted object to own.
    */
    inline RefCountedObjectPtr(ReferenceCountedObjectClass* const refCountedObject)
        : referencedObject(refCountedObject)
    {
        if (refCountedObject != 0)
            refCountedObject->incReferenceCount();
    }

    /** Copies another pointer.
        This will increment the object's reference-count (if it is non-null).

        @param other Another pointer.
    */
    inline RefCountedObjectPtr(const RefCountedObjectPtr& other)
        : referencedObject(other.referencedObject)
    {
        if (referencedObject != 0)
            referencedObject->incReferenceCount();
    }

    /**
      Takes-over the object from another pointer.

      @param other Another pointer.
    */
    inline RefCountedObjectPtr(RefCountedObjectPtr&& other)
        : referencedObject(other.referencedObject)
    {
        other.referencedObject = 0;
    }

    /** Copies another pointer.
        This will increment the object's reference-count (if it is non-null).

        @param other Another pointer.
    */
    template<class DerivedClass>
    inline RefCountedObjectPtr(const RefCountedObjectPtr<DerivedClass>& other)
        : referencedObject(static_cast<ReferenceCountedObjectClass*>(other.getObject()))
    {
        if (referencedObject != 0)
            referencedObject->incReferenceCount();
    }

    /** Changes this pointer to point at a different object.

        The reference count of the old object is decremented, and it might be
        deleted if it hits zero. The new object's count is incremented.

        @param other A pointer to assign from.
        @returns This pointer.
    */
    RefCountedObjectPtr& operator=(const RefCountedObjectPtr& other)
    {
        return operator=(other.referencedObject);
    }

    /** Changes this pointer to point at a different object.
        The reference count of the old object is decremented, and it might be
        deleted if it hits zero. The new object's count is incremented.

        @param other A pointer to assign from.
        @returns This pointer.
    */
    template<class DerivedClass>
    RefCountedObjectPtr& operator=(const RefCountedObjectPtr<DerivedClass>& other)
    {
        return operator=(static_cast<ReferenceCountedObjectClass*>(other.getObject()));
    }

    /**
      Takes-over the object from another pointer.

      @param other A pointer to assign from.
      @returns This pointer.
     */
    RefCountedObjectPtr& operator=(RefCountedObjectPtr&& other)
    {
        std::swap(referencedObject, other.referencedObject);
        return *this;
    }

    /** Changes this pointer to point at a different object.
        The reference count of the old object is decremented, and it might be
        deleted if it hits zero. The new object's count is incremented.

        @param newObject A reference counted object to own.
        @returns This pointer.
    */
    RefCountedObjectPtr& operator=(ReferenceCountedObjectClass* const newObject)
    {
        if (referencedObject != newObject)
        {
            if (newObject != 0)
                newObject->incReferenceCount();

            ReferenceCountedObjectClass* const oldObject = referencedObject;
            referencedObject = newObject;

            if (oldObject != 0)
                oldObject->decReferenceCount();
        }

        return *this;
    }

    /** Destructor.
        This will decrement the object's reference-count, and may delete it if it
        gets to zero.
    */
    ~RefCountedObjectPtr()
    {
        if (referencedObject != 0)
            referencedObject->decReferenceCount();
    }

    /** Returns the object that this pointer references.
        The returned pointer may be null.

        @returns The pointee.
    */
    operator ReferenceCountedObjectClass*() const { return referencedObject; }

    /** Returns the object that this pointer references.
        The returned pointer may be null.

        @returns The pointee.
    */
    ReferenceCountedObjectClass* operator->() const { return referencedObject; }

    /** Returns the object that this pointer references.
        The returned pointer may be null.

        @returns The pointee.
    */
    ReferenceCountedObjectClass* getObject() const { return referencedObject; }

private:
    //==============================================================================
    ReferenceCountedObjectClass* referencedObject;
};

/** Compares two ReferenceCountedObjectPointers. */
template<class ReferenceCountedObjectClass>
bool operator==(const RefCountedObjectPtr<ReferenceCountedObjectClass>& object1,
                ReferenceCountedObjectClass* const object2)
{
    return object1.getObject() == object2;
}

/** Compares two ReferenceCountedObjectPointers. */
template<class ReferenceCountedObjectClass>
bool operator==(const RefCountedObjectPtr<ReferenceCountedObjectClass>& object1,
                const RefCountedObjectPtr<ReferenceCountedObjectClass>& object2)
{
    return object1.getObject() == object2.getObject();
}

/** Compares two ReferenceCountedObjectPointers. */
template<class ReferenceCountedObjectClass>
bool operator==(ReferenceCountedObjectClass* object1,
                RefCountedObjectPtr<ReferenceCountedObjectClass>& object2)
{
    return object1 == object2.getObject();
}

/** Compares two ReferenceCountedObjectPointers. */
template<class ReferenceCountedObjectClass>
bool operator!=(const RefCountedObjectPtr<ReferenceCountedObjectClass>& object1,
                const ReferenceCountedObjectClass* object2)
{
    return object1.getObject() != object2;
}

/** Compares two ReferenceCountedObjectPointers. */
template<class ReferenceCountedObjectClass>
bool operator!=(const RefCountedObjectPtr<ReferenceCountedObjectClass>& object1,
                RefCountedObjectPtr<ReferenceCountedObjectClass>& object2)
{
    return object1.getObject() != object2.getObject();
}

/** Compares two ReferenceCountedObjectPointers. */
template<class ReferenceCountedObjectClass>
bool operator!=(ReferenceCountedObjectClass* object1,
                RefCountedObjectPtr<ReferenceCountedObjectClass>& object2)
{
    return object1 != object2.getObject();
}

//==============================================================================

template<class T>
struct ContainerTraits<RefCountedObjectPtr<T>>
{
    typedef T Type;

    static T* get(RefCountedObjectPtr<T> const& c) { return c.getObject(); }
};

//==============================================================================

} // namespace luabridge

// End File: Source/LuaBridge/RefCountedObject.h

// Begin File: Source/LuaBridge/RefCountedPtr.h

// https://github.com/kunitoki/LuaBridge
// Copyright 2020, Lucio Asnaghi
// Copyright 2019, Dmitry Tarakanov
// Copyright 2012, Vinnie Falco <vinnie.falco@gmail.com>
// Copyright 2007, Nathan Reed
// SPDX-License-Identifier: MIT




namespace luabridge {

namespace detail {

//==============================================================================
/**
  Support for our RefCountedPtr.
*/
struct RefCountedPtrBase
{
    // Declaration of container for the refcounts
    typedef std::unordered_map<const void*, int> RefCountsType;

protected:
    RefCountsType& getRefCounts() const
    {
        static RefCountsType refcounts;
        return refcounts;
    }
};

} // namespace detail

//==============================================================================
/**
  A reference counted smart pointer.

  The api is compatible with boost::RefCountedPtr and std::RefCountedPtr, in the
  sense that it implements a strict subset of the functionality.

  This implementation uses a hash table to look up the reference count
  associated with a particular pointer.

  @tparam T The class type.

  @todo Decompose RefCountedPtr using a policy. At a minimum, the underlying
        reference count should be policy based (to support atomic operations)
        and the delete behavior should be policy based (to support custom
        disposal methods).

  @todo Provide an intrusive version of RefCountedPtr.
*/
template<class T>
class RefCountedPtr : private detail::RefCountedPtrBase
{
public:
    template<typename Other>
    struct rebind
    {
        typedef RefCountedPtr<Other> other;
    };

    /** Construct as nullptr or from existing pointer to T.

        @param p The optional, existing pointer to assign from.
    */
    RefCountedPtr(T* p = 0) : m_p(p) { ++getRefCounts()[m_p]; }

    /** Construct from another RefCountedPtr.

        @param rhs The RefCountedPtr to assign from.
    */
    RefCountedPtr(RefCountedPtr<T> const& rhs) : m_p(rhs.get()) { ++getRefCounts()[m_p]; }

    /** Construct from a RefCountedPtr of a different type.

        @invariant A pointer to U must be convertible to a pointer to T.

        @tparam U   The other object type.
        @param  rhs The RefCountedPtr to assign from.
    */
    template<typename U>
    RefCountedPtr(RefCountedPtr<U> const& rhs) : m_p(static_cast<T*>(rhs.get()))
    {
        ++getRefCounts()[m_p];
    }

    /** Release the object.

        If there are no more references then the object is deleted.
    */
    ~RefCountedPtr() { reset(); }

    /** Assign from another RefCountedPtr.

        @param  rhs The RefCountedPtr to assign from.
        @returns     A reference to the RefCountedPtr.
    */
    RefCountedPtr<T>& operator=(RefCountedPtr<T> const& rhs)
    {
        if (m_p != rhs.m_p)
        {
            reset();
            m_p = rhs.m_p;
            ++getRefCounts()[m_p];
        }
        return *this;
    }

    /** Assign from another RefCountedPtr of a different type.

        @note A pointer to U must be convertible to a pointer to T.

        @tparam U   The other object type.
        @param  rhs The other RefCountedPtr to assign from.
        @returns     A reference to the RefCountedPtr.
    */
    template<typename U>
    RefCountedPtr<T>& operator=(RefCountedPtr<U> const& rhs)
    {
        reset();
        m_p = static_cast<T*>(rhs.get());
        ++getRefCounts()[m_p];
        return *this;
    }

    /** Retrieve the raw pointer.

        @returns A pointer to the object.
    */
    T* get() const { return m_p; }

    /** Retrieve the raw pointer.

        @returns A pointer to the object.
    */
    T* operator*() const { return m_p; }

    /** Retrieve the raw pointer.

        @returns A pointer to the object.
    */
    T* operator->() const { return m_p; }

    /** Determine the number of references.

        @note This is not thread-safe.

        @returns The number of active references.
    */
    long use_count() const { return getRefCounts()[m_p]; }

    /** Release the pointer.

        The reference count is decremented. If the reference count reaches
        zero, the object is deleted.
    */
    void reset()
    {
        if (m_p != 0)
        {
            if (--getRefCounts()[m_p] <= 0)
                delete m_p;

            m_p = 0;
        }
    }

private:
    T* m_p;
};

template<class T>
bool operator==(const RefCountedPtr<T>& lhs, const RefCountedPtr<T>& rhs)
{
    return lhs.get() == rhs.get();
}

template<class T>
bool operator!=(const RefCountedPtr<T>& lhs, const RefCountedPtr<T>& rhs)
{
    return lhs.get() != rhs.get();
}

//==============================================================================

// forward declaration
template<class T>
struct ContainerTraits;

template<class T>
struct ContainerTraits<RefCountedPtr<T>>
{
    typedef T Type;

    static T* get(RefCountedPtr<T> const& c) { return c.get(); }
};

} // namespace luabridge

// End File: Source/LuaBridge/RefCountedPtr.h

// Begin File: Source/LuaBridge/detail/TypeList.h

// https://github.com/kunitoki/LuaBridge
// Copyright 2020, Lucio Asnaghi
// Copyright 2019, George Tokmaji
// Copyright 2012, Vinnie Falco <vinnie.falco@gmail.com>
// Copyright 2007, Nathan Reed
// SPDX-License-Identifier: MIT

//=================================================================================================
/*
  This file incorporates work covered by the following copyright and
  permission notice:

    The Loki Library
    Copyright (c) 2001 by Andrei Alexandrescu
    This code accompanies the book:
    Alexandrescu, Andrei. "Modern C++ Design: Generic Programming and Design
        Patterns Applied". Copyright (c) 2001. Addison-Wesley.
    Permission to use, copy, modify, distribute and sell this software for any
        purpose is hereby granted without fee, provided that the above copyright
        notice appear in all copies and that both that copyright notice and this
        permission notice appear in supporting documentation.
    The author or Addison-Welsey Longman make no representations about the
        suitability of this software for any purpose. It is provided "as is"
        without express or implied warranty.
*/
//=================================================================================================




namespace luabridge {
namespace detail {

//=================================================================================================
/**
 * @brief None type means void parameters or return value.
 */
using None = void;

//=================================================================================================
/**
 * @brief Type list type pack..
 */
template <class Head, class Tail = None>
struct TypeList
{
    typedef Tail TailType;
};

template <class List>
struct TypeListSize
{
    static constexpr size_t value = TypeListSize<typename List::TailType>::value + 1;
};

template <>
struct TypeListSize<None>
{
    static constexpr size_t value = 0u;
};

template <class... Params>
struct MakeTypeList;

template <class Param, class... Params>
struct MakeTypeList<Param, Params...>
{
    using Result = TypeList<Param, typename MakeTypeList<Params...>::Result>;
};

template <>
struct MakeTypeList<>
{
    using Result = None;
};

//=================================================================================================
/**
 * @brief A TypeList with actual values.
 */
template <class List>
struct TypeListValues
{
    static std::string const tostring(bool) { return ""; }
};

//=================================================================================================
/**
 * @brief TypeListValues recursive template definition.
 */
template <class Head, class Tail>
struct TypeListValues<TypeList<Head, Tail>>
{
    Head hd;
    TypeListValues<Tail> tl;

    TypeListValues(Head hd_, const TypeListValues<Tail>& tl_)
        : hd(hd_)
        , tl(tl_)
    {
    }

    static std::string tostring(bool comma = false)
    {
        std::string s;

        if (comma)
            s = ", ";

        s = s + typeid(Head).name();

        return s + TypeListValues<Tail>::tostring(true);
    }
};

// Specializations of type/value list for head types that are references and
// const-references.  We need to handle these specially since we can't count
// on the referenced object hanging around for the lifetime of the list.

template <class Head, class Tail>
struct TypeListValues<TypeList<Head&, Tail>>
{
    Head hd;
    TypeListValues<Tail> tl;

    TypeListValues(Head& hd_, const TypeListValues<Tail>& tl_)
        : hd(hd_)
        , tl(tl_)
    {
    }

    static std::string tostring(bool comma = false)
    {
        std::string s;

        if (comma)
            s = ", ";

        s = s + typeid(Head).name() + "&";

        return s + TypeListValues<Tail>::tostring(true);
    }
};

template <class Head, class Tail>
struct TypeListValues<TypeList<const Head&, Tail>>
{
    Head hd;
    TypeListValues<Tail> tl;

    TypeListValues(Head const& hd_, const TypeListValues<Tail>& tl_)
        : hd(hd_)
        , tl(tl_)
    {
    }

    static std::string tostring(bool comma = false)
    {
        std::string s;

        if (comma)
            s = ", ";

        s = s + typeid(Head).name() + " const&";

        return s + TypeListValues<Tail>::tostring(true);
    }
};

//==============================================================================
/**
 * @brief Type list to tuple forwarder.
 */
template <class Head, class Tail>
auto typeListValuesTuple(TypeListValues<TypeList<Head, Tail>>& tvl)
{
    if constexpr (std::is_same_v<Tail, void>)
    {
        return std::forward_as_tuple(tvl.hd);
    }
    else
    {
        return std::tuple_cat(std::forward_as_tuple(tvl.hd), typeListValuesTuple(tvl.tl));
    }
}

template <class Head, class Tail>
auto typeListValuesTuple(const TypeListValues<TypeList<Head, Tail>>& tvl)
{
    if constexpr (std::is_same_v<Tail, void>)
    {
        return std::forward_as_tuple(tvl.hd);
    }
    else
    {
        return std::tuple_cat(std::forward_as_tuple(tvl.hd), typeListValuesTuple(tvl.tl));
    }
}

//==============================================================================
/**
 * @brief Subclass of a TypeListValues constructable from the Lua stack.
 */
template <class List, size_t Start = 1>
struct ArgList
{
};

template <size_t Start>
struct ArgList<None, Start> : public TypeListValues<None>
{
    ArgList(lua_State*)
    {
    }
};

template <class Head, class Tail, size_t Start>
struct ArgList<TypeList<Head, Tail>, Start> : public TypeListValues<TypeList<Head, Tail>>
{
    ArgList(lua_State* L)
        : TypeListValues<TypeList<Head, Tail>>(Stack<Head>::get(L, Start), ArgList<Tail, Start + 1>(L))
    {
    }
};

} // namespace detail
} // namespace luabridge

// End File: Source/LuaBridge/detail/TypeList.h

// Begin File: Source/LuaBridge/detail/FuncTraits.h

// https://github.com/kunitoki/LuaBridge
// Copyright 2020, Lucio Asnaghi
// Copyright 2020, Dmitry Tarakanov
// Copyright 2019, George Tokmaji
// Copyright 2012, Vinnie Falco <vinnie.falco@gmail.com>
// SPDX-License-Identifier: MIT




namespace luabridge {
namespace detail {

//=================================================================================================
/**
 * @brief Traits class for unrolling the type list values into function arguments.
 */
template <class ReturnType, size_t NUM_PARAMS>
struct Caller
{
    template <class Fn, class Params>
    static ReturnType f(Fn& fn, TypeListValues<Params>& tvl)
    {
        return std::apply(fn, typeListValuesTuple(tvl));
    }

    template <class T, class MemFn, class Params>
    static ReturnType f(T* obj, MemFn& fn, TypeListValues<Params>& tvl)
    {
        auto func = [obj, fn](auto&&... args) { return (obj->*fn)(std::forward<decltype(args)>(args)...); };

        return std::apply(func, typeListValuesTuple(tvl));
    }
};

template <class ReturnType>
struct Caller<ReturnType, 0>
{
    template <class Fn, class Params>
    static ReturnType f(Fn& fn, TypeListValues<Params>&)
    {
        return fn();
    }

    template <class T, class MemFn, class Params>
    static ReturnType f(T* obj, MemFn& fn, TypeListValues<Params>&)
    {
        return (obj->*fn)();
    }
};

template <class ReturnType, class Fn, class Params>
ReturnType doCall(Fn& fn, TypeListValues<Params>& tvl)
{
    return Caller<ReturnType, TypeListSize<Params>::value>::f(fn, tvl);
}

template <class ReturnType, class T, class MemFn, class Params>
ReturnType doCall(T* obj, MemFn& fn, TypeListValues<Params>& tvl)
{
    return Caller<ReturnType, TypeListSize<Params>::value>::f(obj, fn, tvl);
}

//=================================================================================================
/**
 * @brief Traits for function pointers.
 *
 * There are three types of functions: global, non-const member, and const member. These templates determine the type of function, which
 * class type it belongs to if it is a class member, the const-ness if it is a member function, and the type information for the return value and
 * argument list.
 */
template <class MemFn, class D = MemFn>
struct FuncTraits
{
};

// Ordinary function pointers.
template <class R, class... ParamList>
struct FuncTraits<R (*)(ParamList...)>
{
    static constexpr bool isMemberFunction = false;
    static constexpr bool isConstMemberFunction = false;
    using DeclType = R (*)(ParamList...);
    using ReturnType = R;
    using Params = typename MakeTypeList<ParamList...>::Result;

    static R call(const DeclType& fp, TypeListValues<Params>& tvl)
    {
        return doCall<R>(fp, tvl);
    }
};

// Windows: WINAPI (a.k.a. __stdcall) function pointers (32bit only).
#ifdef _M_IX86
template <class R, class... ParamList>
struct FuncTraits<R(__stdcall*)(ParamList...)>
{
    static constexpr bool isMemberFunction = false;
    static constexpr bool isConstMemberFunction = false;
    using DeclType = R(__stdcall*)(ParamList...);
    using ReturnType = R;
    using Params = typename MakeTypeList<ParamList...>::Result;

    static R call(const DeclType& fp, TypeListValues<Params>& tvl)
    {
        return doCall<R>(fp, tvl);
    }
};
#endif // _M_IX86

// Non-const member function pointers.
template <class T, class R, class... ParamList>
struct FuncTraits<R (T::*)(ParamList...)>
{
    static constexpr bool isMemberFunction = true;
    static constexpr bool isConstMemberFunction = false;
    using DeclType = R (T::*)(ParamList...);
    using ClassType = T;
    using ReturnType = R;
    using Params = typename MakeTypeList<ParamList...>::Result;

    static R call(ClassType* obj, const DeclType& fp, TypeListValues<Params>& tvl)
    {
        return doCall<R>(obj, fp, tvl);
    }
};

// Const member function pointers.
template <class T, class R, class... ParamList>
struct FuncTraits<R (T::*)(ParamList...) const>
{
    static constexpr bool isMemberFunction = true;
    static constexpr bool isConstMemberFunction = true;
    using DeclType = R (T::*)(ParamList...) const;
    using ClassType = T;
    using ReturnType = R;
    using Params = typename MakeTypeList<ParamList...>::Result;

    static R call(const ClassType* obj, const DeclType& fp, TypeListValues<Params>& tvl)
    {
        return doCall<R>(obj, fp, tvl);
    }
};

// std::function
template <class R, class... ParamList>
struct FuncTraits<std::function<R(ParamList...)>>
{
    static constexpr bool isMemberFunction = false;
    static constexpr bool isConstMemberFunction = false;
    using DeclType = std::function<R(ParamList...)>;
    using ReturnType = R;
    using Params = typename MakeTypeList<ParamList...>::Result;

    static ReturnType call(DeclType& fn, TypeListValues<Params>& tvl)
    {
        return doCall<ReturnType>(fn, tvl);
    }
};

//=================================================================================================
/**
 * @brief Invoke object that unpacks the arguments into stack values then call the functor.
 */
template< class ReturnType, class Params, int startParam>
struct Invoke
{
    template <class Fn>
    static int run(lua_State* L, Fn& fn)
    {
        try
        {
            ArgList<Params, startParam> args(L);
            Stack<ReturnType>::push(L, FuncTraits<Fn>::call(fn, args));
            return 1;
        }
        catch (const std::exception& e)
        {
            return luaL_error(L, e.what());
        }
    }

    template <class T, class MemFn>
    static int run(lua_State* L, T* object, const MemFn& fn)
    {
        try
        {
            ArgList<Params, startParam> args(L);
            Stack<ReturnType>::push(L, FuncTraits<MemFn>::call(object, fn, args));
            return 1;
        }
        catch (const std::exception& e)
        {
            return luaL_error(L, e.what());
        }
    }
};

template <class Params, int startParam>
struct Invoke<void, Params, startParam>
{
    template <class Fn>
    static int run(lua_State* L, Fn& fn)
    {
        try
        {
            ArgList<Params, startParam> args(L);
            FuncTraits<Fn>::call(fn, args);
            return 0;
        }
        catch (const std::exception& e)
        {
            return luaL_error(L, e.what());
        }
    }

    template <class T, class MemFn>
    static int run(lua_State* L, T* object, const MemFn& fn)
    {
        try
        {
            ArgList<Params, startParam> args(L);
            FuncTraits<MemFn>::call(object, fn, args);
            return 0;
        }
        catch (const std::exception& e)
        {
            return luaL_error(L, e.what());
        }
    }
};

} // namespace detail
} // namespace luabridge

// End File: Source/LuaBridge/detail/FuncTraits.h

// Begin File: Source/LuaBridge/detail/CFunctions.h

// https://github.com/kunitoki/LuaBridge
// Copyright 2020, Lucio Asnaghi
// Copyright 2019, Dmitry Tarakanov
// Copyright 2012, Vinnie Falco <vinnie.falco@gmail.com>
// SPDX-License-Identifier: MIT




namespace luabridge {
namespace detail {

struct CFunc
{
    static void addGetter(lua_State* L, const char* name, int tableIndex)
    {
        assert(name != nullptr);
        assert(lua_istable(L, tableIndex));
        assert(lua_iscfunction(L, -1)); // Stack: getter

        lua_rawgetp(L, tableIndex, getPropgetKey()); // Stack: getter, propget table (pg)
        lua_pushvalue(L, -2); // Stack: getter, pg, getter
        rawsetfield(L, -2, name); // Stack: getter, pg
        lua_pop(L, 2); // Stack: -
    }

    static void addSetter(lua_State* L, const char* name, int tableIndex)
    {
        assert(name != nullptr);
        assert(lua_istable(L, tableIndex));
        assert(lua_iscfunction(L, -1)); // Stack: setter

        lua_rawgetp(L, tableIndex, getPropsetKey()); // Stack: setter, propset table (ps)
        lua_pushvalue(L, -2); // Stack: setter, ps, setter
        rawsetfield(L, -2, name); // Stack: setter, ps
        lua_pop(L, 2); // Stack: -
    }

    //----------------------------------------------------------------------------
    /**
        __index metamethod for a namespace or class static and non-static members.
        Retrieves functions from metatables and properties from propget tables.
        Looks through the class hierarchy if inheritance is present.
    */
    static int indexMetaMethod(lua_State* L)
    {
        assert(lua_istable(L, 1) || lua_isuserdata(L, 1)); // Stack (further not shown): table | userdata, name

        lua_getmetatable(L, 1); // Stack: class/const table (mt)
        assert(lua_istable(L, -1));

        for (;;)
        {
            lua_pushvalue(L, 2); // Stack: mt, field name
            lua_rawget(L, -2); // Stack: mt, field | nil

            if (lua_iscfunction(L, -1)) // Stack: mt, field
            {
                lua_remove(L, -2); // Stack: field
                return 1;
            }

            assert(lua_isnil(L, -1)); // Stack: mt, nil
            lua_pop(L, 1); // Stack: mt

            lua_rawgetp(L, -1, getPropgetKey()); // Stack: mt, propget table (pg)
            assert(lua_istable(L, -1));

            lua_pushvalue(L, 2); // Stack: mt, pg, field name
            lua_rawget(L, -2); // Stack: mt, pg, getter | nil
            lua_remove(L, -2); // Stack: mt, getter | nil

            if (lua_iscfunction(L, -1)) // Stack: mt, getter
            {
                lua_remove(L, -2); // Stack: getter
                lua_pushvalue(L, 1); // Stack: getter, table | userdata
                lua_call(L, 1, 1); // Stack: value
                return 1;
            }

            assert(lua_isnil(L, -1)); // Stack: mt, nil
            lua_pop(L, 1); // Stack: mt

            // It may mean that the field may be in const table and it's constness violation.
            // Don't check that, just return nil

            // Repeat the lookup in the parent metafield,
            // or return nil if the field doesn't exist.
            lua_rawgetp(L, -1, getParentKey()); // Stack: mt, parent mt | nil

            if (lua_isnil(L, -1)) // Stack: mt, nil
            {
                lua_remove(L, -2); // Stack: nil
                return 1;
            }

            // Removethe  metatable and repeat the search in the parent one.
            assert(lua_istable(L, -1)); // Stack: mt, parent mt
            lua_remove(L, -2); // Stack: parent mt
        }

        // no return
    }

    //----------------------------------------------------------------------------
    /**
        __newindex metamethod for namespace or class static members.
        Retrieves properties from propset tables.
    */
    static int newindexStaticMetaMethod(lua_State* L)
    {
        return newindexMetaMethod(L, false);
    }

    //----------------------------------------------------------------------------
    /**
        __newindex metamethod for non-static members.
        Retrieves properties from propset tables.
    */
    static int newindexObjectMetaMethod(lua_State* L)
    {
        return newindexMetaMethod(L, true);
    }

    static int newindexMetaMethod(lua_State* L, bool pushSelf)
    {
        assert(lua_istable(L, 1) || lua_isuserdata(L, 1)); // Stack (further not shown): table | userdata, name, new value

        lua_getmetatable(L, 1); // Stack: metatable (mt)
        assert(lua_istable(L, -1));

        for (;;)
        {
            lua_rawgetp(L, -1, getPropsetKey()); // Stack: mt, propset table (ps) | nil

            if (lua_isnil(L, -1)) // Stack: mt, nil
            {
                lua_pop(L, 2); // Stack: -
                return luaL_error(L, "No member named '%s'", lua_tostring(L, 2));
            }

            assert(lua_istable(L, -1));

            lua_pushvalue(L, 2); // Stack: mt, ps, field name
            lua_rawget(L, -2); // Stack: mt, ps, setter | nil
            lua_remove(L, -2); // Stack: mt, setter | nil

            if (lua_iscfunction(L, -1)) // Stack: mt, setter
            {
                lua_remove(L, -2); // Stack: setter
                if (pushSelf)
                    lua_pushvalue(L, 1); // Stack: setter, table | userdata
                lua_pushvalue(L, 3); // Stack: setter, table | userdata, new value
                lua_call(L, pushSelf ? 2 : 1, 0); // Stack: -
                return 0;
            }

            assert(lua_isnil(L, -1)); // Stack: mt, nil
            lua_pop(L, 1); // Stack: mt

            lua_rawgetp(L, -1, getParentKey()); // Stack: mt, parent mt | nil

            if (lua_isnil(L, -1)) // Stack: mt, nil
            {
                lua_pop(L, 1); // Stack: -
                return luaL_error(L, "No writable member '%s'", lua_tostring(L, 2));
            }

            assert(lua_istable(L, -1)); // Stack: mt, parent mt
            lua_remove(L, -2); // Stack: parent mt
            // Repeat the search in the parent
        }

        // no return
    }

    //----------------------------------------------------------------------------
    /**
        lua_CFunction to report an error writing to a read-only value.

        The name of the variable is in the first upvalue.
    */
    static int readOnlyError(lua_State* L)
    {
        std::string s;

        s = s + "'" + lua_tostring(L, lua_upvalueindex(1)) + "' is read-only";

        return luaL_error(L, s.c_str());
    }

    //----------------------------------------------------------------------------
    /**
        lua_CFunction to get a variable.

        This is used for global variables or class static data members.

        The pointer to the data is in the first upvalue.
    */
    template <class T>
    static int getVariable(lua_State* L)
    {
        assert(lua_islightuserdata(L, lua_upvalueindex(1)));
        T const* ptr = static_cast<T const*>(lua_touserdata(L, lua_upvalueindex(1)));
        assert(ptr != nullptr);
        Stack<T>::push(L, *ptr);
        return 1;
    }

    //----------------------------------------------------------------------------
    /**
        lua_CFunction to set a variable.

        This is used for global variables or class static data members.

        The pointer to the data is in the first upvalue.
    */
    template <class T>
    static int setVariable(lua_State* L)
    {
        assert(lua_islightuserdata(L, lua_upvalueindex(1)));
        T* ptr = static_cast<T*>(lua_touserdata(L, lua_upvalueindex(1)));
        assert(ptr != nullptr);
        *ptr = Stack<T>::get(L, 1);
        return 0;
    }

    //----------------------------------------------------------------------------
    /**
        lua_CFunction to call a function with a return value.

        This is used for global functions, global properties, class static methods,
        and class static properties.

        The function pointer (lightuserdata) in the first upvalue.
    */
    template <class FnPtr>
    struct Call
    {
        using Params = typename FuncTraits<FnPtr>::Params;
        using ReturnType = typename FuncTraits<FnPtr>::ReturnType;

        static int f(lua_State* L)
        {
            assert(lua_islightuserdata(L, lua_upvalueindex(1)));
            FnPtr fnptr = reinterpret_cast<FnPtr>(lua_touserdata(L, lua_upvalueindex(1)));
            assert(fnptr != nullptr);
            return Invoke<ReturnType, Params, 1>::run(L, fnptr);
        }
    };

    //----------------------------------------------------------------------------
    /**
        lua_CFunction to call a class member function with a return value.

        The member function pointer is in the first upvalue.
        The class userdata object is at the top of the Lua stack.
    */
    template <class MemFnPtr, class T>
    struct CallMember
    {
        using Params = typename FuncTraits<MemFnPtr>::Params;
        using ReturnType = typename FuncTraits<MemFnPtr>::ReturnType;

        static int f(lua_State* L)
        {
            assert(isfulluserdata(L, lua_upvalueindex(1)));
            T* const t = Userdata::get<T>(L, 1, false);
            MemFnPtr const& fnptr = *static_cast<MemFnPtr const*>(lua_touserdata(L, lua_upvalueindex(1)));
            assert(fnptr != nullptr);
            return Invoke<ReturnType, Params, 2>::run(L, t, fnptr);
        }
    };

    template <class MemFnPtr, class T>
    struct CallConstMember
    {
        using Params = typename FuncTraits<MemFnPtr>::Params;
        using ReturnType = typename FuncTraits<MemFnPtr>::ReturnType;

        static int f(lua_State* L)
        {
            assert(isfulluserdata(L, lua_upvalueindex(1)));
            T const* const t = Userdata::get<T>(L, 1, true);
            MemFnPtr const& fnptr = *static_cast<MemFnPtr const*>(lua_touserdata(L, lua_upvalueindex(1)));
            assert(fnptr != nullptr);
            return Invoke<ReturnType, Params, 2>::run(L, t, fnptr);
        }
    };

    //--------------------------------------------------------------------------
    /**
        lua_CFunction to call a class member lua_CFunction.

        The member function pointer is in the first upvalue.
        The object userdata ('this') value is at top ot the Lua stack.
    */
    template <class T>
    struct CallMemberCFunction
    {
        static int f(lua_State* L)
        {
            assert(isfulluserdata(L, lua_upvalueindex(1)));
            typedef int (T::*MFP)(lua_State * L);
            T* const t = Userdata::get<T>(L, 1, false);
            MFP const& fnptr = *static_cast<MFP const*>(lua_touserdata(L, lua_upvalueindex(1)));
            assert(fnptr != nullptr);
            return (t->*fnptr)(L);
        }
    };

    template <class T>
    struct CallConstMemberCFunction
    {
        static int f(lua_State* L)
        {
            assert(isfulluserdata(L, lua_upvalueindex(1)));
            typedef int (T::*MFP)(lua_State * L) const;
            T const* const t = Userdata::get<T>(L, 1, true);
            MFP const& fnptr = *static_cast<MFP const*>(lua_touserdata(L, lua_upvalueindex(1)));
            assert(fnptr != nullptr);
            return (t->*fnptr)(L);
        }
    };

    //--------------------------------------------------------------------------
    /**
        lua_CFunction to call on a object.

        The proxy function pointer (lightuserdata) is in the first upvalue.
        The class userdata object is at the top of the Lua stack.
    */
    template <class FnPtr>
    struct CallProxyFunction
    {
        using Params = typename FuncTraits<FnPtr>::Params;
        using ReturnType = typename FuncTraits<FnPtr>::ReturnType;

        static int f(lua_State* L)
        {
            assert(lua_islightuserdata(L, lua_upvalueindex(1)));
            auto fnptr = reinterpret_cast<FnPtr>(lua_touserdata(L, lua_upvalueindex(1)));
            assert(fnptr != nullptr);
            return Invoke<ReturnType, Params, 1>::run(L, fnptr);
        }
    };

    template <class Functor>
    struct CallProxyFunctor
    {
        using Params = typename FuncTraits<Functor>::Params;
        using ReturnType = typename FuncTraits<Functor>::ReturnType;

        static int f(lua_State* L)
        {
            assert(isfulluserdata(L, lua_upvalueindex(1)));
            Functor& fn = *align<Functor>(lua_touserdata(L, lua_upvalueindex(1)));
            return Invoke<ReturnType, Params, 1>::run(L, fn);
        }
    };

    //--------------------------------------------------------------------------

    // SFINAE Helpers

    template <class MemFnPtr, class T, bool isConst>
    struct CallMemberFunctionHelper
    {
        static void add(lua_State* L, char const* name, MemFnPtr mf)
        {
            new (lua_newuserdata(L, sizeof(MemFnPtr))) MemFnPtr(mf);
            lua_pushcclosure(L, &CallConstMember<MemFnPtr, T>::f, 1);
            lua_pushvalue(L, -1);
            rawsetfield(L, -5, name); // const table
            rawsetfield(L, -3, name); // class table
        }
    };

    template <class MemFnPtr, class T>
    struct CallMemberFunctionHelper<MemFnPtr, T, false>
    {
        static void add(lua_State* L, char const* name, MemFnPtr mf)
        {
            new (lua_newuserdata(L, sizeof(MemFnPtr))) MemFnPtr(mf);
            lua_pushcclosure(L, &CallMember<MemFnPtr, T>::f, 1);
            rawsetfield(L, -3, name); // class table
        }
    };

    //--------------------------------------------------------------------------
    /**
        __gc metamethod for a class.
    */
    template <class C>
    static int gcMetaMethod(lua_State* L)
    {
        Userdata* const ud = Userdata::getExact<C>(L, 1);
        ud->~Userdata();
        return 0;
    }

    /**
        __gc metamethod for an arbitrary class.
    */
    template <class T>
    static int gcMetaMethodAny(lua_State* L)
    {
        assert(isfulluserdata(L, 1));
        T* t = align<T>(lua_touserdata(L, 1));
        t->~T();
        return 0;
    }

    //--------------------------------------------------------------------------
    /**
        lua_CFunction to get a class data member.

        The pointer-to-member is in the first upvalue.
        The class userdata object is at the top of the Lua stack.
    */
    template <class C, typename T>
    static int getProperty(lua_State* L)
    {
        C* const c = Userdata::get<C>(L, 1, true);
        T C::** mp = static_cast<T C::**>(lua_touserdata(L, lua_upvalueindex(1)));
        try
        {
            Stack<T&>::push(L, c->**mp);
        }
        catch (const std::exception& e)
        {
            luaL_error(L, e.what());
        }
        return 1;
    }

    //--------------------------------------------------------------------------
    /**
        lua_CFunction to set a class data member.

        The pointer-to-member is in the first upvalue.
        The class userdata object is at the top of the Lua stack.
    */
    template <class C, typename T>
    static int setProperty(lua_State* L)
    {
        C* const c = Userdata::get<C>(L, 1, false);
        T C::** mp = static_cast<T C::**>(lua_touserdata(L, lua_upvalueindex(1)));
        try
        {
            c->** mp = Stack<T>::get(L, 2);
        }
        catch (const std::exception& e)
        {
            luaL_error(L, e.what());
        }
        return 0;
    }
};

} // namespace detail
} // namespace luabridge

// End File: Source/LuaBridge/detail/CFunctions.h

// Begin File: Source/LuaBridge/detail/Constructor.h

// https://github.com/kunitoki/LuaBridge
// Copyright 2012, Vinnie Falco <vinnie.falco@gmail.com>
// Copyright 2007, Nathan Reed
// SPDX-License-Identifier: MIT



namespace luabridge {
namespace detail {

/** Constructor generators.

    These templates call operator new with the contents of a type/value
    list passed to the Constructor with up to 8 parameters. Two versions
    of call() are provided. One performs a regular new, the other performs
    a placement new.
*/
template <class T, class Params>
struct Constructor;

template <class T>
struct Constructor<T, None>
{
    static T* call(const TypeListValues<None>&)
    {
        return new T;
    }

    static T* call(void* ptr, const TypeListValues<None>&)
    {
        return new (ptr) T;
    }
};

template<class T, class Params>
struct Constructor
{
    static T* call(const TypeListValues<Params>& tvl)
    {
        auto alloc = [](auto&&... args) { return new T(std::forward<decltype(args)>(args)...); };
        
        return std::apply(alloc, typeListValuesTuple(tvl));
    }
    
    static T* call(void* ptr, const TypeListValues<Params>& tvl)
    {
        auto alloc = [ptr](auto&&... args) { return new (ptr) T(std::forward<decltype(args)>(args)...); };

        return std::apply(alloc, typeListValuesTuple(tvl));
    }
};

} // namespace detail
} // namespace luabridge

// End File: Source/LuaBridge/detail/Constructor.h

// Begin File: Source/LuaBridge/detail/LuaException.h

// https://github.com/kunitoki/LuaBridge
// Copyright 2012, Vinnie Falco <vinnie.falco@gmail.com>
// Copyright 2008, Nigel Atkinson <suprapilot+LuaCode@gmail.com>
// SPDX-License-Identifier: MIT




namespace luabridge {

class LuaException : public std::exception
{
private:
    lua_State* m_L;
    std::string m_what;

public:
    //----------------------------------------------------------------------------
    /**
        Construct a LuaException after a lua_pcall().
    */
    LuaException(lua_State* L, int /*code*/) : m_L(L) { whatFromStack(); }

    //----------------------------------------------------------------------------

    LuaException(lua_State* L, char const*, char const*, long) : m_L(L) { whatFromStack(); }

    //----------------------------------------------------------------------------

    ~LuaException() throw() {}

    //----------------------------------------------------------------------------

    char const* what() const throw() { return m_what.c_str(); }

    //============================================================================
    /**
        Throw an exception.

        This centralizes all the exceptions thrown, so that we can set
        breakpoints before the stack is unwound, or otherwise customize the
        behavior.
    */
    template<class Exception>
    static void Throw(Exception e)
    {
        throw e;
    }

    //----------------------------------------------------------------------------
    /**
        Wrapper for lua_pcall that throws.
    */
    static void pcall(lua_State* L, int nargs = 0, int nresults = 0, int msgh = 0)
    {
        int code = lua_pcall(L, nargs, nresults, msgh);

        if (code != LUABRIDGE_LUA_OK)
            Throw(LuaException(L, code));
    }

    //----------------------------------------------------------------------------
    /**
        Initializes error handling. Subsequent Lua errors are translated to C++ exceptions.
    */
    static void enableExceptions(lua_State* L) { lua_atpanic(L, throwAtPanic); }

protected:
    void whatFromStack()
    {
        if (lua_gettop(m_L) > 0)
        {
            char const* s = lua_tostring(m_L, -1);
            m_what = s ? s : "";
        }
        else
        {
            // stack is empty
            m_what = "missing error";
        }
    }

private:
    static int throwAtPanic(lua_State* L) { throw LuaException(L, -1); }
};

//----------------------------------------------------------------------------
/**
    Initializes error handling. Subsequent Lua errors are translated to C++ exceptions.
*/
static void enableExceptions(lua_State* L)
{
    LuaException::enableExceptions(L);
}

} // namespace luabridge

// End File: Source/LuaBridge/detail/LuaException.h

// Begin File: Source/LuaBridge/detail/LuaRef.h

// https://github.com/kunitoki/LuaBridge
// Copyright 2020, Lucio Asnaghi
// Copyright 2019, George Tokmaji
// Copyright 2018, Dmitry Tarakanov
// Copyright 2012, Vinnie Falco <vinnie.falco@gmail.com>
// Copyright 2008, Nigel Atkinson <suprapilot+LuaCode@gmail.com>
// SPDX-License-Identifier: MIT




namespace luabridge {

//------------------------------------------------------------------------------
/**
    Helper for iterating through tuple arguments, pushing eash argument to the
    stack. Adapted from https://stackoverflow.com/a/6894436
*/

template <std::size_t I = 0, typename... Tp>
auto push_arguments(lua_State*, const std::tuple<Tp...>&) // Unused arguments are given no names.
    -> std::enable_if_t<I == sizeof...(Tp)>
{
}

template <std::size_t I = 0, typename... Tp>
auto push_arguments(lua_State* L, const std::tuple<Tp...>& t)
    -> std::enable_if_t<I < sizeof...(Tp)>
{
    Stack<typename std::tuple_element<I, std::tuple<Tp...>>::type>::push(L, std::get<I>(t));
    push_arguments<I + 1, Tp...>(L, t);
}

//------------------------------------------------------------------------------
/**
 * @brief Type tag for representing LUA_TNIL.
 *
 * Construct one of these using `LuaNil ()` to represent a Lua nil. This is faster
 * than creating a reference in the registry to nil. Example:
 *
 * @code
 *     LuaRef t (LuaRef::createTable (L));
 *     ...
 *     t ["k"] = LuaNil (); // assign nil
 * @endcode
 */
struct LuaNil
{
};

//------------------------------------------------------------------------------
/**
 * @code Stack specialization for LuaNil.
 */
template<>
struct Stack<LuaNil>
{
    static void push(lua_State* L, LuaNil) { lua_pushnil(L); }

    static bool isInstance(lua_State* L, int index) { return lua_type(L, index) == LUA_TTABLE; }
};

/**
 * Base class for Lua variables and table item reference classes.
 */
template<class Impl, class LuaRef>
class LuaRefBase
{
protected:
    //----------------------------------------------------------------------------
    /**
        Pop the Lua stack.

        Pops the specified number of stack items on destruction. We use this
        when returning objects, to avoid an explicit temporary variable, since
        the destructor executes after the return statement. For example:

            template <class U>
            U cast (lua_State* L)
            {
              StackPop p (L, 1);
              ...
              return U (); // dtor called after this line
            }

        @note The `StackPop` object must always be a named local variable.
    */
    class StackPop
    {
    public:
        /** Create a StackPop object.

            @param L     A Lua state.
            @param count The number of stack entries to pop on destruction.
        */
        StackPop(lua_State* L, int count) : m_L(L), m_count(count) {}

        ~StackPop() { lua_pop(m_L, m_count); }

    private:
        lua_State* m_L;
        int m_count;
    };

    friend struct Stack<LuaRef>;

    //----------------------------------------------------------------------------
    /**
        Type tag for stack construction.
    */
    struct FromStack
    {
    };

    LuaRefBase(lua_State* L) : m_L(L) {}

    //----------------------------------------------------------------------------
    /**
        Create a reference to this reference.

        @returns An index in the Lua registry.
    */
    int createRef() const
    {
        impl().push();
        return luaL_ref(m_L, LUA_REGISTRYINDEX);
    }

public:
    //----------------------------------------------------------------------------
    /**
        Convert to a string using lua_tostring function.

        @returns A string representation of the referred Lua value.
    */
    std::string tostring() const
    {
        lua_getglobal(m_L, "tostring");
        impl().push();
        lua_call(m_L, 1, 1);
        const char* str = lua_tostring(m_L, -1);
        lua_pop(m_L, 1);
        return str;
    }

    //----------------------------------------------------------------------------
    /**
        Print a text description of the value to a stream.
        This is used for diagnostics.

        @param os An output stream.
    */
    void print(std::ostream& os) const
    {
        switch (type())
        {
        case LUA_TNIL:
            os << "nil";
            break;

        case LUA_TNUMBER:
            os << cast<lua_Number>();
            break;

        case LUA_TBOOLEAN:
            os << (cast<bool>() ? "true" : "false");
            break;

        case LUA_TSTRING:
            os << '"' << cast<std::string>() << '"';
            break;

        case LUA_TTABLE:
            os << "table: " << tostring();
            break;

        case LUA_TFUNCTION:
            os << "function: " << tostring();
            break;

        case LUA_TUSERDATA:
            os << "userdata: " << tostring();
            break;

        case LUA_TTHREAD:
            os << "thread: " << tostring();
            break;

        case LUA_TLIGHTUSERDATA:
            os << "lightuserdata: " << tostring();
            break;

        default:
            os << "unknown";
            break;
        }
    }

    //------------------------------------------------------------------------------
    /**
      Insert a Lua value or table item reference to a stream.

      @param os  An output stream.
      @param ref A Lua reference.
      @returns The output stream.
    */
    friend std::ostream& operator<<(std::ostream& os, LuaRefBase const& ref)
    {
        ref.print(os);
        return os;
    }

    //============================================================================
    //
    // This group of member functions is mirrored in TableItem
    //

    /** Retrieve the lua_State associated with the reference.

      @returns A Lua state.
    */
    lua_State* state() const { return m_L; }

    //----------------------------------------------------------------------------
    /**
        Place the object onto the Lua stack.

        @param L A Lua state.
    */
    void push(lua_State* L) const
    {
        assert(equalstates(L, m_L));
        (void) L;
        impl().push();
    }

    //----------------------------------------------------------------------------
    /**
        Pop the top of Lua stack and assign it to the reference.

        @param L A Lua state.
    */
    void pop(lua_State* L)
    {
        assert(equalstates(L, m_L));
        (void) L;
        impl().pop();
    }

    //----------------------------------------------------------------------------
    /**
        Return the Lua type of the referred value. This invokes lua_type().

        @returns The type of the referred value.
        @see lua_type()
    */
    /** @{ */
    int type() const
    {
        impl().push();
        StackPop p(m_L, 1);
        return lua_type(m_L, -1);
    }

    // should never happen
    // bool isNone () const { return m_ref == LUA_NOREF; }

    /// Indicate whether it is a nil reference.
    ///
    /// @returns True if this is a nil reference, false otherwice.
    ///
    bool isNil() const { return type() == LUA_TNIL; }

    /// Indicate whether it is a reference to a boolean.
    ///
    /// @returns True if it is a reference to a boolean, false otherwice.
    ///
    bool isBool() const { return type() == LUA_TBOOLEAN; }

    /// Indicate whether it is a reference to a number.
    ///
    /// @returns True if it is a reference to a number, false otherwise.
    ///
    bool isNumber() const { return type() == LUA_TNUMBER; }

    /// Indicate whether it is a reference to a string.
    ///
    /// @returns True if it is a reference to a string, false otherwise.
    ///
    bool isString() const { return type() == LUA_TSTRING; }

    /// Indicate whether it is a reference to a table.
    ///
    /// @returns True if it is a reference to a table, false otherwise.
    ///
    bool isTable() const { return type() == LUA_TTABLE; }

    /// Indicate whether it is a reference to a function.
    ///
    /// @returns True if it is a reference to a function, false otherwise.
    ///
    bool isFunction() const { return type() == LUA_TFUNCTION; }

    /// Indicate whether it is a reference to a full userdata.
    ///
    /// @returns True if it is a reference to a full userdata, false otherwise.
    ///
    bool isUserdata() const { return type() == LUA_TUSERDATA; }

    /// Indicate whether it is a reference to a Lua thread.
    ///
    /// @returns True if it is a reference to a Lua thread, false otherwise.
    ///
    bool isThread() const { return type() == LUA_TTHREAD; }

    /// Indicate whether it is a reference to a light userdata.
    ///
    /// @returns True if it is a reference to a light userdata, false otherwise.
    ///
    bool isLightUserdata() const { return type() == LUA_TLIGHTUSERDATA; }

    /** @} */

    //----------------------------------------------------------------------------
    /**
        Perform an explicit conversion to the type T.

        @returns A value of the type T converted from this reference.
    */
    template<class T>
    T cast() const
    {
        StackPop p(m_L, 1);

        impl().push();

        return Stack<T>::get(m_L, -1);
    }

    //----------------------------------------------------------------------------
    /**
        Indicate if this reference is convertible to the type T.

        @returns True if the referred value is convertible to the type T,
                false otherwise.
    */
    template<class T>
    bool isInstance() const
    {
        StackPop p(m_L, 1);

        impl().push();

        return Stack<T>::isInstance(m_L, -1);
    }

    //----------------------------------------------------------------------------
    /**
        Type cast operator.

        @returns A value of the type T converted from this reference.
    */
    template<class T>
    operator T() const
    {
        return cast<T>();
    }

    //----------------------------------------------------------------------------
    /** @{ */
    /**
        Compare this reference with a specified value using lua_compare().
        This invokes metamethods.

        @param rhs A value to compare with.
        @returns True if the referred value is equal to the specified one.
    */
    template<class T>
    bool operator==(T rhs) const
    {
        StackPop p(m_L, 2);

        impl().push();

        Stack<T>::push(m_L, rhs);

        return lua_compare(m_L, -2, -1, LUA_OPEQ) == 1;
    }

    /**
        Compare this reference with a specified value using lua_compare().
        This invokes metamethods.

        @param rhs A value to compare with.
        @returns True if the referred value is less than the specified one.
    */
    template<class T>
    bool operator<(T rhs) const
    {
        StackPop p(m_L, 2);

        impl().push();

        Stack<T>::push(m_L, rhs);

        int lhsType = lua_type(m_L, -2);
        int rhsType = lua_type(m_L, -1);
        if (lhsType != rhsType)
            return lhsType < rhsType;

        return lua_compare(m_L, -2, -1, LUA_OPLT) == 1;
    }

    /**
        Compare this reference with a specified value using lua_compare().
        This invokes metamethods.

        @param rhs A value to compare with.
        @returns True if the referred value is less than or equal to the specified one.
    */
    template<class T>
    bool operator<=(T rhs) const
    {
        StackPop p(m_L, 2);

        impl().push();

        Stack<T>::push(m_L, rhs);

        int lhsType = lua_type(m_L, -2);
        int rhsType = lua_type(m_L, -1);
        if (lhsType != rhsType)
            return lhsType <= rhsType;

        return lua_compare(m_L, -2, -1, LUA_OPLE) == 1;
    }

    /**
        Compare this reference with a specified value using lua_compare().
        This invokes metamethods.

        @param rhs A value to compare with.
        @returns True if the referred value is greater than the specified one.
    */
    template<class T>
    bool operator>(T rhs) const
    {
        StackPop p(m_L, 2);

        impl().push();

        Stack<T>::push(m_L, rhs);

        int lhsType = lua_type(m_L, -2);
        int rhsType = lua_type(m_L, -1);
        if (lhsType != rhsType)
            return lhsType > rhsType;

        return lua_compare(m_L, -1, -2, LUA_OPLT) == 1;
    }

    /**
        Compare this reference with a specified value using lua_compare().
        This invokes metamethods.

        @param rhs A value to compare with.
        @returns True if the referred value is greater than or equal to the specified one.
    */
    template<class T>
    bool operator>=(T rhs) const
    {
        StackPop p(m_L, 2);

        impl().push();

        Stack<T>::push(m_L, rhs);

        int lhsType = lua_type(m_L, -2);
        int rhsType = lua_type(m_L, -1);
        if (lhsType != rhsType)
            return lhsType >= rhsType;

        return lua_compare(m_L, -1, -2, LUA_OPLE) == 1;
    }

    /**
        Compare this reference with a specified value using lua_compare().
        This does not invoke metamethods.

        @param rhs A value to compare with.
        @returns True if the referred value is equal to the specified one.
    */
    template<class T>
    bool rawequal(T rhs) const
    {
        StackPop p(m_L, 2);

        impl().push();

        Stack<T>::push(m_L, rhs);
        return lua_rawequal(m_L, -1, -2) == 1;
    }
    /** @} */

    //----------------------------------------------------------------------------
    /**
        Append a value to a referred table.
        If the table is a sequence this will add another element to it.

        @param v A value to append to the table.
    */
    template<class T>
    void append(T v) const
    {
        impl().push();

        Stack<T>::push(m_L, v);
        luaL_ref(m_L, -2);
        lua_pop(m_L, 1);
    }

    //----------------------------------------------------------------------------
    /**
        Return the length of a referred array.
        This is identical to applying the Lua # operator.

        @returns The length of the referred array.
    */
    int length() const
    {
        StackPop p(m_L, 1);

        impl().push();

        return get_length(m_L, -1);
    }

    //----------------------------------------------------------------------------
    /**
        Call Lua code.
        These overloads allow Lua code to be called with up to 8 parameters.
        The return value is provided as a LuaRef (which may be LUA_REFNIL).
        If an error occurs, a LuaException is thrown.

        @returns A result of the call.
    */
    template <typename... Args>
    LuaRef operator()(Args&&... args) const
    {
        impl().push();

        push_arguments(m_L, std::forward_as_tuple(args...));

        LuaException::pcall (m_L, sizeof...(args), 1);

        return LuaRef::fromStack(m_L);
    }

    //============================================================================

protected:
    lua_State* m_L;

private:
    const Impl& impl() const { return static_cast<const Impl&>(*this); }

    Impl& impl() { return static_cast<Impl&>(*this); }
};

//------------------------------------------------------------------------------
/**
    Lightweight reference to a Lua object.

    The reference is maintained for the lifetime of the C++ object.
*/
class LuaRef : public LuaRefBase<LuaRef, LuaRef>
{
    //----------------------------------------------------------------------------
    /**
        A proxy for representing table values.
    */
    class TableItem : public LuaRefBase<TableItem, LuaRef>
    {
        friend class LuaRef;

    public:
        //--------------------------------------------------------------------------
        /**
            Construct a TableItem from a table value.
            The table is in the registry, and the key is at the top of the stack.
            The key is popped off the stack.

            @param L        A lua state.
            @param tableRef The index of a table in the Lua registry.
        */
        TableItem(lua_State* L, int tableRef)
            : LuaRefBase(L), m_tableRef(LUA_NOREF), m_keyRef(luaL_ref(L, LUA_REGISTRYINDEX))
        {
            lua_rawgeti(m_L, LUA_REGISTRYINDEX, tableRef);
            m_tableRef = luaL_ref(L, LUA_REGISTRYINDEX);
        }

        //--------------------------------------------------------------------------
        /**
            Create a TableItem via copy constructor.
            It is best to avoid code paths that invoke this, because it creates
            an extra temporary Lua reference. Typically this is done by passing
            the TableItem parameter as a `const` reference.

            @param other Another Lua table item reference.
        */
        TableItem(TableItem const& other)
            : LuaRefBase(other.m_L), m_tableRef(LUA_NOREF), m_keyRef(LUA_NOREF)
        {
            lua_rawgeti(m_L, LUA_REGISTRYINDEX, other.m_tableRef);
            m_tableRef = luaL_ref(m_L, LUA_REGISTRYINDEX);

            lua_rawgeti(m_L, LUA_REGISTRYINDEX, other.m_keyRef);
            m_keyRef = luaL_ref(m_L, LUA_REGISTRYINDEX);
        }

        //--------------------------------------------------------------------------
        /**
            Destroy the proxy.
            This does not destroy the table value.
        */
        ~TableItem()
        {
            luaL_unref(m_L, LUA_REGISTRYINDEX, m_keyRef);
            luaL_unref(m_L, LUA_REGISTRYINDEX, m_tableRef);
        }

        //--------------------------------------------------------------------------
        /**
            Assign a new value to this table key.
            This may invoke metamethods.

            @tparam T The type of a value to assing.
            @param  v A value to assign.
            @returns This reference.
        */
        template<class T>
        TableItem& operator=(T v)
        {
            StackPop p(m_L, 1);
            lua_rawgeti(m_L, LUA_REGISTRYINDEX, m_tableRef);
            lua_rawgeti(m_L, LUA_REGISTRYINDEX, m_keyRef);
            Stack<T>::push(m_L, v);
            lua_settable(m_L, -3);
            return *this;
        }

        //--------------------------------------------------------------------------
        /**
            Assign a new value to this table key.
            The assignment is raw, no metamethods are invoked.

            @tparam T The type of a value to assing.
            @param  v A value to assign.
            @returns This reference.
        */
        template<class T>
        TableItem& rawset(T v)
        {
            StackPop p(m_L, 1);
            lua_rawgeti(m_L, LUA_REGISTRYINDEX, m_tableRef);
            lua_rawgeti(m_L, LUA_REGISTRYINDEX, m_keyRef);
            Stack<T>::push(m_L, v);
            lua_rawset(m_L, -3);
            return *this;
        }

        //--------------------------------------------------------------------------
        /**
            Push the value onto the Lua stack.
        */
        using LuaRefBase::push;

        void push() const
        {
            lua_rawgeti(m_L, LUA_REGISTRYINDEX, m_tableRef);
            lua_rawgeti(m_L, LUA_REGISTRYINDEX, m_keyRef);
            lua_gettable(m_L, -2);
            lua_remove(m_L, -2); // remove the table
        }

        //--------------------------------------------------------------------------
        /**
            Access a table value using a key.
            This invokes metamethods.

            @tparam T   The type of a key.
            @param  key A key value.
            @returns A Lua table item reference.
        */
        template<class T>
        TableItem operator[](T key) const
        {
            return LuaRef(*this)[key];
        }

        //--------------------------------------------------------------------------
        /**
            Access a table value using a key.
            The operation is raw, metamethods are not invoked. The result is
            passed by value and may not be modified.

            @tparam T   The type of a key.
            @param  key A key value.
            @returns A Lua value reference.
        */
        template<class T>
        LuaRef rawget(T key) const
        {
            return LuaRef(*this).rawget(key);
        }

    private:
        int m_tableRef;
        int m_keyRef;
    };

    friend struct Stack<TableItem>;
    friend struct Stack<TableItem&>;

    //----------------------------------------------------------------------------
    /**
        Create a reference to an object at the top of the Lua stack and pop it.
        This constructor is private and not invoked directly.
        Instead, use the `fromStack` function.

        @param L A Lua state.
        @note The object is popped.
    */
    LuaRef(lua_State* L, FromStack) : LuaRefBase(L), m_ref(luaL_ref(m_L, LUA_REGISTRYINDEX)) {}

    //----------------------------------------------------------------------------
    /**
        Create a reference to an object on the Lua stack.
        This constructor is private and not invoked directly.
        Instead, use the `fromStack` function.

        @param L     A Lua state.
        @param index The index of the value on the Lua stack.
        @note The object is not popped.
    */
    LuaRef(lua_State* L, int index, FromStack) : LuaRefBase(L), m_ref(LUA_NOREF)
    {
        lua_pushvalue(m_L, index);
        m_ref = luaL_ref(m_L, LUA_REGISTRYINDEX);
    }

public:
    //----------------------------------------------------------------------------
    /**
        Create a nil reference.
        The Lua reference may be assigned later.

        @param L A Lua state.
    */
    LuaRef(lua_State* L) : LuaRefBase(L), m_ref(LUA_NOREF) {}

    //----------------------------------------------------------------------------
    /**
        Push a value onto a Lua stack and return a reference to it.

        @param L A Lua state.
        @param v A value to push.
    */
    template<class T>
    LuaRef(lua_State* L, T v) : LuaRefBase(L), m_ref(LUA_NOREF)
    {
        Stack<T>::push(m_L, v);
        m_ref = luaL_ref(m_L, LUA_REGISTRYINDEX);
    }

    //----------------------------------------------------------------------------
    /**
        Create a reference to a table item.

        @param v A table item reference.
    */
    LuaRef(TableItem const& v) : LuaRefBase(v.state()), m_ref(v.createRef()) {}

    //----------------------------------------------------------------------------
    /**
        Create a new reference to an existing Lua value.

        @param other An existing reference.
    */
    LuaRef(LuaRef const& other) : LuaRefBase(other.m_L), m_ref(other.createRef()) {}

    //----------------------------------------------------------------------------
    /**
        Destroy a reference.

        The corresponding Lua registry reference will be released.

        @note If the state refers to a thread, it is the responsibility of the
              caller to ensure that the thread still exists when the LuaRef
              is destroyed.
    */
    ~LuaRef() { luaL_unref(m_L, LUA_REGISTRYINDEX, m_ref); }

    //----------------------------------------------------------------------------
    /**
        Return a reference to a top Lua stack item.
        The stack item is not popped.

        @param L A Lua state.
        @returns A reference to a value on the top of a Lua stack.
    */
    static LuaRef fromStack(lua_State* L) { return LuaRef(L, FromStack()); }

    //----------------------------------------------------------------------------
    /**
        Return a reference to a Lua stack item with a specified index.
        The stack item is not removed.

        @param L     A Lua state.
        @param index An index in the Lua stack.
        @returns A reference to a value in a Lua stack.
    */
    static LuaRef fromStack(lua_State* L, int index)
    {
        lua_pushvalue(L, index);
        return LuaRef(L, FromStack());
    }

    //----------------------------------------------------------------------------
    /**
        Create a new empty table on the top of a Lua stack
        and return a reference to it.

        @param L A Lua state.
        @returns A reference to the newly created table.
        @see luabridge::newTable()
    */
    static LuaRef newTable(lua_State* L)
    {
        lua_newtable(L);
        return LuaRef(L, FromStack());
    }

    //----------------------------------------------------------------------------
    /**
        Return a reference to a named global Lua variable.

        @param L    A Lua state.
        @param name The name of a global variable.
        @returns A reference to the Lua variable.
        @see luabridge::getGlobal()
    */
    static LuaRef getGlobal(lua_State* L, char const* name)
    {
        lua_getglobal(L, name);
        return LuaRef(L, FromStack());
    }

    //----------------------------------------------------------------------------
    /**
        Assign another LuaRef to this LuaRef.

        @param rhs A reference to assign from.
        @returns This reference.
    */
    LuaRef& operator=(LuaRef const& rhs)
    {
        LuaRef ref(rhs);
        swap(ref);
        return *this;
    }

    //----------------------------------------------------------------------------
    /**
        Assign a table item reference.

        @param rhs A table item reference.
        @returns This reference.
    */
    LuaRef& operator=(LuaRef::TableItem const& rhs)
    {
        LuaRef ref(rhs);
        swap(ref);
        return *this;
    }

    //----------------------------------------------------------------------------
    /**
      Assign nil to this reference.

      @returns This reference.
    */
    LuaRef& operator=(LuaNil const&)
    {
        LuaRef ref(m_L);
        swap(ref);
        return *this;
    }

    //----------------------------------------------------------------------------
    /**
        Assign a different value to this reference.

        @param rhs A value to assign.
        @returns This reference.
    */
    template<class T>
    LuaRef& operator=(T rhs)
    {
        LuaRef ref(m_L, rhs);
        swap(ref);
        return *this;
    }

    //----------------------------------------------------------------------------
    /**
        Place the object onto the Lua stack.
    */
    using LuaRefBase::push;

    void push() const { lua_rawgeti(m_L, LUA_REGISTRYINDEX, m_ref); }

    //----------------------------------------------------------------------------
    /**
        Pop the top of Lua stack and assign the ref to m_ref
    */
    void pop()
    {
        luaL_unref(m_L, LUA_REGISTRYINDEX, m_ref);
        m_ref = luaL_ref(m_L, LUA_REGISTRYINDEX);
    }

    //----------------------------------------------------------------------------
    /**
        Access a table value using a key.
        This invokes metamethods.

        @param key A key in the table.
        @returns A reference to the table item.
    */
    template<class T>
    TableItem operator[](T key) const
    {
        Stack<T>::push(m_L, key);
        return TableItem(m_L, m_ref);
    }

    //--------------------------------------------------------------------------
    /**
        Access a table value using a key.
        The operation is raw, metamethods are not invoked. The result is
        passed by value and may not be modified.

        @param key A key in the table.
        @returns A reference to the table item.
    */
    template<class T>
    LuaRef rawget(T key) const
    {
        StackPop(m_L, 1);
        push(m_L);
        Stack<T>::push(m_L, key);
        lua_rawget(m_L, -2);
        return LuaRef(m_L, FromStack());
    }

private:
    void swap(LuaRef& other)
    {
        std::swap(m_L, other.m_L);
        std::swap(m_ref, other.m_ref);
    }

    int m_ref;
};

//------------------------------------------------------------------------------
/**
 * Stack specialization for `LuaRef`.
 */
template<>
struct Stack<LuaRef>
{
    // The value is const& to prevent a copy construction.
    //
    static void push(lua_State* L, LuaRef const& v) { v.push(L); }

    static LuaRef get(lua_State* L, int index) { return LuaRef::fromStack(L, index); }
};

//------------------------------------------------------------------------------
/**
 * Stack specialization for `TableItem`.
 */
template<>
struct Stack<LuaRef::TableItem>
{
    // The value is const& to prevent a copy construction.
    //
    static void push(lua_State* L, LuaRef::TableItem const& v) { v.push(L); }
};

//------------------------------------------------------------------------------
/**
    Create a reference to a new, empty table.

    This is a syntactic abbreviation for LuaRef::newTable ().
*/
inline LuaRef newTable(lua_State* L)
{
    return LuaRef::newTable(L);
}

//------------------------------------------------------------------------------
/**
    Create a reference to a value in the global table.

    This is a syntactic abbreviation for LuaRef::getGlobal ().
*/
inline LuaRef getGlobal(lua_State* L, char const* name)
{
    return LuaRef::getGlobal(L, name);
}

//------------------------------------------------------------------------------

// more C++-like cast syntax
//
template<class T>
T LuaRef_cast(LuaRef const& lr)
{
    return lr.cast<T>();
}

} // namespace luabridge

// End File: Source/LuaBridge/detail/LuaRef.h

// Begin File: Source/LuaBridge/detail/Iterator.h

// https://github.com/kunitoki/LuaBridge
// Copyright 2020, Lucio Asnaghi
// Copyright 2018, Dmitry Tarakanov
// Copyright 2012, Vinnie Falco <vinnie.falco@gmail.com>
// SPDX-License-Identifier: MIT




namespace luabridge {

/** Allows table iteration.
 */
class Iterator
{
    lua_State* m_L;
    LuaRef m_table;
    LuaRef m_key;
    LuaRef m_value;

    void next()
    {
        m_table.push();
        m_key.push();
        if (lua_next(m_L, -2))
        {
            m_value.pop();
            m_key.pop();
        }
        else
        {
            m_key = LuaNil();
            m_value = LuaNil();
        }
        lua_pop(m_L, 1);
    }

public:
    explicit Iterator(const LuaRef& table, bool isEnd = false)
        : m_L(table.state())
        , m_table(table)
        , m_key(table.state()) // m_key is nil
        , m_value(table.state()) // m_value is nil
    {
        if (!isEnd)
        {
            next(); // get the first (key, value) pair from table
        }
    }

    /// Return an associated Lua state.
    ///
    /// @returns A Lua state.
    ///
    lua_State* state() const { return m_L; }

    /// Dereference the iterator.
    ///
    /// @returns A key-value pair for a current table entry.
    ///
    std::pair<LuaRef, LuaRef> operator*() const { return std::make_pair(m_key, m_value); }

    /// Return the value referred by the iterator.
    ///
    /// @returns A value for the current table entry.
    ///
    LuaRef operator->() const { return m_value; }

    /// Compare two iterators.
    ///
    /// @param rhs Another iterator.
    /// @returns True if iterators point to the same entry of the same table,
    ///         false otherwise.
    ///
    bool operator!=(const Iterator& rhs) const
    {
        assert(m_L == rhs.m_L);
        return !m_table.rawequal(rhs.m_table) || !m_key.rawequal(rhs.m_key);
    }

    /// Move the iterator to the next table entry.
    ///
    /// @returns This iterator.
    ///
    Iterator& operator++()
    {
        if (isNil())
        {
            // if the iterator reaches the end, do nothing
            return *this;
        }
        else
        {
            next();
            return *this;
        }
    }

    /// Check if the iterator points after the last table entry.
    ///
    /// @returns True if there are no more table entries to iterate,
    ///         false otherwise.
    ///
    bool isNil() const { return m_key.isNil(); }

    /// Return the key for the current table entry.
    ///
    /// @returns A reference to the entry key.
    ///
    LuaRef key() const { return m_key; }

    /// Return the key for the current table entry.
    ///
    /// @returns A reference to the entry value.
    ///
    LuaRef value() const { return m_value; }

private:
    // Don't use postfix increment, it is less efficient
    Iterator operator++(int);
};

namespace detail {

class Range
{
    Iterator m_begin;
    Iterator m_end;

public:
    Range(const Iterator& begin, const Iterator& end) : m_begin(begin), m_end(end) {}

    const Iterator& begin() const { return m_begin; }
    const Iterator& end() const { return m_end; }
};

} // namespace detail

/// Return a range for the Lua table reference.
///
/// @returns A range suitable for range-based for statement.
///
inline detail::Range pairs(const LuaRef& table)
{
    return detail::Range(Iterator(table, false), Iterator(table, true));
}

} // namespace luabridge

// End File: Source/LuaBridge/detail/Iterator.h

// Begin File: Source/LuaBridge/detail/Security.h

// https://github.com/vinniefalco/LuaBridge
// Copyright 2012, Vinnie Falco <vinnie.falco@gmail.com>
// SPDX-License-Identifier: MIT



namespace luabridge {

//------------------------------------------------------------------------------
/**
security options.
*/
class Security
{
public:
    static bool hideMetatables() { return getSettings().hideMetatables; }

    static void setHideMetatables(bool shouldHide) { getSettings().hideMetatables = shouldHide; }

private:
    struct Settings
    {
        Settings() : hideMetatables(true) {}

        bool hideMetatables;
    };

    static Settings& getSettings()
    {
        static Settings settings;
        return settings;
    }
};

//------------------------------------------------------------------------------
/**
Set a global value in the lua_State.

@note This works on any type specialized by `Stack`, including `LuaRef` and
its table proxies.
*/
template<class T>
inline void setGlobal(lua_State* L, T t, char const* name)
{
    push(L, t);
    lua_setglobal(L, name);
}

//------------------------------------------------------------------------------
/**
Change whether or not metatables are hidden (on by default).
*/
inline void setHideMetatables(bool shouldHide)
{
    Security::setHideMetatables(shouldHide);
}

} // namespace luabridge

// End File: Source/LuaBridge/detail/Security.h

// Begin File: Source/LuaBridge/detail/Namespace.h

// https://github.com/kunitoki/LuaBridge
// Copyright 2020, Lucio Asnaghi
// Copyright 2019, Dmitry Tarakanov
// Copyright 2012, Vinnie Falco <vinnie.falco@gmail.com>
// Copyright 2007, Nathan Reed
// SPDX-License-Identifier: MIT




namespace luabridge {
namespace detail {

/**
 * Base for class and namespace registration.
 * Maintains Lua stack in the proper state.
 * Once beginNamespace, beginClass or deriveClass is called the parent
 * object upon its destruction may no longer clear the Lua stack.
 * Then endNamespace or endClass is called, a new parent is created
 * and the child transfers the responsibility for clearing stack to it.
 * So there can be maximum one "active" registrar object.
 */
class Registrar
{
protected:
    lua_State* const L;
    int mutable m_stackSize;

    Registrar(lua_State* L) : L(L), m_stackSize(0) {}

    Registrar(const Registrar& rhs) : L(rhs.L), m_stackSize(rhs.m_stackSize)
    {
        rhs.m_stackSize = 0;
    }

#ifndef _MSC_VER
    // MS compiler thinks it's the 2nd copy ctor
    Registrar(Registrar& rhs) : L(rhs.L), m_stackSize(rhs.m_stackSize) { rhs.m_stackSize = 0; }
#endif // ifndef _MSC_VER

    Registrar& operator=(const Registrar& rhs)
    {
        Registrar tmp(rhs);
        std::swap(m_stackSize, tmp.m_stackSize);
        return *this;
    }

    ~Registrar()
    {
        if (m_stackSize > 0)
        {
            assert(m_stackSize <= lua_gettop(L));
            lua_pop(L, m_stackSize);
        }
    }

    void assertIsActive() const
    {
        if (m_stackSize == 0)
        {
            throw std::logic_error("Unable to continue registration");
        }
    }
};

} // namespace detail

/** Provides C++ to Lua registration capabilities.

    This class is not instantiated directly, call `getGlobalNamespace` to start
    the registration process.
*/
class Namespace : public detail::Registrar
{
    typedef detail::CFunc CFunc;

    //============================================================================
#if 0
  /**
    Error reporting.

    VF: This function looks handy, why aren't we using it?
  */
  static int luaError (lua_State* L, std::string message)
  {
    assert (lua_isstring (L, lua_upvalueindex (1)));
    std::string s;

    // Get information on the caller's caller to format the message,
    // so the error appears to originate from the Lua source.
    lua_Debug ar;
    int result = lua_getstack (L, 2, &ar);
    if (result != 0)
    {
      lua_getinfo (L, "Sl", &ar);
      s = ar.short_src;
      if (ar.currentline != -1)
      {
        // poor mans int to string to avoid <strstrream>.
        lua_pushnumber (L, ar.currentline);
        s = s + ":" + lua_tostring (L, -1) + ": ";
        lua_pop (L, 1);
      }
    }

    s = s + message;

    return luaL_error (L, s.c_str ());
  }
#endif

    /**
      Factored base to reduce template instantiations.
    */
    class ClassBase : public detail::Registrar
    {
    public:
        explicit ClassBase(Namespace& parent) : Registrar(parent) {}

        using Registrar::operator=;

    protected:
        //--------------------------------------------------------------------------
        /**
          Create the const table.
        */
        void createConstTable(const char* name, bool trueConst = true)
        {
            assert(name != nullptr);
            
            std::string type_name = std::string(trueConst ? "const " : "") + name;

            // Stack: namespace table (ns)
            lua_newtable(L); // Stack: ns, const table (co)
            lua_pushvalue(L, -1); // Stack: ns, co, co
            lua_setmetatable(L, -2); // co.__metatable = co. Stack: ns, co

            lua_pushstring(L, type_name.c_str());
            lua_rawsetp(L, -2, detail::getTypeKey()); // co [typeKey] = name. Stack: ns, co

            lua_pushcfunction(L, &CFunc::indexMetaMethod);
            rawsetfield(L, -2, "__index");

            lua_pushcfunction(L, &CFunc::newindexObjectMetaMethod);
            rawsetfield(L, -2, "__newindex");

            lua_newtable(L);
            lua_rawsetp(L, -2, detail::getPropgetKey());

            if (Security::hideMetatables())
            {
                lua_pushnil(L);
                rawsetfield(L, -2, "__metatable");
            }
        }

        //--------------------------------------------------------------------------
        /**
          Create the class table.

          The Lua stack should have the const table on top.
        */
        void createClassTable(char const* name)
        {
            assert(name != nullptr);

            // Stack: namespace table (ns), const table (co)

            // Class table is the same as const table except the propset table
            createConstTable(name, false); // Stack: ns, co, cl

            lua_newtable(L); // Stack: ns, co, cl, propset table (ps)
            lua_rawsetp(L, -2, detail::getPropsetKey()); // cl [propsetKey] = ps. Stack: ns, co, cl

            lua_pushvalue(L, -2); // Stack: ns, co, cl, co
            lua_rawsetp(L, -2, detail::getConstKey()); // cl [constKey] = co. Stack: ns, co, cl

            lua_pushvalue(L, -1); // Stack: ns, co, cl, cl
            lua_rawsetp(L, -3, detail::getClassKey()); // co [classKey] = cl. Stack: ns, co, cl
        }

        //--------------------------------------------------------------------------
        /**
          Create the static table.
        */
        void createStaticTable(char const* name)
        {
            assert(name != nullptr);

            // Stack: namespace table (ns), const table (co), class table (cl)
            lua_newtable(L); // Stack: ns, co, cl, visible static table (vst)
            lua_newtable(L); // Stack: ns, co, cl, st, static metatable (st)
            lua_pushvalue(L, -1); // Stack: ns, co, cl, vst, st, st
            lua_setmetatable(L, -3); // st.__metatable = mt. Stack: ns, co, cl, vst, st
            lua_insert(L, -2); // Stack: ns, co, cl, st, vst
            rawsetfield(L, -5, name); // ns [name] = vst. Stack: ns, co, cl, st

#if 0
            lua_pushlightuserdata (L, this);
            lua_pushcclosure (L, &tostringMetaMethod, 1);
            rawsetfield (L, -2, "__tostring");
#endif

            lua_pushcfunction(L, &CFunc::indexMetaMethod);
            rawsetfield(L, -2, "__index");

            lua_pushcfunction(L, &CFunc::newindexStaticMetaMethod);
            rawsetfield(L, -2, "__newindex");

            lua_newtable(L); // Stack: ns, co, cl, st, proget table (pg)
            lua_rawsetp(L, -2, detail::getPropgetKey()); // st [propgetKey] = pg. Stack: ns, co, cl, st

            lua_newtable(L); // Stack: ns, co, cl, st, propset table (ps)
            lua_rawsetp(L, -2, detail::getPropsetKey()); // st [propsetKey] = pg. Stack: ns, co, cl, st

            lua_pushvalue(L, -2); // Stack: ns, co, cl, st, cl
            lua_rawsetp(L, -2, detail::getClassKey()); // st [classKey] = cl. Stack: ns, co, cl, st

            if (Security::hideMetatables())
            {
                lua_pushnil(L);
                rawsetfield(L, -2, "__metatable");
            }
        }

        //==========================================================================
        /**
          lua_CFunction to construct a class object wrapped in a container.
        */
        template<class Params, class C>
        static int ctorContainerProxy(lua_State* L)
        {
            using T = typename ContainerTraits<C>::Type;
            
            detail::ArgList<Params, 2> args(L);
            T* const p = detail::Constructor<T, Params>::call(args);
            detail::UserdataSharedHelper<C, false>::push(L, p);

            return 1;
        }

        //--------------------------------------------------------------------------
        /**
          lua_CFunction to construct a class object in-place in the userdata.
        */
        template<class Params, class T>
        static int ctorPlacementProxy(lua_State* L)
        {
            detail::ArgList<Params, 2> args(L);
            detail::UserdataValue<T>* value = detail::UserdataValue<T>::place(L);
            detail::Constructor<T, Params>::call(value->getObject(), args);
            value->commit();
            return 1;
        }

        void assertStackState() const
        {
            // Stack: const table (co), class table (cl), static table (st)
            assert(lua_istable(L, -3));
            assert(lua_istable(L, -2));
            assert(lua_istable(L, -1));
        }
    };

    //============================================================================
    //
    // Class
    //
    //============================================================================
    /**
      Provides a class registration in a lua_State.

      After construction the Lua stack holds these objects:
        -1 static table
        -2 class table
        -3 const table
        -4 enclosing namespace table
    */
    template<class T>
    class Class : public ClassBase
    {
        typedef detail::CFunc CFunc;

    public:
        //==========================================================================
        /**
          Register a new class or add to an existing class registration.

          @param name   The new class name.
          @param parent A parent namespace object.
        */
        Class(char const* name, Namespace& parent) : ClassBase(parent)
        {
            assert(name != nullptr);
            assert(lua_istable(L, -1)); // Stack: namespace table (ns)

            rawgetfield(L, -1, name); // Stack: ns, static table (st) | nil

            if (lua_isnil(L, -1)) // Stack: ns, nil
            {
                lua_pop(L, 1); // Stack: ns

                createConstTable(name); // Stack: ns, const table (co)
                lua_pushcfunction(L, &CFunc::gcMetaMethod<T>); // Stack: ns, co, function
                rawsetfield(L, -2, "__gc"); // co ["__gc"] = function. Stack: ns, co
                ++m_stackSize;

                createClassTable(name); // Stack: ns, co, class table (cl)
                lua_pushcfunction(L, &CFunc::gcMetaMethod<T>); // Stack: ns, co, cl, function
                rawsetfield(L, -2, "__gc"); // cl ["__gc"] = function. Stack: ns, co, cl
                ++m_stackSize;

                createStaticTable(name); // Stack: ns, co, cl, st
                ++m_stackSize;

                // Map T back to its tables.
                lua_pushvalue(L, -1); // Stack: ns, co, cl, st, st
                lua_rawsetp(L, LUA_REGISTRYINDEX, detail::getStaticRegistryKey<T>()); // Stack: ns, co, cl, st
                lua_pushvalue(L, -2); // Stack: ns, co, cl, st, cl
                lua_rawsetp(L, LUA_REGISTRYINDEX, detail::getClassRegistryKey<T>()); // Stack: ns, co, cl, st
                lua_pushvalue(L, -3); // Stack: ns, co, cl, st, co
                lua_rawsetp(L, LUA_REGISTRYINDEX, detail::getConstRegistryKey<T>()); // Stack: ns, co, cl, st
            }
            else
            {
                assert(lua_istable(L, -1)); // Stack: ns, st
                ++m_stackSize;

                // Map T back from its stored tables

                lua_rawgetp(L, LUA_REGISTRYINDEX, detail::getConstRegistryKey<T>()); // Stack: ns, st, co
                lua_insert(L, -2); // Stack: ns, co, st
                ++m_stackSize;

                lua_rawgetp(L, LUA_REGISTRYINDEX, detail::getClassRegistryKey<T>()); // Stack: ns, co, st, cl
                lua_insert(L, -2); // Stack: ns, co, cl, st
                ++m_stackSize;
            }
        }

        //==========================================================================
        /**
          Derive a new class.

          @param name     The class name.
          @param parent A parent namespace object.
          @param staticKey
        */
        Class(char const* name, Namespace& parent, void const* const staticKey) : ClassBase(parent)
        {
            assert(name != nullptr);
            assert(lua_istable(L, -1)); // Stack: namespace table (ns)

            createConstTable(name); // Stack: ns, const table (co)
            lua_pushcfunction(L, &CFunc::gcMetaMethod<T>); // Stack: ns, co, function
            rawsetfield(L, -2, "__gc"); // co ["__gc"] = function. Stack: ns, co
            ++m_stackSize;

            createClassTable(name); // Stack: ns, co, class table (cl)
            lua_pushcfunction(L, &CFunc::gcMetaMethod<T>); // Stack: ns, co, cl, function
            rawsetfield(L, -2, "__gc"); // cl ["__gc"] = function. Stack: ns, co, cl
            ++m_stackSize;

            createStaticTable(name); // Stack: ns, co, cl, st
            ++m_stackSize;

            lua_rawgetp( L, LUA_REGISTRYINDEX, staticKey); // Stack: ns, co, cl, st, parent st (pst) | nil
            if (lua_isnil(L, -1)) // Stack: ns, co, cl, st, nil
            {
                ++m_stackSize;
                throw std::runtime_error("Base class is not registered");
            }

            assert(lua_istable(L, -1)); // Stack: ns, co, cl, st, pst

            lua_rawgetp(L, -1, detail::getClassKey()); // Stack: ns, co, cl, st, pst, parent cl (pcl)
            assert(lua_istable(L, -1));

            lua_rawgetp(L, -1, detail::getConstKey()); // Stack: ns, co, cl, st, pst, pcl, parent co (pco)
            assert(lua_istable(L, -1));

            lua_rawsetp(L, -6, detail::getParentKey()); // co [parentKey] = pco. Stack: ns, co, cl, st, pst, pcl
            lua_rawsetp(L, -4, detail::getParentKey()); // cl [parentKey] = pcl. Stack: ns, co, cl, st, pst
            lua_rawsetp(L, -2, detail::getParentKey()); // st [parentKey] = pst. Stack: ns, co, cl, st

            lua_pushvalue(L, -1); // Stack: ns, co, cl, st, st
            lua_rawsetp(L, LUA_REGISTRYINDEX, detail::getStaticRegistryKey<T>()); // Stack: ns, co, cl, st
            lua_pushvalue(L, -2); // Stack: ns, co, cl, st, cl
            lua_rawsetp(L, LUA_REGISTRYINDEX, detail::getClassRegistryKey<T>()); // Stack: ns, co, cl, st
            lua_pushvalue(L, -3); // Stack: ns, co, cl, st, co
            lua_rawsetp(L, LUA_REGISTRYINDEX, detail::getConstRegistryKey<T>()); // Stack: ns, co, cl, st
        }

        //--------------------------------------------------------------------------
        /**
          Continue registration in the enclosing namespace.

          @returns A parent registration object.
        */
        Namespace endClass()
        {
            assert(m_stackSize > 3);

            m_stackSize -= 3;
            lua_pop(L, 3);
            return Namespace(*this);
        }

        //--------------------------------------------------------------------------
        /**
          Add or replace a static property.

          @tparam U          The type of the property.
          @param  name       The property name.
          @param  value      A property value pointer.
          @param  isWritable True for a read-write, false for read-only property.
          @returns This class registration object.
        */
        template <class U>
        Class<T>& addStaticProperty(char const* name, U* value, bool isWritable = true)
        {
            return addStaticData(name, value, isWritable);
        }

        //--------------------------------------------------------------------------
        /**
          Add or replace a static property.

          @tparam U          The type of the property.
          @param  name       The property name.
          @param  value      A property value pointer.
          @param  isWritable True for a read-write, false for read-only property.
          @returns This class registration object.
        */
        template <class U>
        Class<T>& addStaticData(char const* name, U* value, bool isWritable = true)
        {
            assert(name != nullptr);
            assertStackState(); // Stack: const table (co), class table (cl), static table (st)

            lua_pushlightuserdata(L, value); // Stack: co, cl, st, pointer
            lua_pushcclosure(L, &CFunc::getVariable<U>, 1); // Stack: co, cl, st, getter
            CFunc::addGetter(L, name, -2); // Stack: co, cl, st

            if (isWritable)
            {
                lua_pushlightuserdata(L, value); // Stack: co, cl, st, ps, pointer
                lua_pushcclosure(L, &CFunc::setVariable<U>, 1); // Stack: co, cl, st, ps, setter
            }
            else
            {
                lua_pushstring(L, name); // Stack: co, cl, st, name
                lua_pushcclosure(L, &CFunc::readOnlyError, 1); // Stack: co, cl, st, error_fn
            }

            CFunc::addSetter(L, name, -2); // Stack: co, cl, st

            return *this;
        }

        //--------------------------------------------------------------------------
        /// Add or replace a static property member.
        ///
        /// @tparam U          The type of the property.
        /// @param  name       The property name.
        /// @param  get        A property getter function pointer.
        /// @param  set        A property setter function pointer, optional, nullable.
        ///                    Omit or pass nullptr for a read-only property.
        /// @returns This class registration object.
        ///
        template <class U>
        Class<T>& addStaticProperty(char const* name, U (*get)(), void (*set)(U) = nullptr)
        {
            assert(name != nullptr);
            assertStackState(); // Stack: const table (co), class table (cl), static table (st)

            lua_pushlightuserdata(L, reinterpret_cast<void*>(get)); // Stack: co, cl, st, function ptr
            lua_pushcclosure(L, &CFunc::Call<U (*)()>::f, 1); // Stack: co, cl, st, getter
            CFunc::addGetter(L, name, -2); // Stack: co, cl, st

            if (set != nullptr)
            {
                lua_pushlightuserdata(L, reinterpret_cast<void*>(set)); // Stack: co, cl, st, function ptr
                lua_pushcclosure(L, &CFunc::Call<void (*)(U)>::f, 1); // Stack: co, cl, st, setter
            }
            else
            {
                lua_pushstring(L, name); // Stack: co, cl, st, ps, name
                lua_pushcclosure(L, &CFunc::readOnlyError, 1); // Stack: co, cl, st, error_fn
            }

            CFunc::addSetter(L, name, -2); // Stack: co, cl, st

            return *this;
        }

        //--------------------------------------------------------------------------
        /**
          Add or replace a static member function.
        */
        template <class FP>
        Class<T>& addStaticFunction(char const* name, FP const fp)
        {
            assert(name != nullptr);
            assertStackState(); // Stack: const table (co), class table (cl), static table (st)

            lua_pushlightuserdata(L, reinterpret_cast<void*>(fp)); // Stack: co, cl, st, function ptr
            lua_pushcclosure(L, &CFunc::Call<FP>::f, 1); // co, cl, st, function
            rawsetfield(L, -2, name); // co, cl, st

            return *this;
        }

        //--------------------------------------------------------------------------
        /**
          Add or replace a static member function by std::function.
        */
        template <class ReturnType, class... Params>
        Class<T>& addStaticFunction(char const* name, std::function<ReturnType(Params...)> function)
        {
            using FnType = decltype(function);

            assert(name != nullptr);
            assertStackState(); // Stack: const table (co), class table (cl), static table (st)

            lua_newuserdata_aligned<FnType>(L, std::move(function)); // Stack: co, cl, st, function userdata (ud)
            lua_newtable(L); // Stack: co, cl, st, ud, ud metatable (mt)
            lua_pushcfunction(L, &CFunc::gcMetaMethodAny<FnType>); // Stack: co, cl, st, ud, mt, gc function
            rawsetfield(L, -2, "__gc"); // Stack: co, cl, st, ud, mt
            lua_setmetatable(L, -2); // Stack: co, cl, st, ud
            lua_pushcclosure(L, &CFunc::CallProxyFunctor<FnType>::f, 1); // Stack: co, cl, st, function
            rawsetfield(L, -2, name); // Stack: co, cl, st

            return *this;
        }

        //--------------------------------------------------------------------------
        /**
          Add or replace a lua_CFunction.

          @param name The name of the function.
          @param fp   A C-function pointer.
          @returns This class registration object.
        */
        Class<T>& addStaticFunction(char const* name, int (*const fp)(lua_State*))
        {
            return addStaticCFunction(name, fp);
        }

        //--------------------------------------------------------------------------
        /**
          Add or replace a lua_CFunction.
        */
        Class<T>& addStaticCFunction(char const* name, int (*const fp)(lua_State*))
        {
            assert(name != nullptr);
            assertStackState(); // Stack: const table (co), class table (cl), static table (st)

            lua_pushcfunction(L, fp); // co, cl, st, function
            rawsetfield(L, -2, name); // co, cl, st

            return *this;
        }

        //--------------------------------------------------------------------------
        template <class U>
        Class<T>& addProperty(char const* name, U T::*mp, bool isWritable = true)
        {
            return addData(name, mp, isWritable);
        }

        //--------------------------------------------------------------------------
        /**
          Add or replace a data member.
        */
        template <class U>
        Class<T>& addData(char const* name, U T::*mp, bool isWritable = true)
        {
            typedef const U T::*mp_t;

            assert(name != nullptr);
            assertStackState(); // Stack: const table (co), class table (cl), static table (st)

            new (lua_newuserdata(L, sizeof(mp_t))) mp_t(mp); // Stack: co, cl, st, field ptr
            lua_pushcclosure(L, &CFunc::getProperty<T, U>, 1); // Stack: co, cl, st, getter
            lua_pushvalue(L, -1); // Stack: co, cl, st, getter, getter
            CFunc::addGetter(L, name, -5); // Stack: co, cl, st, getter
            CFunc::addGetter(L, name, -3); // Stack: co, cl, st

            if (isWritable)
            {
                new (lua_newuserdata(L, sizeof(mp_t))) mp_t(mp); // Stack: co, cl, st, field ptr
                lua_pushcclosure(L, &CFunc::setProperty<T, U>, 1); // Stack: co, cl, st, setter
                CFunc::addSetter(L, name, -3); // Stack: co, cl, st
            }

            return *this;
        }

        //--------------------------------------------------------------------------
        /**
          Add or replace a property member.
        */
        template <class TG, class TS = TG>
        Class<T>& addProperty(char const* name, TG (T::*get)() const, void (T::*set)(TS) = nullptr)
        {
            typedef TG (T::*get_t)() const;
            typedef void (T::*set_t)(TS);

            assert(name != nullptr);
            assertStackState(); // Stack: const table (co), class table (cl), static table (st)

            new (lua_newuserdata(L, sizeof(get_t))) get_t(get); // Stack: co, cl, st, funcion ptr
            lua_pushcclosure(L, &CFunc::CallConstMember<get_t, T>::f, 1); // Stack: co, cl, st, getter
            lua_pushvalue(L, -1); // Stack: co, cl, st, getter, getter
            CFunc::addGetter(L, name, -5); // Stack: co, cl, st, getter
            CFunc::addGetter(L, name, -3); // Stack: co, cl, st

            if (set != 0)
            {
                new (lua_newuserdata(L, sizeof(set_t))) set_t(set); // Stack: co, cl, st, function ptr
                lua_pushcclosure(L, &CFunc::CallMember<set_t, T>::f, 1); // Stack: co, cl, st, setter
                CFunc::addSetter(L, name, -3); // Stack: co, cl, st
            }

            return *this;
        }

        //--------------------------------------------------------------------------
        /**
          Add or replace a property member.
        */
        template <class TG, class TS = TG>
        Class<T>& addProperty(char const* name, TG (T::*get)(lua_State*) const, void (T::*set)(TS, lua_State*) = nullptr)
        {
            typedef TG (T::*get_t)(lua_State*) const;
            typedef void (T::*set_t)(TS, lua_State*);

            assert(name != nullptr);
            assertStackState(); // Stack: const table (co), class table (cl), static table (st)

            new (lua_newuserdata(L, sizeof(get_t))) get_t(get); // Stack: co, cl, st, funcion ptr
            lua_pushcclosure(L, &CFunc::CallConstMember<get_t, T>::f, 1); // Stack: co, cl, st, getter
            lua_pushvalue(L, -1); // Stack: co, cl, st, getter, getter
            CFunc::addGetter(L, name, -5); // Stack: co, cl, st, getter
            CFunc::addGetter(L, name, -3); // Stack: co, cl, st

            if (set != nullptr)
            {
                new (lua_newuserdata(L, sizeof(set_t))) set_t(set); // Stack: co, cl, st, function ptr
                lua_pushcclosure(L, &CFunc::CallMember<set_t, T>::f, 1); // Stack: co, cl, st, setter
                CFunc::addSetter(L, name, -3); // Stack: co, cl, st
            }

            return *this;
        }

        //--------------------------------------------------------------------------
        /**
          Add or replace a property member, by proxy.

          When a class is closed for modification and does not provide (or cannot
          provide) the function signatures necessary to implement get or set for
          a property, this will allow non-member functions act as proxies.

          Both the get and the set functions require a T const* and T* in the first
          argument respectively.
        */
        template <class TG, class TS = TG>
        Class<T>& addProperty(char const* name, TG (*get)(T const*), void (*set)(T*, TS) = nullptr)
        {
            assert(name != nullptr);
            assertStackState(); // Stack: const table (co), class table (cl), static table (st)

            lua_pushlightuserdata(L, reinterpret_cast<void*>(get)); // Stack: co, cl, st, function ptr
            lua_pushcclosure(L, &CFunc::Call<TG (*)(const T*)>::f, 1); // Stack: co, cl, st, getter
            lua_pushvalue(L, -1); // Stack: co, cl, st,, getter, getter
            CFunc::addGetter(L, name, -5); // Stack: co, cl, st, getter
            CFunc::addGetter(L, name, -3); // Stack: co, cl, st

            if (set != nullptr)
            {
                lua_pushlightuserdata( L, reinterpret_cast<void*>(set)); // Stack: co, cl, st, function ptr
                lua_pushcclosure(L, &CFunc::Call<void (*)(T*, TS)>::f, 1); // Stack: co, cl, st, setter
                CFunc::addSetter(L, name, -3); // Stack: co, cl, st
            }

            return *this;
        }

        //--------------------------------------------------------------------------
        /**
          Add or replace a property member, by proxy C-function.

          When a class is closed for modification and does not provide (or cannot
          provide) the function signatures necessary to implement get or set for
          a property, this will allow non-member functions act as proxies.

          The object userdata ('this') value is at the index 1.
          The new value for set function is at the index 2.
        */
        Class<T>& addProperty(char const* name, int (*get)(lua_State*), int (*set)(lua_State*) = nullptr)
        {
            assert(name != nullptr);
            assertStackState(); // Stack: const table (co), class table (cl), static table (st)

            lua_pushcfunction(L, get);
            lua_pushvalue(L, -1); // Stack: co, cl, st,, getter, getter
            CFunc::addGetter(L, name, -5); // Stack: co, cl, st,, getter
            CFunc::addGetter(L, name, -3); // Stack: co, cl, st,

            if (set != nullptr)
            {
                lua_pushcfunction(L, set);
                CFunc::addSetter(L, name, -3); // Stack: co, cl, st,
            }

            return *this;
        }

        template <class TG, class TS = TG>
        Class<T>& addProperty(char const* name, std::function<TG(const T*)> get, std::function<void(T*, TS)> set = nullptr)
        {
            using GetType = decltype(get);

            assert(name != nullptr);
            assertStackState(); // Stack: const table (co), class table (cl), static table (st)

            lua_newuserdata_aligned<GetType>(L, std::move(get)); // Stack: co, cl, st, function userdata (ud)
            lua_newtable(L); // Stack: co, cl, st, ud, ud metatable (mt)
            lua_pushcfunction(L, &CFunc::gcMetaMethodAny<GetType>); // Stack: co, cl, st, ud, mt, gc function
            rawsetfield(L, -2, "__gc"); // Stack: co, cl, st, ud, mt
            lua_setmetatable(L, -2); // Stack: co, cl, st, ud
            lua_pushcclosure(L, &CFunc::CallProxyFunctor<GetType>::f, 1); // Stack: co, cl, st, getter
            lua_pushvalue(L, -1); // Stack: co, cl, st, getter, getter
            CFunc::addGetter(L, name, -4); // Stack: co, cl, st, getter
            CFunc::addGetter(L, name, -4); // Stack: co, cl, st

            if (set != nullptr)
            {
                using SetType = decltype(set);

                lua_newuserdata_aligned<SetType>(L, std::move(set)); // Stack: co, cl, st, function userdata (ud)
                lua_newtable(L); // Stack: co, cl, st, ud, ud metatable (mt)
                lua_pushcfunction(L, &CFunc::gcMetaMethodAny<SetType>); // Stack: co, cl, st, ud, mt, gc function
                rawsetfield(L, -2, "__gc"); // Stack: co, cl, st, ud, mt
                lua_setmetatable(L, -2); // Stack: co, cl, st, ud
                lua_pushcclosure(L, &CFunc::CallProxyFunctor<SetType>::f, 1); // Stack: co, cl, st, setter
                CFunc::addSetter(L, name, -3); // Stack: co, cl, st
            }

            return *this;
        }

        //--------------------------------------------------------------------------
        /**
            Add or replace a member function by std::function.
        */
        template <class ReturnType, class... Params>
        Class<T>& addFunction(char const* name, std::function<ReturnType(T*, Params...)> function)
        {
            using FnType = decltype(function);

            assert(name != nullptr);
            assertStackState(); // Stack: const table (co), class table (cl), static table (st)

            lua_newuserdata_aligned<FnType>(L, std::move(function)); // Stack: co, cl, st, function userdata (ud)
            lua_newtable(L); // Stack: co, cl, st, ud, ud metatable (mt)
            lua_pushcfunction(L, &CFunc::gcMetaMethodAny<FnType>); // Stack: co, cl, st, ud, mt, gc function
            rawsetfield(L, -2, "__gc"); // Stack: co, cl, st, ud, mt
            lua_setmetatable(L, -2); // Stack: co, cl, st, ud
            lua_pushcclosure(L, &CFunc::CallProxyFunctor<FnType>::f, 1); // Stack: co, cl, st, function
            rawsetfield(L, -3, name); // Stack: co, cl, st

            return *this;
        }

        //--------------------------------------------------------------------------
        /**
            Add or replace a const member function by std::function.
        */
        template <class ReturnType, class... Params>
        Class<T>& addFunction(char const* name, std::function<ReturnType(const T*, Params...)> function)
        {
            using FnType = decltype(function);

            assert(name != nullptr);
            assertStackState(); // Stack: const table (co), class table (cl), static table (st)

            lua_newuserdata_aligned<FnType>(L, std::move(function)); // Stack: co, cl, st, function userdata (ud)
            lua_newtable(L); // Stack: co, cl, st, ud, ud metatable (mt)
            lua_pushcfunction(L, &CFunc::gcMetaMethodAny<FnType>); // Stack: co, cl, st, ud, mt, gc function
            rawsetfield(L, -2, "__gc"); // Stack: co, cl, st, ud, mt
            lua_setmetatable(L, -2); // Stack: co, cl, st, ud
            lua_pushcclosure(L, &CFunc::CallProxyFunctor<FnType>::f, 1); // Stack: co, cl, st, function
            lua_pushvalue(L, -1); // Stack: co, cl, st, function, function
            rawsetfield(L, -4, name); // Stack: co, cl, st, function
            rawsetfield(L, -4, name); // Stack: co, cl, st

            return *this;
        }

        //--------------------------------------------------------------------------
        /**
            Add or replace a member function.
        */
        template <class U, class ReturnType, class... Params>
        Class<T>& addFunction(char const* name, ReturnType (U::*mf)(Params...))
        {
            static_assert(std::is_base_of_v<U, T>);

            using MemFn = ReturnType (U::*)(Params...);

            assert(name != nullptr);
            assertStackState(); // Stack: const table (co), class table (cl), static table (st)

            static const std::string GC = "__gc";
            if (name == GC)
                throw std::logic_error(GC + " metamethod registration is forbidden");

            CFunc::CallMemberFunctionHelper<MemFn, T, false>::add(L, name, mf);
            return *this;
        }

        template <class U, class ReturnType, class... Params>
        Class<T>& addFunction(char const* name, ReturnType (U::*mf)(Params...) const)
        {
            static_assert(std::is_base_of_v<U, T>);

            using MemFn = ReturnType (U::*)(Params...) const;

            assert(name != nullptr);
            assertStackState(); // Stack: const table (co), class table (cl), static table (st)

            static const std::string GC = "__gc";
            if (name == GC)
                throw std::logic_error(GC + " metamethod registration is forbidden");

            CFunc::CallMemberFunctionHelper<MemFn, T, true>::add(L, name, mf);
            return *this;
        }

        //--------------------------------------------------------------------------
        /**
            Add or replace a proxy function.
        */
        template <class ReturnType, class... Params>
        Class<T>& addFunction(char const* name, ReturnType (*proxyFn)(T* object, Params...))
        {
            assert(name != nullptr);
            assertStackState(); // Stack: const table (co), class table (cl), static table (st)

            static const std::string GC = "__gc";
            if (name == GC)
                throw std::logic_error(GC + " metamethod registration is forbidden");

            using FnType = decltype(proxyFn);
            lua_pushlightuserdata(L, reinterpret_cast<void*>(proxyFn)); // Stack: co, cl, st, function ptr
            lua_pushcclosure(L, &CFunc::CallProxyFunction<FnType>::f, 1); // Stack: co, cl, st, function
            rawsetfield(L, -3, name); // Stack: co, cl, st
            return *this;
        }

        template <class ReturnType, class... Params>
        Class<T>& addFunction(char const* name, ReturnType (*proxyFn)(const T* object, Params...))
        {
            assert(name != nullptr);
            assertStackState(); // Stack: const table (co), class table (cl), static table (st)

            static const std::string GC = "__gc";
            if (name == GC)
                throw std::logic_error(GC + " metamethod registration is forbidden");

            using FnType = decltype(proxyFn);
            lua_pushlightuserdata(L, reinterpret_cast<void*>(proxyFn)); // Stack: co, cl, st, function ptr
            lua_pushcclosure(L, &CFunc::CallProxyFunction<FnType>::f, 1); // Stack: co, cl, st, function
            lua_pushvalue(L, -1); // Stack: co, cl, st, function, function
            rawsetfield(L, -4, name); // Stack: co, cl, st, function
            rawsetfield(L, -4, name); // Stack: co, cl, st
            return *this;
        }

        //--------------------------------------------------------------------------
        /**
            Add or replace a member lua_CFunction.
        */
        template <class U>
        Class<T>& addFunction(char const* name, int (U::*mfp)(lua_State*))
        {
            static_assert(std::is_base_of_v<U, T>);

            return addCFunction<U>(name, mfp);
        }

        //--------------------------------------------------------------------------
        /**
            Add or replace a member lua_CFunction.
        */
        template <class U>
        Class<T>& addCFunction(char const* name, int (U::*mfp)(lua_State*))
        {
            static_assert(std::is_base_of_v<U, T>);

            typedef int (U::*MFP)(lua_State*);

            assert(name != nullptr);
            assertStackState(); // Stack: const table (co), class table (cl), static table (st)

            new (lua_newuserdata(L, sizeof(mfp))) MFP(mfp); // Stack: co, cl, st, function ptr
            lua_pushcclosure(L, &CFunc::CallMemberCFunction<T>::f, 1); // Stack: co, cl, st, function
            rawsetfield(L, -3, name); // Stack: co, cl, st

            return *this;
        }

        //--------------------------------------------------------------------------
        /**
            Add or replace a const member lua_CFunction.
        */
        template <class U>
        Class<T>& addFunction(char const* name, int (U::*mfp)(lua_State*) const)
        {
            static_assert(std::is_base_of_v<U, T>);

            return addCFunction<U>(name, mfp);
        }

        //--------------------------------------------------------------------------
        /**
            Add or replace a const member lua_CFunction.
        */
        template <class U>
        Class<T>& addCFunction(char const* name, int (U::*mfp)(lua_State*) const)
        {
            static_assert(std::is_base_of_v<U, T>);
            
            typedef int (U::*MFP)(lua_State*) const;

            assert(name != nullptr);
            assertStackState(); // Stack: const table (co), class table (cl), static table (st)

            new (lua_newuserdata(L, sizeof(mfp))) MFP(mfp);
            lua_pushcclosure(L, &CFunc::CallConstMemberCFunction<T>::f, 1);
            lua_pushvalue(L, -1); // Stack: co, cl, st, function, function
            rawsetfield(L, -4, name); // Stack: co, cl, st, function
            rawsetfield(L, -4, name); // Stack: co, cl, st

            return *this;
        }

        //--------------------------------------------------------------------------
        /**
          Add or replace a primary Constructor.

          The primary Constructor is invoked when calling the class type table
          like a function.

          The template parameter should be a function pointer type that matches
          the desired Constructor (since you can't take the address of a Constructor
          and pass it as an argument).
        */
        template <class MemFn, class C>
        Class<T>& addConstructor()
        {
            assertStackState(); // Stack: const table (co), class table (cl), static table (st)

            lua_pushcclosure(L, &ctorContainerProxy<typename detail::FuncTraits<MemFn>::Params, C>, 0);
            rawsetfield(L, -2, "__call");

            return *this;
        }

        template <class MemFn>
        Class<T>& addConstructor()
        {
            assertStackState(); // Stack: const table (co), class table (cl), static table (st)

            lua_pushcclosure(L, &ctorPlacementProxy<typename detail::FuncTraits<MemFn>::Params, T>, 0);
            rawsetfield(L, -2, "__call");

            return *this;
        }
    };

private:
    //----------------------------------------------------------------------------
    /**
        Open the global namespace for registrations.

        @param L A Lua state.
    */
    explicit Namespace(lua_State* L) : Registrar(L)
    {
        lua_getglobal(L, "_G");
        ++m_stackSize;
    }

    //----------------------------------------------------------------------------
    /**
        Open a namespace for registrations.
        The namespace is created if it doesn't already exist.

        @param name   The namespace name.
        @param parent The parent namespace object.
        @pre The parent namespace is at the top of the Lua stack.
    */
    Namespace(char const* name, Namespace& parent) : Registrar(parent)
    {
        assert(name != nullptr);
        assert(lua_istable(L, -1)); // Stack: parent namespace (pns)

        rawgetfield(L, -1, name); // Stack: pns, namespace (ns) | nil

        if (lua_isnil(L, -1)) // Stack: pns, nil
        {
            lua_pop(L, 1); // Stack: pns

            lua_newtable(L); // Stack: pns, ns
            lua_pushvalue(L, -1); // Stack: pns, ns, ns

            // na.__metatable = ns
            lua_setmetatable(L, -2); // Stack: pns, ns

            // ns.__index = indexMetaMethod
            lua_pushcfunction(L, &CFunc::indexMetaMethod);
            rawsetfield(L, -2, "__index"); // Stack: pns, ns

            // ns.__newindex = newindexMetaMethod
            lua_pushcfunction(L, &CFunc::newindexStaticMetaMethod);
            rawsetfield(L, -2, "__newindex"); // Stack: pns, ns

            lua_newtable(L); // Stack: pns, ns, propget table (pg)
            lua_rawsetp(L, -2, detail::getPropgetKey()); // ns [propgetKey] = pg. Stack: pns, ns

            lua_newtable(L); // Stack: pns, ns, propset table (ps)
            lua_rawsetp(L, -2, detail::getPropsetKey()); // ns [propsetKey] = ps. Stack: pns, ns

            // pns [name] = ns
            lua_pushvalue(L, -1); // Stack: pns, ns, ns
            rawsetfield(L, -3, name); // Stack: pns, ns
        }

        ++m_stackSize;
    }

    //----------------------------------------------------------------------------
    /**
        Close the class and continue the namespace registrations.

        @param child A child class registration object.
    */
    explicit Namespace(ClassBase& child)
        : Registrar(child)
    {
    }

    using Registrar::operator=;

public:
    //----------------------------------------------------------------------------
    /**
      Retrieve the global namespace.
      It is recommended to put your namespace inside the global namespace, and
      then add your classes and functions to it, rather than adding many classes
      and functions directly to the global namespace.

      @param L A Lua state.
      @returns A namespace registration object.
    */
    static Namespace getGlobalNamespace(lua_State* L)
    {
        enableExceptions(L);
        return Namespace(L);
    }

    //----------------------------------------------------------------------------
    /**
        Open a new or existing namespace for registrations.

        @param name The namespace name.
        @returns A namespace registration object.
    */
    Namespace beginNamespace(char const* name)
    {
        assertIsActive();
        return Namespace(name, *this);
    }

    //----------------------------------------------------------------------------
    /**
        Continue namespace registration in the parent.
        Do not use this on the global namespace.

        @returns A parent namespace registration object.
    */
    Namespace endNamespace()
    {
        if (m_stackSize == 1)
            throw std::logic_error("endNamespace () called on global namespace");

        assert(m_stackSize > 1);
        --m_stackSize;
        lua_pop(L, 1);
        return Namespace(*this);
    }

    //----------------------------------------------------------------------------
    /**
        Add or replace a property.

        @param name       The property name.
        @param value      A value pointer.
        @param isWritable True for a read-write, false for read-only property.
        @returns This namespace registration object.
    */
    template <class T>
    Namespace& addProperty(char const* name, T* value, bool isWritable = true)
    {
        return addVariable(name, value, isWritable);
    }

    //----------------------------------------------------------------------------
    /**
        Add or replace a property.

        @param name       The property name.
        @param value      A value pointer.
        @param isWritable True for a read-write, false for read-only property.
        @returns This namespace registration object.
    */
    template <class T>
    Namespace& addVariable(char const* name, T* value, bool isWritable = true)
    {
        if (m_stackSize == 1)
            throw std::logic_error("addProperty () called on global namespace");

        assert(name != nullptr);
        assert(lua_istable(L, -1)); // Stack: namespace table (ns)

        lua_pushlightuserdata(L, value); // Stack: ns, pointer
        lua_pushcclosure(L, &CFunc::getVariable<T>, 1); // Stack: ns, getter
        CFunc::addGetter(L, name, -2); // Stack: ns

        if (isWritable)
        {
            lua_pushlightuserdata(L, value); // Stack: ns, pointer
            lua_pushcclosure(L, &CFunc::setVariable<T>, 1); // Stack: ns, setter
        }
        else
        {
            lua_pushstring(L, name); // Stack: ns, ps, name
            lua_pushcclosure(L, &CFunc::readOnlyError, 1); // Stack: ns, error_fn
        }

        CFunc::addSetter(L, name, -2); // Stack: ns

        return *this;
    }

    //----------------------------------------------------------------------------
    /**
        Add or replace a property.
        If the set function is omitted or null, the property is read-only.

        @param name       The property name.
        @param get  A pointer to a property getter function.
        @param set  A pointer to a property setter function, optional.
        @returns This namespace registration object.
    */
    template <class TG, class TS = TG>
    Namespace& addProperty(char const* name, TG (*get)(), void (*set)(TS) = 0)
    {
        if (m_stackSize == 1)
            throw std::logic_error("addProperty () called on global namespace");

        assert(name != nullptr);
        assert(lua_istable(L, -1)); // Stack: namespace table (ns)

        lua_pushlightuserdata(L, reinterpret_cast<void*>(get)); // Stack: ns, function ptr
        lua_pushcclosure(L, &CFunc::Call<TG (*)()>::f, 1); // Stack: ns, getter
        CFunc::addGetter(L, name, -2);

        if (set != 0)
        {
            lua_pushlightuserdata(L, reinterpret_cast<void*>(set)); // Stack: ns, function ptr
            lua_pushcclosure(L, &CFunc::Call<void (*)(TS)>::f, 1);
        }
        else
        {
            lua_pushstring(L, name);
            lua_pushcclosure(L, &CFunc::readOnlyError, 1);
        }

        CFunc::addSetter(L, name, -2);

        return *this;
    }

    //----------------------------------------------------------------------------
    /**
        Add or replace a property.
        If the set function is omitted or null, the property is read-only.

        @param name The property name.
        @param get  A pointer to a property getter function.
        @param set  A pointer to a property setter function, optional.
        @returns This namespace registration object.
    */
    Namespace& addProperty(char const* name, int (*get)(lua_State*), int (*set)(lua_State*) = 0)
    {
        if (m_stackSize == 1)
            throw std::logic_error("addProperty () called on global namespace");

        assert(name != nullptr);
        assert(lua_istable(L, -1)); // Stack: namespace table (ns)

        lua_pushcfunction(L, get); // Stack: ns, getter
        CFunc::addGetter(L, name, -2); // Stack: ns

        if (set != 0)
        {
            lua_pushcfunction(L, set); // Stack: ns, setter
            CFunc::addSetter(L, name, -2); // Stack: ns
        }
        else
        {
            lua_pushstring(L, name); // Stack: ns, name
            lua_pushcclosure(L, &CFunc::readOnlyError, 1); // Stack: ns, name, readOnlyError
            CFunc::addSetter(L, name, -2); // Stack: ns
        }

        return *this;
    }

    //----------------------------------------------------------------------------
    /**
        Add or replace a namespace function by std::function.
    */
    template <class ReturnType, class... Params>
    Namespace& addFunction(char const* name, std::function<ReturnType(Params...)> function)
    {
        using FnType = decltype(function);

        assert(name != nullptr);
        assert(lua_istable(L, -1)); // Stack: namespace table (ns)

        lua_newuserdata_aligned<FnType>(L, std::move(function)); // Stack: ns, function userdata (ud)
        lua_newtable(L); // Stack: ns, ud, ud metatable (mt)
        lua_pushcfunction(L, &CFunc::gcMetaMethodAny<FnType>); // Stack: ns, ud, mt, gc function
        rawsetfield(L, -2, "__gc"); // Stack: ns, ud, mt
        lua_setmetatable(L, -2); // Stack: ns, ud
        lua_pushcclosure(L, &CFunc::CallProxyFunctor<FnType>::f, 1); // Stack: ns, function
        rawsetfield(L, -2, name); // Stack: ns

        return *this;
    }

    //----------------------------------------------------------------------------
    /**
        Add or replace a free function.
    */
    template <class ReturnType, class... Params>
    Namespace& addFunction(char const* name, ReturnType (*fp)(Params...))
    {
        using FnType = decltype(fp);

        assert(name != nullptr);
        assert(lua_istable(L, -1)); // Stack: namespace table (ns)

        lua_pushlightuserdata(L, reinterpret_cast<void*>(fp)); // Stack: ns, function ptr
        lua_pushcclosure(L, &CFunc::Call<FnType>::f, 1); // Stack: ns, function
        rawsetfield(L, -2, name); // Stack: ns

        return *this;
    }

    //----------------------------------------------------------------------------
    /**
        Add or replace a lua_CFunction.

        @param name The function name.
        @param fp   A C-function pointer.
        @returns This namespace registration object.
    */
    Namespace& addFunction(char const* name, int (*const fp)(lua_State*))
    {
        return addCFunction(name, fp);
    }

    //----------------------------------------------------------------------------
    /**
        Add or replace a lua_CFunction.

        @param name The function name.
        @param fp   A C-function pointer.
        @returns This namespace registration object.
    */
    Namespace& addCFunction(char const* name, int (*const fp)(lua_State*))
    {
        assert(name != nullptr);
        assert(lua_istable(L, -1)); // Stack: namespace table (ns)

        lua_pushcfunction(L, fp); // Stack: ns, function
        rawsetfield(L, -2, name); // Stack: ns

        return *this;
    }

    //----------------------------------------------------------------------------
    /**
        Open a new or existing class for registrations.

        @param name The class name.
        @returns A class registration object.
    */
    template <class T>
    Class<T> beginClass(char const* name)
    {
        assertIsActive();
        return Class<T>(name, *this);
    }

    //----------------------------------------------------------------------------
    /**
        Derive a new class for registrations.
        Call deriveClass() only once.
        To continue registrations for the class later, use beginClass().

        @param name The class name.
        @returns A class registration object.
    */
    template <class Derived, class Base>
    Class<Derived> deriveClass(char const* name)
    {
        assertIsActive();
        return Class<Derived>(name, *this, detail::getStaticRegistryKey<Base>());
    }
};

//------------------------------------------------------------------------------
/**
    Retrieve the global namespace.
    It is recommended to put your namespace inside the global namespace, and
    then add your classes and functions to it, rather than adding many classes
    and functions directly to the global namespace.

    @param L A Lua state.
    @returns A namespace registration object.
*/
inline Namespace getGlobalNamespace(lua_State* L)
{
    return Namespace::getGlobalNamespace(L);
}

} // namespace luabridge

// End File: Source/LuaBridge/detail/Namespace.h

// Begin File: Source/LuaBridge/LuaBridge.h

// https://github.com/kunitoki/LuaBridge
// Copyright 2020, Lucio Asnaghi
// Copyright 2020, Dmitry Tarakanov
// Copyright 2012, Vinnie Falco <vinnie.falco@gmail.com>
// Copyright 2007, Nathan Reed
// SPDX-License-Identifier: MIT


// All #include dependencies are listed here
// instead of in the individual header files.

#define LUABRIDGE_MAJOR_VERSION 3
#define LUABRIDGE_MINOR_VERSION 0
#define LUABRIDGE_VERSION 300

#ifndef LUA_VERSION_NUM
#error "Lua headers must be included prior to LuaBridge ones"
#endif


// End File: Source/LuaBridge/LuaBridge.h

// Begin File: Source/LuaBridge/List.h

// https://github.com/kunitoki/LuaBridge
// Copyright 2020, Lucio Asnaghi
// Copyright 2020, Dmitry Tarakanov
// SPDX-License-Identifier: MIT




namespace luabridge {

template<class T>
struct Stack<std::list<T>>
{
    static void push(lua_State* L, std::list<T> const& list)
    {
        lua_createtable(L, static_cast<int>(list.size()), 0);
        typename std::list<T>::const_iterator item = list.begin();
        for (std::size_t i = 1; i <= list.size(); ++i)
        {
            lua_pushinteger(L, static_cast<lua_Integer>(i));
            Stack<T>::push(L, *item);
            lua_settable(L, -3);
            ++item;
        }
    }

    static std::list<T> get(lua_State* L, int index)
    {
        if (!lua_istable(L, index))
        {
            luaL_error(L, "#%d argument must be a table", index);
        }

        std::list<T> list;

        int const absindex = lua_absindex(L, index);
        lua_pushnil(L);
        while (lua_next(L, absindex) != 0)
        {
            list.push_back(Stack<T>::get(L, -1));
            lua_pop(L, 1);
        }
        return list;
    }

    static bool isInstance(lua_State* L, int index) { return lua_istable(L, index); }
};

} // namespace luabridge

// End File: Source/LuaBridge/List.h

// Begin File: Source/LuaBridge/Array.h

// https://github.com/kunitoki/LuaBridge
// Copyright 2020, Lucio Asnaghi
// Copyright 2020, Dmitry Tarakanov
// SPDX-License-Identifier: MIT




namespace luabridge {

template<class T, std::size_t s>
struct Stack<std::array<T, s>>
{
    static void push(lua_State* L, std::array<T, s> const& array)
    {
        lua_createtable(L, static_cast<int>(s), 0);
        for (std::size_t i = 0; i < s; ++i)
        {
            lua_pushinteger(L, static_cast<lua_Integer>(i + 1));
            Stack<T>::push(L, array[i]);
            lua_settable(L, -3);
        }
    }

    static std::array<T, s> get(lua_State* L, int index)
    {
        if (!lua_istable(L, index))
        {
            luaL_error(L, "#%d argments must be table", index);
            throw std::runtime_error("Array get () must receive a table");
        }
        if (index != s)
        {
            luaL_error(L, "array size should be %d ", s);
        }

        std::array<T, s> array;

        int const absindex = lua_absindex(L, index);
        lua_pushnil(L);
        int arr_index = 0;
        while (lua_next(L, absindex) != 0)
        {
            array[arr_index] = Stack<T>::get(L, -1);
            lua_pop(L, 1);
            arr_index++;
        }
        return array;
    }
};

} // namespace luabridge

// End File: Source/LuaBridge/Array.h

// Begin File: Source/LuaBridge/Vector.h

// https://github.com/kunitoki/LuaBridge
// Copyright 2020, Lucio Asnaghi
// Copyright 2018, Dmitry Tarakanov
// SPDX-License-Identifier: MIT




namespace luabridge {

template<class T>
struct Stack<std::vector<T>>
{
    static void push(lua_State* L, std::vector<T> const& vector)
    {
        lua_createtable(L, static_cast<int>(vector.size()), 0);
        for (std::size_t i = 0; i < vector.size(); ++i)
        {
            lua_pushinteger(L, static_cast<lua_Integer>(i + 1));
            Stack<T>::push(L, vector[i]);
            lua_settable(L, -3);
        }
    }

    static std::vector<T> get(lua_State* L, int index)
    {
        if (!lua_istable(L, index))
        {
            luaL_error(L, "#%d argument must be a table", index);
        }

        std::vector<T> vector;
        vector.reserve(static_cast<std::size_t>(get_length(L, index)));

        int const absindex = lua_absindex(L, index);
        lua_pushnil(L);
        while (lua_next(L, absindex) != 0)
        {
            vector.push_back(Stack<T>::get(L, -1));
            lua_pop(L, 1);
        }
        return vector;
    }

    static bool isInstance(lua_State* L, int index) { return lua_istable(L, index); }
};

} // namespace luabridge

// End File: Source/LuaBridge/Vector.h

