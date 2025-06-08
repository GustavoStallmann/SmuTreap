#ifndef PROCESSOR_QRY_H
#define PROCESSOR_QRY_H

#include "processor_dir.h"
#include "smu_treap.h"
#include "form_selection.h"
/* 
 * Process the query file and perform their operations
 * @param qry the qry file directory
 * @param txt the log txt file directory
 * @param smu_treap the treap tree to perform the operations
 * @param selection_manager the manager of treap selections 
 * @param destroyed_forms the list of the destroyed forms
*/
void qry_process(Dir qry, Dir txt, SmuTreap smu_treap, SelectionManager selection_manager, List destroyed_forms);

#endif