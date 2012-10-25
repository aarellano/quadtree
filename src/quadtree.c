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
static rectangle_t *cross_axis(rectangle_t *P, bnode_t *R, int Cv, int Lv, axis V, int *bin_node_number);
static int rect_intersect(rectangle_t *P, int Cx, int Cy, int Lx, int Ly);
static rectangle_t *cif_search(rectangle_t *P, cnode_t *R, int Cx, int Cy, int Lx, int Ly, int *quad_node_number);
static void delete_from_btree(bnode_t **node);

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
	if (((P->center[V] - P->lenght[V]) <= Cv) && (Cv < ((P->center[V] + P->lenght[V]))))
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

static rectangle_t *cross_axis(rectangle_t *P, bnode_t *R, int Cv, int Lv, axis V, int *bin_node_number) {
	int F[]= {-1, 1};
	direction D;

	if (trace)
		printf("%d%c ", *bin_node_number, V == 0 ? 'X' : 'Y');

	if (R == NULL)
		return NULL;
	else if ((R->rect != NULL) && (rect_intersect(P, R->rect->center[X], R->rect->center[Y], R->rect->lenght[X], R->rect->lenght[Y])))
		return R->rect;
	else {
		D = bin_compare(P, Cv, V);
		Lv = Lv / 2;
		if (Lv == 1)
			return NULL;
		*bin_node_number = *bin_node_number * 2;
		if (D == BOTH) {
			rectangle_t *intersected_rect;
			*bin_node_number = *bin_node_number + 1;
			intersected_rect = cross_axis(P, R->bson[LEFT], Cv - Lv, Lv, V, bin_node_number);
			if (intersected_rect)
				return intersected_rect;
			*bin_node_number = *bin_node_number + 1;
			intersected_rect = cross_axis(P, R->bson[LEFT], Cv + Lv, Lv, V, bin_node_number);
			if (intersected_rect)
				return intersected_rect;
		}
		else if (R->bson[D] == NULL)
			return NULL;
		else
			return cross_axis(P, R->bson[D], Cv + F[D] * Lv, Lv, V, bin_node_number);
	}
	return NULL;
}

static int rect_intersect(rectangle_t *P, int Cx, int Cy, int Lx, int Ly) {
	int intersect_x = 0, intersect_y = 0;
	if ((P->center[X] - P->lenght[X] >= Cx - Lx) && (P->center[X] - P->lenght[X] <= Cx + Lx - 1))
		intersect_x = 1;
	if ((P->center[X] + P->lenght[X] - 1 >= Cx - Lx) && (P->center[X] + P->lenght[X] <= Cx + Lx - 1))
		intersect_x = 1;
	if ((P->center[Y] - P->lenght[Y] >= Cy - Ly) && (P->center[Y] - P->lenght[Y] <= Cy + Ly - 1))
		intersect_y = 1;
	if ((P->center[Y] + P->lenght[Y] - 1 >= Cy - Ly) && (P->center[Y] + P->lenght[Y] <= Cy + Ly - 1))
		intersect_y = 1;

	if (intersect_y && intersect_x)
		return 1;
	else
		return 0;
}

static rectangle_t *cif_search(rectangle_t *P, cnode_t *R, int Cx, int Cy, int Lx, int Ly, int *quad_node_number) {
	int Sx[] = {-1, 1, -1, 1};
	int Sy[] = {1, 1, -1, -1};
	rectangle_t *intersected_rect;
	int x_counter = 0, y_counter = 0;
	quadrant Q;

	if (trace)
		printf("%d ", *quad_node_number);

	if (R == NULL)
		return NULL;
	else if (!rect_intersect(P, Cx, Cy, Lx, Ly)) // the rectangle must at least intersect the MX-CIF node quadrant (but since we're using cif_compare(...), this shouldn't be neccessary)
		return NULL;
	else {
		intersected_rect = cross_axis(P, R->bson[X], Cx, Lx, X, &x_counter);
		if (intersected_rect == NULL)
			intersected_rect = cross_axis(P, R->bson[Y], Cy, Ly, Y, &y_counter);
		if (intersected_rect)
			return intersected_rect;
	}

	Lx = Lx / 2;
	Ly = Ly / 2;

	Q = cif_compare(P, Cx, Cy);
	*quad_node_number = *quad_node_number * 4 + Q + 1;
	if (R->qson[Q])
		intersected_rect = cif_search(P, R->qson[Q], Cx + Sx[Q] * Lx, Cy + Sy[Q] * Ly, Lx, Ly, quad_node_number);
	if (intersected_rect != NULL)
		return intersected_rect;

	return NULL;
}

