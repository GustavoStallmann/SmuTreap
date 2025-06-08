#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <math.h>

#include "smu_treap.h"
#include "form.h"
#include "form_state.h"
#include "list.h"

typedef struct {
    struct Node *root;
    int hitCount, maxPriority;
    double promotionRate, epsilon; 
} SmuTreap_st;

typedef struct {
    double x, y, w, h; 
} BoundingBox; 

typedef struct nd {
    double x, y; 
    int priority, hitCount; 
    DescritorTipoInfo formType; 
    Info form; 
    BoundingBox form_bb; 
    BoundingBox sub_bb; 
    struct nd *left, *right; 
} Node_st; 

static void alloc_error() { 
    fprintf(stderr, "(ERROR) smu_treap: insufficient memory for smu_treap allocation");
    exit(EXIT_FAILURE); 
}

static inline int get_random_priority(int min, int max) {
    return rand() % (max - min + 1) + min;
}

SmuTreap newSmuTreap(int hitCount, double promotionRate, double epsilon, int prio) {
    if (hitCount < 1) {
        fprintf(stderr, "(ERROR) smu_treap: hitCount cannot be lower than 1"); 
        return NULL; 
    }

    if (promotionRate < 0) {
        fprintf(stderr, "(ERROR) smu_treap: promotionRate cannot be lower than 0");
        return NULL; 
    }

    SmuTreap_st *new_node = (SmuTreap_st *) malloc(sizeof(SmuTreap_st));
    if (new_node == NULL) {
        alloc_error();
        return NULL;
    }

    new_node->root = NULL;
    new_node->hitCount = hitCount; 
    new_node->promotionRate = promotionRate; 
    new_node->epsilon = epsilon; 
    new_node->maxPriority = prio; 

    return new_node; 
}

static Node newSmuTreapNode(Info form, DescritorTipoInfo formType, double x, double y, double priority) {
    Node_st* new_node = calloc(1, sizeof(Node_st)); 
    if (new_node == NULL) {
        alloc_error( ); 
        return NULL; 
    }

    new_node->left = NULL; 
    new_node->right = NULL; 
    new_node->formType = formType; 
    new_node->form = form; 
    new_node->x = x; 
    new_node->y = y; 
    new_node->priority = priority;
    new_node->hitCount = 0;

    return new_node; 
}

static bool bounding_box_intersects(const BoundingBox* bb1, const BoundingBox* bb2) {
    if (bb1->x >= bb2->x + bb2->w) {
        return false;
    }
    
    if (bb1->x + bb1->w <= bb2->x) {
        return false;
    }

    if (bb1->y >= bb2->y + bb2->h) {
        return false;
    }

    if (bb1->y + bb1->h <= bb2->y) {
        return false;
    }

    return true;
}

static bool is_point_inside_bounding_box(BoundingBox *bb, double x, double y) {
    if (bb == NULL) return false;
    
    return (x >= bb->x && x <= (bb->x + bb->w) &&
            y >= bb->y && y <= (bb->y + bb->h));
}

static void join_bounding_boxes(BoundingBox *first_bb, BoundingBox *second_bb, double *x, double *y, double *w, double *h) {
    if (first_bb == NULL || second_bb == NULL) return;
    
    double min_x = first_bb->x < second_bb->x ? first_bb->x : second_bb->x;
    double min_y = first_bb->y < second_bb->y ? first_bb->y : second_bb->y;
    
    double max_x = (first_bb->x + first_bb->w) > (second_bb->x + second_bb->w) ? 
                   (first_bb->x + first_bb->w) : (second_bb->x + second_bb->w);
    double max_y = (first_bb->y + first_bb->h) > (second_bb->y + second_bb->h) ? 
                (first_bb->y + first_bb->h) : (second_bb->y + second_bb->h);
    
    *x = min_x;
    *y = min_y;
    *w = max_x - min_x;
    *h = max_y - min_y;
}

