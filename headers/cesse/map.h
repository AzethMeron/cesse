#ifndef CESSE_MAP_H
#define CESSE_MAP_H

/**
* @file map.h
* @author Jakub Grzana
* @date August 2026
* @brief String-keyed hash map that can store borrowed objects (as void*)
*/

#include "cesse/bool.h"
#include "cesse/utils.h"
#include "cesse/array.h"
#include "cesse/functions.h"

#include <stddef.h>

/**
* Map type: a separate-chaining hash table (FNV-1a) keyed by C strings.
*
* Keys are C strings, always owned by the Map: map_set stores its own
* copy of the key, never the caller's pointer, and that copy is freed
* automatically on removal/clear/delete. Keys returned via map_keylist
* are, by contrast, copies owned by the caller.
*
* Values are borrowed, the same convention as Array and Stack: by
* default nothing is freed on clear/delete unless a function_delete is
* supplied.
*
* The bucket array grows (doubling, rounded to a power of two) whenever
* the load factor would exceed 0.75, keeping the common operations at
* amortized O(1) on average, same spirit as Array's amortized push/pop.
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
* Unless function_delete is provided, it does NOT free the stored
* values themselves.
*
* Time complexity: O(n), where n is the number of entries still stored.
* \param map Pointer-to-pointer of the map. Once freed, the pointer is
*        set to NULL (hence the double pointer). Passing a
*        pointer-to-NULL is a safe no-op, mirroring free(NULL).
* \param error Pointer to ErrorCode object, to be populated with error if one occurs. Pass NULL to ignore.
*        Possible codes: CESSE_ERR_NULLARG (map itself, i.e. the
*        pointer-to-pointer, is NULL).
* \param freer Function used to free values still stored in the map. Pass NULL to ignore.
*/
void map_delete(Map** map, ErrorCode* error, function_delete freer);

/**
* Remove every entry from a Map without deleting the Map itself,
* leaving it empty and reusable. Frees every stored key (the map's own
* copy) regardless of freer.
*
* Time complexity: O(n), where n is the number of entries.
* \param map The map to clear. Must not be NULL.
* \param error Pointer to ErrorCode object, to be populated with error if one occurs. Pass NULL to ignore.
*        Possible codes: CESSE_ERR_NULLARG.
* \param freer Function used to free the values being removed. Pass NULL to ignore (values are left untouched).
*        A failure reported by freer itself is printed to stderr but does not abort the clear.
*/
void map_clear(Map* map, ErrorCode* error, function_delete freer);

/**
* Associate key with value, storing a fresh copy of key (the map takes
* ownership of that copy) and the given value (borrowed). If key
* already exists, its value is replaced and the previous value is
* returned rather than freed -- the caller decides what to do with it.
*
* Time complexity: amortized O(1) average; O(n) on the rare occasion a
* resize is triggered, or worst-case O(n) if many keys collide into the
* same bucket (not expected in normal use with FNV-1a).
* \param map The map to modify. Must not be NULL.
* \param key The key to set (copied internally; the caller's own key
*        buffer can be freed or reused immediately after this call returns). Must not be NULL.
* \param value The value to associate with key (borrowed). Must not be NULL.
* \param error Pointer to ErrorCode object, to be populated with error if one occurs. Pass NULL to ignore.
*        Possible codes: CESSE_ERR_NULLARG (map, key, or value is NULL),
*        CESSE_ERR_ALLOC (the key copy, a new entry, or a growth-triggered
*        reallocation failed), CESSE_ERR_OVERFLOW (the bucket array
*        would need to grow beyond what's representable).
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
*        Possible codes: CESSE_ERR_NULLARG, CESSE_ERR_KEY_NOT_FOUND (key does not exist).
* \return The associated value (still owned by the map), or NULL if an
*         error occurred (including key not being found).
*/
void* map_get(Map* map, const char* key, ErrorCode* error);

/**
* Check whether key exists in the map.
*
* Time complexity: O(1) average; O(n) worst case (see map_set).
* \param map The map to query. Must not be NULL.
* \param key The key to check. Must not be NULL.
* \param error Pointer to ErrorCode object, to be populated with error if one occurs. Pass NULL to ignore.
*        Possible codes: CESSE_ERR_NULLARG. Note key simply not existing
*        is not itself an error here (unlike map_get/map_remove) -- it's
*        reported as a normal false return.
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
* \return The entry count, or 0 if an error occurred (0 is otherwise a
*         perfectly valid, non-error result too, for a genuinely empty map).
*/
size_t map_size(Map* map, ErrorCode* error);

/**
* Return an Array holding a copy of every key currently in the map, in
* no particular order. The caller owns the Array and every string in it.
*
* Time complexity: O(n), where n is the number of entries.
* \param map The map to query. Must not be NULL.
* \param error Pointer to ErrorCode object, to be populated with error if one occurs. Pass NULL to ignore.
*        Possible codes: CESSE_ERR_NULLARG, CESSE_ERR_ALLOC (a key copy
*        or the Array's own storage failed to allocate), CESSE_ERR_OVERFLOW
*        (from allocating the Array).
* \return A new Array of owned char* key copies, or NULL if an error occurred.
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
*        Possible codes: CESSE_ERR_NULLARG, CESSE_ERR_KEY_NOT_FOUND (key does not exist).
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
* might already own them.
*
* Time complexity: O(n), where n is the number of entries.
* \param map The map to copy. Must not be NULL.
* \param error Pointer to ErrorCode object, to be populated with error if one occurs. Pass NULL to ignore.
*        Possible codes: CESSE_ERR_NULLARG (map, copier, or freer is
*        NULL), CESSE_ERR_ALLOC or CESSE_ERR_OVERFLOW (from allocating
*        the new map or growing it), or whatever code copier itself reports on failure.
* \param copier Function used to duplicate each stored value. Must not be NULL.
* \param freer Function used to clean up already-copied values if the
*        copy fails partway through. Must not be NULL.
* \return The new, independent Map, or NULL if an error occurred.
*/
Map* map_copy(Map* map, ErrorCode* error, function_copy copier, function_delete freer);

#endif
