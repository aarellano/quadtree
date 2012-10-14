#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "quadtree.h"
#include "drawing_c.h"

struct mxcif *mx_cif_tree; //MX-CIF Quadtree
bnode_t *rect_tree; //Rectangle bin tree, sorted with respect to rect names

const double DISPLAY_SIZE = 128;
double scale_factor;
int trace = 0;

static bnode_t *find_btree(bnode_t *tree, bnode_t *node);
static void traverse_bintree(bnode_t *node);
static void traverse_quadtree(cnode_t *node);

static void init_mx_cif_tree(void) {
	mx_cif_tree = (struct mxcif *)malloc(sizeof(struct mxcif));
	mx_cif_tree->mx_cif_root = NULL;
	mx_cif_tree->world.rect_name = "MX-CIF";
}

static void init_rect_tree(void) {
	 rect_tree = NULL;
 }

static void print_in_order(bnode_t *node) {
	if (node != NULL) {
		print_in_order(node->bson[LEFT]);
		printf("%s(%d,%d,%d,%d) ", node->rect->rect_name, node->rect->center[X], node->rect->center[Y], node->rect->lenght[X], node->rect->lenght[Y]);
		print_in_order(node->bson[RIGHT]);
	}
}

static void print_pre_order(bnode_t *node) {
	if (node != NULL) {
		printf("%s", node->rect->rect_name);
		print_pre_order(node->bson[LEFT]);
		print_pre_order(node->bson[RIGHT]);
	}
}

static direction bin_compare(rectangle_t *P, long Cv, axis V) {
	/*
	** Determines whether rectangle P lies to the left of, right of, or contains line V=Cv
	*/
	if (((P->center[V] - P->lenght[V]) < Cv) && (Cv < ((P->center[V] + P->lenght[V]))))
		return BOTH;
	else if (Cv < P->center[V])
		return RIGHT;
	else
		return LEFT;
}

static quadrant cif_compare(rectangle_t *P, int Cx, int Cy) {
	/*
	** Return the quadrant of the MX-CIF quadtree rooted at position (Cx,Cy) that contains
	** the centroid of rectangle P
	*/

	if (P->center[X] < Cx)
		if (P->center[Y] < Cy)
			return SW;
		else
			return NW;
	else
		if (P->center[Y] < Cy)
			return SE;
		else
			return NE;
}

static bnode_t *create_bnode(void) {
	bnode_t *node = (bnode_t *)malloc(sizeof(bnode_t));
	node->rect = NULL;
	node->bson[X] = node->bson[Y] = NULL;
	return node;
}

static cnode_t *create_cnode(void) {
	cnode_t *node = (cnode_t *)malloc(sizeof(cnode_t));
	node->qson[NW] = node->qson[NE] = node->qson[SW] = node->qson[SE] = NULL;
	node->bson[X] = node->bson[Y] = NULL;
	return node;
}

static void insert_axis(rectangle_t *P, cnode_t *R, int Cv, int Lv, axis V) {
	bnode_t *T;
	int F[] = {-1, 1};
	direction D;
	int node_number = 0;

	if (trace)
		printf("%d%c ", node_number, V == 0 ? 'X' : 'Y');

	if (R->bson[V] == NULL)
		R->bson[V] = create_bnode();

	T = R->bson[V];
	D = bin_compare(P, Cv, V);
	while (D != BOTH) {
		if (T->bson[D] == NULL)
			T->bson[D] = create_bnode();
		T = T->bson[D];
		Lv = Lv / 2;
		Cv = Cv + F[D] * Lv;
		node_number = 2 * node_number + D + 1;
		if (trace)
			printf("%d%c ", node_number, V == 0 ? 'X' : 'Y');
		D = bin_compare(P, Cv, V);
	}
	T->rect = P;
}

