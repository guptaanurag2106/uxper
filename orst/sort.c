#include <bits/time.h>
#include <math.h>
#include <omp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "../utils/utils.h"

typedef struct {
    int x;
    char c;
} DataPoint;

Vector(DataPoint, Data);

int compare(const void *a, const void *b) {
    const DataPoint *da = (const DataPoint *)a;
    const DataPoint *db = (const DataPoint *)b;
    return (da->x != db->x) ? (da->x < db->x ? -1 : 1)
                            : (da->c < db->c    ? -1
                               : da->c == db->c ? 0
                                                : 1);  // sort by int then char
}

typedef int (*compareFn)(const void *, const void *);
typedef void (*sortFn)(Data *, compareFn);

void swap(DataPoint *a, DataPoint *b) {
    DataPoint t = *a;
    *a = *b;
    *b = t;
}
static Data empty_data = {0};
static Data single_entry_data = {0};
static Data random_data = {0};
static Data reverse_data = {0};
static Data duplicate_data = {0};
static Data sorted_data = {0};
static Data mostly_sorted_data = {0};

void init_data(int data_size) {
    vec_free(&empty_data);
    vec_free(&single_entry_data);
    vec_free(&random_data);
    vec_free(&reverse_data);
    vec_free(&duplicate_data);
    vec_free(&sorted_data);
    vec_free(&mostly_sorted_data);

    vec_push(&single_entry_data, ((DataPoint){.x = 1, .c = 'a'}));

    for (int i = 0; i < data_size; i++) {
        const DataPoint p = {.x = (int)rng_u32_tls(),
                             .c = (char)('a' + rngi_range_tls(0, 25))};
        vec_push(&random_data, p);
    }

    // numbers start from data_size*20 + [0..99]
    int max = data_size * 20 + rngi_range_tls(0, 99);
    char maxc = 'z';
    for (int i = 0; i < data_size; i++) {
        const DataPoint p = {.x = max, .c = maxc};
        vec_push(&reverse_data, p);
        max -= rngi_range_tls(1, 20);
        maxc -= (rng_f32_tls() < 26.0f / (float)data_size);
        if (maxc < 'a') maxc = 'a';
    }

    int dup_size = (data_size < 10) ? 1 : data_size / 10;
    for (int i = 0; i < data_size; i++) {
        const DataPoint p = {.x = rngi_range_tls(0, dup_size - 1),
                             .c = 'a'};  // 10 buckets of identical x, same 'c'
        vec_push(&duplicate_data, p);
    }

    int min = rngi_range_tls(0, 99);  // numbers start from [0..99]
    char minc = 'a';
    for (int i = 0; i < data_size; i++) {
        const DataPoint p = {.x = min, .c = minc};
        vec_push(&sorted_data, p);
        vec_push(&mostly_sorted_data, p);
        min += rngi_range_tls(1, 20);
        minc += (rng_f32_tls() < 26.0f / (float)data_size);
        if (minc > 'z') minc = 'z';
    }
    const int missorted_count = data_size / 10;  // 10%
    for (int i = 0; i < missorted_count; i++) {
        const int random_i = rngi_range_tls(0, data_size - 2);
        swap(&mostly_sorted_data.items[random_i],
             &mostly_sorted_data.items[random_i + 1]);  // swap neighbours
    }
}

bool validate_sorted(const Data *d) {
    size_t i = 0, n = d->size;
    while (i < n) {
        i++;
        while (i < n && (d->items[i].x == d->items[i - 1].x)) {
            if (d->items[i].c < d->items[i - 1].c) return false;
            i++;
        }
        if (i == n) return true;
        if (d->items[i].x < d->items[i - 1].x) return false;
    }
    return true;
}

void bubblesort(Data *data, compareFn compare) {
    size_t size = data->size;
    DataPoint *items = data->items;
    for (size_t i = 0; i < size; i++) {
        bool swapped = false;
        for (size_t j = 0; j < size - i - 1; j++) {
            if (compare(&items[j], &items[j + 1]) > 0) {  // maintains order
                swap(&items[j], &items[j + 1]);
                swapped = true;
            }
        }
        if (!swapped) return;
    }
}

