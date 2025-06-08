#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "processor_qry.h"
#include "file.h"
#include "form.h"
#include "form_state.h"
#include "form_style.h"
#include "list.h"
#include "processor_form.h"
#include "smu_treap.h"
#include "form_selection.h"

#define MAX_LINE_LENGTH 512

static SelectionManager g_selection_manager = NULL; 

static bool process_command(char *line_buffer, char *command_type) {
    if (sscanf(line_buffer, "%9s", command_type) == 0) {
        if (line_buffer[0] != '\n' && line_buffer[0] != '\0') {
            fprintf(stderr, "ERROR: processor_qry unknown line type\n"); 
        } 
        return false;
    }; 

    return true; 
}

static void print_selr_selection_callback(void *value, callback_data call_data) {
    assert(value); 
    assert(call_data); 

    Node node = (Node) value;
    FILE *txt_file = (FILE *) call_data; 

    Info node_form = getInfoSmuT(NULL, node);
    DescritorTipoInfo node_form_type = getTypeInfoSmuT(NULL, node);
    int form_id = get_form_id(node_form_type, node_form); 

    fprintf(txt_file, "\t%d: %s\n",form_id, get_form_name(node_form_type));
}

static void save_node_forms_to_export(SmuTreap t, Node node, List destroyed_forms) {
    Info form = getInfoSmuT(t, node); 
    DescritorTipoInfo form_type = getTypeInfoSmuT(t, node); 
    int form_id = get_form_id(form_type, form); 
    double x, y; 
    get_form_coordinates(form_type, form, &x, &y);

    FormState form_state = get_form_state(form_type, form);
    bool is_blown = is_form_state_blown(form_state); 
    bool is_hit = is_form_state_hit(form_state); 
    bool is_selected = is_form_state_selected(form_state); 
    bool is_camouflaged = is_form_state_camouflaged(form_state);
    
    FormInfo new_form_info = clone_form(form_type, form, form_id, x, y);
    Info new_form = get_form_info(new_form_info); 
    DescritorTipoInfo new_form_type = get_form_type(new_form_info);

    FormState new_form_state = get_form_state(new_form_type, new_form);
    set_form_state_blown(new_form_state, is_blown);
    set_form_state_hit(new_form_state, is_hit);
    set_form_state_selected(new_form_state, is_selected);
    set_form_state_camouflaged(new_form_state, is_camouflaged);   

    list_insert(destroyed_forms, new_form_info);
}

static void selr(SmuTreap t, char *line_buffer, List *selections_list, FILE *txt_file) {
    int n;
    double x, y, w, h;
    int parsed = sscanf(line_buffer, "%*s %d %lf %lf %lf %lf", &n, &x, &y, &w, &h);
    if (parsed != 5) {
        fprintf(stderr, "ERROR: processor_qry selr command requires 5 parameters\n");
        return; 
    }

    if (n < 0 || n >= selection_manager_get_region_count()) {
        fprintf(stderr, "ERROR: processor_qry selr command requires a valid selection index (0-%d)\n", selection_manager_get_region_count()-1);
        return; 
    }

    if (selections_list[n] == NULL) {
        selections_list[n] = new_list();
    }

    double x2 = x + w;
    double y2 = y + h;
    fprintf(txt_file, "[*] selr %lf %lf %lf %f\n", x, y, w, h); 

    if (g_selection_manager != NULL) {
        bool region_exists = false;
        int region_count = selection_manager_get_region_count();
        
        for (int i = 0; i < region_count; i++) {
            if (!selection_manager_is_region_active(g_selection_manager, i)) continue;

            double existing_x, existing_y, existing_w, existing_h;
            selection_manager_get_region_data(g_selection_manager, i, &existing_x, &existing_y, &existing_w, &existing_h);
            
            if (existing_x == x && existing_y == y && existing_w == w && existing_h == h) {
                region_exists = true;
                break;
            }
        }
        
        if (!region_exists) {
            selection_manager_add_region(g_selection_manager, x, y, w, h);
        }
    }

    getInfosDentroRegiaoSmuT(t, x, y, x2, y2, &is_form_inside_region, selections_list[n]);
    list_foreach(selections_list[n], &print_selr_selection_callback, txt_file);
}

