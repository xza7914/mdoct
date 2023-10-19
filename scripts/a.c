#include <stdlib.h>
#include "..\include\status_tree.h"

void build_node() {
	struct node *root = malloc(sizeof(struct node));
	__CPROVER_assume(root != NULL);
	root->children = malloc(sizeof(struct item) * 5);
	__CPROVER_assume(root->children != NULL);

	struct node *node1_1 = malloc(sizeof(struct node));
	__CPROVER_assume(node1_1 != NULL);
	node1_1->children = malloc(sizeof(struct item) * 4);
	__CPROVER_assume(node1_1->children != NULL);

	struct node *node2_1 = malloc(sizeof(struct node));
	__CPROVER_assume(node2_1 != NULL);
	node2_1->children = malloc(sizeof(struct item) * 1);
	__CPROVER_assume(node2_1->children != NULL);

	struct node *termination = malloc(sizeof(struct node));
	__CPROVER_assume(termination != NULL);
	node2_1->children[0] = termination;

	node1_1->children[0] = node2_1;

	struct node *node2_2 = malloc(sizeof(struct node));
	__CPROVER_assume(node2_2 != NULL);
	node2_2->children = malloc(sizeof(struct item) * 1);
	__CPROVER_assume(node2_2->children != NULL);

	struct node *termination = malloc(sizeof(struct node));
	__CPROVER_assume(termination != NULL);
	node2_2->children[0] = termination;

	node1_1->children[1] = node2_2;

	struct node *node2_3 = malloc(sizeof(struct node));
	__CPROVER_assume(node2_3 != NULL);
	node2_3->children = malloc(sizeof(struct item) * 1);
	__CPROVER_assume(node2_3->children != NULL);

	struct node *termination = malloc(sizeof(struct node));
	__CPROVER_assume(termination != NULL);
	node2_3->children[0] = termination;

	node1_1->children[2] = node2_3;

	struct node *node2_4 = malloc(sizeof(struct node));
	__CPROVER_assume(node2_4 != NULL);
	node2_4->children = malloc(sizeof(struct item) * 1);
	__CPROVER_assume(node2_4->children != NULL);

	struct node *0x05 success = malloc(sizeof(struct node));
	__CPROVER_assume(0x05 success != NULL);
	node2_4->children[0] = 0x05 success;

	node1_1->children[3] = node2_4;

	root->children[0] = node1_1;

	struct node *node1_2 = malloc(sizeof(struct node));
	__CPROVER_assume(node1_2 != NULL);
	node1_2->children = malloc(sizeof(struct item) * 3);
	__CPROVER_assume(node1_2->children != NULL);

	struct node *node2_5 = malloc(sizeof(struct node));
	__CPROVER_assume(node2_5 != NULL);
	node2_5->children = malloc(sizeof(struct item) * 1);
	__CPROVER_assume(node2_5->children != NULL);

	struct node *termination = malloc(sizeof(struct node));
	__CPROVER_assume(termination != NULL);
	node2_5->children[0] = termination;

	node1_2->children[0] = node2_5;

	struct node *node2_6 = malloc(sizeof(struct node));
	__CPROVER_assume(node2_6 != NULL);
	node2_6->children = malloc(sizeof(struct item) * 1);
	__CPROVER_assume(node2_6->children != NULL);

	struct node *termination = malloc(sizeof(struct node));
	__CPROVER_assume(termination != NULL);
	node2_6->children[0] = termination;

	node1_2->children[1] = node2_6;

	struct node *node2_7 = malloc(sizeof(struct node));
	__CPROVER_assume(node2_7 != NULL);
	node2_7->children = malloc(sizeof(struct item) * 2);
	__CPROVER_assume(node2_7->children != NULL);

	struct node *node3_1 = malloc(sizeof(struct node));
	__CPROVER_assume(node3_1 != NULL);
	node3_1->children = malloc(sizeof(struct item) * 1);
	__CPROVER_assume(node3_1->children != NULL);

	struct node *termination = malloc(sizeof(struct node));
	__CPROVER_assume(termination != NULL);
	node3_1->children[0] = termination;

	node2_7->children[0] = node3_1;

	struct node *node3_2 = malloc(sizeof(struct node));
	__CPROVER_assume(node3_2 != NULL);
	node3_2->children = malloc(sizeof(struct item) * 2);
	__CPROVER_assume(node3_2->children != NULL);

	struct node *node4_1 = malloc(sizeof(struct node));
	__CPROVER_assume(node4_1 != NULL);
	node4_1->children = malloc(sizeof(struct item) * 1);
	__CPROVER_assume(node4_1->children != NULL);

	struct node *termination = malloc(sizeof(struct node));
	__CPROVER_assume(termination != NULL);
	node4_1->children[0] = termination;

	node3_2->children[0] = node4_1;

	struct node *node4_2 = malloc(sizeof(struct node));
	__CPROVER_assume(node4_2 != NULL);
	node4_2->children = malloc(sizeof(struct item) * 1);
	__CPROVER_assume(node4_2->children != NULL);

	struct node *0x01 success = malloc(sizeof(struct node));
	__CPROVER_assume(0x01 success != NULL);
	node4_2->children[0] = 0x01 success;

	node3_2->children[1] = node4_2;

	node2_7->children[1] = node3_2;

	node1_2->children[2] = node2_7;

	root->children[1] = node1_2;

	struct node *node1_3 = malloc(sizeof(struct node));
	__CPROVER_assume(node1_3 != NULL);
	node1_3->children = malloc(sizeof(struct item) * 1);
	__CPROVER_assume(node1_3->children != NULL);

	struct node *0x12 success = malloc(sizeof(struct node));
	__CPROVER_assume(0x12 success != NULL);
	node1_3->children[0] = 0x12 success;

	root->children[2] = node1_3;

	struct node *node1_4 = malloc(sizeof(struct node));
	__CPROVER_assume(node1_4 != NULL);
	node1_4->children = malloc(sizeof(struct item) * 1);
	__CPROVER_assume(node1_4->children != NULL);

	struct node *0x13 success = malloc(sizeof(struct node));
	__CPROVER_assume(0x13 success != NULL);
	node1_4->children[0] = 0x13 success;

	root->children[3] = node1_4;

	struct node *node1_5 = malloc(sizeof(struct node));
	__CPROVER_assume(node1_5 != NULL);
	node1_5->children = malloc(sizeof(struct item) * 3);
	__CPROVER_assume(node1_5->children != NULL);

	struct node *node2_8 = malloc(sizeof(struct node));
	__CPROVER_assume(node2_8 != NULL);
	node2_8->children = malloc(sizeof(struct item) * 1);
	__CPROVER_assume(node2_8->children != NULL);

	struct node *termination = malloc(sizeof(struct node));
	__CPROVER_assume(termination != NULL);
	node2_8->children[0] = termination;

	node1_5->children[0] = node2_8;

	struct node *node2_9 = malloc(sizeof(struct node));
	__CPROVER_assume(node2_9 != NULL);
	node2_9->children = malloc(sizeof(struct item) * 1);
	__CPROVER_assume(node2_9->children != NULL);

	struct node *termination = malloc(sizeof(struct node));
	__CPROVER_assume(termination != NULL);
	node2_9->children[0] = termination;

	node1_5->children[1] = node2_9;

	struct node *0x03 success = malloc(sizeof(struct node));
	__CPROVER_assume(0x03 success != NULL);
	node1_5->children[2] = 0x03 success;

	root->children[4] = node1_5;

}
