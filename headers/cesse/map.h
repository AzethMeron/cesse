#ifndef CESSE_MAP_H
#define CESSE_MAP_H

/**
* @file map.h
* @author Jakub Grzana
* @date August 2026
* @brief String-keyed hash map that stores borrowed objects (as void*)
*/

#include "cesse/bool.h"
#include "cesse/utils.h"
#include "cesse/array.h"
#include "cesse/functions.h"

#include <stddef.h>

/**
* Map type: a separate-chaining hash table (FNV-1a) keyed by C strings.
*
* Keys are C strings, always owned by the Map. Values are borrowed, the same convention as Array and Stack: 
* by default nothing is freed on clear/delete unless a function_delete is supplied.
*
* The bucket array grows (doubling, rounded to a power of two) whenever
* the load factor would exceed 0.75, keeping the common operations at
* amortized O(1) on average.
*/
typedef struct Map Map;

/**
* Create a new, empty Map on the heap and pass ownership to the caller.
*
* Time complexity: O(1).
* \param error Pointer to ErrorCode object, to be populated with error if one occurs. Pass NULL to ignore.
*        Possible codes: CESSE_ERR_ALLOC.
* \return Pointer to the created Map, or NULL if an error occurred.
*/
Map* map_new(ErrorCode* error);

/**
* Delete a Map and free its internal keys and bucket storage.
* Unless function_delete is provided, it does NOT free stored object.
* It's recommanded to first drain the map and free objects on your own, as this gives better error-handling options.
*
* Time complexity: O(n), where n is the number of entries still stored.
* \param map Pointer-to-pointer of the map. Once freed, the pointer is
*        set to NULL (hence the double pointer). Passing a
*        pointer-to-NULL is a safe no-op.
* \param error Pointer to ErrorCode object, to be populated with error if one occurs. Pass NULL to ignore.
*        Possible codes: CESSE_ERR_NULLARG 
* \param freer Function used to free the objects being removed. Pass NULL to ignore (meaning memory leak if there're objects in container)
*        A failure reported by freer itself is printed to stderr but does not abort the clear (that also will lead to memory leaks).
*/
void map_delete(Map** map, ErrorCode* error, function_delete freer);

/**
* Remove every entry from a Map without deleting the Map itself,
* leaving it empty and reusable.
*
* Time complexity: O(n), where n is the number of entries.
* \param map The map to clear. Must not be NULL.
* \param error Pointer to ErrorCode object, to be populated with error if one occurs. Pass NULL to ignore.
*        Possible codes: CESSE_ERR_NULLARG.
* \param freer Function used to free the objects being removed. Pass NULL to ignore (meaning memory leak if there're objects in container)
*        A failure reported by freer itself is printed to stderr but does not abort the clear (that also will lead to memory leaks).
*/
void map_clear(Map* map, ErrorCode* error, function_delete freer);

/**
* Associate key with value. If key
* already exists, its value is replaced and the previous value is
* returned rather than freed -- the caller decides what to do with it.
* Note that key is copied and owned by the map while value is borrowed and owned by user.
*
* Time complexity: amortized O(1) average; O(n) on rehash or in worst-case scenario where all keys collide
* \param map The map to modify. Must not be NULL.
* \param key The key to set. Must not be NULL.
* \param value The value to associate with key (borrowed). Must not be NULL.
* \param error Pointer to ErrorCode object, to be populated with error if one occurs. Pass NULL to ignore.
*        Possible codes: CESSE_ERR_NULLARG, CESSE_ERR_ALLOC, CESSE_ERR_OVERFLOW.
* \return The previous value associated with key, or NULL if key is new
*         (or if an error occurred -- check error to distinguish the two).
*/
void* map_set(Map* map, const char* key,  void* value, ErrorCode* error);

/**
* Look up the value associated with key.
*
* Time complexity: O(1) average; O(n) worst case (see map_set).
* \param map The map to query. Must not be NULL.
* \param key The key to look up. Must not be NULL.
* \param error Pointer to ErrorCode object, to be populated with error if one occurs. Pass NULL to ignore.
*        Possible codes: CESSE_ERR_NULLARG, CESSE_ERR_KEY_NOT_FOUND.
* \return The associated value, or NULL if an error occurred (including key not being found).
*/
void* map_get(Map* map, const char* key, ErrorCode* error);

/**
* Check whether key exists in the map.
*
* Time complexity: O(1) average; O(n) worst case (see map_set).
* \param map The map to query. Must not be NULL.
* \param key The key to check. Must not be NULL.
* \param error Pointer to ErrorCode object, to be populated with error if one occurs. Pass NULL to ignore.
*        Possible codes: CESSE_ERR_NULLARG. 
* \return true if key exists, false otherwise (including on error).
*/
bool map_contains(Map* map, const char* key, ErrorCode* error);

/**
* Return the number of entries currently stored.
*
* Time complexity: O(1).
* \param map The map to query. Must not be NULL.
* \param error Pointer to ErrorCode object, to be populated with error if one occurs. Pass NULL to ignore.
*        Possible codes: CESSE_ERR_NULLARG.
* \return The entry count, or 0 if an error occurred (0 is also valid for empty map so use error to distinguish)
*/
size_t map_size(Map* map, ErrorCode* error);

/**
* Return an Array holding a copy of every key currently in the map, in
* no particular order. The caller owns the Array and every string in it.
*
* Time complexity: O(n), where n is the number of entries.
* \param map The map to query. Must not be NULL.
* \param error Pointer to ErrorCode object, to be populated with error if one occurs. Pass NULL to ignore.
*        Possible codes: CESSE_ERR_NULLARG, CESSE_ERR_ALLOC, CESSE_ERR_OVERFLOW
* \return A new Array of char* key copies, or NULL if an error occurred.
*/
Array* map_keylist(Map* map, ErrorCode* error);

/**
* Remove key from the map and return its value. The map's own copy of
* the key is freed; the returned value is not.
*
* Time complexity: O(1) average; O(n) worst case (see map_set).
* \param map The map to modify. Must not be NULL.
* \param key The key to remove. Must not be NULL.
* \param error Pointer to ErrorCode object, to be populated with error if one occurs. Pass NULL to ignore.
*        Possible codes: CESSE_ERR_NULLARG, CESSE_ERR_KEY_NOT_FOUND.
* \return The removed value, now the caller's responsibility, or NULL if
*         an error occurred (including key not being found).
*/
void* map_remove(Map* map, const char* key, ErrorCode* error);

/**
* Produce a new Map holding independent copies of every value, under
* copies of the same keys, using copier to duplicate each value. The
* original map is left untouched.
*
* On a failure partway through, everything already copied into the new
* map is cleaned up via freer before returning NULL -- freer is
* therefore required (not optional), since the values being cleaned up
* were just created by copier, not borrowed from anywhere else that
* might already own them. Recovery is best-effor but no-guarantee.
*
* Time complexity: O(n), where n is the number of entries.
* \param map The map to copy. Must not be NULL.
* \param error Pointer to ErrorCode object, to be populated with error if one occurs. Pass NULL to ignore.
*        Possible codes: CESSE_ERR_NULLARG, CESSE_ERR_ALLOC, CESSE_ERR_OVERFLOW, whatever code copier itself reports on failure.
* \param copier Function used to duplicate each stored value. Must not be NULL.
* \param freer Function used to clean up already-copied values if the
*        copy fails partway through. Must not be NULL.
* \return The new, independent Map, or NULL if an error occurred.
*/
Map* map_copy(Map* map, ErrorCode* error, function_copy copier, function_delete freer);

#endif