static void seli(SmuTreap t, char *line_buffer, List *selections_list, FILE *txt_file) {
    int n; 
    double x, y; 
    int parsed = sscanf(line_buffer, "%*s %d %lf %lf", &n, &x, &y);
    if (parsed != 3) {
        fprintf(stderr, "ERROR: processor_qry seli command requires 3 parameters\n");
        return; 
    }

    if (n < 0 || n >= selection_manager_get_region_count()) {
        fprintf(stderr, "ERROR: processor_qry seli command requires a valid selection index (0-%d)\n", selection_manager_get_region_count()-1);
        return; 
    }

    fprintf(txt_file, "[*] seli %lf %lf\n", x, y); 
    Node found_node = getNodeSmuT(t, x, y); 
    if (found_node == NULL) {
        fprintf(stderr, "ERROR: processor_qry seli command did not find a form at the given coordinates (%lf/%lf)\n", x, y);
        return; 
    }

    Info node_form = getInfoSmuT(NULL, found_node);
    DescritorTipoInfo node_form_type = getTypeInfoSmuT(NULL, found_node);
    int form_id = get_form_id(node_form_type, node_form); 
    fprintf(txt_file, "\t%d: %s\n",form_id, get_form_name(node_form_type)); 

    if (selections_list[n] == NULL) {
        selections_list[n] = new_list();
    }

    if (g_selection_manager != NULL) {
        selection_manager_set_region_data(g_selection_manager, n, x, y, 1, 1);
    }

    list_insert(selections_list[n], found_node);
}

struct cln_data {
    int max_id;
    SmuTreap tree;
    double target_x;
    double target_y;
};

static bool find_node_by_id(SmuTreap t, Node n, Info i, double x, double y, void *aux) {
    (void)x; 
    (void)y; 
    int id = get_form_id(getTypeInfoSmuT(t, n), i); 
    int *target_id = (int *) aux;
    if (id == *target_id) {
        return true; 
    }

    return false;
}

static void get_max_tree_id(SmuTreap t, Node node, Info i, double x, double y, void *aux) {
    (void)t;
    (void)i; 
    (void)x;
    (void)y; 

    struct cln_data *data = (struct cln_data *) aux;
    if (data == NULL || node == NULL) {
        fprintf(stderr, "ERROR: processor_qry get_max_info_id requires valid data and node\n");
        return; 
    }

    int id = get_form_id(getTypeInfoSmuT(data->tree, node), getInfoSmuT(data->tree, node));
    if (id > (data->max_id)) {
        (data->max_id) = id;
    }
}

void cln_helper(void *value, callback_data call_data) {
    struct cln_data *data = (struct cln_data *) call_data;
    Node node = (Node) value;
    if (data->tree == NULL || node == NULL) {
        fprintf(stderr, "ERROR: processor_qry cln command requires a valid treap and node\n");
        return; 
    }

    DescritorTipoInfo form_type = getTypeInfoSmuT(data->tree, node);
    Info form_info = getInfoSmuT(data->tree, node);
    if (form_type == -1 || form_info == NULL) {
        fprintf(stderr, "ERROR: processor_qry cln command requires valid form type and info\n");
        return; 
    }
    
    double new_x, new_y;
    get_form_coordinates(form_type, form_info, &new_x, &new_y);

    new_x += data->target_x; 
    new_y += data->target_y; 

    FormInfo cloned_form_info = clone_form(form_type, form_info, ++data->max_id, new_x, new_y);
    if (cloned_form_info == NULL) {
        fprintf(stderr, "ERROR: processor_qry cln command failed to clone form\n");
        return;
    }
    
    Info cloned_form = get_form_info(cloned_form_info);
    insertSmuT(data->tree, new_x, new_y, cloned_form, form_type, &get_form_bounding_box);
    
    free_form_info_wrapper_only(cloned_form_info);
}

static void cln(SmuTreap t, char *line_buffer, List *selections_list) {
    int n;
    double x, y;
    int parsed = sscanf(line_buffer, "%*s %d %lf %lf",  &n, &x, &y);
    if (parsed != 3) {
        fprintf(stderr, "ERROR: processor_qry cln command requires 3 parameters\n");
        return; 
    }
    
    if (n < 0 || n >= selection_manager_get_region_count()) {
        fprintf(stderr, "ERROR: processor_qry cln command requires a valid selection index (0-%d)\n", selection_manager_get_region_count()-1);
        return; 
    }
    
    if (selections_list[n] == NULL) {
        fprintf(stderr, "ERROR: processor_qry cln command requires a valid selection (selection %d is empty)\n", n);
        return;
    }
    
    List selection = selections_list[n];
    if (selection == NULL) {
        fprintf(stderr, "ERROR: processor_qry cln command requires a valid selection (selection %d is NULL)\n", n);
        return; 
    }

    struct cln_data data = {.max_id = 0, .tree = t, .target_x = x, .target_y = y};
    visitaProfundidadeSmuT(t, &get_max_tree_id, &data); // lookup for the maximum id in the tree
    list_foreach(selection, &cln_helper, &data); // clone the forms from the given selection
}

