#include <errno.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdio.h>
#include "../tests/testpoint.c"



#define EOK 0
/* generic version will cost a bit more (doing a "which type of retrieve() am I doing? 
which type of store() am I doing?) but will have a simple API. NO IT WON'T, IT WILL HAVE A TON OF OPTIONS
AND PEOPLE WHO WANT TO USE SOMETHING SIMPLE WILL HAVE TO KNOW NOT TO FUCK AROUND WITH THOSE OPTIONS.  But there are
defaults for a reason... though it would require either the head be stored in a struct somewhere as 
a void pointer, or a bunch of casting would be needed.  As it is, the void * will have to be casted behind the scenes
in each of the functions*/

struct node_t {
	struct node_t * left;
	struct node_t * right;
	//this_struct_t ** ptr_storage;
	//void * data;
	int data;
};

//be careful, this might be undefined on other systems XXX
#define node_t struct node_t

node_t mynode;

typedef enum {
	BINARY,
	AVL,
	SPLAY,
	B,
	B_PLUS,
	RED_BLACK,
	SCAPEGOAT,
	TREAP
} tree_type_t;

//XXX
typedef enum {
	INDEX_YES,
	INDEX_NO
} index_t;

typedef struct {
	tree_type_t type;
	//XXX
	/* Remember the indexing you did for the 391 project? add that as an option to turn on or off */
	// Max depth?
	index_t index;
} tree_attr_t;



void create_tree_attr(tree_attr_t * attr) {
	attr->type = BINARY;
	attr->index = INDEX_YES; //XXX
}


/* if attr is null, can we assign it to a chunk of memory we set up with create_tree_attr? maybe with a malloc? */
void * create_tree(tree_attr_t * attr) {
	tree_attr_t attrIn;
	
	//if (attr) {
		//
	
	if (attr == NULL) {
		create_tree_attr(&attrIn);
	} else {
		//XXX gotta be a better way? memcopy?
		attrIn.type = attr->type;
		attrIn.index = attr->index;
	}
	
	switch (attrIn.type) {
		case BINARY:
		default:
			break;
	}
	
	return EOK;
}

/*****************************************************************************************
*                                                                                        
*    Function 
*           create_tree_test
*
******************************************************************************************/
int create_tree_test() {
	teststart("create_tree()");
	
	pointstart("create_tree(NULL)");
	create_tree(NULL);
	pointpass("create_tree(NULL) %d %s", 12, "hello");
	
	pointstart("pass in ptr assigned value of NULL");
	tree_attr_t * attr = NULL;
	create_tree(attr);
	pointpass("");
	
	pointstart("pass in attributes, but uninitialized");
	tree_attr_t attr2;
	create_tree(&attr2);
	pointpass("tree_attr_t attr2");
	
	testend("create_tree()");
	
	return 0;
}
		 
		 
void setup() {
	setlinebuf(stdout);
	setlinebuf(stderr);
}

int main() {

	setup();
	
	tree_attr_t myTreeAttr;
	create_tree_attr(&myTreeAttr);
	
	create_tree_test();
	
	memset(&mynode, 0, sizeof(node_t));
	return 0;
	
	
}
	
	

	