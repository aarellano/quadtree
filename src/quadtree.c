/*
 * data_structure.c
 *
 *  Created on: Sep 24, 2012
 *      Author: andres
 */

#include <stdio.h>
#include <strings.h>

#include "util.h"

#define MAX_STRING_LEN 256
#define MAX_NAME_LEN 6

#define NDIR_1D 2 //number of directions in 1d space
#define NDIR_2D 4 ///number of directions in 2d space

typedef enum {X, Y} axis;
typedef enum {NW, NE, SW, SE} quadrant; //Use this ordering in traversal
typedef enum {LEFT, RIGHT, BOTH} direction;

typedef char tName[MAX_NAME_LEN + 1];

struct Rectangle {
	tName Name; //Name of the rectangle
	struct Rectangle *binSon[NDIR_1D]; //Left and right sons
	int Center[NDIR_1D]; //Centroid
	int	Lenght[NDIR_1D]; //Distance to the borders of rect
	int Label; //Used for LABEL() operation
};

//Node in binary tree of the set of rectangles
struct bNode {
	struct bNode *binSon[NDIR_1D]; //Left and right sons
	struct Rectangle *Rect; //Pointer to the rectangle whose area contains the axis subdivision point
};

//Node in MX-CIF quadtree
struct cNode {
	struct cNode *spcSon[NDIR_2D]; //Four principal quad directions
	struct bNode *binSon[NDIR_1D]; //Pointers to rectangle sets for each of the axis
};

struct mxCif {
	struct cNode *mxCifRoot; //Root Node
	struct Rectangle World; //World extent
	int Id; //Quadtree ID
};

/*these are variables holdings roots of corresponding trees, they are
 * global to avoid additional asterisks in calls */

struct mxCif mxCifTree; //MX-CIF Quadtree
struct Rectangle *rectTree; //Rectangle bin tree, sorted with respect to rect names

void initMxCifTree(void) {
	mxCifTree.mxCifRoot = NULL;
	strcpy(mxCifTree.World.Name, "MX-CIF");
	mxCifTree.World.binSon[LEFT] = NULL;
	mxCifTree.World.binSon[RIGHT] = NULL;
	//the world size will have to be assigned later
}

void initRectTree(void) {
	 rectTree = NULL;
 }

void create_rectangle(char args[][10]) {
	char *name = args[0];
	int cx = atoi(args[1]);
	int cy = atoi(args[2]);
	int lx = atoi(args[3]);
	int ly = atoi(args[4]);



	printf("%s%s%s%d%s%d%s%d%s%d%s", "CREATED RECTANGLE ", name, "(", cx, ",", cy, ",", lx, ",", ly, ")");
}

void init_quadtree(char args[][10]) {
	int width = atoi(args[0]);
	mxCifTree.World.Lenght[X] = 2 << width;
	mxCifTree.World.Lenght[Y] = 2 << width;

	printf("%s %d", "MX-CIF QUADTREE 0 INITIALIZED WITH PARAMETER ", width);
}

void decode_command(char *command, char args[][10])
{
	if (strcmp(command, "INIT_QUADTREE") == 0)
		init_quadtree(args);
	else if (strcmp(command, "DISPLAY") == 0)
		return;
	else if (strcmp(command, "LIST_RECTANGLES") == 0)
		return;
	else if (strcmp(command, "CREATE_RECTANGLE") == 0)
		create_rectangle(args);
	else if (strcmp(command, "SEARCH_POINT") == 0)
		return;
	else if (strcmp(command, "RECTANGLE_SEARCH") == 0)
		return;
	else if (strcmp(command, "INSERT") == 0)
		return;
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

void read_command()
{
	char c;
	int i = 0, j=0, k=0;

while(1){
	char args[10][10];
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
	initMxCifTree();
	initRectTree();

	read_command();
	//MORE CODE HERE
	return (0);
}

#include <stdio.h>
#include <string.h>