static void transp(SmuTreap t, char *line_buffer, FILE *txt_file) {
    int id; 
    double x, y;
    int parsed = sscanf(line_buffer, "%*s %d %lf %lf", &id, &x, &y);
    if (parsed != 3) {
        fprintf(stderr, "ERROR: processor_qry transp command requires 3 parameters\n");
        return; 
    }
    fprintf(txt_file, "[*] transp %d %lf %lf\n", id, x, y); 

    Node found_node = procuraNoSmuT(t, &find_node_by_id, &id);
    if (found_node == NULL) {
        fprintf(stderr, "ERROR: processor_qry transp command did not find a form with id %d\n", id);
        return; 
    }
    DescritorTipoInfo form_type = getTypeInfoSmuT(t, found_node);
    Info form_info = getInfoSmuT(t, found_node);
    if (form_type == -1 || form_info == NULL) {
        fprintf(stderr, "ERROR: processor_qry transp command requires valid form type and info\n");
        return; 
    }

    double org_x, org_y;
    get_form_coordinates(form_type, form_info, &org_x, &org_y);

    fprintf(txt_file, "\t%s: pos original (%lf, %lf)\n", get_form_name(form_type), org_x, org_y); 

    transp_form(form_type, form_info,x, y);
}

static void cmflg(SmuTreap t, char *line_buffer, FILE *txt_file) {
    int id; 
    char cb[10] = {0}, cp[10] = {0}, w[10] = {0};
    int parsed = sscanf(line_buffer, "%*s %d %s %s %s", &id, cb, cp, w);
    if (parsed != 4) {
        fprintf(stderr, "ERROR: processor_qry cmflg command requires 3 parameters\n");
        return; 
    }
    
    fprintf(txt_file, "[*] cmflg %d %s %s %s\n", id, cb, cp, w); 
    Node found_node = procuraNoSmuT(t, &find_node_by_id, &id);
    if (found_node == NULL) {
        fprintf(stderr, "ERROR: processor_qry cmflg command did not find a form with id %d\n", id);
        return; 
    }
    DescritorTipoInfo form_type = getTypeInfoSmuT(t, found_node);
    Info form_info = getInfoSmuT(t, found_node);
    if (form_type == -1 || form_info == NULL) {
        fprintf(stderr, "ERROR: processor_qry cmflg command requires valid form type and info\n");
        return; 
    }
    
    fprintf(txt_file, "\t %s: camuflado(a)\n", get_form_name(form_type)); 

    FormStyle style = get_form_style(form_type, form_info);
    set_form_border_color(style, cb);
    set_form_fill_color(style, cp);
    set_form_style_stroke_width(style, w);
}

static double distance_between_points(double x1, double y1, double x2, double y2) {
    return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
}

bool is_point_internal_to_form(SmuTreap t, Node n, Info i, double x, double y) {
    DescritorTipoInfo form_type = getTypeInfoSmuT(t, n);
    if (form_type == -1 || i == NULL) {
        fprintf(stderr, "ERROR: processor_qry is_point_internal_to_form requires valid form type and info\n");
        return false; 
    }

    double form_x, form_y, form_w, form_h;
    get_form_coordinates(form_type, i, &form_x, &form_y);
    get_form_dimensions(form_type, i, &form_w, &form_h);

    if (form_type == LINE) {
        double dist_p1 = distance_between_points(x, y, form_x, form_y);
        double dist_p2 = distance_between_points(x, y, form_w, form_h);
        double line_length = distance_between_points(form_x, form_y, form_w, form_h);

        return (fabs(dist_p1 + dist_p2) - line_length) < EPSILON;
    }

    bool is_internal = (x >= form_x && x <= form_x + form_w) && (y >= form_y && y <= form_y + form_h);
    return is_internal;
}

bool is_point_at_the_same_coordinate(SmuTreap t, Node n, Info i, double x, double y) {
    DescritorTipoInfo form_type = getTypeInfoSmuT(t, n);
    if (form_type == -1 || i == NULL) {
        fprintf(stderr, "ERROR: processor_qry is_point_internal_to_form requires valid form type and info\n");
        return false; 
    }

    double form_x, form_y;
    get_form_coordinates(form_type, i, &form_x, &form_y);

    return (fabs(x - form_x) < EPSILON) && (fabs(y - form_y) < EPSILON); 
}

