#include "cesse/bool.h"
#include "cesse/sort.h"
#include "cesse/utils.h"
#include "cesse/macros.h"
#include "cesse/functions.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

/* Descending (largest-first) merge sort.
 *
 * Stable: the merge step below always takes from the left run on ties.
 * By induction over the whole recursion tree, that alone is the entire
 * stability guarantee -- no auxiliary bookkeeping needed.
 *
 * O(n log n) unconditionally: every split is exactly balanced, so
 * unlike quicksort there's no adversarial input that degrades it. The
 * same balance also bounds recursion depth at O(log n) for any input
 * -- unlike quicksort's data-dependent worst case, there's no way to
 * make this recurse deeper than that, so plain C recursion carries
 * none of the stack-overflow risk it would for quicksort.
 *
 * Not in-place: merging two runs needs somewhere to hold elements
 * from one of them while interleaving with the other. One O(n)
 * scratch buffer is allocated up front and reused for every merge.
 */

#define MAX_BUFFER_LENGTH (SIZE_MAX/sizeof(void*))

static inline bool is_before(function_compare_lt compare_lt, void* a, void* b) {
        return compare_lt(b, a);
}

/* Merges the two already-sorted runs anchor[lo..mid] and
 * anchor[mid+1..hi] via scratch, then writes the result back. */
static void merge(void** anchor, size_t lo, size_t mid, size_t hi, function_compare_lt compare_lt, void** scratch) {
        size_t i = lo, j = mid + 1, k = lo;
        while (i <= mid && j <= hi) {
                if (is_before(compare_lt, anchor[j], anchor[i])) {
                        scratch[k++] = anchor[j++];
                } else {
                        scratch[k++] = anchor[i++]; /* tie -> left run wins: stability */
                }
        }
        while (i <= mid) { scratch[k++] = anchor[i++]; }
        while (j <= hi) { scratch[k++] = anchor[j++]; }
        memcpy(anchor + lo, scratch + lo, (hi - lo + 1) * sizeof(void*));
}

static void merge_sort(void** anchor, size_t lo, size_t hi, function_compare_lt compare_lt, void** scratch) {
        if (lo >= hi) { return; } /* 0 or 1 elements: already sorted */
        size_t mid = lo + (hi - lo) / 2;
        merge_sort(anchor, lo, mid, compare_lt, scratch);
        merge_sort(anchor, mid + 1, hi, compare_lt, scratch);
        merge(anchor, lo, mid, hi, compare_lt, scratch);
}

void sort(void** begin, const size_t length, function_compare_lt compare_lt, ErrorCode* error) {
	ASSURE_ERROR_OK(error);
    ERROR_ON_COND(begin==NULL, error, CESSE_ERR_NULLARG, return;);
    ERROR_ON_COND(compare_lt==NULL, error, CESSE_ERR_NULLARG, return;);
    if (length < 2) { return; }

	ERROR_ON_COND(length > MAX_BUFFER_LENGTH, error, CESSE_ERR_OVERFLOW, return;);
    void** scratch = malloc(length * sizeof(void*));
    ERROR_ON_COND(scratch==NULL, error, CESSE_ERR_ALLOC, return;);

    merge_sort(begin, 0, length - 1, compare_lt, scratch);

    free(scratch);
}