static void cif_insert(rectangle_t *P, struct mxcif *cif_tree, int Cx, int Cy, int Lx, int Ly) {
	int Sx[] = {-1, 1, -1, 1};
	int Sy[] = {1, 1, -1, -1};
	cnode_t *T;
	quadrant Q;
	direction Dx, Dy;
	cnode_t *R;
	int node_number = 0;

	if (cif_tree->mx_cif_root == NULL)
		cif_tree->mx_cif_root = create_cnode();

	R = cif_tree->mx_cif_root;
	T = R;
	Dx = bin_compare(P, Cx, X);
	Dy = bin_compare(P, Cy, Y);

	if (trace)
		printf("%d ", node_number);

	while ((Dx != BOTH) && (Dy != BOTH)) {
		Q = cif_compare(P, Cx, Cy);
		if (T->qson[Q] == NULL)
			T->qson[Q] = create_cnode();
		T = T->qson[Q];
		Lx = Lx / 2;
		Ly = Ly / 2;
		Cx = Cx + Sx[Q] * Lx;
		Cy = Cy + Sy[Q] * Ly;
		Dx = bin_compare(P, Cx, X);
		Dy = bin_compare(P, Cy, Y);
		node_number = 4 * node_number + Q + 1;
		if (trace)
			printf("%d ", node_number);
	}

	if (Dx == BOTH)
		insert_axis(P, T, Cy, Ly, Y);
	else
		insert_axis(P, T, Cx, Lx, X);
}

static void insert_rectangle(char args[][MAX_NAME_LEN + 1]) {
	char *name = args[0];
	rectangle_t *rectangle;
	bnode_t *node;

	rectangle = (rectangle_t *)malloc(sizeof(rectangle_t));
	rectangle->rect_name = name;

	node = (bnode_t *)malloc(sizeof(bnode_t));
	node->rect = rectangle;

	node = find_btree(rect_tree, node);

	if (((node->rect->center[X] + node->rect->lenght[X]) > mx_cif_tree->world.lenght[X]) || ((node->rect->center[Y] + node->rect->lenght[Y]) > mx_cif_tree->world.lenght[Y]))
		printf("INSERTION OF RECTANGLE %s(%d,%d,%d,%d) FAILED AS %s LIES PARTIALLY OUTSIDE SPACE SPANNED BY MX-CIF QUADTREE\n", node->rect->rect_name, node->rect->center[X], node->rect->center[Y], node->rect->lenght[X], node->rect->lenght[Y], node->rect->rect_name);
	else {
		cif_insert(node->rect, mx_cif_tree, mx_cif_tree->world.center[X], mx_cif_tree->world.center[Y], mx_cif_tree->world.center[X], mx_cif_tree->world.center[Y]);
		if (trace)
			printf("\n");
		printf("RECTANGLE %s(%d,%d,%d,%d) INSERTED\n", node->rect->rect_name, node->rect->center[X], node->rect->center[Y], node->rect->lenght[X], node->rect->lenght[Y]);
	}
}

static void list_rectangles(void) {
	print_in_order(rect_tree);
	printf("\n");
}

static bnode_t *find_btree(bnode_t *tree, bnode_t *node) {
	if (!tree)
		return NULL;

	if (strcmp(tree->rect->rect_name, node->rect->rect_name) == 0)
		return node = tree;
	else if (strcmp(tree->rect->rect_name, node->rect->rect_name) > 0)
		node = find_btree(tree->bson[LEFT], node);
	else
		node = find_btree(tree->bson[RIGHT], node);

	return node;
}

static void insert_to_btree(bnode_t **root, bnode_t *newNode) {
	if (*root == NULL) {
		*root = newNode;
		return;
	}

	if (strcmp((*root)->rect->rect_name, newNode->rect->rect_name) > 0)
		insert_to_btree(&(*root)->bson[LEFT], newNode);
	else if (strcmp((*root)->rect->rect_name, newNode->rect->rect_name ))
		insert_to_btree(&(*root)->bson[RIGHT], newNode);
}

static void create_rectangle(char args[][MAX_NAME_LEN + 1]) {
	char *name = args[0];
	int cx = atoi(args[1]);
	int cy = atoi(args[2]);
	int lx = atoi(args[3]);
	int ly = atoi(args[4]);

	rectangle_t *new_rectangle = (rectangle_t *)malloc(sizeof(rectangle_t));
	new_rectangle->rect_name = strdup(name);
	new_rectangle->bson[LEFT] = new_rectangle->bson[RIGHT] = NULL;
	new_rectangle->center[X] = cx;
	new_rectangle->center[Y] = cy;
	new_rectangle->lenght[X] = lx;
	new_rectangle->lenght[Y] = ly;

	bnode_t *new_node = (bnode_t *)malloc(sizeof(bnode_t));
	new_node->rect = new_rectangle;
	new_node->bson[LEFT] = new_node->bson[RIGHT] = NULL;
	insert_to_btree(&rect_tree, new_node);

	printf("CREATED RECTANGLE(%s,%d,%d,%d,%d)\n", name, cx, cy, lx, ly);
}

