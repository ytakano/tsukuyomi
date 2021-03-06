#include "../../src/lunar_hash.hpp"
#include "../../src/lunar_slab_allocator.hpp"
#include "hopscotch.hpp"

#include <unordered_set>

#include <sys/time.h>

#define NTRIAL 1
#define NUM 10000000

double diff_tm(timeval &tm0, timeval &tm1)
{
    double t0 = tm0.tv_sec + tm0.tv_usec * 1e-6;
    double t1 = tm1.tv_sec + tm1.tv_usec * 1e-6;

    return t1 - t0;
}

uint64_t
bench_lunar_hash_set()
{
    lunar::hash_set<uint64_t> hs;
    timeval  tm0, tm1;
    uint64_t n = 0;

    gettimeofday(&tm0, nullptr);

    for (int j = 0; j < NTRIAL; j++) {
        for (int i = 0; i < NUM; i++) {
            hs.insert(i);
        }
    }

    gettimeofday(&tm1, nullptr);
    printf("lunar::hash_set: insertion:\t\t\t%lf[ops/s]\n", NUM * NTRIAL / diff_tm(tm0, tm1));
    gettimeofday(&tm0, nullptr);

    for (int j = 0; j < NTRIAL; j++) {
        for (int i = 0; i < NUM; i++) {
            hs.erase(i);
            hs.insert(i);
        }
    }

    gettimeofday(&tm1, nullptr);
    printf("lunar::hash_set: insertion & deletion:\t\t%lf[ops/s]\n", NUM * NTRIAL / diff_tm(tm0, tm1));


    gettimeofday(&tm0, nullptr);


    for (int j = 0; j < NTRIAL; j++) {
        for (int i = 0; i < NUM; i++) {
            if (hs.find(i) != hs.end())
                n += i;
        }
    }

    gettimeofday(&tm1, nullptr);
    printf("lunar::hash_set: lookup:\t\t\t%lf[ops/s]\n", NUM * NTRIAL / diff_tm(tm0, tm1));

    return n;
}

uint64_t
bench_lunar_hash_map()
{
    lunar::hash_map<uint64_t, uint64_t> hm;
    timeval  tm0, tm1;
    uint64_t n = 0;

    gettimeofday(&tm0, nullptr);

    for (int j = 0; j < NTRIAL; j++) {
        for (int i = 0; i < NUM; i++) {
            hm.insert({i, i});
        }
    }

    gettimeofday(&tm1, nullptr);
    printf("lunar::hash_map: insertion:\t\t\t%lf[ops/s]\n", NUM * NTRIAL / diff_tm(tm0, tm1));
    gettimeofday(&tm0, nullptr);

    for (int j = 0; j < NTRIAL; j++) {
        for (int i = 0; i < NUM; i++) {
            hm.erase(i);
            hm.insert({i, i});
        }
    }

    gettimeofday(&tm1, nullptr);
    printf("lunar::hash_map: insertion & deletion:\t\t%lf[ops/s]\n", NUM * NTRIAL / diff_tm(tm0, tm1));


    gettimeofday(&tm0, nullptr);

    for (int j = 0; j < NTRIAL; j++) {
        for (int i = 0; i < NUM; i++) {
            if (hm.find(i) != hm.end())
                n += i;
        }
    }

    gettimeofday(&tm1, nullptr);
    printf("lunar::hash_map: lookup:\t\t\t%lf[ops/s]\n", NUM * NTRIAL / diff_tm(tm0, tm1));

    return n;
}

uint64_t
bench_unordered()
{
    std::unordered_set<uint64_t> hs;
    timeval  tm0, tm1;
    uint64_t n = 0;

    gettimeofday(&tm0, nullptr);

    for (int j = 0; j < NTRIAL; j++) {
        for (int i = 0; i < NUM; i++) {
            hs.insert(i);
        }
    }

    gettimeofday(&tm1, nullptr);
    printf("unordered_set: insertion:\t\t\t%lf[ops/s]\n", NUM * NTRIAL / diff_tm(tm0, tm1));
    gettimeofday(&tm0, nullptr);

    for (int j = 0; j < NTRIAL; j++) {
        for (int i = 0; i < NUM; i++) {
            hs.erase(i);
            hs.insert(i);
        }
    }

    gettimeofday(&tm1, nullptr);
    printf("unordered_set: insertion & deletion:\t\t%lf[ops/s]\n", NUM * NTRIAL / diff_tm(tm0, tm1));


    gettimeofday(&tm0, nullptr);

    for (int j = 0; j < NTRIAL; j++) {
        for (int i = 0; i < NUM; i++) {
            if (hs.find(i) != hs.end())
                n += i;
        }
    }

    gettimeofday(&tm1, nullptr);
    printf("unordered_set: lookup:\t\t\t\t%lf[ops/s]\n", NUM * NTRIAL / diff_tm(tm0, tm1));

    return n;
}