void selectionsort(Data *data, compareFn compare) {
    size_t size = data->size;
    DataPoint *items = data->items;
    if (size <= 1) return;
    for (size_t i = 0; i < size - 1; i++) {
        size_t min_i = i;
        for (size_t j = i + 1; j < size; j++) {
            if (compare(&items[j], &items[min_i]) < 0) {
                min_i = j;
            }
        }
        if (i != min_i) swap(&items[i], &items[min_i]);
    }
}

void insertionsort(Data *data, compareFn compare) {
    size_t size = data->size;
    DataPoint *items = data->items;
    for (size_t i = 1; i < size; i++) {
        int j = (int)i - 1;
        for (; j >= 0; j--) {
            if (compare(&items[j], &items[i]) <= 0 &&
                compare(&items[i], &items[j + 1]) <= 0) {
                break;
            }
        }

        DataPoint t = items[i];
        for (int k = (int)i; k > j + 1; k--) {
            items[k] = items[k - 1];
        }
        items[j + 1] = t;
    }
}

void mergesort__merge(Data *data, size_t start, size_t mid, size_t end,
                      compareFn compare, Data *scratch) {
    vec_clear(scratch);

    DataPoint *items = data->items;
    size_t p1 = start;
    size_t p2 = mid;
    while (p1 < mid && p2 < end) {
        if (compare(&items[p1], &items[p2]) <= 0) {  // <= for stable
            vec_push(scratch, items[p1]);
            p1++;
        } else {
            vec_push(scratch, items[p2]);
            p2++;
        }
    }
    while (p1 < mid) {
        vec_push(scratch, items[p1++]);
    }
    while (p2 < end) {
        vec_push(scratch, items[p2++]);
    }

    memcpy(&data->items[start], &scratch->items[0],
           sizeof(scratch->items[0]) * (end - start));
}

void mergesort__impl(Data *data, compareFn compare, size_t start, size_t end,
                     Data *scratch) {
    if ((end - start) < 2) return;
    size_t mid = start + (end - start) / 2;
    mergesort__impl(data, compare, start, mid, scratch);
    mergesort__impl(data, compare, mid, end, scratch);
    mergesort__merge(data, start, mid, end, compare, scratch);
}

void mergesort(Data *data, compareFn compare) {
    Data scratch = {0};
    vec_reserve(&scratch, data->size);
    mergesort__impl(data, compare, 0, data->size, &scratch);
    vec_free(&scratch);
}

// Hoare partition https://en.wikipedia.org/wiki/Quicksort
// end is inclusive
size_t quicksort__part(Data *data, compareFn compare, size_t start, size_t end,
                       int pivot_method) {
    DataPoint *items = data->items;

    size_t pivot = 0;
    if (pivot_method == 0)
        pivot = start;
    else if (pivot_method == 1)
        pivot = end;
    else if (pivot_method == 2)
        pivot = start + (size_t)rngi_range_tls(0, (int)(end - start));
    else
        UNREACHABLE("unknown pivot method");
    DataPoint pivot_val = items[pivot];

    ptrdiff_t i = (ptrdiff_t)start;
    ptrdiff_t j = (ptrdiff_t)end;
    swap(&items[pivot], &items[start]);

    while (true) {
        while (compare(&items[i], &pivot_val) < 0) i++;

        while (compare(&items[j], &pivot_val) > 0) j--;

        if (i >= j) return (size_t)j;

        swap(&items[i], &items[j]);
        i++;
        j--;
    }
}

// end inclusive
void quicksort__impl(Data *data, compareFn compare, size_t start, size_t end,
                     int pivot_method) {
    if (start >= end) return;

    size_t p = quicksort__part(data, compare, start, end, pivot_method);
    quicksort__impl(data, compare, start, p, pivot_method);
    quicksort__impl(data, compare, p + 1, end, pivot_method);
}

void quicksort_first(Data *data, compareFn compare) {
    if (data->size <= 1) return;
    quicksort__impl(data, compare, 0, data->size - 1, 0);
}

void quicksort_last(Data *data, compareFn compare) {
    if (data->size <= 1) return;
    quicksort__impl(data, compare, 0, data->size - 1, 1);
}

void quicksort_rand(Data *data, compareFn compare) {
    if (data->size <= 1) return;
    quicksort__impl(data, compare, 0, data->size - 1, 2);
}

