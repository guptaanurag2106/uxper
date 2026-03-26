#include <omp.h>
#include <stdalign.h>
#include <stdio.h>

#define THREAD_COUNT 8
void pi(void) {
    double start = omp_get_wtime();
    double pi = 0;
    int steps = 100000000;
    double width = 1.0 / steps;

    for (int i = 0; i < steps; i++) {
        double x = (i + 0.5) * width;
        pi += 4.0 / (1 + x * x);
    }
    pi *= width;
    printf("pi is %f, calculated in %fs\n", pi, omp_get_wtime() - start);
}

void pi2(void) {
    double start = omp_get_wtime();
    alignas(64) double pi_vals[THREAD_COUNT] = {0};
    double pi = 0;
    int steps = 100000000, nthreads = -1;
    double width = 1.0 / steps;

#pragma omp parallel
    {
        int id = omp_get_thread_num();
        nthreads = omp_get_num_threads();
        double res = 0.0;

        for (int i = id; i < steps; i += nthreads) {
            double x = (i + 0.5) * width;
            res += 4.0 / (1 + x * x);
        }
        pi_vals[id] = res;
    }

    for (int i = 0; i < nthreads; i++) pi += pi_vals[i];
    pi *= width;
    printf("pi2 is %f, calculated in %fs using %d threads\n", pi,
           omp_get_wtime() - start, nthreads);
}

void pi3(void) {
    double start = omp_get_wtime();
    double pi_vals[THREAD_COUNT][8] = {0}, pi = 0;
    int steps = 100000000, nthreads = -1;
    double width = 1.0 / steps;

#pragma omp parallel
    {
        int id = omp_get_thread_num();
        nthreads = omp_get_num_threads();
        double res = 0.0;

        for (int i = id; i < steps; i += nthreads) {
            double x = (i + 0.5) * width;
            res += 4.0 / (1 + x * x);
        }
        pi_vals[id][0] = res;
    }

    for (int i = 0; i < nthreads; i++) pi += pi_vals[i][0];
    pi *= width;
    printf("pi3 is %f, calculated in %fs using %d threads\n", pi,
           omp_get_wtime() - start, nthreads);
}

void pi4(void) {
    double start = omp_get_wtime();
    double pi = 0;
    int steps = 100000000, nthreads = 1;
    double width = 1.0 / steps;

#pragma omp parallel
    {
#pragma omp master
        nthreads = omp_get_num_threads();
        int id = omp_get_thread_num();

        double res = 0;

        for (int i = id; i < steps; i += nthreads) {
            double x = (i + 0.5) * width;
            res += 4.0 / (1 + x * x);
        }

#pragma omp atomic
        pi += res;
    }

    pi *= width;
    printf("pi4 is %f, calculated in %fs using %d threads\n", pi,
           omp_get_wtime() - start, nthreads);
}

void pi5(void) {
    double start = omp_get_wtime();
    double pi = 0;
    int steps = 100000000, nthreads = 1;
    double width = 1.0 / steps;

#pragma omp parallel
    {
#pragma omp single nowait
        nthreads = omp_get_num_threads();

#pragma omp for reduction(+ : pi) schedule(auto)
        for (int i = 0; i < steps; i++) {
            double x = (i + 0.5) * width;
            pi += 4.0 / (1 + x * x);
        }
    }

    pi *= width;
    printf("pi5 is %f, calculated in %fs using %d threads\n", pi,
           omp_get_wtime() - start, nthreads);
}

void sections(void) {
#pragma omp parallel
    {
#pragma omp sections
        {
#pragma omp section
            {
                printf("Hello section 1\n");
            }
#pragma omp section
            {
                printf("Hello section 2\n");
            }
#pragma omp section
            {
                printf("Hello section 3\n");
            }
        }
    }
}

void static_test_helper(omp_lock_t *lock) {
    static int count = 0;
    omp_set_lock(lock);
    printf("static test count: %d, thread: %d\n", count++,
           omp_get_thread_num());
    omp_unset_lock(lock);
}

void static_test(void) {
    omp_lock_t lock;
    omp_init_lock(&lock);
#pragma omp parallel
    {
        static_test_helper(&lock);
    }
    omp_destroy_lock(&lock);
}

void private_test(void) {
    int a = 1, b = 1, c = 1, d = 1;
#pragma omp parallel for private(b) firstprivate(c) lastprivate(d)
    for (int i = 0; i < 200; i++) {
        a = 2, b = 2, c = 2, d = i;
    }
    printf("After private a: %d, b: %d, c: %d, d: %d\n", a, b, c, d);
}

int main(void) {
    omp_set_num_threads(THREAD_COUNT);
    printf("max_threads: %d, num_procs: %d\n", omp_get_max_threads(),
           omp_get_num_procs());

    // #pragma omp parallel
    //     {
    //         int id = omp_get_thread_num();
    //         printf("Hello World %d %d\n", id, omp_get_active_level());
    //     }

    pi();
    pi2();
    pi3();
    pi4();
    pi5();
    sections();
    static_test();
    private_test();

    return 0;
}
