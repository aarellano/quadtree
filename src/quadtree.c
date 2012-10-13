#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "quadtree.h"
#include "drawing_c.h"

struct mxCif *mxCifTree; //MX-CIF Quadtree
bNode *rectTree; //Rectangle bin tree, sorted with respect to rect names

const double DISPLAY_SIZE = 128;
double scale_factor;

static bNode *find_btree(bNode *tree, bNode *node);

static void init_mx_cif_tree(void) {
	mxCifTree = (struct mxCif *)malloc(sizeof(struct mxCif));
	mxCifTree->mxCifRoot = NULL;
	mxCifTree->World.Name = "MX-CIF";
}

static void init_rect_tree(void) {
	 rectTree = NULL;
 }

static void print_in_order(bNode *node) {
	if (node != NULL) {
		print_in_order(node->binSon[LEFT]);
		printf("%s(%d,%d,%d,%d) ", node->Rect->Name, node->Rect->Center[X], node->Rect->Center[Y], node->Rect->Lenght[X], node->Rect->Lenght[Y]);
		print_in_order(node->binSon[RIGHT]);
	}
}

static void print_pre_order(bNode *node) {
	if (node != NULL) {
		printf("%s", node->Rect->Name);
		print_pre_order(node->binSon[LEFT]);
		print_pre_order(node->binSon[RIGHT]);
	}
}

static direction bin_compare(Rectangle *P, long Cv, axis V) {
	/*
	** Determines whether rectangle P lies to the left of, right of, or contains line V=Cv
	*/
	if (((P->Center[V] - P->Lenght[V]) < Cv) && (Cv < ((P->Center[V] + P->Lenght[V]))))
		return BOTH;
	else if (Cv < P->Center[V])
		return RIGHT;
	else
		return LEFT;
}

static quadrant cif_compare(Rectangle *P, int Cx, int Cy) {
	/*
	** Return the quadrant of the MX-CIF quadtree rooted at position (Cx,Cy) that contains
	** the centroid of rectangle P
	*/

	if (P->Center[X] < Cx)
		if (P->Center[Y] < Cy)
			return SW;
		else
			return NW;
	else
		if (P->Center[Y] < Cy)
			return SE;
		else
			return NE;
}

static bNode *create_bnode(void) {
	bNode *node = (bNode *)malloc(sizeof(bNode));
	node->Rect = NULL;
	node->binSon[X] = node->binSon[Y] = NULL;
	return node;
}

static cNode *create_cnode(void) {
	cNode *node = (cNode *)malloc(sizeof(cNode));
	node->spcSon[NW] = node->spcSon[NE] = node->spcSon[SW] = node->spcSon[SE] = NULL;
	node->binSon[X] = node->binSon[Y] = NULL;
	return node;
}

static void insert_axis(Rectangle *P, cNode *R, int Cv, int Lv, axis V) {
	bNode *T;
	int F[] = {-1, 1};
	direction D;

	if (R->binSon[V] == NULL)
		R->binSon[V] = create_bnode();

	T = R->binSon[V];
	D = bin_compare(P, Cv, V);
	while (D != BOTH) {
		if (T->binSon[D] == NULL)
			T->binSon[D] = create_bnode();
		T = T->binSon[D];
		Lv = Lv / 2;
		Cv = Cv + F[D] * Lv;
		D = bin_compare(P, Cv, V);
	}
	T->Rect = P;
}

static void cif_insert(Rectangle *P, struct mxCif *cifTree, int Cx, int Cy, int Lx, int Ly) {
	int Sx[] = {-1, 1, -1, 1};
	int Sy[] = {1, 1, -1, -1};
	cNode *T;
	quadrant Q;
	direction Dx, Dy;
	cNode *R;

	printf("INSERTING RECTANGLE %s WITH CENTER COORDS %d,%d\n", P->Name, P->Center[X], P->Center[Y]);
	if (cifTree->mxCifRoot == NULL)
		cifTree->mxCifRoot = create_cnode();

	R = cifTree->mxCifRoot;
	T = R;
	Dx = bin_compare(P, Cx, X);
	Dy = bin_compare(P, Cy, Y);

	while ((Dx != BOTH) && (Dy != BOTH)) {
		Q = cif_compare(P, Cx, Cy);
		if (T->spcSon[Q] == NULL)
			T->spcSon[Q] = create_cnode();
		T = T->spcSon[Q];
		Lx = Lx/2;
		Ly = Ly/2;
		Cx = Cx + Sx[Q] * Lx;
		Cy = Cy + Sy[Q] * Ly;
		Dx = bin_compare(P, Cx, X);
		Dy = bin_compare(P, Cy, Y);
	}

	if (Dx == BOTH)
		insert_axis(P, T, Cy, Ly, Y);
	else
		insert_axis(P, T, Cx, Lx, X);
}

