#ifndef DATA_STRUCTURES_H_
#define DATA_STRUCTURES_H_

#define MAX_STRING_LEN 256
#define MAX_NAME_LEN 6

#define NDIR_1D 2 //number of directions in 1d space
#define NDIR_2D 4 ///number of directions in 2d space

typedef enum {X, Y} axis;
typedef enum {NW, NE, SW, SE} quadrant; //Use this ordering in traversal
typedef enum {LEFT, RIGHT, BOTH} direction;

typedef struct {
	char *Name; //Name of the rectangle
	struct Rectangle *binSon[NDIR_1D]; //Left and right sons
	int Center[NDIR_1D]; //Centroid
	int	Lenght[NDIR_1D]; //Distance to the borders of rect
	int Label; //Used for LABEL() operation
} Rectangle;

//Node in binary tree of the set of rectangles
typedef struct bNode {
	struct bNode *binSon[NDIR_1D]; //Left and right sons
	Rectangle *Rect; //Pointer to the rectangle whose area contains the axis subdivision point
} bNode;

//Node in MX-CIF quadtree
typedef struct cNode {
	struct cNode *spcSon[NDIR_2D]; //Four principal quad directions
	bNode *binSon[NDIR_1D]; //Pointers to rectangle sets for each of the axis
} cNode;

struct mxCif {
	struct cNode *mxCifRoot; //Root Node
	Rectangle World; //World extent
	int Id; //Quadtree ID
};

#endif /* DATA_STRUCTURES_H_ */