void heapsort__insert(DataPoint *d, Data *heap, compareFn compare) {
    vec_push(heap, *d);
    size_t index = heap->size - 1;
    DataPoint *items = heap->items;
    while (index > 1) {
        size_t parent_index = index / 2;
        if (compare(d, &items[parent_index]) < 0) {
            swap(&items[index], &items[parent_index]);
            index = parent_index;
        } else
            break;
    }
}

DataPoint heapsort__pop(Data *heap) {
    DataPoint *items = heap->items;
    DataPoint result = items[1];

    vec_remove_swap(heap, 1);
    size_t index = 1;

    while (true) {
        size_t left = 2 * index;
        size_t right = left + 1;

        if (left >= heap->size) break;

        size_t smallest = left;

        if (right < heap->size && compare(&items[right], &items[left]) < 0)
            smallest = right;

        if (compare(&items[index], &items[smallest]) <= 0) break;

        swap(&items[index], &items[smallest]);
        index = smallest;
    }

    return result;
}

void heapsort(Data *data, compareFn compare) {
    size_t size = data->size;
    Data heap = {0};
    vec_reserve(&heap, size + 1);  // starting heap from index 1
    vec_push(&heap, (DataPoint){0});

    DataPoint *items = data->items;
    DataPoint *items_end = &items[size - 1];
    for (; items <= items_end; items++) {
        heapsort__insert(items, &heap, compare);
    }

    items = data->items;
    for (size_t i = 0; i < size; i++) {
        DataPoint d = heapsort__pop(&heap);
        *items = d;
        items++;
    }
    vec_free(&heap);
}

void shellsort__impl(Data *data, compareFn compare, size_t *gaps,
                     size_t gapsn) {
    size_t size = data->size;
    DataPoint *items = data->items;

    for (size_t gi = 0; gi < gapsn; gi++) {
        size_t gap = gaps[gi];

        for (size_t i = gap; i < size; i++) {
            DataPoint temp = items[i];

            size_t j = i;

            for (; j >= gap && compare(&items[j - gap], &temp) > 0; j -= gap) {
                items[j] = items[j - gap];
            }
            items[j] = temp;
        }
    }
}

// different gaps https://en.wikipedia.org/wiki/Shellsort
void shellsort_ciura(Data *data, compareFn compare) {
    size_t gaps[] = {701, 301, 132, 57, 23, 10, 4, 1};
    shellsort__impl(data, compare, gaps, ARRAY_LENGTH(gaps));
}

void shellsort_papernov(Data *data, compareFn compare) {
    Vector(size_t, Sizes);
    Sizes gaps = {0};
    vec_push(&gaps, 1);

    size_t pow = 2;
    while (pow < data->size) {
        vec_push(&gaps, pow + 1);
        pow *= 2;
    }

    vec_reverse(&gaps);  // TODO: just insert opposite??

    shellsort__impl(data, compare, gaps.items, gaps.size);
}

void shellsort_knuth(Data *data, compareFn compare) {
    Vector(size_t, Sizes);
    Sizes gaps = {0};

    size_t pow = 3;
    const size_t max = (data->size + 2) / 3;
    while (true) {
        size_t gap = (pow - 1) / 2;
        if (gap > max) break;
        vec_push(&gaps, gap);
        pow *= 3;
    }

    vec_reverse(&gaps);  // TODO: just insert opposite??

    shellsort__impl(data, compare, gaps.items, gaps.size);
}

// https://en.wikipedia.org/wiki/Timsort
// take 6 most significant digits and add 1 if any other are 1
size_t timsort__get_min_run(size_t size) {
    const size_t min_run = 32;
    size_t r = 0;

    while (size > min_run) {
        r |= (size & 1);
        size >>= 1;
    }

    return r + size;
}