static Node rotate_left(Node nd) {
    Node_st *node = (Node_st *) nd; 
    if (node == NULL || node->right == NULL) return node;  

    Node_st *right_node = node->right; 
    Node_st *right_node_left = right_node->left; 

    right_node->left = node; 
    node->right = right_node_left; 

    // Recalcula a bounding box do nó não promovido (agora filho esquerdo)
    // Inicia com a bounding box da forma do próprio nó
    node->sub_bb = node->form_bb;
    if (node->left != NULL)
        join_bounding_boxes(&node->sub_bb, &node->left->sub_bb, &node->sub_bb.x, &node->sub_bb.y, &node->sub_bb.w, &node->sub_bb.h);
    if (node->right != NULL)
        join_bounding_boxes(&node->sub_bb, &node->right->sub_bb, &node->sub_bb.x, &node->sub_bb.y, &node->sub_bb.w, &node->sub_bb.h);

    // Recalcula a bounding box do nó promovido (agora raiz)
    // Inicia com a bounding box da forma do próprio nó
    right_node->sub_bb = right_node->form_bb;
    if (right_node->left != NULL)
        join_bounding_boxes(&right_node->sub_bb, &right_node->left->sub_bb, &right_node->sub_bb.x, &right_node->sub_bb.y, &right_node->sub_bb.w, &right_node->sub_bb.h);
    if (right_node->right != NULL)
        join_bounding_boxes(&right_node->sub_bb, &right_node->right->sub_bb, &right_node->sub_bb.x, &right_node->sub_bb.y, &right_node->sub_bb.w, &right_node->sub_bb.h);

    return (Node) right_node;
}

static Node rotate_right(Node nd) {
    Node_st* node = (Node_st *) nd; 
    if (node == NULL || node->left == NULL) return node; 

    Node_st *left_node = node->left; 
    Node_st *left_node_right = left_node->right; 

    left_node->right = node;
    node->left = left_node_right;

    // Recalcula BB do node (agora filho direito)
    // Inicia com a bounding box da forma do próprio nó
    node->sub_bb = node->form_bb;
    if (node->left != NULL) 
        join_bounding_boxes(&node->sub_bb, &node->left->sub_bb, &node->sub_bb.x, &node->sub_bb.y, &node->sub_bb.w, &node->sub_bb.h);
    if (node->right != NULL) 
        join_bounding_boxes(&node->sub_bb, &node->right->sub_bb, &node->sub_bb.x, &node->sub_bb.y, &node->sub_bb.w, &node->sub_bb.h);

    // Recalcula BB do left_node (agora raiz)
    // Inicia com a bounding box da forma do próprio nó
    left_node->sub_bb = left_node->form_bb;
    if (left_node->left != NULL) 
        join_bounding_boxes(&left_node->sub_bb, &left_node->left->sub_bb, &left_node->sub_bb.x, &left_node->sub_bb.y, &left_node->sub_bb.w, &left_node->sub_bb.h);
    if (left_node->right != NULL) 
        join_bounding_boxes(&left_node->sub_bb, &left_node->right->sub_bb, &left_node->sub_bb.x, &left_node->sub_bb.y, &left_node->sub_bb.w, &left_node->sub_bb.h);

    return (Node) left_node; 
}

static Node fixHeapProperty_aux(Node nd) {
    if (nd == NULL) return NULL;
    
    Node_st *node = (Node_st *) nd;
    
    node->left = fixHeapProperty_aux(node->left);
    node->right = fixHeapProperty_aux(node->right);
    
    bool left_violates = node->left && ((Node_st*)node->left)->priority > node->priority;
    bool right_violates = node->right && ((Node_st*)node->right)->priority > node->priority;

    if (left_violates && right_violates) {
        if (((Node_st*)node->left)->priority >= ((Node_st*)node->right)->priority) {
            return rotate_right(nd);
        } else {
            return rotate_left(nd);
        }
    } else if (left_violates) {
        return rotate_right(nd);
    } else if (right_violates) {
        return rotate_left(nd);
    }
    
    return nd;
}

void promoteNodeSmuT(SmuTreap t, Node n, double promotionRate) {
    assert(t); 
    assert(n);

    SmuTreap_st *tree = (SmuTreap_st *) t; 
    Node_st *node = (Node_st *) n; 

    double new_priority = promotionRate * node->priority; 
    if (new_priority > tree->maxPriority) {
        new_priority = tree->maxPriority; 
    }

    node->priority = (int)new_priority; 
    tree->root = fixHeapProperty_aux(tree->root);
}