static void init_quadtree(char args[][MAX_NAME_LEN + 1]) {
	int width = atoi(args[0]);

	scale_factor = DISPLAY_SIZE / (1 << width);

	mx_cif_tree->world.lenght[X] = 1 << width;
	mx_cif_tree->world.lenght[Y] = 1 << width;
	mx_cif_tree->world.center[X] = mx_cif_tree->world.lenght[X] / 2;
	mx_cif_tree->world.center[Y] = mx_cif_tree->world.lenght[Y] / 2;

	printf("MX-CIF QUADTREE 0 INITIALIZED WITH PARAMETER %d\n", width);
}

static void traverse_bintree(bnode_t *node) {
	if (node != NULL) {
		if (node->rect)
			printf("%s\n", node->rect->rect_name);
		traverse_bintree(node->bson[LEFT]);
		traverse_bintree(node->bson[RIGHT]);
	}
}

static void traverse_quadtree(cnode_t *node) {
	if (node != NULL) {
		traverse_bintree(node->bson[X]);
		traverse_bintree(node->bson[Y]);

		traverse_quadtree(node->qson[NW]);
		traverse_quadtree(node->qson[NE]);
		traverse_quadtree(node->qson[SW]);
		traverse_quadtree(node->qson[SE]);
	}
}

static void display(void) {
	StartPicture(DISPLAY_SIZE + 1, DISPLAY_SIZE + 1);
	SetLineDash(3, 3);
	DrawRect(0, DISPLAY_SIZE, DISPLAY_SIZE, 0);
	SetLineDash(3, 3);
	traverse_quadtree(mx_cif_tree->mx_cif_root);
	EndPicture();
}

static void decode_command(char *command, char args[][MAX_NAME_LEN + 1])
{
	if (strcmp(command, "INIT_QUADTREE") == 0)
		init_quadtree(args);
	else if (strcmp(command, "DISPLAY") == 0)
		display();
	else if (strcmp(command, "LIST_RECTANGLES") == 0)
		list_rectangles();
	else if (strcmp(command, "CREATE_RECTANGLE") == 0)
		create_rectangle(args);
	else if (strcmp(command, "SEARCH_POINT") == 0)
		return;
	else if (strcmp(command, "RECTANGLE_SEARCH") == 0)
		return;
	else if (strcmp(command, "INSERT") == 0)
		insert_rectangle(args);
	else if (strcmp(command, "DELETE_RECTANGLE") == 0 || strcmp(command, "DELETE_POINT") == 0)
		return;
	else if (strcmp(command, "MOVE") == 0)
		return;
	else if (strcmp(command, "TOUCH") == 0)
		return;
	else if (strcmp(command, "WITHIN") == 0)
		return;
	else if (strcmp(command, "HORIZ_NEIGHBOR") == 0 || strcmp(command, "VERT_NEIGHBOR") == 0)
		return;
	else if (strcmp(command, "NEAREST_RECTANGLE") == 0)
		return;
	else if (strcmp(command, "WINDOW") == 0)
		return;
	else if (strcmp(command, "NEAREST_NEIGHBOR") == 0)
		return;
	else if (strcmp(command, "LEXICALLY_GREATER_NEAREST_NEIGHBOR") == 0)
		return;
	else if (strcmp(command, "LABEL") == 0)
		return;
	else if (strcmp(command, "SPATIAL_JOIN") == 0)
		return;
	else
		return;
}

static void read_command(void)
{
	char c;
	int i = 0, j=0, k=0;

while(1){
	char args[10][MAX_NAME_LEN + 1];
	char input[100];
	char command_name[100];

	for (i =0; (c = getchar()) != '\n'; i++) {
		input[i] = c;
		if (c == EOF)
			return;
	}

	i = 0;
	while(input[i] != '(' && input[i] != ' ')
	{
		command_name[i] = input[i];
		++i;
	}
	command_name[i] = '\0';

	if (strcmp(command_name, "TRACE") != 0) {
		++i;
		while(input[i] != ')')
		{
			if(input[i] == ',')
			{
				args[j][k] = '\0';
				++j;
				k=0;
			}
			else
			{
				args[j][k] = input[i];
				++k;
				if (input[i+1] == ')')
					args[j][k] = '\0';
			}
			++i;
		}

		decode_command(command_name, args);
	} else {
		i = i + 2;
		if (input[i] == 'N') // "N" from "ON"
			trace = 1;
		else
			trace = 0;
	}

	i = 0;
	j = 0;
	k = 0;
}
	return;
}

int main(void) {
	init_mx_cif_tree();
	init_rect_tree();

	read_command();

	return (0);
}
