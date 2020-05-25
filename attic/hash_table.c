/*
 * @(#) a simple hash table for dictionaries, basically so I can implement environments in carML
 * @(#) uses FNV1a as a hash function, and just stores them linearly...
 * @(#) 
 * @(#) the purpose of this and all the dictionary tests is to determine which of these is faster,
 * @(#) and at what point. If you think about it, most users will not actually store that much
 * @(#) in the environment frame, so there's little point in making it optimized for storing 
 * @(#) huge amounts of data. Instead, it's generally easier to optimize this for storing small
 * @(#) amounts of data. I can never find it, but I recall reading somewhere that Clojure (used to?)
 * @(#) store dictionaries as two linear arrays, because for small dictionaries were more common and
 * @(#) that method was Good Enough(TM) for most purposes.
 * @(#) 
 * @(#) we need to test a few different things:
 * @(#)
 * @(#) - average storage time
 * @(#) - average retrieval time
 * @(#) - worst-case storage time
 * @(#) - worst-case retrieval time
 * @(#) - are there collisions?
 * @(#) - performance of long names vs short names
 * @(#) - repeated stores
 * @(#) - repeated lookups
 * @(#)
 * @(#) and for each, we want to ignore the time taken to generate random names and 
 * @(#) the like. We also want to keep track of how long it takes for short names vs
 * @(#) long names, and at what point does it no longer make sense. For example, at 
 * @(#) what point does a linear table scan make sense for an evironment, and when
 * @(#) should it be upgraded to something meant to hold more data, like a trie?
 */

#include <stdint.h>
#include <stdio.h>
#include <sys/time.h>

struct _DICT {
    uint64_t keys[128];
    int values[128];
};

typedef struct _DICT dict;

uint64_t fnv1a(char *, uint32_t);

/* 
 * all of the dictionary objects should implement the following protocol:
 */
uint8_t store(dict *, char *, int);
uint8_t exists(dict *, char *);
uint8_t retrieve(dict *, char *, int *);
uint8_t clear(dict *);

/*
 * our test harness, also should be the same across versions...
 * @randname(buffer, length, uniform?)
 */
char *randname(char *, int, uint8_t);

/*
 * this could (and probably should) be the same across all 
 * implementations...
 */
int
main(void) {
    dict foo;
    // should make this configurable, so that we can
    // march this upwards until it no longer makes sense...
    char *randnames[32] = {0};
    char buf[128] = {0};
    uint32_t idx = 0, val = 0, time_idx = 0;
    // store all run times, then calculate average cases
    suseconds_t runtimes[256], avg_s_store, avg_r_store;
    suseconds_t avg_s_retrieve, avg_r_retrieve;
    // hold all our various interstitial timing information
    struct timeval stime, etime;

    for(; idx < 32; idx++) {
        // first pass, generate uniform names
        randnames[idx] = strdup(randname(buf, 128, YES));
    }    

    /*
     * test storage...
     */
    for(idx = 0; idx < 32; idx++) {
        // need to time these and record the timings...
        val = arc4random();
        gettimeofday(&stime, nil);
        store(foo, randnames[idx], val);
        gettimeofday(&etime, nil);
        runtimes[time_idx] = etime.tv_usec - stime.tv_usec;
        time_idx++;
        printf("%s:%ld\n", randnames[idx], val);
    }

    /*
     * test retrieval...
     */
    for(idx = 0; idx < 32; idx++) {
        // need to time these and record the timings...
        gettimeofday(&stime, nil);
        retrieve(foo, randnames[idx], &val);
        gettimeofday(&etime, nil);
        runtimes[time_idx] = etime.tv_usec - stime.tv_usec;
        time_idx++;
        printf("%s:%ld\n", randnames[idx], val);
    }

    /*
     * repeatedly look up a value...
     */
    val = arc4random_uniform(32);
    for(idx = 0; idx < 256; idx++) {
        gettimeofday(&stime, nil);
        if(exists(foo, randnames[val])) {
            printf("yes, it exists...\n");
        } else
            printf("no, it doesn't!\n");
        }
        gettimeofday(&etime, nil);
        runtimes[time_idx] = etime.tv_usec - stime.tv_usec;
        time_idx++;
    }

    /*
     * repeatedly store a value...
     */
    tidx = arc4random_uniform(32);
    for(idx = 0; idx < 256; idx++) {
        // using val here so that I can remove
        // arc4random from the timing loop...
        val = arc4random();
        gettimeofday(&stime, nil);
        store(foo, randnames[tidx], val);
        gettimeofday(&etime, nil);
        runtimes[time_idx] = etime.tv_usec - stime.tv_usec;
        time_idx++;
    }

    // ok, now that we've done that, stress test the whole thing...
    clear(foo);

    for(idx = 0; idx < 32; idx++) {
        free(randnames[idx]);
    }

    /*
     * larger smoke test...
     */

    for(; idx < 32; idx++) {
        // first pass, generate uniform names
        randnames[idx] = strdup(randname(buf, 128, NO));
    }    

    /*
     * test storage...
     */
    for(idx = 0; idx < 32; idx++) {
        // need to time these and record the timings...
        val = arc4random();
        store(foo, randnames[idx], val);
        printf("%s:%ld\n", randnames[idx], val);
    }

    /*
     * test retrieval...
     */
    for(idx = 0; idx < 32; idx++) {
        // need to time these and record the timings...
        retrieve(foo, randnames[idx], &val);
        printf("%s:%ld\n", randnames[idx], val);
    }

    /*
     * repeatedly look up a value...
     */
    val = arc4random_uniform(32);
    for(idx = 0; idx < 256; idx++) {
        if(exists(foo, randnames[val])) {
            printf("yes, it exists...\n");
        } else
            printf("no, it doesn't!\n");
        }
    }

    /*
     * repeatedly store a value...
     */
    val = arc4random_uniform(32);
    for(idx = 0; idx < 256; idx++) {
        store(foo, randnames[val], arc4random());
    }

    clear(dict);

    for(idx = 0; idx < 32; idx++) {
        free(randnames[idx]);
    }

    return 0;
}

uint64_t
fnv1a(char *key, uint32_t len)
{
    uint64_t hash = 14695981039346656037;
    uint32_t idx = 0;
    for(; idx < len; idx++)
    {
        hash ^= key[idx];
        hash *= 1099511628211;
    }
    return hash;
}

char *
randname(char *buf, size_t maxlength, uint8_t uniformp) {
    char *rc = nil;
    uint32_t base = 0, coin_flip = 0;
    size_t len = maxlength;

    if(buf != nil) {
        rc = buf;
    } else {
        rc = (char *)malloc(sizeof(char) * maxlength);
        if(!rc) {
            return nil;
        }
    }

    if(uniformp == NO) {
        len = arc4random_uniform(maxlength);
    }

    for(size_t idx = 0; idx < len; idx++) {
        base = arc4random_uniform(26);
        coin_flip = arc4random_uniform(1);
        if(coinflip == 0) {
            rc[idx] = 'a' + base;
        } else {
            rc[idx] = 'A' + base;
        }
    }

    return rc;
}
