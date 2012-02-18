//       _________ __                 __
//      /   _____//  |_____________ _/  |______     ____  __ __  ______
//      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
//      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ |
//     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
//             \/                  \/          \//_____/            \/
//  ______________________                           ______________________
//                        T H E   W A R   B E G I N S
//         Stratagus - A free fantasy real time strategy game engine
//
/**@name unit_cache.h - The unit cache headerfile. */
//
//      (c) Copyright 2008 by Rafal Bursig
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; only version 2 of the License.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program; if not, write to the Free Software
//      Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//      02111-1307, USA.

#ifndef __UNIT_CACHE_H__
#define __UNIT_CACHE_H__

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include <vector>
#include <algorithm>
#include <stdio.h>

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

class CUnit;
class CMap;
/**
**  Unit cache
*/
class CUnitCache
{
public:
	typedef std::vector<CUnit *>::iterator iterator;
	typedef std::vector<CUnit *>::const_iterator const_iterator;

public:
	CUnitCache() : Units()
	{
	}

	size_t size() const { return Units.size(); }

	void clear() { Units.clear(); }

	const_iterator begin() const { return Units.begin(); }
	iterator begin() { return Units.begin(); }
	const_iterator end() const { return Units.end(); }
	iterator end() { return Units.end(); }

	CUnit * operator[] (const unsigned int index) const
	{
		//Assert(index < Units.size());
		return Units[index];
	}
	CUnit * operator[] (const unsigned int index) {
		//Assert(index < Units.size());
		return Units[index];
	}

	/**
	 *  @brief Find the first unit in a tile cache for which a predicate is true.
	 *  @param  pred   A predicate object vith bool operator()(const CUnit *).
	 *  @return   The first unit u in the cache
	 *  such that @p pred(u) is true, or NULL if no such unit exists.
	 */
	template<typename _T>
	CUnit *find(const _T &pred) const
	{
		std::vector<CUnit *>::const_iterator ret = std::find_if(Units.begin(), Units.end(), pred);

		return ret != Units.end() ? (*ret) : NULL;
	}

	/**
	 *  @brief Apply a function to every element of a cache.
	 *  @param  functor A unary function object vith void operator()(CUnit *).
	 *  @return count of visited element.
	 *
	 *  Applies the function object @p f to each element in the cache.
	 *  @p functor must not modify the order of the cache.
	 */
	template<typename _T>
	void for_each(const _T functor)
	{
		const size_t size = Units.size();

		for (size_t i = 0; i != size; ++i) {
			functor(Units[i]);
		}
	}

	/**
	 *  @brief Apply a function to every element of a cache.
	 *  @param  functor A unary function object vith bool operator()(CUnit *).
	 *  @return count of visited element.
	 *
	 *  Applies the function object @p f to each element in the cache.
	 *  @p functor must not modify the order of the cache.
	 *  If @p functor return false then loop is exited.
	 */
	template<typename _T>
	int for_each_if(const _T &functor)
	{
		const size_t size = Units.size();

		for (size_t count = 0; count != size; ++count) {
			if (functor(Units[count]) == false) {
				return count;
			}
		}
		return size;
	}


	/**
	**  Remove unit on index from unit cache.
	**
	**  @param index  Unit index to remove from container.
	**  @return pointer to removed element.
	*/
	CUnit *Remove(const unsigned int index)
	{
		const size_t size = Units.size();
		Assert(index < size);
		CUnit *tmp = Units[index];
		if (size > 1) {
			Units[index] = Units[size - 1];
		}
		Units.pop_back();
		return tmp;
	}

	/**
	**  Remove unit from unit cache.
	**
	**  @param unit  Unit pointer to remove from container.
	*/
	bool Remove(CUnit *const unit)
	{
#ifndef SECURE_UNIT_REMOVING
		const size_t size = Units.size();
		if (size == 1 && unit == Units[0]) {
			Units.pop_back();
			return true;
		} else {
			for (unsigned int i = 0; i < size; ++i) {
				// Do we care on unit sequence in tile cache ?
				if (Units[i] == unit) {
					Units[i] = Units[size - 1];
					Units.pop_back();
					return true;
				}
			}
		}
#else
		for (std::vector<CUnit *>::iterator i(Units.begin()), end(Units.end()); i != end; ++i) {
			if ((*i) == unit) {
				Units.erase(i);
				return true;
			}
		}
#endif
		return false;
	}

	/**
	**  Remove unit from unit cache.
	**
	**  @param unit  Unit pointer to remove from container.
	*/
	void RemoveS(CUnit *const unit)
	{
		for (std::vector<CUnit *>::iterator i(Units.begin()), end(Units.end()); i != end; ++i) {
			if ((*i) == unit) {
				Units.erase(i);
				return;
			}
		}
	}

	/**
	**  Insert new unit into tile cache.
	**  Sorted version for binary searching.
	**
	**  @param unit  Unit pointer to place in cache.
	**  @return false if unit is already in cache and nothing is added.
	*/
	bool InsertS(CUnit *unit) {
		if (!binary_search(Units.begin(), Units.end(), unit))
		{
  			Units.insert(std::lower_bound(Units.begin(), Units.end(), unit), unit);
  			return true;
		}
		return false;
	}

	/**
	**  Insert new unit into tile cache.
	**
	**  @param unit  Unit pointer to place in cache.
	*/
	void Insert(CUnit *unit) {
		Units.push_back(unit);
	}

public:
	std::vector<CUnit *> Units;
};


//@}

#endif // !__UNIT_CACHE_H__

