# rbt

## About

Yet another red-black tree implementation, C99. Because I needed one for a dictionary / general-purpose dynamic index. Three-pointer (parent + two-child array), plus value pointer and uint32_t keys, node colour as extra bool. No pointer bit reuse, nothing too clever. No thread safety or cache awareness. As basic as it gets, no-nonsense code. Completely non-recursive, although using stacks and FIFO queues.

Supports:

- search, insertion, deletion (bottom-up),
- verification of red-black tree invariants / correctness,
- in-order traversal with callback and optional height and black height tracking for each node inspected (which allows for fast verification),
- in-order ranged traversal, same as above,
- breadth-first traversal with the same (simple dynamic FIFO queue implemented for this, `fq.h`/`fq.c` - two versions, pointer queue and data queue),
- displaying an ASCII dump of the tree.

## Example

Example usage (`rbt_example.c`):

```c
#include <stdio.h>
#include "rbt.h"

static RbNode* callback(RB_CB_ARGS) {
	printf(" %u", node->key);
	return node;
}

int main(int argc, char **argv) {

	uint32_t count;
	RbTree *tree = rbCreate();

	for(int i = 0; i < 13; i++) {
		rbInsert(tree, i);
	}

	rbVerify(tree);
	rbDisplay(tree, 80, 11, RB_NO_NULL);

	printf("In order:");
	rbInOrder(tree, callback, NULL, RB_ASC);
	printf("\n");

	printf("Breadth first:");
	rbBreadthFirst(tree, callback, NULL, RB_ASC);
	printf("\n");

	printf("Between 4 (inclusive) and 9 (exclusive):");
	count = rbInOrderRange(tree, callback, NULL, RB_ASC,
		4, RB_INCL, 9, RB_EXCL);
	printf(", in range: %u nodes\n", count);

	printf("Between 4 (exclusive) and 9 (inclusive):");
	count = rbInOrderRange(tree, callback, NULL, RB_DESC,
		4, RB_EXCL, 9, RB_INCL);
	printf(", in range: %u nodes\n", count);

#if 0
	printf("Couldn't stop myself from adding this because of \n"
		"the increasing width of these lines of code here...");
#endif

	rbFree(tree);
	return 0;

}

```
Output:
```
$ ./rbt_example 
Valid red-black tree, node count 13, max height 5, black height 3

................................................................................
.......................................B3.......................................
................................................................................
...................B1......................................B7...................
................................................................................
.........B0..................B2..................R5..................R9.........
................................................................................
............................................B4........B6........B8........B11...
................................................................................
........................................................................R10.R12.
................................................................................

In order: 0 1 2 3 4 5 6 7 8 9 10 11 12
Breadth first: 3 1 7 0 2 5 9 4 6 8 11 10 12
Between 4 (inclusive) and 9 (exclusive): 4 5 6 7 8, in range: 5 nodes
Between 4 (exclusive) and 9 (inclusive): 9 8 7 6 5, in range: 5 nodes

```

## Testing

Provided is a test program / benchmark, `rbt_test.c`. Options:
```
$ ./rbt_test -h
rbt_test (c) 2018: Wojciech Owczarek, simple red-black tree implementation

usage: rbt_test [-w NUMBER] [-H NUMBER] [-n NUMBER] [-r NUMBER] [-b NUMBER]
                [-s] [-m] [-e] [-l] [-o] [-i NUMBER]

-w NUMBER       Width of text block displaying the final tree, default 80
-H NUMBER       Height of text block displaying the final tree, default 20
-n NUMBER       Number of random nodes to insert into tree, default 1000
-r NUMBER       Number of nodes to leave on tree after removal, default 20
-b NUMBER       Number of nodes to randomly turn red to invalidate tree, default 0
-s              Test insertion only, generate CSV output on stdout
-m              Test removal only, CSV output to stdout
-e              Test search only, CSV output to stdout
-l              Test incremental search only (during insertion), CSV output to stdout
-o              Test decremental search only (during removal), CSV output to stdout
-i NUMBER       CSV log output interval, default every 1000 nodes,  unless
                1000 < 1% node count, then 1% node count is used.
```

Example output (mind that this ran on a shite Atom box, so performance is indicative of its shiteness):