static rectangle_t *delete_from_axis(rectangle_t *P, bnode_t **R, int Cv, int Lv, axis V, int *bin_node_number) {
	int F[]= {-1, 1};
	direction D;

	if (trace)
		printf("%d%c ", *bin_node_number, V == 0 ? 'X' : 'Y');

	if (*(R) == NULL)
		return NULL;
	else if (((*R)->rect != NULL) && (rect_intersect(P, (*R)->rect->center[X], (*R)->rect->center[Y], (*R)->rect->lenght[X], (*R)->rect->lenght[Y]))) {
		rectangle_t	*return_rect = (*R)->rect;
		delete_from_btree(R);
		return return_rect;
	}
	else {
		D = bin_compare(P, Cv, V);
		Lv = Lv / 2;
		if (Lv == 1)
			return NULL;
		*bin_node_number = *bin_node_number * 2;
		if (D == BOTH) {
			rectangle_t *intersected_rect;
			*bin_node_number = *bin_node_number + 1;
			intersected_rect = delete_from_axis(P, &((*R)->bson[LEFT]), Cv - Lv, Lv, V, bin_node_number);
			if (intersected_rect) {
				delete_from_btree(R);
				return intersected_rect;
			}
			*bin_node_number = *bin_node_number + 1;
			intersected_rect = delete_from_axis(P, &((*R)->bson[LEFT]), Cv + Lv, Lv, V, bin_node_number);
			if (intersected_rect) {
				delete_from_btree(R);
				return intersected_rect;
			}
		}
		else if ((*R)->bson[D] == NULL)
			return NULL;
		else
			return delete_from_axis(P, &((*R)->bson[D]), Cv + F[D] * Lv, Lv, V, bin_node_number);
	}
	return NULL;
}

static rectangle_t *cif_delete(rectangle_t *P, cnode_t *R, int Cx, int Cy, int Lx, int Ly, int *quad_node_number) {
	int Sx[] = {-1, 1, -1, 1};
	int Sy[] = {1, 1, -1, -1};
	rectangle_t *intersected_rect;
	int Cv, Lv, v_counter = 0;
	quadrant Q;
	axis V;

	if (trace)
		printf("%d ", *quad_node_number);

	if (R == NULL)
		return NULL;
	else if (!rect_intersect(P, Cx, Cy, Lx, Ly)) // the rectangle must at least intersect the MX-CIF node quadrant (but since we're using cif_compare(...), this shouldn't be neccessary)
		return NULL;
	else {
		V = X;
		Cv = Cx;
		Lv = Lx;
		v_counter = 0;
		intersected_rect = delete_from_axis(P, &(R->bson[V]), Cv, Lv, V, &v_counter);
		if (intersected_rect == NULL) {
			V = Y;
			Cv = Cy;
			Lv = Ly;
			v_counter = 0;
			intersected_rect = delete_from_axis(P, &(R->bson[V]), Cv, Lv, V, &v_counter);
		}
		if (intersected_rect)
			return intersected_rect;
	}

	Lx = Lx / 2;
	Ly = Ly / 2;

	Q = cif_compare(P, Cx, Cy);
	*quad_node_number = *quad_node_number * 4 + Q + 1;
	if (R->qson[Q])
		return cif_delete(P, R->qson[Q], Cx + Sx[Q] * Lx, Cy + Sy[Q] * Ly, Lx, Ly, quad_node_number);

	return NULL;
}

static void delete_from_btree(bnode_t **node) {
	bnode_t *old_bnode = *node;
	if ((*node)->bson[LEFT] == NULL) {
		*node = (*node)->bson[RIGHT];
		free(old_bnode);
	} else if ((*node)->bson[RIGHT] == NULL) {
		*node = (*node)->bson[LEFT];
		free(old_bnode);
	} else {
		bnode_t *pred = (*node)->bson[LEFT];
		while (pred->bson[RIGHT] != NULL)
			pred = pred->bson[RIGHT];
		void *temp = pred->rect;
		pred->rect = (*node)->rect;
		(*node)->rect = temp;

		delete_from_btree(&pred);
	}
}

