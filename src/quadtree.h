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
	char *rect_name; //Name of the rectangle
	struct rectangle *bson[NDIR_1D]; //Left and right sons
	int center[NDIR_1D]; //Centroid
	int	lenght[NDIR_1D]; //Distance to the borders of rect
	int label; //Used for LABEL() operation
} rectangle_t;

typedef struct bnode {
	struct bnode *bson[NDIR_1D]; //Left and right sons
	rectangle_t *rect; //Pointer to the rectangle whose area contains the axis subdivision point
} bnode_t;

typedef struct cnode {
	struct cnode *qson[NDIR_2D]; //Four principal quad directions
	bnode_t *bson[NDIR_1D]; //Pointers to rectangle sets for each of the axis
} cnode_t;

struct mxcif {
	struct cnode *mx_cif_root; //Root Node
	rectangle_t world; //World extent
	int id; //Quadtree ID
};

#endif /* DATA_STRUCTURES_H_ */