uint64_t
bench_unordered_slab()
{
    std::unordered_set<uint64_t, std::hash<uint64_t>, std::equal_to<uint64_t>, lunar::slab_allocator<uint64_t>> hs;
    timeval  tm0, tm1;
    uint64_t n = 0;

    gettimeofday(&tm0, nullptr);

    for (int j = 0; j < NTRIAL; j++) {
        for (int i = 0; i < NUM; i++) {
            hs.insert(i);
        }
    }

    gettimeofday(&tm1, nullptr);
    printf("unordered_set<slab>: insertion:\t\t\t%lf[ops/s]\n", NUM * NTRIAL / diff_tm(tm0, tm1));
    gettimeofday(&tm0, nullptr);

    for (int j = 0; j < NTRIAL; j++) {
        for (int i = 0; i < NUM; i++) {
            hs.erase(i);
            hs.insert(i);
        }
    }

    gettimeofday(&tm1, nullptr);
    printf("unordered_set<slab>: insertion & deletion:\t%lf[ops/s]\n", NUM * NTRIAL / diff_tm(tm0, tm1));


    gettimeofday(&tm0, nullptr);

    for (int j = 0; j < NTRIAL; j++) {
        for (int i = 0; i < NUM; i++) {
            if (hs.find(i) != hs.end())
                n += i;
        }
    }

    gettimeofday(&tm1, nullptr);
    printf("unordered_set<slab>: lookup:\t\t\t%lf[ops/s]\n", NUM * NTRIAL / diff_tm(tm0, tm1));

    return n;
}

uint64_t
bench_hopscotch()
{
    nanahan::Map<uint64_t, uint64_t, std::hash<uint64_t>, std::equal_to<uint64_t>, lunar::slab_allocator<std::pair<const uint64_t, uint64_t>>> hs;
    timeval  tm0, tm1;
    uint64_t n = 0;

    gettimeofday(&tm0, nullptr);

    for (int j = 0; j < NTRIAL; j++) {
        for (int i = 0; i < NUM; i++) {
            hs.insert({i, i});
        }
    }

    gettimeofday(&tm1, nullptr);
    printf("hopscotch<slab>: insertion:\t\t\t%lf[ops/s]\n", NUM * NTRIAL / diff_tm(tm0, tm1));
    gettimeofday(&tm0, nullptr);

    for (int j = 0; j < NTRIAL; j++) {
        for (int i = 0; i < NUM; i++) {
            hs.erase(hs.find(i));
            hs.insert({i, i});
        }
    }

    gettimeofday(&tm1, nullptr);
    printf("hopscotch<slab>: insertion & deletion:\t\t%lf[ops/s]\n", NUM * NTRIAL / diff_tm(tm0, tm1));


    gettimeofday(&tm0, nullptr);

    for (int j = 0; j < NTRIAL; j++) {
        for (int i = 0; i < NUM; i++) {
            if (hs.find(i) != hs.end())
                n += i;
        }
    }

    gettimeofday(&tm1, nullptr);
    printf("hopscotch<slab>: lookup:\t\t\t%lf[ops/s]\n", NUM * NTRIAL / diff_tm(tm0, tm1));

    return n;
}

int
main(int argc, char *argv[])
{
    printf("lunar::hash_set:     n = %llu\n", bench_lunar_hash_set());
    printf("lunar::hash_map:     n = %llu\n", bench_lunar_hash_map());
    printf("unordered_set:       n = %llu\n", bench_unordered());
    printf("unordered_set<slab>: n = %llu\n", bench_unordered_slab());
    printf("hopscotch<slab>:     n = %llu\n", bench_hopscotch());

    return 0;
}