static void set_form_as_hit(void *value, callback_data call_data) {
    Node node = (Node) value;
    SmuTreap t = (SmuTreap) call_data;
    if (node == NULL || t == NULL) {
        fprintf(stderr, "ERROR: processor_qry set_form_as_hit requires valid node and treap\n");
        return; 
    }

    DescritorTipoInfo form_type = getTypeInfoSmuT(t, node);
    Info form_info = getInfoSmuT(t, node);
    if (form_type == -1 || form_info == NULL) {
        fprintf(stderr, "ERROR: processor_qry set_form_as_hit requires valid form type and info\n");
        return; 
    }

    FormState state = get_form_state(form_type, form_info);
    set_form_state_hit(state, true);
}

static void report_hit_nodes(void *value, callback_data call_data) {
    assert(value); 
    assert(call_data); 

    Node node = (Node) value; 
    FILE *txt_file = (FILE *) call_data; 
    Info form = getInfoSmuT(NULL, node);
    DescritorTipoInfo form_type = getTypeInfoSmuT(NULL, node); 
    fprintf(txt_file, "\t%s (%d) -> foi alvo da operação\n", get_form_name(form_type), get_form_id(form_type, form)); 
}

struct blow_context {
    Node exploding_node;
    SmuTreap tree;
    FILE *txt_file;
    List destroyed_forms;
};

static void process_hit_node_except_exploding(void *value, callback_data call_data) {
    Node node = (Node) value;
    struct blow_context *context = (struct blow_context *) call_data;
    
    if (node == context->exploding_node) {
        return;
    }
    
    set_form_as_hit(node, context->tree);
    report_hit_nodes(node, context->txt_file);
    save_node_forms_to_export(context->tree, node, context->destroyed_forms);
    removeNoSmuT(context->tree, node);
}

static void blow(SmuTreap t, char *line_buffer, FILE *txt_file, List destroyed_forms) {
    int id; 
    int parsed = sscanf(line_buffer, "%*s %d", &id);
    if (parsed != 1) {
        fprintf(stderr, "ERROR: processor_qry blow command requires 1 parameter\n");
        return; 
    }
    fprintf(txt_file, "[*] blow %d\n", id); 

    Node found_node = procuraNoSmuT(t, &find_node_by_id, &id);
    if (found_node == NULL) {
        fprintf(stderr, "ERROR: processor_qry blow command did not find a form with id %d\n", id);
        return; 
    }
    
    DescritorTipoInfo form_type = getTypeInfoSmuT(t, found_node);
    Info form_info = getInfoSmuT(t, found_node);
    if (form_type == -1 || form_info == NULL) {
        fprintf(stderr, "ERROR: processor_qry blow command requires valid form type and info\n");
        return; 
    }
    fprintf(txt_file, "\t%s -> foi explodido\n", get_form_name(form_type)); 

    double x, y;
    get_form_coordinates(form_type, form_info, &x, &y);
    FormState state = get_form_state(form_type, form_info);
    set_form_state_blown(state, true);

    List hit_nodes = new_list();
    bool is_hit_nodes = getInfosAtingidoPontoSmuT(t, x, y, &is_point_internal_to_form, hit_nodes);
    if (is_hit_nodes) {
        struct blow_context context = {found_node, t, txt_file, destroyed_forms};
        list_foreach(hit_nodes, &process_hit_node_except_exploding, &context);
    }

    save_node_forms_to_export(t, found_node, destroyed_forms);

    removeNoSmuT(t, found_node);
    list_free(hit_nodes, NULL);
}

static void calc_disp_final_point(double startX, double startY, double lineX1, double lineY1, 
    double lineX2, double lineY2, double distance, double *endX, double *endY) {

    double dx = lineX2 - lineX1;
    double dy = lineY2 - lineY1;

    double vector_length = sqrt(dx * dx + dy * dy);

    if (vector_length == 0.0) {
        *endX = startX;
        *endY = startY;
        return;
    }

    double movX = (dx / vector_length) * distance;
    double movY = (dy / vector_length) * distance;

    *endX = startX + movX;
    *endY = startY + movY;
}

struct disp_context {
    Info line_info;
    SmuTreap tree;
    FILE *txt_file;
    List destroyed_forms;
};

