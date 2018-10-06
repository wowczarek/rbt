/* BSD 2-Clause License
 *
 * Copyright (c) 2018, Wojciech Owczarek
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/**
 * @file   rbt_test.c
 * @date   Fri Sep 14 23:27:00 2018
 *
 * @brief  red-black tree implementation test code
 *
 */

/* because clock_gettime */
#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <time.h>
#include "rbt.h"
#include "rbt_display.h"

/* constants */
#define TESTSIZE 1000
#define KEEPSIZE 20
#define HSIZE 80
#define VSIZE 20

/* basic duration measurement macros */
#define DUR_INIT(name) unsigned long long name##_delta; struct timespec name##_t1, name##_t2;
#define DUR_START(name) clock_gettime(CLOCK_MONOTONIC,&name##_t1);
#define DUR_END(name) clock_gettime(CLOCK_MONOTONIC,&name##_t2); name##_delta = (name##_t2.tv_sec * 1000000000 + name##_t2.tv_nsec) - (name##_t1.tv_sec * 1000000000 + name##_t1.tv_nsec);
#define DUR_PRINT(name, msg) fprintf(stderr, "%s: %llu ns\n", msg, name##_delta);
#define DUR_EPRINT(name, msg) DUR_END(name); fprintf(stderr, "%s: %llu ns\n", msg, name##_delta);

enum {
	BENCH_NONE,
	BENCH_INSERT,
	BENCH_REMOVE,
	BENCH_SEARCH,
	BENCH_INC_SEARCH,
	BENCH_DEC_SEARCH
};

/* generate a Fisher-Yates shuffled array of n uint32s */
static uint32_t* randArrayU32(const int count) {

    int i;
    struct timeval t;

    /* good idea oh Lord */
    gettimeofday(&t, NULL);
    /* of course it's a good idea! */
    srand(t.tv_sec + t.tv_usec);

    uint32_t* ret = malloc(count * sizeof(uint32_t));

    for(i = 0; i < count; i++) {
	ret[i] = i;
    }

    for(i = 0; i < count; i++) {
	uint32_t j = i + rand() % (count - i);
	uint32_t tmp = ret[j];
	ret[j] = ret[i];
	ret[i] = tmp;
    }

    return ret;

}

static void usage() {

    fprintf(stderr, "rbt_test (c) 2018: Wojciech Owczarek, a simple red-black tree implementation\n\n"
	   "usage: rbt_test [-w NUMBER] [-H NUMBER] [-n NUMBER] [-r NUMBER] [-b NUMBER]\n"
	   "                [-s] [-m] [-e] [-l] [-o] [-i NUMBER]\n"
	   "\n"
	   "-w NUMBER       Width of text block displaying the final tree, default %d\n"
	   "-H NUMBER       Height of text block displaying the final tree, default %d\n"
	   "-n NUMBER       Number of random nodes to insert into tree, default %d\n"
	   "-r NUMBER       Number of nodes to leave on tree after removal, default %d\n"
	   "-b NUMBER       Number of nodes to randomly turn red to invalidate tree, default 0\n"
	   "-s              Test insertion only, generate CSV output on stdout\n"
	   "-m              Test removal only, CSV output to stdout\n"
	   "-e              Test search only, CSV output to stdout\n"
	   "-l              Test incremental search only (during insertion), CSV output to stdout\n"
	   "-o              Test decremental search only (during removal), CSV output to stdout\n"
	   "-i NUMBER       CSV log output interval, default every 1000 nodes,  unless\n"
	   "                1000 < 1%% node count, then 1%% node count is used.\n"
	   "\n", HSIZE, VSIZE, TESTSIZE, KEEPSIZE);

}

