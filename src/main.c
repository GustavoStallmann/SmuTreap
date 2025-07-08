#include <stdio.h>
#include <stdlib.h>
#include "list.h"
#include "processor_args.h"
#include "file.h"
#include "file_svg.h"
#include "processor_form.h"
#include "smu_treap.h"
#include "form.h"
#include "processor_dir.h"
#include "processor_geo.h"
#include "processor_qry.h"

typedef struct {
    char *base_input_dir;
    char *base_output_dir;
    char *geo_file;
    char *qry_file;
    int max_priority;
    int hit_count;
    double promotion_rate;
} ProgramConfig;

static ArgManager check_args(int argc, char *argv[]); 
static ProgramConfig extract_config_from_args(ArgManager argm);

static SmuTreap process_geo_file_and_create_treap(const ProgramConfig *config);
static void process_treap_operation(const ProgramConfig *config, SmuTreap smu_treap);

static void export_initial_files(const ProgramConfig *config, SmuTreap smu_treap);
static void export_query_results(const ProgramConfig *config, SmuTreap smu_treap, SelectionManager selection_manager, List destroyed_forms) ;

static void callback_insert_on_smu_treap(void *value, callback_data call_data); 
static void export_form_svg(SmuTreap t, Node n, Info info, double x, double y, void *aux);
static void combine_file_names(char *str1, char *str2, char *file_extension, char *result, int size);
static void export_svg_treap(SmuTreap smu_treap, char *base_output_dir, char *filename, SelectionManager selection_manager, List destroyed_forms);
static void export_dot_treap(SmuTreap smu_treap, char *base_output_dir, char *geo_name, char *qry_name);

int main(int argc, char *argv[]) {
    ArgManager argm = check_args(argc, argv);
    ProgramConfig config = extract_config_from_args(argm);
    
    SmuTreap smu_treap = process_geo_file_and_create_treap(&config);
    export_initial_files(&config, smu_treap);
    
    if (config.qry_file != NULL) {
        process_treap_operation(&config, smu_treap);
    } else {
        fprintf(stderr, "WARNING: No query file provided, skipping query processing.\n");
    }
    
    killSmuTreap(smu_treap); 
    free_arg_manager(argm);
    return 0;
}

static ProgramConfig extract_config_from_args(ArgManager argm) {
    ProgramConfig config;
    
    get_arg_value_by_particle(argm, "-e", &config.base_input_dir);
    get_arg_value_by_particle(argm, "-o", &config.base_output_dir);
    get_arg_value_by_particle(argm, "-f", &config.geo_file);
    get_arg_value_by_particle(argm, "-q", &config.qry_file);
    
    get_arg_value_by_particle(argm, "-p", &config.max_priority);
    get_arg_value_by_particle(argm, "-hc", &config.hit_count);
    get_arg_value_by_particle(argm, "-pr", &config.promotion_rate);
    
    return config;
}

static SmuTreap process_geo_file_and_create_treap(const ProgramConfig *config) {
    Dir geo_dir = dir_combine_path_and_file(config->base_input_dir, config->geo_file);
    List forms_list = geo_process(geo_dir);
    if (forms_list == NULL) {
        fprintf(stderr, "ERROR: Couldn't process the geo file\n");
        dir_free(geo_dir);
        exit(EXIT_FAILURE);
    }
    
    SmuTreap smu_treap = newSmuTreap(config->hit_count, config->promotion_rate, EPSILON, config->max_priority);
    list_foreach(forms_list, &callback_insert_on_smu_treap, smu_treap);
    
    list_free(forms_list, &free_form_info_wrapper_only);
    dir_free(geo_dir);
    
    return smu_treap;
}

static void export_initial_files(const ProgramConfig *config, SmuTreap smu_treap) {
    Dir geo_dir = dir_combine_path_and_file(config->base_input_dir, config->geo_file);
    char *geo_name = get_dir_file_name(geo_dir);
    
    export_dot_treap(smu_treap, config->base_output_dir, geo_name, NULL);
    
    char svg_file_name[50];
    combine_file_names(geo_name, NULL, "svg", svg_file_name, sizeof(svg_file_name));
    export_svg_treap(smu_treap, config->base_output_dir, svg_file_name, NULL, NULL);
    
    dir_free(geo_dir);
}

static void process_treap_operation(const ProgramConfig *config, SmuTreap smu_treap) {
    Dir qry_dir = dir_combine_path_and_file(config->base_input_dir, config->qry_file);
    Dir geo_dir = dir_combine_path_and_file(config->base_input_dir, config->geo_file);

    char txt_file_name[50]; 
    combine_file_names(get_dir_file_name(geo_dir), get_dir_file_name(qry_dir), "txt", txt_file_name, sizeof(txt_file_name));
    Dir txt_dir = dir_combine_path_and_file(config->base_output_dir, txt_file_name);
    
    SelectionManager selection_manager = new_selection_manager();
    List destroyed_forms = new_list(); 
    
    qry_process(qry_dir, txt_dir, smu_treap, selection_manager, destroyed_forms);
    export_query_results(config, smu_treap, selection_manager, destroyed_forms);
    
    list_free(destroyed_forms, &free_form_info);
    selection_manager_free(selection_manager);
    dir_free(qry_dir);
    dir_free(geo_dir); 
    dir_free(txt_dir);
}