static void process_displaced_node(void *value, callback_data call_data) {
    Node node = (Node) value;
    struct disp_context *context = (struct disp_context *) call_data;
    
    DescritorTipoInfo form_type = getTypeInfoSmuT(context->tree, node);
    Info form_info = getInfoSmuT(context->tree, node);
    FormState state = get_form_state(form_type, form_info);
    if (form_type == -1 || form_info == NULL || state == NULL) {
        fprintf(stderr, "ERROR: processor_qry process_displaced_node requires valid form type and info\n");
        return; 
    }

    double x, y;
    get_form_coordinates(form_type, form_info, &x, &y);

    double lineX1, lineY1, lineX2, lineY2;
    get_form_coordinates(LINE, context->line_info, &lineX1, &lineY1);
    get_form_dimensions(LINE, context->line_info, &lineX2, &lineY2);

    double distance = get_form_distance_disp(form_type, form_info);
    if (distance <= 0) {
        fprintf(stderr, "ERROR: processor_qry disp command requires a positive distance for displacement\n");
        return; 
    }

    double endX, endY;
    calc_disp_final_point(x, y, lineX1, lineY1, lineX2, lineY2, distance, &endX, &endY);
    
    // Deslocar a forma para a nova posição
    transp_form(form_type, form_info, endX, endY);
    set_form_state_blown(state, true);

    // Encontrar todos os nós atingidos na nova posição
    List hit_nodes = new_list();
    bool is_hit_nodes = getInfosAtingidoPontoSmuT(context->tree, endX, endY, &is_point_internal_to_form, hit_nodes);
    
    if (is_hit_nodes) {
        // Processar cada nó atingido (exceto o próprio nó deslocado) - usando o mesmo padrão de blow
        struct blow_context hit_context = {node, context->tree, context->txt_file, context->destroyed_forms};
        list_foreach(hit_nodes, &process_hit_node_except_exploding, &hit_context);
    }
    
    list_free(hit_nodes, NULL);
}

static void disp(SmuTreap t, char *line_buffer, List *selections_list, FILE *txt_file, List destroyed_forms) {
    int id, n; 
    int parsed = sscanf(line_buffer, "%*s %d %d", &id, &n);
    if (parsed != 2) {
        fprintf(stderr, "ERROR: processor_qry disp command requires 2 parameters\n");
        return; 
    }

    fprintf(txt_file, "[*] disp %d %d\n", id, n); 

    Node found_node = procuraNoSmuT(t, &find_node_by_id, &id);
    if (found_node == NULL) {
        fprintf(stderr, "ERROR: processor_qry disp command did not find a form with id %d\n", id);
        return; 
    }

    DescritorTipoInfo form_type = getTypeInfoSmuT(t, found_node);
    if (form_type != LINE) {
        fprintf(stderr, "ERROR: processor_qry disp command requires a LINE to direct the forms\n");
        return; 
    }

    Info form_info = getInfoSmuT(t, found_node);
    if (form_info == NULL) {
        fprintf(stderr, "ERROR: processor_qry disp command requires valid form info\n");
        return; 
    }

    if (n < 0 || n >= selection_manager_get_region_count()) {
        fprintf(stderr, "ERROR: processor_qry disp command requires a valid selection index (0-%d)\n", selection_manager_get_region_count()-1);
        return; 
    }
    
    if (selections_list[n] == NULL) {
        fprintf(stderr, "ERROR: processor_qry disp command requires a valid selection (selection %d is empty)\n", n);
        return;
    }
    
    List selection = selections_list[n];
    
    // Processar cada nó da seleção seguindo o padrão de blow
    struct disp_context context = {form_info, t, txt_file, destroyed_forms};
    list_foreach(selection, &process_displaced_node, &context);
}
static void spy(SmuTreap t, char *line_buffer, FILE *txt_file) {
    int id; 
    int parsed = sscanf(line_buffer, "%*s %d", &id);
    if (parsed != 1) {
        fprintf(stderr, "ERROR: processor_qry spy command requires 1 parameter\n");
        return; 
    }
    fprintf(txt_file, "[*] spy %d\n", id); 

    Node found_node = procuraNoSmuT(t, &find_node_by_id, &id);
    if (found_node == NULL) {
        fprintf(stderr, "ERROR: processor_qry spy command did not find a form with id %d\n", id);
        return; 
    }
    
    DescritorTipoInfo form_type = getTypeInfoSmuT(t, found_node);
    Info form_info = getInfoSmuT(t, found_node);
    if (form_type == -1 || form_info == NULL) {
        fprintf(stderr, "ERROR: processor_qry spy command requires valid form type and info\n");
        return; 
    }

    List forms_selected = new_list(); 

    switch (form_type) {
        case TEXT: {
            double x, y; 
            get_form_coordinates(form_type, form_info, &x, &y); 
            getNodesDentroRegiaoSmuT(t, x-EPSILON, y-EPSILON, x+EPSILON, y+EPSILON, forms_selected);
            break;
        } 
        case RECT: {
            double x, y, w, h; 
            get_form_coordinates(form_type, form_info, &x, &y); 
            get_form_dimensions(form_type, form_info, &w, &h); 

            getInfosDentroRegiaoSmuT(t, x, y, w, h, &is_form_inside_region, forms_selected); 
            break; 
        }
        default:
            fprintf(stderr, "ERROR: spy requires a line or rect element got: %s\n", get_form_name(form_type)); 
        break; 
    }
    list_foreach(forms_selected, &report_hit_nodes, txt_file); 
    list_free(forms_selected, NULL); 
}