static void runBench(RbTree *tree, const int benchtype, const int testsize, int testinterval, uint32_t *iarr, uint32_t *rarr, uint32_t *sarr) {

    DUR_INIT(test);
    int found = 0;
    int i;

    switch(benchtype) {

	case BENCH_REMOVE:
	case BENCH_SEARCH:
	case BENCH_DEC_SEARCH:

	    fprintf(stderr, "Inserting %d random keys... ", testsize);
	    fflush(stderr);
	    for(i = 0; i < testsize; i++) {
		rbInsert(tree, iarr[i]);
	    }
	    fprintf(stderr, "done.\n");

	    break;

	default:
	    break;

    }

    switch(benchtype) {

	case BENCH_INSERT:

	    fprintf(stderr, "Generating CSV output for insertion of %d random keys... ", testsize);
	    fflush(stderr);

	    fprintf(stdout, "node_count,ns_per_insertion\n");

	    for (int j = 0; j < testsize; j += testinterval) {

		DUR_START(test);
		for(i = j; i < j + testinterval; i++) {
		    rbInsert(tree, iarr[i]);
		}
		DUR_END(test);
		fprintf(stdout, "%d,%llu\n", ((j + testinterval) > testsize) ? testsize : j + testinterval, test_delta / testinterval);

	    }

	    fprintf(stderr, "done.\n");

	    break;

	case BENCH_REMOVE:

	    fprintf(stderr, "Generating CSV output for removal of %d random keys... ", testsize);
	    fflush(stderr);

	    fprintf(stdout, "node_count,ns_per_removal\n");

	    for (int j = 0; j < testsize; j += testinterval) {

		DUR_START(test);
		for(i = j; i < j + testinterval; i++) {
		    rbDeleteKey(tree, rarr[i]);
		}
		DUR_END(test);
		fprintf(stdout, "%d,%llu\n", ((j + testinterval) > testsize) ? testsize : j + testinterval, test_delta / testinterval);

	    }

	    fprintf(stderr, "done.\n");

	    break;

	case BENCH_SEARCH:

	    found = 0;
	    fprintf(stderr, "Generating CSV output for search of %d random keys... ", testsize);
	    fflush(stderr);

	    fprintf(stdout, "iterations,ns_per_search\n");

	    for (int j = 0; j < testsize; j += testinterval) {

		DUR_START(test);
		for(i = j; i < j + testinterval; i++) {

		    RbNode* n = rbSearch(tree->root, sarr[i]);

		    if(n != NULL && n->key == sarr[i]) {
			found++;
		    }

		}
		DUR_END(test);
		fprintf(stdout, "%d,%llu\n", ((j + testinterval) > testsize) ? testsize : j + testinterval, test_delta / testinterval);

	    }

	    fprintf(stderr, "%d found.\n", found);

	    break;

	case BENCH_INC_SEARCH:

	    found = 0;

	    fprintf(stderr, "Generating CSV output for incremental search during insertion of %d random keys... ", testsize);
	    fflush(stdout);

	    fprintf(stdout, "node_count,ns_per_search\n");

	    for (int j = 0; j < testsize; j += testinterval) {

		i = 0;
		for(i = j; i < j + testinterval; i++) {
		    rbInsert(tree, iarr[i]);
		}

		uint32_t larr[testinterval];

		/* __almost__ a n ideal random n-sample from all of current iarr */
		for(int k = 0; k < testinterval; k++) {
		    larr[k] = iarr[sarr[ j + k ] % i];
		}

		DUR_START(test);
		for(int k = 0; k < testinterval; k++) {
		    RbNode* n = rbSearch(tree->root, larr[k]);

		    if(n != NULL && n->key == larr[k]) {
			found++;
		    }
		}
		DUR_END(test);
		fprintf(stdout, "%d,%llu\n", ((j + testinterval) > testsize) ? testsize : j + testinterval, test_delta / testinterval);
	    }

	    fprintf(stderr, "%d found.\n", found);

	    break;

	case BENCH_DEC_SEARCH:

	    found = 0;

	    fprintf(stderr, "Generating CSV output for search during removal of %d random keys... ", testsize); 
	    fflush(stdout);

	    fprintf(stdout, "node_count,ns_per_search\n");

	    for (int j = 0; j < testsize; j += testinterval) {

		DUR_START(test);
		for(int i = j; i < j + testinterval; i++) {
		    RbNode* n = rbSearch(tree->root, rarr[i]);

		    if(n != NULL && n->key == rarr[i]) {
			found++;
		    }
		}
		DUR_END(test);
		fprintf(stdout, "%d,%llu\n", ((j + testinterval) > testsize) ? testsize : j + testinterval, test_delta / testinterval);

		for(i = j; i < j + testinterval; i++) {
		    rbDeleteKey(tree, rarr[i]);
		}

	    }

	    fprintf(stderr, "%d found.\n", found);


	    break;

	case BENCH_NONE:
	default:
	    break;
    }


}