static Node insertSmuT_aux(Node r, Node i) {
    Node_st *root = (Node_st*) r; 
    Node_st *insertion_node = (Node_st *) i; 

    if (root == NULL) return insertion_node; 

    if (insertion_node->x <= root->x) { // left insertion
        root->left = insertSmuT_aux((Node) root->left, insertion_node); 

        if (root->left != NULL && ((Node_st*)root->left)->priority > root->priority) {
            return rotate_right((Node) root);
        }
    } else { // right insertion
        root->right = insertSmuT_aux((Node) root->right, insertion_node); 
        
        if (root->right != NULL && ((Node_st*)root->right)->priority > root->priority) {
            return rotate_left((Node) root); 
        }
    }

    // Recalcula a bounding box do nó atual incluindo seus filhos
    // Inicia com a bounding box da forma do próprio nó
    root->sub_bb = root->form_bb;
    
    if (root->left != NULL) {
        join_bounding_boxes(&root->sub_bb, &root->left->sub_bb, &root->sub_bb.x, &root->sub_bb.y, &root->sub_bb.w, &root->sub_bb.h);
    }
    if (root->right != NULL) {
        join_bounding_boxes(&root->sub_bb, &root->right->sub_bb, &root->sub_bb.x, &root->sub_bb.y, &root->sub_bb.w, &root->sub_bb.h);
    }
    return (Node) root;
}

Node insertSmuT(SmuTreap t, double x, double y, Info form, DescritorTipoInfo formType, FCalculaBoundingBox fCalcBb) {
    assert(t); 
    
    SmuTreap_st *tree = (SmuTreap_st *) t; 
    Node_st *new_node = (Node_st *) newSmuTreapNode(form, formType, x, y, get_random_priority(0, tree->maxPriority)); 
    fCalcBb(formType, new_node->form, &new_node->form_bb.x, &new_node->form_bb.y, &new_node->form_bb.w, &new_node->form_bb.h); 
    new_node->sub_bb = new_node->form_bb; 

    tree->root = insertSmuT_aux((Node) tree->root, new_node); 
    
    return new_node; 
} 

Info getBoundingBoxSmuT(SmuTreap t, Node n, double *x, double *y, double *w, double *h) {
    (void)t;
    assert(n);

    Node_st *node = (Node_st *) n; 
    *x = node->sub_bb.x; 
    *y = node->sub_bb.y;
    *w = node->sub_bb.w;
    *h = node->sub_bb.h;
    
    return node->form;
}

static void visitaProfundidadeSmuT_aux(SmuTreap t, Node nd, FvisitaNo f, void *aux) {
    if (nd == NULL) return; 
        
    Node_st *node = (Node_st *) nd; 
    
    f(t, nd, node->form, node->x, node->y, aux);
    visitaProfundidadeSmuT_aux(t, ((Node_st *) nd)->left, f, aux); 
    visitaProfundidadeSmuT_aux(t, ((Node_st *) nd)->right, f, aux); 
}

void visitaProfundidadeSmuT(SmuTreap t, FvisitaNo f, void *aux) {
    assert(t); 
    
    SmuTreap_st *tree = (SmuTreap_st *) t; 
    visitaProfundidadeSmuT_aux(t, tree->root, f, aux);
}

static void promote_node_by_hit_count(SmuTreap t, Node_st *root) {
    assert(root);
    assert(t);

    SmuTreap_st *tree = (SmuTreap_st *) t;

    root->hitCount++;
    
    if (root->hitCount >= tree->hitCount) {
        promoteNodeSmuT(t, (Node) root, tree->promotionRate);
        root->hitCount = 0; 
    }
}

static bool compare_nodes(ListValue value, void *target) {
   Node *node = (Node *) value; 
   Node *target_node = (Node *) target; 

   Info node_form = getInfoSmuT(NULL, node); 
   DescritorTipoInfo node_form_type = getTypeInfoSmuT(NULL, node); 
   Info node_target_form = getInfoSmuT(NULL, target_node); 
   DescritorTipoInfo node_target_form_type = getTypeInfoSmuT(NULL, target_node); 


   int node_id = get_form_id(node_form_type, node_form); 
   int target_node_id = get_form_id(node_target_form_type, node_target_form); 
   return node_id == target_node_id;
}