static void qry_execute(FILE *qry_file, FILE *txt_file, SmuTreap smu_treap, List destroyed_forms) {
    assert(qry_file);

    char line_buffer[MAX_LINE_LENGTH]; 
    char command_type[10]; 
    List selections_list[MAX_SELECTION_REGIONS]; 
    
    for (int i = 0; i < MAX_SELECTION_REGIONS; i++) {
        selections_list[i] = NULL;
    }

    while (fgets(line_buffer, sizeof(line_buffer), qry_file) != NULL) {
        if (process_command(line_buffer, command_type) == false) continue; 

        if (strcmp(command_type, "selr") == 0) {
            selr(smu_treap, line_buffer, selections_list, txt_file);
        } else if (strcmp(command_type, "seli") == 0) {
            seli(smu_treap, line_buffer, selections_list, txt_file);
        } else if (strcmp(command_type, "cln") == 0) {
            cln(smu_treap, line_buffer, selections_list);
        } else if (strcmp(command_type, "transp") == 0) {
            transp(smu_treap, line_buffer, txt_file);
        } else if (strcmp(command_type, "cmflg") == 0) {
            cmflg(smu_treap, line_buffer, txt_file);
        } else if (strcmp(command_type, "blow") == 0) {
            blow(smu_treap, line_buffer, txt_file, destroyed_forms);
        } else if (strcmp(command_type, "disp") == 0) {
            disp(smu_treap, line_buffer, selections_list, txt_file, destroyed_forms);
        } else if (strcmp(command_type, "spy") == 0) {
            spy(smu_treap, line_buffer, txt_file);
        } else {
            fprintf(stderr, "ERROR: processor_qry unknown command type '%s'\n", command_type);
        }
    }

    for (int i = 0; i < MAX_SELECTION_REGIONS; i++) {
        if (selections_list[i] == NULL) continue; 
        list_free(selections_list[i], NULL);
    }
}

void qry_process(Dir qry, Dir txt, SmuTreap smu_treap, SelectionManager selection_manager, List destroyed_forms) {
    char *file_extension = get_dir_file_extension(qry);
    if (strcmp(file_extension, "qry") != 0) {
        fprintf(stderr, "ERROR: processor_qry requires a .qry file extension\n"); 
        return; 
    }

    if (txt == NULL) {
        fprintf(stderr, "ERROR: processor_qry requires a .txt file to reports\n"); 
        return; 
    }

    FILE *qry_file = file_open_readable(qry);
    if (qry_file == NULL) {
        fprintf(stderr, "ERROR: processor_qry couldn't open the .qry file\n"); 
        return; 
    }
    
    FILE *txt_file = file_open_writable(txt);
    if (txt_file == NULL) {
        fprintf(stderr, "ERROR: processor_qry couldn't open the .txt file\n"); 
        file_close(qry_file);
        return; 
    }

    if (destroyed_forms == NULL) {
        fprintf(stderr, "ERROR: processor_qry requires a list for the destroyed forms");
        return; 
    }

    g_selection_manager = selection_manager;
    if (selection_manager != NULL) {
        for (int i = 0; i < selection_manager_get_region_count(); i++) {
            selection_manager_set_region_active(g_selection_manager, i, false);
        }
    }

    qry_execute(qry_file, txt_file, smu_treap, destroyed_forms);
    file_close(qry_file);
    file_close(txt_file);
    
    g_selection_manager = NULL;
}