int main(int argc, char **argv) {

    int c;
    int i;
    int hsize = HSIZE;
    int vsize = VSIZE;
    int testsize = TESTSIZE;
    int keepsize = KEEPSIZE;
    int breaksize = 0;
    int testinterval = 0;
    int found = 0;
    int bench = BENCH_NONE;
    char obuf[2001];
    char *buf = obuf;
    char *dump;
    RbTree *tree = rbCreate();
    uint32_t *iarr, *rarr, *sarr;
    DUR_INIT(test);

    memset(obuf, 0, sizeof(obuf));

	while ((c = getopt(argc, argv, "?hw:H:n:r:b:smeloi:")) != -1) {

	    switch(c) {
		case 'w':
		    hsize = atoi(optarg);
		    if(hsize <= 0) {
			hsize = HSIZE;
		    }
		    break;
		case 'H':
		    vsize = atoi(optarg);
		    if(vsize <= 0) {
			vsize = VSIZE;
		    }
		    break;
		case 'n':
		    testsize = atoi(optarg);
		    if(testsize <= 0) {
			testsize = TESTSIZE;
		    }
		    break;
		case 'r':
		    keepsize = atoi(optarg);
		    if(keepsize <= 0) {
			keepsize = KEEPSIZE;
		    }
		    break;
		case 'b':
		    breaksize = atoi(optarg);
		    if(breaksize < 0 || breaksize > keepsize) {
			breaksize = 0;
		    }
		    break;
		case 's':
		    bench = BENCH_INSERT;
		    break;
		case 'm':
		    bench = BENCH_REMOVE;
		    break;
		case 'e':
		    bench = BENCH_SEARCH;
		    break;
		case 'l':
		    bench = BENCH_INC_SEARCH;
		    break;
		case 'o':
		    bench = BENCH_DEC_SEARCH;
		    break;
		case 'i':
		    testinterval = atoi(optarg);
		    if(testinterval <= 0) {
			testinterval = testsize / 100;
		    }
		    break;
		case '?':
		case 'h':
		default:
		    usage();
		    return -1;
	    }
	}

    if(testinterval == 0) {
	testinterval = 1000;
    }

    if((testsize / testinterval) < 100) {
	testinterval = testsize / 100;
    }

    if(testinterval < 2) {
	testinterval = 2;
    }

    fprintf(stderr, "Generating %d size random insertion, removal and search key arrays... ", testsize);
    fflush(stderr);

    iarr = randArrayU32(testsize);
    rarr = randArrayU32(testsize);
    sarr = randArrayU32(testsize);

    fprintf(stderr, "done.\n");

    if(bench != BENCH_NONE) {
	runBench(tree, bench, testsize, testinterval, iarr, rarr, sarr);
	goto cleanup;
    }

    fprintf(stderr, "Inserting %d random keys... ", testsize);
    fflush(stderr);

    DUR_START(test);
    for(i = 0; i < testsize; i++) {
	rbInsert(tree, iarr[i]);
    }
    DUR_END(test);
    fprintf(stderr, "done.\n");

    buf += sprintf(buf, "+---------------------------------+-------------+---------+\n");
    buf += sprintf(buf, "| Test                            | result      | unit    |\n");
    buf += sprintf(buf, "+---------------------------------+-------------+---------+\n");
    buf += sprintf(buf, "| Insertion, count %-10d     "   "| %-11llu "  "| ns/key  |\n", testsize, test_delta / testsize);
    buf += sprintf(buf, "| Insertion, rate                 | %-11.0f "  "| nodes/s |\n", (1000000000.0 / test_delta) * testsize );

    fprintf(stderr, "Verifying red-black tree... ");
    fflush(stderr);

    DUR_START(test);
    if(!rbVerify(tree, RB_CHATTY)) {
	dump = rbDisplay(tree, hsize, vsize, RB_NO_NULL);
	printf("%s\n\n", dump);
	free(dump);
	fprintf(stderr, "Call me stupid, but this tree is broken. Node insertion implementation FAIL.\n");
	return -1;
    }
    DUR_END(test);
    buf += sprintf(buf, "| Verification, rate              | %-11.0f "  "| nodes/s |\n", (1000000000.0 / test_delta) * testsize );

    fprintf(stderr, "Finding all %d keys in random order... ", testsize);
    fflush(stderr);

    found = 0;
    DUR_START(test);
    for(i = 0; i < testsize; i++) {

	RbNode* n = rbSearch(tree->root, sarr[i]);

	if(n != NULL && n->key == sarr[i]) {
	    found++;
	}

    }
    DUR_END(test);
    fprintf(stderr, "%d found.\n", found);
    buf += sprintf(buf, "| Search, count %-10d        "   "| %-11llu "  "| ns/key  |\n", testsize, test_delta / testsize);
    buf += sprintf(buf, "| Search, rate                    "   "| %-11.0f "  "| hit/s   |\n", (1000000000.0 / test_delta) * testsize);

    fprintf(stderr, "Finding all %d keys in sequential order... ", testsize);
    fflush(stderr);

    found = 0;
    DUR_START(test);
    for(i = 0; i < testsize; i++) {

	RbNode* n = rbSearch(tree->root, i);

	if(n != NULL && n->key == i) {
	    found++;
	}

    }
    DUR_END(test);
    fprintf(stderr, "%d found.\n", found);
    buf += sprintf(buf, "| Seq search, count %-10d    "   "| %-11llu "  "| ns/key  |\n", testsize, test_delta / testsize);
    buf += sprintf(buf, "| Seq search, rate                "   "| %-11.0f "  "| hit/s   |\n", (1000000000.0 / test_delta) * testsize);

    fprintf(stderr, "Performing in-order traversal with height and black height tracking... ");
    fflush(stderr);

    DUR_START(test);
    rbInOrderTrack(tree, rbDummyCallback, NULL, RB_ASC);
    DUR_END(test);
    fprintf(stderr, "done.\n");
    buf += sprintf(buf, "| In-order, with tracking, rate   | %-11.0f "  "| nodes/s |\n", (1000000000.0 / test_delta) * testsize );

    fprintf(stderr, "Performing in-order traversal without height and black height tracking... ");
    fflush(stderr);

    DUR_START(test);
    rbInOrder(tree, rbDummyCallback, NULL, RB_ASC);
    DUR_END(test);
    fprintf(stderr, "done.\n");
    buf += sprintf(buf, "| In-order, fast, rate            | %-11.0f "  "| nodes/s |\n", (1000000000.0 / test_delta) * testsize );

    fprintf(stderr, "Performing breadth-first traversal with height and black height tracking... ");
    fflush(stderr);

    DUR_START(test);
    rbBreadthFirstTrack(tree, rbDummyCallback, NULL, RB_ASC);
    DUR_END(test);
    fprintf(stderr, "done.\n");
    buf += sprintf(buf, "| Breadth first, tracking, rate   | %-11.0f "  "| nodes/s |\n", (1000000000.0 / test_delta) * testsize );

    fprintf(stderr, "Performing breadth-first traversal without height and black height tracking... ");
    fflush(stderr);

    DUR_START(test);
    rbBreadthFirst(tree, rbDummyCallback, NULL, RB_ASC);
    DUR_END(test);
    fprintf(stderr, "done.\n");
    buf += sprintf(buf, "| Breadth first, fast, rate       | %-11.0f "  "| nodes/s |\n", (1000000000.0 / test_delta) * testsize );

    fprintf(stderr, "Destroying tree... ");
    fflush(stderr);
    DUR_START(test);
    rbFree(tree);
    DUR_END(test);
    fprintf(stderr, "done.\n");
    buf += sprintf(buf, "| Destruction, rate               | %-11.0f "  "| nodes/s |\n", (1000000000.0 / test_delta) * testsize );

    tree = rbCreate();

    fprintf(stderr, "Re-adding %d keys in random order... ", testsize);
    fflush(stderr);
    for(i = 0; i < testsize; i++) {
	rbInsert(tree, iarr[i]);
    }
    fprintf(stderr, "done.\n");

    fprintf(stderr, "Removing all %d keys in sequential order... ", testsize);
    fflush(stderr);

    DUR_START(test);
    for(i = 0; i < testsize; i++) {
	rbDeleteKey(tree, i);
    }
    DUR_END(test);
    fprintf(stderr, "done.\n");
    buf += sprintf(buf, "| Seq removal, count %-10d   "   "| %-11llu "  "| ns/key  |\n", testsize, test_delta / testsize);
    buf += sprintf(buf, "| Seq removal, rate               | %-11.0f "  "| nodes/s |\n", (1000000000.0 / test_delta) * testsize );

    fprintf(stderr, "Re-adding %d keys in sequential order... ", testsize);
    fflush(stderr);
    DUR_START(test);
    for(i = 0; i < testsize; i++) {
	rbInsert(tree, i);
    }
    DUR_END(test);
    fprintf(stderr, "done.\n");
    buf += sprintf(buf, "| Seq insertion, count %-10d "   "| %-11llu "  "| ns/key  |\n", testsize, test_delta / testsize);
    buf += sprintf(buf, "| Seq insertion, rate             | %-11.0f "  "| nodes/s |\n", (1000000000.0 / test_delta) * testsize );

    fprintf(stderr, "Removing all %d keys in sequential order again... ", testsize);
    fflush(stderr);
    for(i = 0; i < testsize; i++) {
	rbDeleteKey(tree, i);
    }
    fprintf(stderr, "done.\n");

    fprintf(stderr, "Re-adding %d keys in random order... ", testsize);
    fflush(stderr);
    for(i = 0; i < testsize; i++) {
	rbInsert(tree, iarr[i]);
    }
    fprintf(stderr, "done.\n");

    if(keepsize < testsize) {
	fprintf(stderr, "Removing %d keys in random order to leave %d keys... ", testsize - keepsize, keepsize);
	fflush(stderr);

	DUR_START(test);
	for(i = 0; i < testsize; i++) {

	    if(rarr[i] >= keepsize) {
		rbDeleteKey(tree, rarr[i]);
	    }
	}
	DUR_END(test);
	fprintf(stderr, "done.\n");
	buf += sprintf(buf, "| Removal, count %-10d       "   "| %-11llu "  "| ns/key  |\n", testsize - keepsize, (testsize <= keepsize) ? 0 : test_delta / (testsize - keepsize));
	buf += sprintf(buf, "| Removal, rate                   | %-11.0f "  "| nodes/s |\n", (1000000000.0 / test_delta) * testsize );
    }

    buf += sprintf(buf, "+---------------------------------+-------------+---------+\n");

    fprintf(stdout, "\nTest results:\n\n%s\n", obuf);

    fprintf(stderr, "Final tree with %d nodes:\n", tree->count);

    dump = rbDisplay(tree, hsize, vsize, RB_NO_NULL);
    printf("%s\n\n", dump);
    free(dump);

    fprintf(stderr, "Verifying red-black tree... ");
    fflush(stderr);

    if(!rbVerify(tree, RB_CHATTY)) {
	dump = rbDisplay(tree, hsize, vsize, RB_NO_NULL);
	printf("%s\n\n", dump);
	free(dump);
	fprintf(stderr, "Call me stupid, but this tree is broken. Node removal implementation FAIL.\n");
	return -1;
    }

    if(breaksize > 0) {
	fprintf(stderr, "\nPainting %d random nodes red in attempt to invalidate tree... ", breaksize);
	fflush(stderr);
	for(int i = 0; i < breaksize; i++) {

	    RbNode* n = rbSearch(tree->root, rand() % keepsize);
	    if(n != NULL) {
		n->red = true;
	    }

	}

	fprintf(stderr, "done.\n");
	fprintf(stderr, "\nMost likely broken tree with %d nodes:\n", keepsize);
	dump = rbDisplay(tree, hsize, vsize, RB_NO_NULL);
	printf("%s\n\n", dump);
	free(dump);
	if(rbVerify(tree, RB_CHATTY)) {
	    fprintf(stderr, "Tree still valid... jammy bastard.\n");
	}

    }

cleanup:
    fprintf(stderr, "Cleaning up... ");

    fflush(stderr);

    rbFree(tree);

    free(iarr);
    free(rarr);
    free(sarr);

    fprintf(stderr, "done.\n");

    return 0;

}