static void search_point(char args[][MAX_NAME_LEN + 1]) {
	int px = atoi(args[0]), py = atoi(args[1]);
	rectangle_t w;
	rectangle_t *point_rect = (rectangle_t *)malloc(sizeof(rectangle_t));
	point_rect->center[X] = px;
	point_rect->center[Y] = py;
	int counter = 0;

	w = mx_cif_tree->world;
	rectangle_t *intersected_rect = cif_search(point_rect, mx_cif_tree->mx_cif_root, w.center[X], w.center[Y], w.lenght[X], w.lenght[Y], &counter);
	if (intersected_rect != NULL) {
		if (trace)
			printf("\n");
		printf("POINT (%d,%d) CONTAINED BY RECTANGLE %s(%d,%d,%d,%d)\n", point_rect->center[X], point_rect->center[Y],
			intersected_rect->rect_name, intersected_rect->center[X], intersected_rect->center[Y], intersected_rect->lenght[X], intersected_rect->lenght[Y]);
	}
	else {
		if (trace)
			printf("\n");
		printf("POINT (%d,%d) NOT CONTAINED BY ANY RECTANGLE\n", point_rect->center[X], point_rect->center[Y]);
	}
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

	rectangle_t w = mx_cif_tree->world;
	if (((node->rect->center[X] + node->rect->lenght[X]) > w.center[X] + w.lenght[X]) || ((node->rect->center[Y] + node->rect->lenght[Y]) > w.center[Y] + w.lenght[Y]))
		printf("INSERTION OF RECTANGLE %s(%d,%d,%d,%d) FAILED AS %s LIES PARTIALLY OUTSIDE SPACE SPANNED BY MX-CIF QUADTREE\n", node->rect->rect_name, node->rect->center[X], node->rect->center[Y], node->rect->lenght[X], node->rect->lenght[Y], node->rect->rect_name);
	else {
		cif_insert(node->rect, mx_cif_tree, w.center[X], w.center[Y], w.lenght[X], w.lenght[Y]);
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

	printf("CREATED RECTANGLE %s(%d,%d,%d,%d)\n", name, cx, cy, lx, ly);
}

static void init_quadtree(char args[][MAX_NAME_LEN + 1]) {
	int width = atoi(args[0]);

	scale_factor = DISPLAY_SIZE / (1 << width);

	mx_cif_tree->world.lenght[X] = (1 << width) / 2;
	mx_cif_tree->world.lenght[Y] = (1 << width) / 2;
	mx_cif_tree->world.center[X] = (1 << width) / 2;
	mx_cif_tree->world.center[Y] = (1 << width) / 2;

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

static void rectangle_search(char args[][MAX_NAME_LEN + 1]) {
	char *name = args[0];
	rectangle_t *search_rect, w;
	bnode_t *node;
	int counter = 0;

	// Find the rectangle in the DB (BST) by its name
	search_rect = (rectangle_t *)malloc(sizeof(rectangle_t));
	search_rect->rect_name = name;
	node = (bnode_t *)malloc(sizeof(bnode_t));
	node->rect = search_rect;
	node = find_btree(rect_tree, node);

	// Find an intersecting rectangle in the MX-CIF
	w = mx_cif_tree->world;
	rectangle_t *over_rect = cif_search(node->rect, mx_cif_tree->mx_cif_root, w.center[X], w.center[Y], w.lenght[X], w.lenght[Y], &counter);
	if (trace)
		printf("\n");
	if (over_rect != NULL)
		printf("RECTANGLE %s(%d,%d,%d,%d) OVERLAPS RECTANGLE %s(%d,%d,%d,%d)\n",
			node->rect->rect_name, node->rect->center[X], node->rect->center[Y], node->rect->lenght[X], node->rect->lenght[Y],
			over_rect->rect_name, over_rect->center[X], over_rect->center[Y], over_rect->lenght[X], over_rect->lenght[Y]);
	else
		printf("RECTANGLE %s(%d,%d,%d,%d) DOES NOT OVERLAP ANY RECTANGLES\n",
			node->rect->rect_name, node->rect->center[X], node->rect->center[Y], node->rect->lenght[X], node->rect->lenght[Y]);
}

static void delete_rectangle(char args[][MAX_NAME_LEN + 1]) {
	char *name = args[0];
	rectangle_t *search_rect, w;
	bnode_t *node;
	int counter = 0;

	// Find the rectangle in the DB (BST) by its name
	search_rect = (rectangle_t *)malloc(sizeof(rectangle_t));
	search_rect->rect_name = name;
	node = (bnode_t *)malloc(sizeof(bnode_t));
	node->rect = search_rect;
	node = find_btree(rect_tree, node);

	w = mx_cif_tree->world;
	rectangle_t *deleted_rect = cif_delete(node->rect, mx_cif_tree->mx_cif_root, w.center[X], w.center[Y], w.lenght[X], w.lenght[Y], &counter);
	if (trace)
		printf("\n");
	if (deleted_rect != NULL){
		printf("RECTANGLE %s(%d,%d,%d,%d) DELETED\n",
			deleted_rect->rect_name, deleted_rect->center[X], deleted_rect->center[Y], deleted_rect->lenght[X], deleted_rect->lenght[Y]);
		}
	else
		printf("RECTANGLE %s(%d,%d,%d,%d) DOES NOT EXIST IN THE QUADTREE\n",
			node->rect->rect_name, node->rect->center[X], node->rect->center[Y], node->rect->lenght[X], node->rect->lenght[Y]);
}

static void delete_point(char args[][MAX_NAME_LEN + 1]) {
	int px = atoi(args[0]);
	int py = atoi(args[1]);
	rectangle_t *search_rect, *point_rect, w;
	int counter = 0;
	w = mx_cif_tree->world;

	point_rect = (rectangle_t *)malloc(sizeof(rectangle_t));
	point_rect->center[X] = px;
	point_rect->center[Y] = py;
	point_rect->lenght[X] = point_rect->lenght[Y] = 0;
	search_rect = cif_search(point_rect, mx_cif_tree->mx_cif_root, w.center[X], w.center[Y], w.lenght[X], w.lenght[Y], &counter);

	if (search_rect != NULL)
		delete_rectangle(search_rect->rect_name);
	else {
		if (trace)
			printf("\n");
			printf("POINT (%d,%d) NOT IN ANY RECTANGLE\n", px, py);
	}
}

static void move(char args[][MAX_NAME_LEN +1]) {
	char *name = args[0];
	int cx = atoi(args[1]);
	int cy = atoi(args[2]);
	rectangle_t *search_rect, *moved_rect, w;
	bnode_t *node;
	int counter = 0;
	w = mx_cif_tree->world;

	// Find the rectangle in the DB (BST) by its name
	search_rect = (rectangle_t *)malloc(sizeof(rectangle_t));
	search_rect->rect_name = name;
	node = (bnode_t *)malloc(sizeof(bnode_t));
	node->rect = search_rect;
	node = find_btree(rect_tree, node);

	moved_rect = (rectangle_t *)malloc(sizeof(rectangle_t));
	*moved_rect = *(node->rect);
	moved_rect->center[X] = moved_rect->center[X] + cx;
	moved_rect->center[Y] = moved_rect->center[Y] + cy;

	w = mx_cif_tree->world;
	rectangle_t *over_rect = cif_search(moved_rect, mx_cif_tree->mx_cif_root, w.center[X], w.center[Y], w.lenght[X], w.lenght[Y], &counter);
	counter = 0;
	if (trace)
		printf("\n");
	if (over_rect != NULL)
		printf("RECTANGLE %s(%d,%d,%d,%d) OVERLAPS RECTANGLE %s(%d,%d,%d,%d)\n",
			node->rect->rect_name, node->rect->center[X], node->rect->center[Y], node->rect->lenght[X], node->rect->lenght[Y],
			over_rect->rect_name, over_rect->center[X], over_rect->center[Y], over_rect->lenght[X], over_rect->lenght[Y]);
	else {
		cif_delete(node->rect, mx_cif_tree->mx_cif_root, w.center[X], w.center[Y], w.lenght[X], w.lenght[Y], &counter);
		counter = 0;
		if (trace)
			printf("\n");
		cif_insert(moved_rect, mx_cif_tree, w.center[X], w.center[Y], w.lenght[X], w.lenght[Y]);
		if (trace)
			printf("\n");
		printf("RECTANGLE %s MOVED TO (%d,%d)\n", moved_rect->rect_name, moved_rect->center[X], moved_rect->center[Y]);
	}
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
		search_point(args);
	else if (strcmp(command, "RECTANGLE_SEARCH") == 0)
		rectangle_search(args);
	else if (strcmp(command, "INSERT") == 0)
		insert_rectangle(args);
	else if (strcmp(command, "DELETE_RECTANGLE") == 0)
		delete_rectangle(args);
	else if (strcmp(command, "DELETE_POINT") == 0)
		delete_point(args);
	else if (strcmp(command, "MOVE") == 0)
		move(args);
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