```
$ ./rbt_test -n 1000000 -b 5
Generating 1000000 size random insertion, removal and search key arrays... done
Inserting 1000000 random keys... done
Verifying red-black tree... Valid red-black tree, node count 1000000, max height 24, black height 12
Finding all 1000000 keys in random order... 1000000 found
Finding all 1000000 keys in sequential order... 1000000 found
Performing in-order traversal with height and black height tracking... done
Performing in-order traversal without height and black height tracking... done
Performing breadth-first traversal with height and black height tracking... done
Performing breadth-first traversal without height and black height tracking... done
Destroying tree... done.
Re-adding 1000000 keys in random order... done
Removing all 1000000 keys in sequential order... done
Re-adding 1000000 keys in sequential order... done
Removing all 1000000 keys in sequential order again... done
Re-adding 1000000 keys in random order... done
Removing 999980 keys in random order to leave 20 keys... done

Test results:

+---------------------------------+-------------+---------+
| Test                            | result      | unit    |
+---------------------------------+-------------+---------+
| Insertion, count 1000000        | 1576        | ns/key  |
| Insertion, rate                 | 634395      | nodes/s |
| Verification, rate              | 6439571     | nodes/s |
| Search, count 1000000           | 1437        | ns/key  |
| Search, rate                    | 695772      | hit/s   |
| Seq search, count 1000000       | 242         | ns/key  |
| Seq search, rate                | 4118549     | hit/s   |
| In-order, with tracking, rate   | 6940080     | nodes/s |
| In-order, fast, rate            | 7751622     | nodes/s |
| Breadth first, tracking, rate   | 3464460     | nodes/s |
| Breadth first, fast, rate       | 7113365     | nodes/s |
| Destruction, rate               | 1906188     | nodes/s |
| Seq removal, count 1000000      | 272         | ns/key  |
| Seq removal, rate               | 3665748     | nodes/s |
| Seq insertion, count 1000000    | 536         | ns/key  |
| Seq insertion, rate             | 1865072     | nodes/s |
| Removal, count 999980           | 1903        | ns/key  |
| Removal, rate                   | 525460      | nodes/s |
+---------------------------------+-------------+---------+

Final tree with 20 nodes:

................................................................................
.......................................B12......................................
................................................................................
...................R5......................................B17..................
................................................................................
.........B1..................B7..................R15.................B19........
................................................................................
....B0........B3........B6........R9........B14.......B16.......R18.............
................................................................................
............R2..R4..............B8..B11...R13...................................
................................................................................
...................................R10..........................................
................................................................................
................................................................................
................................................................................
................................................................................
................................................................................
................................................................................
................................................................................
................................................................................

Verifying red-black tree... Valid red-black tree, node count 20, max height 6, black height 3

Painting random 5 nodes red to invalidate tree... done

Broken tree with 20 nodes:

................................................................................
.......................................R12......................................
................................................................................
...................R5......................................B17..................
................................................................................
.........B1..................B7..................R15.................R19........
................................................................................
....B0........B3........B6........R9........B14.......R16.......R18.............
................................................................................
............R2..R4..............B8..B11...R13...................................
................................................................................
...................................R10..........................................
................................................................................
................................................................................
................................................................................
................................................................................
................................................................................
................................................................................
................................................................................
................................................................................

Red root violation
Red-red violation: key 5 red -> parent key 12 red
Black height violation: key 16 black height 1 != previous black height seen 2
Red-red violation: key 16 red -> parent key 15 red
Red-red violation: key 18 red -> parent key 19 red
```

## Some benchmarks

Below are some plots taken from the CSV output for tests at different key insertion counts. This was done on a fairly decent Xeon box with 64G RAM. There are some spikes which could be the CPU doing something else; I have not really investigated those. The duration measurement is done with simple clock_gettime, which itself is non-instant (usually some 20 ns for a start/stop call pair), so the more iterations per measurement, the closer the number is to reality. The actual performance is clearly dominated by cache misses (and L2 / L3 cache size is also the source of the sawtooth-like patterns); that is not the point. What is important is that this pretty clearly shows that the total time per insertion / deletion / search is a function of *log<sub>2</sub>(n)*, and that search time is a significant contributor to both insertion and deletion. If the implementation was to be rewritten for top-down, the search and rebalance parts would have been combined, likely resulting in shaving off some cycles.

100k nodes:

![rbt benchmark with 100k nodes](https://github.com/wowczarek/rbt/raw/master/img/rbt_100k.png "rbt benchmark with 100k nodes")

1M nodes:

![rbt benchmark with 1M nodes](https://github.com/wowczarek/rbt/raw/master/img/rbt_1m.png "rbt benchmark with 1M nodes")

10M nodes:

![rbt benchmark with 10M nodes](https://github.com/wowczarek/rbt/raw/master/img/rbt_10m.png "rbt benchmark with 10M nodes")

500M nodes:

![rbt benchmark with 500M nodes](https://github.com/wowczarek/rbt/raw/master/img/rbt_500m.png "rbt benchmark with 500M nodes")

1000M nodes (a billion):

![rbt benchmark with 1000M nodes](https://github.com/wowczarek/rbt/raw/master/img/rbt_1000m.png "rbt benchmark with 1000M nodes")

Again, I did not spend enough time analysing these plots, so for now the noise towards the end of the 1000m insertion cycle remains to be explained.

*All plots were made using [kst2 / kst-plot](https://kst-plot.kde.org/), which remains my all-time favourite graphing and data analysis package.*