static void insert_rectangle(char args[][MAX_NAME_LEN + 1]) {
	char *name = args[0];
	Rectangle *rectangle;
	bNode *node;

	rectangle = (Rectangle *)malloc(sizeof(Rectangle));
	rectangle->Name = name;

	node = (bNode *)malloc(sizeof(bNode));
	node->Rect = rectangle;

	node = find_btree(rectTree, node);
	cif_insert(node->Rect, mxCifTree, mxCifTree->World.Center[X], mxCifTree->World.Center[Y], mxCifTree->World.Center[X], mxCifTree->World.Center[Y]);
}

static void list_rectangles(void) {
	print_in_order(rectTree);
	printf("\n");
}

static bNode *find_btree(bNode *tree, bNode *node) {
	if (!tree)
		return NULL;

	if (strcmp(tree->Rect->Name, node->Rect->Name) == 0)
		return node = tree;
	else if (strcmp(tree->Rect->Name, node->Rect->Name) > 0)
		node = find_btree(tree->binSon[LEFT], node);
	else
		node = find_btree(tree->binSon[RIGHT], node);

	return node;
}

static void insert_to_btree(bNode **root, bNode *newNode) {
	if (*root == NULL) {
		*root = newNode;
		return;
	}

	if (strcmp((*root)->Rect->Name, newNode->Rect->Name) > 0)
		insert_to_btree(&(*root)->binSon[LEFT], newNode);
	else if (strcmp((*root)->Rect->Name, newNode->Rect->Name ))
		insert_to_btree(&(*root)->binSon[RIGHT], newNode);
}

static void create_rectangle(char args[][MAX_NAME_LEN + 1]) {
	char *name = args[0];
	int cx = atoi(args[1]);
	int cy = atoi(args[2]);
	int lx = atoi(args[3]);
	int ly = atoi(args[4]);

	Rectangle *newRectangle = (Rectangle *)malloc(sizeof(Rectangle));
	newRectangle->Name = strdup(name);
	newRectangle->binSon[LEFT] = newRectangle->binSon[RIGHT] = NULL;
	newRectangle->Center[X] = cx;
	newRectangle->Center[Y] = cy;
	newRectangle->Lenght[X] = lx;
	newRectangle->Lenght[Y] = ly;

	bNode *newNode = (bNode *)malloc(sizeof(bNode));
	newNode->Rect = newRectangle;
	newNode->binSon[LEFT] = newNode->binSon[RIGHT] = NULL;
	insert_to_btree(&rectTree, newNode);

	printf("CREATED RECTANGLE(%s,%d,%d,%d,%d)\n", name, cx, cy, lx, ly);
}

static void init_quadtree(char args[][MAX_NAME_LEN + 1]) {
	int width = atoi(args[0]);

	scale_factor = DISPLAY_SIZE / (1 << width);

	mxCifTree->World.Lenght[X] = 1 << width;
	mxCifTree->World.Lenght[Y] = 1 << width;
	mxCifTree->World.Center[X] = mxCifTree->World.Lenght[X] / 2;
	mxCifTree->World.Center[Y] = mxCifTree->World.Lenght[Y] / 2;

	printf("MX-CIF QUADTREE 0 INITIALIZED WITH PARAMETER %d\n", width);
}

static void traverse_bintree(bNode *node) {
	if (node != NULL) {
		printf("%s\n", node->Rect->Name);
		traverse_bintree(node->binSon[LEFT]);
		traverse_bintree(node->binSon[RIGHT]);
	}
}

static void traverse_quadtree(cNode *node) {
	if (node != NULL) {
		traverse_bintree(node->binSon[X]);
		traverse_bintree(node->binSon[Y]);

		traverse_quadtree(node->spcSon[NW]);
		traverse_quadtree(node->spcSon[NE]);
		traverse_quadtree(node->spcSon[SW]);
		traverse_quadtree(node->spcSon[SE]);
	}
}

static void display(void) {
	StartPicture(DISPLAY_SIZE + 1, DISPLAY_SIZE + 1);
	SetLineDash(3, 3);
	DrawRect(0, DISPLAY_SIZE, DISPLAY_SIZE, 0);
	SetLineDash(3, 3);
	traverse_quadtree(mxCifTree->mxCifRoot);
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
