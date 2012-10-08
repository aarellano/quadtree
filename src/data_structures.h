/*
 * data_structures.h
 *
 *  Created on: Oct 7, 2012
 *      Author: andres
 */

#ifndef DATA_STRUCTURES_H_
#define DATA_STRUCTURES_H_

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



#endif /* DATA_STRUCTURES_H_ */