void timsort(Data *data, compareFn compare) {
    size_t size = data->size;
    DataPoint *items = data->items;
    const size_t min_run = timsort__get_min_run(size);

    if (size < min_run) {
        insertionsort(data, compare);
        // heapsort(data, compare);
        // quicksort_rand(data, compare);
        return;
    }

    typedef struct {
        size_t start;
        size_t end;
    } Run;
    Vector(Run, Runs);
    Runs runs = {0};

    Data scratch = {0};
    vec_reserve(&scratch, data->size);

    size_t i = 0;
    while (i < size) {
        size_t run_end = i + 1;
        if (run_end != size) {
            if (compare(&items[i], &items[run_end]) < 0) {  // ascending
                while (run_end < size &&
                       compare(&items[run_end - 1], &items[run_end]) <= 0) {
                    run_end++;
                }
            } else {
                while (run_end < size &&
                       compare(&items[run_end - 1], &items[run_end]) > 0) {
                    run_end++;
                }
                Data subvec = vec_subvec(data, i, run_end);
                vec_reverse(&subvec);
            }
        }

        size_t run_len = run_end - i;

        if (run_len < min_run) {
            size_t end = (i + min_run < size) ? (i + min_run) : size;
            Data subvec = vec_subvec(data, i, end);
            insertionsort(&subvec, compare);
            run_end = end;
        }

        vec_push(&runs, ((Run){i, run_end}));

        i = run_end;

        while (runs.size > 1) {
            const size_t r_size = runs.size;
            size_t l1 = runs.items[r_size - 2].start;
            size_t r1 = runs.items[r_size - 2].end;
            size_t l2 = runs.items[r_size - 1].start;
            size_t r2 = runs.items[r_size - 1].end;

            size_t len1 = r1 - l1;
            size_t len2 = r2 - l2;

            if (len1 <= len2) {
                mergesort__merge(data, l1, r1, r2, compare, &scratch);
                runs.size--;
                runs.items[runs.size - 1] = (Run){l1, r2};
            } else
                break;
        }
    }

    while (runs.size > 1) {
        const size_t r_size = runs.size;
        size_t l1 = runs.items[r_size - 2].start;
        size_t r1 = runs.items[r_size - 2].end;
        size_t r2 = runs.items[r_size - 1].end;

        mergesort__merge(data, l1, r1, r2, compare, &scratch);
        runs.size--;
        runs.items[runs.size - 1] = (Run){l1, r2};
    }
}

void introsort__impl(Data *data, compareFn compare, size_t start, size_t end,
                     size_t max_depth) {
    Data *subvec = &vec_subvec(data, start, end + 1);
    if ((end - start + 1) < 16) {
        insertionsort(subvec, compare);
    } else if (max_depth == 0) {
        heapsort(subvec, compare);
    } else {
        size_t p = quicksort__part(data, compare, start, end, 2);
        introsort__impl(data, compare, start, p, max_depth - 1);
        introsort__impl(data, compare, p + 1, end, max_depth - 1);
    }
}

void introsort(Data *data, compareFn compare) {
    if (data->size <= 1) return;
    size_t size = data->size;
    const size_t max_depth = 2 * (size_t)(logf((float)size) / logf(2.0f));
    introsort__impl(data, compare, 0, size - 1, max_depth);
}

// Trying to make quicksort parallel
void parallelsort__impl(Data *data, compareFn compare, size_t start,
                        size_t end) {
    if (start >= end) return;
    if ((end - start + 1) < 64) {  // arbitrary
        quicksort__impl(data, compare, start, end, 2);
        return;
    }

    size_t p = quicksort__part(data, compare, start, end, 2);
#pragma omp task
    parallelsort__impl(data, compare, start, p);
#pragma omp task
    parallelsort__impl(data, compare, p + 1, end);

#pragma omp taskwait
}

void parallelsort(Data *data, compareFn compare) {
    if (data->size <= 1) return;
#pragma omp parallel
    {
        rng_seed_tls(0x12345678u ^
                     (uint32_t)(omp_get_thread_num() + 1) * 0x9e3779b9u);
#pragma omp single nowait
        parallelsort__impl(data, compare, 0, data->size - 1);
    }
}

// Final sorting function choosing the best
void sort(Data *data, compareFn compare) { quicksort_rand(data, compare); }

void libc_qsort(Data *data, compareFn compare) {
    qsort(data->items, data->size, sizeof(*data->items), compare);
}

bool testFAndD(sortFn sort_f, Data *data) {
    Data copy = {0};
    vec_copy(data, &copy);
    sort_f(&copy, &compare);
    bool result = validate_sorted(&copy);
    vec_free(&copy);
    return result;
}

bool testAllF(sortFn sort_f) {
    if (!testFAndD(sort_f, &empty_data)) {
        return false;
    }
    if (!testFAndD(sort_f, &single_entry_data)) {
        return false;
    }
    if (!testFAndD(sort_f, &random_data)) {
        return false;
    }
    if (!testFAndD(sort_f, &reverse_data)) {
        return false;
    }
    if (!testFAndD(sort_f, &duplicate_data)) {
        return false;
    }
    if (!testFAndD(sort_f, &sorted_data)) {
        return false;
    }
    if (!testFAndD(sort_f, &mostly_sorted_data)) {
        return false;
    }
    return true;
}