static void getInfosDentroRegiaoSmuT_aux(SmuTreap t, Node_st *root, BoundingBox interest_bb, List L, FdentroDeRegiao is_form_inside_region, double x1, double y1, double x2, double y2) {
    if (root == NULL) return;
    if (bounding_box_intersects(&root->sub_bb, &interest_bb) == false) return; // node subtree does not intersect the interest region
    
    if (is_form_inside_region(NULL, root, root->form, x1, y1, x2, y2)) {
        FormState state = get_form_state(root->formType, root->form);
        if (!list_includes(L, root, &compare_nodes)) {
            list_insert(L, root);
            promote_node_by_hit_count(t, root);
            
            set_form_state_selected(state, true); 
        }
    }

    getInfosDentroRegiaoSmuT_aux(t, root->left, interest_bb, L, is_form_inside_region, x1, y1, x2, y2);
    getInfosDentroRegiaoSmuT_aux(t, root->right, interest_bb, L, is_form_inside_region, x1, y1, x2, y2);
}

bool getInfosDentroRegiaoSmuT(SmuTreap t, double x1, double y1, double x2, double y2, FdentroDeRegiao f, List L) {
    assert(t);
    SmuTreap_st *tree = (SmuTreap_st *) t;

    double min_x = (x1 < x2) ? x1 : x2;
    double max_x = (x1 < x2) ? x2 : x1;
    double min_y = (y1 < y2) ? y1 : y2;
    double max_y = (y1 < y2) ? y2 : y1;

    BoundingBox interest_bb = {
        .x = min_x, 
        .y = min_y, 
        .w = max_x - min_x, 
        .h = max_y - min_y
    };
    
    getInfosDentroRegiaoSmuT_aux(t, (Node_st *) tree->root, interest_bb, L, f, x1, y1, x2, y2);
    if (list_get_size(L) <= 0) return false; 

    return true; 
}

static void getNodesDentroRegiaoSmuT_aux(Node r, BoundingBox interest_bb, List L, SmuTreap t) {
    Node_st *root = (Node_st *) r; 
    if (root == NULL) return;

    if (bounding_box_intersects(&root->sub_bb, &interest_bb) == false) return; // node subtree does not intersect the interest region 

    if (is_point_inside_bounding_box(&interest_bb, root->x, root->y)) {
        if (!list_includes(L, root, &compare_nodes)) {
            list_insert(L, root);
            promote_node_by_hit_count(t, root);
        }
    }

    getNodesDentroRegiaoSmuT_aux(root->left, interest_bb, L, t);
    getNodesDentroRegiaoSmuT_aux(root->right, interest_bb, L, t);
}

bool getNodesDentroRegiaoSmuT(SmuTreap t, double x1, double y1, double x2, double y2, List L) {
    SmuTreap_st *tree = (SmuTreap_st *) t;

    double min_x = (x1 < x2) ? x1 : x2;
    double max_x = (x1 < x2) ? x2 : x1;
    double min_y = (y1 < y2) ? y1 : y2;
    double max_y = (y1 < y2) ? y2 : y1;

    BoundingBox interest_bb = {
        .x = min_x, 
        .y = min_y, 
        .w = max_x - min_x, 
        .h = max_y - min_y
    };
    
    getNodesDentroRegiaoSmuT_aux( tree->root, interest_bb, L, t);

    if (list_get_size(L) <= 0) return false;
    return true;
}

DescritorTipoInfo getTypeInfoSmuT(SmuTreap t, Node n) {
    (void)t; // ignore arg
    assert(n);
    if (n == NULL) return -1; 
    
    Node_st *node = (Node_st *) n; 
    return node->formType; 
}

static Node getNodeSmuT_aux(Node current, double x, double y, double epsilon, SmuTreap t) {
    if (current == NULL) return NULL;
    
    Node_st *node = (Node_st *) current;

    if (fabs(node->x - x) <= epsilon && fabs(node->y - y) <= epsilon) {
        promoteNodeSmuT(t, current, ((SmuTreap_st *)t)->promotionRate);
        return current;
    }

    if (x <= node->x) {
        return getNodeSmuT_aux(node->left, x, y, epsilon, t);
    } else {
        return getNodeSmuT_aux(node->right, x, y, epsilon, t);
    }
}