static void export_query_results(const ProgramConfig *config, SmuTreap smu_treap, SelectionManager selection_manager, List destroyed_forms) {
    Dir geo_dir = dir_combine_path_and_file(config->base_input_dir, config->geo_file);
    Dir qry_dir = dir_combine_path_and_file(config->base_input_dir, config->qry_file);
    
    char *geo_name = get_dir_file_name(geo_dir);
    char *qry_name = get_dir_file_name(qry_dir);
    
    export_dot_treap(smu_treap, config->base_output_dir, geo_name, qry_name);
    
    char svg_qry_file_name[50];
    combine_file_names(geo_name, qry_name, "svg", svg_qry_file_name, sizeof(svg_qry_file_name));
    export_svg_treap(smu_treap, config->base_output_dir, svg_qry_file_name, selection_manager, destroyed_forms);
    
    dir_free(geo_dir);
    dir_free(qry_dir);
}

static void export_svg_destroyed_forms(void *value, callback_data call_data) {
    FormInfo form_info = (FormInfo) value; 
    FILE *svg_file = (FILE *) call_data; 

    Info form = get_form_info(form_info);
    DescritorTipoInfo form_type = get_form_type(form_info);

    svg_export_form(svg_file, form, form_type);
}

static void export_svg_treap(SmuTreap smu_treap, char *base_output_dir, char *filename, SelectionManager selection_manager, List destroyed_forms) {
    Dir svg_dir = dir_combine_path_and_file(base_output_dir, filename);
    FILE *svg_file = file_open_writable(svg_dir);
    dir_free(svg_dir);
    
    svg_init(svg_file, 1920, 1080);
    visitaProfundidadeSmuT(smu_treap, &export_form_svg, svg_file);

    if (destroyed_forms != NULL) {
        list_foreach(destroyed_forms, &export_svg_destroyed_forms, svg_file);
    }
    
    if (selection_manager != NULL) {
        svg_export_selection_regions(svg_file, selection_manager);
    }
    
    svg_close(svg_file);
    file_close(svg_file);
}

static void combine_file_names(char *str1, char *str2, char *file_extension, char *result, int size) {
    if (str2 != NULL) {
        snprintf(result, size, "%s-%s.%s", str1, str2, file_extension);
        return; 
    }
    snprintf(result, size, "%s.%s", str1, file_extension);
}

static ArgManager check_args(int argc, char *argv[]) {
    ArgManager argm = new_arg_manager();
    
    add_new_arg(argm, "-e", false, "Diretório-base de entrada (BED)", STR, "./"); 
    add_new_arg(argm, "-q", false, "Arquivo com consultas (QRY)", STR, NULL); 
    add_new_arg(argm, "-p", false, "Prioridade máxima (número inteiro)", INT, "10000"); 
    add_new_arg(argm, "-hc", false, "Parâmetro hit count usado na árvore (número inteiro)", INT, "3"); 
    add_new_arg(argm, "-pr", false, "Fator de promoção a ser usado quando hit count é atingido (número decimal)", DOUBLE, "1.10"); 
    add_new_arg(argm, "-o", true, "Diretório-base de saída", STR, "./"); 
    add_new_arg(argm, "-f", true, "Arquivo com a descrição da treap (GEO)", STR, NULL); 
    
    bool status = verify_args(argm, argc, argv); 
    if (!status) {
        free_arg_manager(argm);
        exit(EXIT_FAILURE);
    }

    return argm; 
}

static void callback_insert_on_smu_treap(void *value, callback_data call_data) {
    SmuTreap smu_treap = (SmuTreap) call_data;
    FormInfo form_info = (FormInfo) value;
    
    DescritorTipoInfo form_type = get_form_type(form_info);
    Info form = get_form_info(form_info);

    double x, y; 
    get_form_coordinates(form_type, form, &x, &y);

    insertSmuT(smu_treap, x, y, form, form_type, &get_form_bounding_box);
}

static void export_form_svg(SmuTreap t, Node n, Info info, double x, double y, void *aux) {
    (void)x; 
    (void)y; 
    (void)t; 
    (void)n; 
    
    FILE *svg_file = (FILE *) aux;
    DescritorTipoInfo info_type = getTypeInfoSmuT(t, n);

    svg_export_form(svg_file, info, info_type);
}

static void export_dot_treap(SmuTreap smu_treap, char *base_output_dir, char *geo_name, char *qry_name) {
    char dot_file_name[500], dot_full_path[500];
    
    combine_file_names(geo_name, qry_name, "dot", dot_file_name, sizeof(dot_file_name));
    
    Dir dot_dir = dir_combine_path_and_file(base_output_dir, dot_file_name);
    get_full_dir(dot_dir, dot_full_path);
    printDotSmuTreap(smu_treap, dot_full_path);
    
    dir_free(dot_dir);
}