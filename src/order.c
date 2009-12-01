/* 

Copyright (c) 2009 Thomas Junier and Evgeny Zdobnov, University of Geneva
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.
* Neither the name of the University of Geneva nor the names of its
    contributors may be used to endorse or promote products derived from this
    software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/
/* order.c - orders children nodes */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "tree.h"
#include "parser.h"
#include "to_newick.h"
#include "list.h"
#include "rnode.h"
#include "order_tree.h"

enum order_criterion { ORDER_ALNUM_LBL, ORDER_NUM_DESCENDANTS };

struct parameters {
	enum order_criterion criterion;
};

void help (char *argv[])
{
	printf (
"Orders nodes according to various criteria, preserving topology\n"
"\n"
"Synopsis\n"
"--------\n"
"\n"
"%s [-hn] <newick trees filename|->\n"
"\n"
"Input\n"
"-----\n"
"\n"
"Argument is the name of a file that contains Newick trees, or '-' (in\n"
"which case trees are read from standard input).\n"
"\n"
"Output\n"
"------\n"
"\n"
"Orders the tree and prints it out on standard output. By default, the\n"
"ordering field is the node's label for leaves, or the first child's\n"
"order field for inner nodes. The tree's topology is not altered: the\n"
"biological information contained in the tree is left intact.\n"
"\n"
"This is useful for comparing trees, because isomorphic trees will yield\n"
"different Newick representations if the nodes are ordered differently.\n"
"\n"
"Options\n"
"-------\n"
"\n"
"    -h: print this message and exit\n"
"    -n: order tree by number of descendants. Nodes with fewer descendans\n"
"        appear first.\n"
"\n"
"Examples\n"
"--------\n"
"\n"
"# These two trees look different...\n"
"$ diff -q data/falconiformes data/falconiformes_2 \n"
"Files data/falconiformes and data/falconiformes_2 differ\n"
"\n"
"# but are they really?\n"
"$ %s data/falconiformes > f1\n"
"$ %s data/falconiformes_2 > f2\n"
"\n"
"# No, they're in fact the same\n"
"$ diff -s f1 f2\n"
"Files f1 and f2 are identical\n",
	argv[0],
	argv[0],
	argv[0]
	       );
}

struct parameters get_params(int argc, char *argv[])
{

	struct parameters params;
	params.criterion = ORDER_ALNUM_LBL;

	int opt_char;
	while ((opt_char = getopt(argc, argv, "hn")) != -1) {
		switch (opt_char) {
		case 'h':
			help(argv);
			exit(EXIT_SUCCESS);
		case 'n':
			params.criterion = ORDER_NUM_DESCENDANTS;
			break;
		default:
			fprintf (stderr, "Unknown option '-%c'\n", opt_char);
			exit (EXIT_FAILURE);
		}
	}

	/* check arguments */
	if ((argc - optind) == 1)	{
		if (0 != strcmp("-", argv[optind])) {
			FILE *fin = fopen(argv[optind], "r");
			extern FILE *nwsin;
			if (NULL == fin) {
				perror(NULL);
				exit(EXIT_FAILURE);
			}
			nwsin = fin;
		}
	} else {
		fprintf(stderr, "Usage: %s [-h] <filename|->\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	return params;
}

int main(int argc, char *argv[])
{
	struct rooted_tree *tree;	
	
	struct parameters params = get_params(argc, argv);

	while (NULL != (tree = parse_tree())) {
		switch(params.criterion) {
		case ORDER_ALNUM_LBL:
			if (! order_tree_lbl(tree)) { perror(NULL); exit(EXIT_FAILURE); }
			break;
		case ORDER_NUM_DESCENDANTS:
			if (! order_tree_num_desc(tree)) { perror(NULL); exit(EXIT_FAILURE); }
			break;
		default:
			assert(0); // programmer error
		}
		char *newick = to_newick(tree->root);
		printf ("%s\n", newick);
		free(newick);
		destroy_tree(tree, DONT_FREE_NODE_DATA);
	}

	return 0;
}