Node getNodeSmuT(SmuTreap t, double x, double y) {
    assert(t);
    SmuTreap_st *tree = (SmuTreap_st *) t; 
    return getNodeSmuT_aux(tree->root, x, y, tree->epsilon, t);
}

Info getInfoSmuT(SmuTreap t, Node n) {
    (void)t; 
    assert(n);
    
    Node_st *node = (Node_st *) n; 
    return node->form; 
}

static void killSmuTreap_aux(SmuTreap t, Node n, Info i, double x, double y, void *aux) {
    (void)x; 
    (void)y; 
    (void)aux; 
    
    DescritorTipoInfo formType = getTypeInfoSmuT(t, n);
    free_form(formType, i);
}

static void killSmuTreap_nodes_aux(Node nd) {
    if (nd == NULL) return;
    
    Node_st *node = (Node_st *) nd;

    killSmuTreap_nodes_aux(node->left);
    killSmuTreap_nodes_aux(node->right);
    free(node);
}

void killSmuTreap(SmuTreap t) {
    assert(t); 
    SmuTreap_st *tree = (SmuTreap_st *) t;
    
    visitaProfundidadeSmuT(t, &killSmuTreap_aux, NULL);
    killSmuTreap_nodes_aux(tree->root); 
    free(t);
}

static void get_node_color(DescritorTipoInfo formType, char *str, char *color) {
    switch (formType) {
        case CIRCLE:
            strcpy(str, "circle");
            strcpy(color, "blue");
            break;
        case RECT:
            strcpy(str, "rectangle");
            strcpy(color, "green");
            break;
        case TEXT:
            strcpy(str, "text");
            strcpy(color, "orange");
            break;
        case LINE:
            strcpy(str, "line");
            strcpy(color, "purple");
            break;
        default:
            strcpy(str, "unknown");
            strcpy(color, "black");
    }
}

static void printDotSmuTreap_aux(Node_st *node, FILE *file) {
    if (node == NULL) return;

    char color[20], str[20];
    get_node_color(node->formType, str, color);

    fprintf(file, "\t\"%p\" [label=\"%s\np: %d\nhc: %d\nX: %d\\nY: %d\", color=black, fontcolor=white, style=filled, fillcolor=%s];\n", node, str, node->priority, node->hitCount, (int) node->x, (int) node->y, color);

    if (node->left != NULL) {
        fprintf(file, "\t\"%p\" -- \"%p\";\n", node, node->left);
        printDotSmuTreap_aux(node->left, file);
    }

    if (node->right != NULL) {
        fprintf(file, "\t\"%p\" -- \"%p\";\n", node, node->right);
        printDotSmuTreap_aux(node->right, file);
    }
}

bool printDotSmuTreap(SmuTreap t, char *fn) {
    assert(t); 
    assert(fn); 

    SmuTreap_st *tree = (SmuTreap_st *) t;

    FILE *file = fopen(fn, "w");
    if (file == NULL) {
        fprintf(stderr, "(ERROR) smu_treap: could not open file %s for writing\n", fn);
        return false; 
    }

    fprintf(file, "graph G {\n");
    fprintf(file, "\trankdir=TB;\n"); 
    fprintf(file, "\tnode [shape=circle];\n");

    printDotSmuTreap_aux((Node_st *) tree->root, file);
    fprintf(file, "}\n");
    fclose(file);
    return true; 
}

Node procuraNoSmuT_aux(SmuTreap t, Node nd, FsearchNo f, void *aux) {
    if (nd == NULL) return NULL;

    Node_st *node = (Node_st *) nd; 

    Info info = getInfoSmuT(t, nd);
    if (f(t, nd, info, node->x, node->y, aux)) {
        promote_node_by_hit_count(t, node);
        return nd; 
    }

    Node found = procuraNoSmuT_aux(t, node->left, f, aux);
    if (found != NULL) {
        return found;
    }

    found = procuraNoSmuT_aux(t, node->right, f, aux);
    if (found != NULL) {
        return found;
    }

    return NULL;
}

