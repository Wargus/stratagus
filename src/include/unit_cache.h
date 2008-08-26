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

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

class CUnit;

/**
**  Unit cache
*/
struct CUnitCache {
	std::vector<CUnit *> Units;

	CUnitCache() : Units() { Units.clear();}
	
	inline size_t size() const
	{
		return Units.size();
	}

	inline CUnit * operator[] (const unsigned int index) {
		Assert(index < Units.size());
		return Units[index];
	}

	/**
	 *  @brief Find the first unit in a tile chache for which a predicate is true.
	 *  @param  pred   A predicate object vith bool operator()(const CUnit *).
	 *  @return   The first unit i in the cache
	 *  such that @p pred(*i) is true, or NULL if no such iterator exists.
	 */
	template<typename _T>
	inline CUnit *find(const _T &pred) const
	{
#ifdef _MSC_VER			
		if(Units.size()) {
			std::vector<CUnit *>::const_iterator beg(Units.begin()), end(Units.end());
			std::vector<CUnit *>::const_iterator ret = std::find_if(beg, end, pred);
			return ret != end ? (*ret) : NULL;
		}
		return NULL;
		//const size_t size = Units.size();
		//int i = 0;
		//while(size && !pred(Units[i]) && ++i < size);
		//return (i < size ? Units[i] : NULL);
#else
		const size_t size = Units.size();
		if(size) {
			const CUnit *unit;
			int n = (size+3)/4;
			const CUnit **cache = (const CUnit **)Units.data();
			switch (size & 3) {
				case 0: 
				do {
					unit = *cache;
					if(pred(unit))
						return (CUnit *)unit;
					cache++;
				case 3:	
					unit = *cache;
					if(pred(unit))
						return (CUnit *)unit;
					cache++;
				case 2:	
					unit = *cache;
					if(pred(unit))
						return (CUnit *)unit;
					cache++;
				case 1:
					unit = *cache;
					if(pred(unit))
						return (CUnit *)unit;
					cache++;
				} while ( --n > 0 );
			}
		}
		return NULL;
#endif		
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
	inline void for_each(_T &functor)
	{	
		const size_t size = Units.size();
#ifdef _MSC_VER
		for(unsigned int i = 0; i < size; ++i)
			functor(Units[i]);
#else
		if(size) {
			int n = (size+3)/4;
			CUnit **cache = (CUnit **)Units.data();
			switch (size & 3) {
				case 0: do { 
								functor(*cache++);
				case 3:			functor(*cache++);
				case 2:			functor(*cache++);
				case 1:			functor(*cache++);
					} while ( --n > 0 );
			}
		}
#endif
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
	inline int for_each_if(_T &functor)
	{	
		const size_t size = Units.size();
		int count = 0;		
#ifdef _MSC_VER
		while(size && functor(Units[count]) && ++count < size);		
#else
		if(size) {
			int n = (size+3)/4;
			switch (size & 3) {
				case 0: 
				do {
					if(!functor(Units[count]))
						return count;
					count++;	
				case 3:	
					if(!functor(Units[count]))
						return count;
					count++;	
				case 2:	
					if(!functor(Units[count]))
						return count;
					count++;	
				case 1:
					if(!functor(Units[count]))
						return count ;
					count++;	
					} while ( --n > 0 );
			}
		}
#endif
		return count;
	}


	/**
	**  Remove unit on index from unit cache.
	**
	**  @param index  Unit index to remove from container.
	**  @return pointer to removed element.
	*/	
	inline CUnit * Remove(const unsigned int index)
	{
		const size_t size = Units.size();
		Assert(index < size);
		CUnit *tmp = Units[index];
		if(size > 1) {
			Units[index] = Units[size - 1];
			Units[size - 1] = tmp;
		}
		Units.pop_back();
		return tmp;
	}
	
	/**
	**  Remove unit from unit cache.
	**
	**  @param unit  Unit pointer to remove from container.
	*/	
	inline void Remove(CUnit *const unit)
	{
#ifndef SECURE_UNIT_REMOVING		
		const size_t size = Units.size();
		if(size == 1 && unit == Units[0]) {
			Units.pop_back();
		} else {
			for(unsigned int i = 0; i < size; ++i) {
				// Do we care on unit sequence in tile cache ?
				if (Units[i] == unit) {
					CUnit *tmp = Units[size - 1];
					Units[size - 1] = unit;
					Units[i] = tmp;
					Units.pop_back();
					return;
				}
			}
		}	
#else
		for(std::vector<CUnit *>::iterator i(Units.begin()), end(Units.end());
			 i != end; ++i) {
			if ((*i) == unit) {
				Units.erase(i);
				return;
			}
		}
#endif
	}

	/**
	**  Remove unit from unit cache.
	**
	**  @param unit  Unit pointer to remove from container.
	*/	
	inline void RemoveS(CUnit *const unit)
	{
		for(std::vector<CUnit *>::iterator i(Units.begin()), end(Units.end());
			 i != end; ++i) {
			if ((*i) == unit) {
				Units.erase(i);
				return;
			}
		}
	}
 
	/**
	**  Insert new unit into tile cache.
	**	Sorted version for binary searching.
	**
	**  @param unit  Unit pointer to place in cache.
	**  @return false if unit is already in cache and nothing is added.
	*/
	inline bool InsertS(CUnit *unit) {
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
	inline void Insert(CUnit *unit) {
		Units.push_back(unit);
	}
};


//@}

#endif // !__UNIT_CACHE_H__

