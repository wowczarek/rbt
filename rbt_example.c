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

#include <stdio.h>
#include <stdlib.h>
#include "rbt.h"
#include "rbt_display.h"

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

	rbVerify(tree, RB_CHATTY, RB_FULL);
	char* d =
	rbDisplay(tree, 80, 11, RB_NO_NULL);
	printf("%s\n\n", d); free(d);

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