Node procuraNoSmuT(SmuTreap t, FsearchNo f, void *aux) {
    assert(t);
    assert(f);

    SmuTreap_st *tree = (SmuTreap_st *) t; 
    return procuraNoSmuT_aux(t, tree->root, f, aux);
}

void getInfosAtingidoPontoSmuT_aux(SmuTreap t, Node nd, double x, double y, FpontoInternoAInfo f, List L) {
    if (nd == NULL) return; 

    Node_st *node = (Node_st *) nd; 

    if (!is_point_inside_bounding_box(&node->sub_bb, x, y)) return; 
    
    if (f(t, nd, node->form, x, y)) {
        list_insert(L, node);
        promote_node_by_hit_count(t, node);
    }

    getInfosAtingidoPontoSmuT_aux(t, node->left, x, y, f, L);
    getInfosAtingidoPontoSmuT_aux(t, node->right, x, y, f, L);
}

bool getInfosAtingidoPontoSmuT(SmuTreap t, double x, double y, FpontoInternoAInfo f, List L) {
    assert(t);
    assert(f);
    assert(L);

    SmuTreap_st *tree = (SmuTreap_st *) t; 
    getInfosAtingidoPontoSmuT_aux(t, tree->root, x, y, f, L);
    
    if (list_get_size(L) <= 0) return false; 

    return true;
}

static Node removeNodeByIdSmuT_aux(Node root, int target_id) {
    if (root == NULL) return NULL;

    Node_st *root_node = (Node_st *) root;
    int current_id = get_form_id(root_node->formType, root_node->form);

    if (current_id == target_id) {
        if (root_node->left == NULL && root_node->right == NULL) {
            DescritorTipoInfo formType = root_node->formType;
            free_form(formType, root_node->form);
            free(root_node);
            return NULL;
        }

        if (root_node->left == NULL) {
            Node rotated = rotate_left(root);
            Node_st *promotedNode = (Node_st *) rotated;
            promotedNode->left = removeNodeByIdSmuT_aux(promotedNode->left, target_id);
            return rotated;
        }

        if (root_node->right == NULL) {
            Node rotated = rotate_right(root);
            Node_st *promotedNode = (Node_st *) rotated;
            promotedNode->right = removeNodeByIdSmuT_aux(promotedNode->right, target_id);
            return rotated;
        }

        if (((Node_st*)root_node->left)->priority > ((Node_st*)root_node->right)->priority) {
            Node rotated = rotate_right(root);
            Node_st *promotedNode = (Node_st *) rotated;
            promotedNode->right = removeNodeByIdSmuT_aux(promotedNode->right, target_id);
            return rotated;
        } else {
            Node rotated = rotate_left(root);
            Node_st *promotedNode = (Node_st *) rotated;
            promotedNode->left = removeNodeByIdSmuT_aux(promotedNode->left, target_id);
            return rotated;
        }
    }

    root_node->left = removeNodeByIdSmuT_aux(root_node->left, target_id);
    root_node->right = removeNodeByIdSmuT_aux(root_node->right, target_id);

    root_node->sub_bb = root_node->form_bb;
    if (root_node->left != NULL) {
        join_bounding_boxes(&root_node->sub_bb, &root_node->left->sub_bb, 
                            &root_node->sub_bb.x, &root_node->sub_bb.y, 
                            &root_node->sub_bb.w, &root_node->sub_bb.h);
    }
    if (root_node->right != NULL) {
        join_bounding_boxes(&root_node->sub_bb, &root_node->right->sub_bb, 
                            &root_node->sub_bb.x, &root_node->sub_bb.y, 
                            &root_node->sub_bb.w, &root_node->sub_bb.h);
    }

    return root;
}

void removeNoSmuT(SmuTreap t, Node n) {
    assert(t);
    assert(n);
    
    SmuTreap_st *tree = (SmuTreap_st *) t;
    Node_st *node = (Node_st *) n;
    
    int target_id = get_form_id(node->formType, node->form);
    tree->root = removeNodeByIdSmuT_aux(tree->root, target_id);
}