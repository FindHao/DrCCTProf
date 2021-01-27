/*
 *  Copyright (c) 2020 Xuhpclab. All rights reserved.
 *  Licensed under the MIT License.
 *  See LICENSE file for more information.
 */
#include "dr_api.h"
#include "drwrap.h"
#include "drutil.h"
#include "drsyms.h"
#include "drmgr.h"

#include "drcctlib.h"
#include <sanitizer_callbacks.h>
#include <iostream>

#define DRCCTLIB_PRINTF(format, args...) \
    DRCCTLIB_PRINTF_TEMPLATE("all_instr_cct_no_cache", format, ##args)
#define DRCCTLIB_EXIT_PROCESS(format, args...) \
    DRCCTLIB_CLIENT_EXIT_PROCESS_TEMPLATE("all_instr_cct_no_cache", format, ##args)

#include "gpupunk_api.h"

using std::cout;
using std::endl;
using namespace gpupunk;

extern LockableMap <uint64_t, MemoryMap> memory_snapshot;

static void
ClientInit(int argc, const char *argv[]) {
}

static void
ClientExit(void) {
    drcctlib_exit();
}

static inline app_pc
moudle_get_function_entry(const module_data_t *info, const char *func_name,
                          bool check_internal_func) {

    app_pc functionEntry;
    if (check_internal_func) {
        size_t offs;
        if (drsym_lookup_symbol(info->full_path, func_name, &offs, DRSYM_DEMANGLE) ==
            DRSYM_SUCCESS) {
            functionEntry = offs + info->start;
        } else {
            functionEntry = NULL;
        }
    } else {
        functionEntry = (app_pc) dr_get_proc_address(info->handle, func_name);
    }
    return functionEntry;
}


static void
gpupunk_memory_register_callback(void *wrapcxt, void **user_data) {

    void *drcontext = (void *) drwrap_get_drcontext(wrapcxt);

    auto md = (Sanitizer_ResourceMemoryData *) drwrap_get_arg(wrapcxt, 0);
    gp_result_t result = gpupunk_memory_register(1, 2, md->address, md->address + md->size);
    if (result == GPUPUNK_ERROR_DUPLICATE_ENTRY){
    }else if (result == GPUPUNK_ERROR_NOT_EXIST_ENTRY){
    }

    // dmem_alloc_size += MEM_RED_ZONE;
    // drwrap_set_arg(wrapcxt, 0, (void *)dmem_alloc_size);
}

static void
gpupunk_memory_unregister_callback(void *wrapcxt, void **user_data) {

    void *drcontext = (void *) drwrap_get_drcontext(wrapcxt);

    auto md = (Sanitizer_ResourceMemoryData *) drwrap_get_arg(wrapcxt, 0);
    // dmem_alloc_size += MEM_RED_ZONE;
    // drwrap_set_arg(wrapcxt, 0, (void *)dmem_alloc_size);
}

static void
RegisteBeforeWrapFunc(void *drcontext, const module_data_t *info, bool loaded) {
    cout << info->full_path << endl;
    app_pc gmrt = moudle_get_function_entry(info, "gpupunk_memory_register_trigger", true);
    if (gmrt != NULL) {
        drwrap_wrap(gmrt, gpupunk_memory_register_callback, NULL);
    }
    app_pc gmut = moudle_get_function_entry(info, "gpupunk_memory_unregister_trigger", true);
    if (gmut != NULL) {
        drwrap_wrap(gmut, gpupunk_memory_unregister_callback, NULL);
    }

}

 static void
 RegisteAfterWrapFunc(void *drcontext, const module_data_t *info, bool loaded)
 {
     cout << info->full_path << endl;
     app_pc gmrt = moudle_get_function_entry(info, "gpupunk_memory_register_trigger", true);
     if (gmrt != NULL) {
         drwrap_wrap(gmrt, gpupunk_memory_register_callback, NULL);
     }
     app_pc gmut = moudle_get_function_entry(info, "gpupunk_memory_unregister_trigger", true);
     if (gmut != NULL) {
         drwrap_wrap(gmut, gpupunk_memory_unregister_callback, NULL);
     }
 }

#ifdef __cplusplus
extern "C" {
#endif

DR_EXPORT void
dr_client_main(client_id_t id, int argc, const char *argv[]) {
    dr_set_client_name("DynamoRIO Client 'drcctlib_all_instr_cct_no_cache'",
                       "http://dynamorio.org/issues");
    ClientInit(argc, argv);
    drcctlib_init_ex(DRCCTLIB_FILTER_ALL_INSTR, INVALID_FILE, NULL, NULL, NULL,
                     DRCCTLIB_DEFAULT);

    drmgr_priority_t before_drcctlib_module_load = {sizeof(before_drcctlib_module_load),
                                                    "before_drcctlib_module_load", NULL,
                                                    NULL,
                                                    DRCCTLIB_MODULE_REGISTER_PRI + 1};
    drmgr_priority_t after_drcctlib_module_load = {sizeof(after_drcctlib_module_load),
                                                   "after_drcctlib_module_load", NULL,
                                                   NULL,
                                                   DRCCTLIB_MODULE_REGISTER_PRI - 1};
//    drmgr_register_module_load_event_ex(RegisteBeforeWrapFunc,
//                                        &before_drcctlib_module_load);
    drmgr_register_module_load_event_ex(RegisteAfterWrapFunc,
                                        &after_drcctlib_module_load);
    dr_register_exit_event(ClientExit);
}

#ifdef __cplusplus
}
#endif