#define TEST(f)                              \
    do {                                     \
        if (testAllF(&f)) {                  \
            fprintf(stdout, #f " passed\n"); \
        } else {                             \
            fprintf(stderr, #f " failed\n"); \
            return 1;                        \
        }                                    \
    } while (0)

double benchOne(sortFn sortF, const Data *data) {
    Data copy = {0};
    vec_copy(data, &copy);
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    sortF(&copy, &compare);
    clock_gettime(CLOCK_MONOTONIC, &end);
    // if (!validate_sorted(&copy)) {
    //     printf("failed\n");
    //     exit(1);
    // }
    vec_free(&copy);
    return (double)(end.tv_nsec - start.tv_nsec) * 1e-6 +
           (double)(end.tv_sec - start.tv_sec) * 1000.0;
}

void bench(int sort_count, ...) {
    const int sizes[] = {5, 10, 50, 100, 501, 1000, 5001, 10000, 100000};
    const char *names[sort_count];
    sortFn sortFs[sort_count];

    va_list args;
    va_start(args, sort_count);
    for (int i = 0; i < sort_count; i++) {
        names[i] = va_arg(args, const char *);
        sortFs[i] = va_arg(args, sortFn);
    }
    va_end(args);

    for (size_t i = 0; i < ARRAY_LENGTH(sizes); i++) {
        init_data(sizes[i]);
        printf("size=%d\n", sizes[i]);
        printf("%-15s", "dataset");
        for (int j = 0; j < sort_count; j++) {
            printf(" %18s", names[j]);
        }
        printf("\n");

        printf("%-15s", "random_data");
        for (int j = 0; j < sort_count; j++) {
            printf(" %16.3fms", benchOne(sortFs[j], &random_data));
        }
        printf("\n");

        printf("%-15s", "reverse_data");
        for (int j = 0; j < sort_count; j++) {
            printf(" %16.3fms", benchOne(sortFs[j], &reverse_data));
        }
        printf("\n");

        printf("%-15s", "duplicate_data");
        for (int j = 0; j < sort_count; j++) {
            printf(" %16.3fms", benchOne(sortFs[j], &duplicate_data));
        }
        printf("\n");

        printf("%-15s", "sorted_data");
        for (int j = 0; j < sort_count; j++) {
            printf(" %16.3fms", benchOne(sortFs[j], &sorted_data));
        }
        printf("\n");

        printf("%-15s", "mostly_sorted");
        for (int j = 0; j < sort_count; j++) {
            printf(" %16.3fms", benchOne(sortFs[j], &mostly_sorted_data));
        }
        printf("\n\n");
    }
}

// http://aturing.umcs.maine.edu/~sudarshan.chawathe/200801/capstone/n/qsort.c.html
int main(void) {
    // rng_seed_tls((uint32_t)time(NULL));

    rng_seed_tls(0);
    init_data(51);
    TEST(bubblesort);
    TEST(selectionsort);
    TEST(insertionsort);
    TEST(mergesort);
    TEST(quicksort_first);
    TEST(quicksort_last);
    TEST(quicksort_rand);
    TEST(heapsort);
    TEST(shellsort_ciura);
    TEST(shellsort_papernov);
    TEST(shellsort_knuth);
    TEST(timsort);
    TEST(introsort);
    TEST(parallelsort);
    TEST(sort);
    TEST(libc_qsort);
    printf("\n");

    bench(16, "bubblesort", bubblesort, "selectionsort", selectionsort,
          "insertionsort", insertionsort, "mergesort", mergesort,
          "quicksort_first", quicksort_first, "quicksort_last", quicksort_last,
          "quicksort_rand", quicksort_rand, "heapsort", heapsort,
          "shellsort_ciura", shellsort_ciura, "shellsort_papernov",
          shellsort_papernov, "shellsort_knuth", shellsort_knuth, "timsort",
          timsort, "introsort", introsort, "parallelsort", parallelsort, "sort",
          sort, "libc_qsort", libc_qsort);
    return 0;
}
