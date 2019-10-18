/*
   WinAFL - DynamoRIO client (instrumentation) code
   ------------------------------------------------

   Written and maintained by Ivan Fratric <ifratric@google.com>

   Copyright 2016 Google Inc. All Rights Reserved.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

*/

#define _CRT_SECURE_NO_WARNINGS

#define MAP_SIZE 65536

#include "dr_api.h"
#include "drmgr.h"
#include "drx.h"
#include "drreg.h"
#include "drwrap.h"

#include "ini.c"

#ifdef USE_DRSYMS
#include "drsyms.h"
#endif



#include "modules.h"
/*
extern "C" {
#include "modules.h"
}
*/

#include "utils.h"
#include "hashtable.h"
#include "drtable.h"
#include "limits.h"
#include <string.h>
#include <stdlib.h>
#include <windows.h>


// CString
#include <atlstr.h>

// error : unable to load client library :winafl  Unable to locate imports of client library.
//#include "qstringlist.h"
//#include "qstring.h"

#include "tools.h"

#define UNKNOWN_MODULE_ID USHRT_MAX

#ifndef PF_FASTFAIL_AVAILABLE
#define PF_FASTFAIL_AVAILABLE 23
#endif

#ifndef STATUS_FATAL_APP_EXIT
#define STATUS_FATAL_APP_EXIT ((DWORD)0x40000015L)
#endif

#ifndef STATUS_HEAP_CORRUPTION
#define STATUS_HEAP_CORRUPTION 0xC0000374
#endif

static uint verbose;

#define NOTIFY(level, fmt, ...) do {          \
    if (verbose >= (level))                   \
        dr_fprintf(STDERR, fmt, __VA_ARGS__); \
} while (0)

#define OPTION_MAX_LENGTH MAXIMUM_PATH

#define COVERAGE_BB 0
#define COVERAGE_EDGE 1

//fuzz modes
enum persistence_mode_t { native_mode = 0,	in_app = 1,};

typedef struct _target_module_t {
    char module_name[MAXIMUM_PATH];
    struct _target_module_t *next;
} target_module_t;

typedef struct _winafl_option_t {
    /* Use nudge to notify the process for termination so that
     * event_exit will be called.
     */
    bool nudge_kills;
    bool debug_mode;
	int persistence_mode;
    int coverage_kind;
    char logdir[MAXIMUM_PATH];
    target_module_t *target_modules;
    char fuzz_module[MAXIMUM_PATH];
    char fuzz_method[MAXIMUM_PATH];
    char pipe_name[MAXIMUM_PATH];
    char shm_name[MAXIMUM_PATH];
    unsigned long fuzz_offset;
    int fuzz_iterations;
    void **func_args;
    int num_fuz_args;
    drwrap_callconv_t callconv;
    bool thread_coverage;
    bool no_loop;
	bool dr_persist_cache;

	//MARK
	char testcase_path[MAXIMUM_PATH];
	unsigned int par_id; // parallel id the same as fuzzer_id?
	char interceptor_config_path[MAXIMUM_PATH];
	// true_flag support drrun and afl-fuzz without recompile
	bool true_flag;


} winafl_option_t;
static winafl_option_t options;

#define NUM_THREAD_MODULE_CACHE 4

typedef struct _winafl_data_t {
    module_entry_t *cache[NUM_THREAD_MODULE_CACHE];
    file_t  log;
    unsigned char *fake_afl_area; //used for thread_coverage
    unsigned char *afl_area;
} winafl_data_t;
static winafl_data_t winafl_data;

static int winafl_tls_field;

typedef struct _fuzz_target_t {

	reg_t eax;			// add eight common registers for x86
	reg_t ebx;
	reg_t ecx;
	reg_t edx;
	reg_t esi;
	reg_t edi;
	reg_t ebp;
	reg_t esp;

    reg_t xsp;            /* stack level at entry to the fuzz target */
    app_pc func_pc;
    int iteration;
} fuzz_target_t;
static fuzz_target_t fuzz_target;

typedef struct _debug_data_t {
    int pre_hanlder_called;
    int post_handler_called;
} debug_data_t;
static debug_data_t debug_data;

static module_table_t *module_table;
static client_id_t client_id;
	
static volatile bool go_native;


static int mfc_ui_input_count = 3;
static int mfc_ui_input_index = 0;

static const char *afl_input_path = "c:\\out\\afl.input";
static char *global_afl_input = NULL;
static int global_afl_input_size = 0;

// enable out trace to log file
static bool enable_trace = true;

//npc
static int npc;

static int start_skip = 0;

static void
event_exit(void);

static void
event_thread_exit(void *drcontext);

static HANDLE pipe;

ini_t *interceptor_config;
const char *interceptor_config_path = ".\\interceptor_config.ini";

static int read_afl_input(const char *testcase_path = afl_input_path){
	FILE * fp;

	//if (mfc_ui_input_index > 0) return;

	//fp = fopen(afl_input_path, "r"); // "c:\\out\\afl.input"/
	fp = fopen(testcase_path, "r"); // "c:\\out\\afl.input" or -testcase
	if (!fp) {
		dr_fprintf(winafl_data.log, "Can not open afl-fuzz input file %s\n", testcase_path);
		return 0;
	}
	fseek(fp, 0, SEEK_END);
    global_afl_input_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    global_afl_input = (char *)malloc(global_afl_input_size+1); //TODO: free it
    fread(global_afl_input, global_afl_input_size, 1, fp);
    global_afl_input[global_afl_input_size] = 0;
    fclose(fp);
	return global_afl_input_size;
}

static int get_afl_input_size(const char *testcase_path = afl_input_path){
	FILE * fp;
	int afl_input_length = 0;

	fp = fopen(testcase_path, "r");
	if (!fp) {
		dr_fprintf(winafl_data.log, "Can not open afl-fuzz input file %s\n", testcase_path);
		return 0;
	}
	fseek(fp, 0, SEEK_END);
    afl_input_length = ftell(fp);
    
    fclose(fp);
	return afl_input_length;
}


static void empty_afl_input(){
	FILE * fp;

	global_afl_input = NULL;
	fp = fopen(afl_input_path, "w");
	fclose(fp);

}
/****************************************************************************
 * Nudges
 */

enum {
    NUDGE_TERMINATE_PROCESS = 1,
};




static void
event_nudge(void *drcontext, uint64 argument)
{
    int nudge_arg = (int)argument;
    int exit_arg  = (int)(argument >> 32);
    if (nudge_arg == NUDGE_TERMINATE_PROCESS) {
        static int nudge_term_count;
        /* handle multiple from both NtTerminateProcess and NtTerminateJobObject */
        uint count = dr_atomic_add32_return_sum(&nudge_term_count, 1);
        if (count == 1) {
            dr_exit_process(exit_arg);
        }
    }
    ASSERT(nudge_arg == NUDGE_TERMINATE_PROCESS, "unsupported nudge");
    ASSERT(false, "should not reach"); /* should not reach */
}

static bool
event_soft_kill(process_id_t pid, int exit_code)
{
    /* we pass [exit_code, NUDGE_TERMINATE_PROCESS] to target process */
    dr_config_status_t res;
    res = dr_nudge_client_ex(pid, client_id,
                             NUDGE_TERMINATE_PROCESS | (uint64)exit_code << 32,
                             0);
    if (res == DR_SUCCESS) {
        /* skip syscall since target will terminate itself */
        return true;
    }
    /* else failed b/c target not under DR control or maybe some other
     * error: let syscall go through
     */
    return false;
}

/****************************************************************************
 * Event Callbacks
 */

char ReadCommandFromPipe()
{
	DWORD num_read;
	char result;
	ReadFile(pipe, &result, 1, &num_read, NULL);
	return result;
}

void WriteCommandToPipe(char cmd)
{
	DWORD num_written;
	WriteFile(pipe, &cmd, 1, &num_written, NULL);
}

static void
dump_winafl_data()
{
    dr_write_file(winafl_data.log, winafl_data.afl_area, MAP_SIZE);
}

static bool
onexception(void *drcontext, dr_exception_t *excpt) {
    DWORD exception_code = excpt->record->ExceptionCode;

    if(options.debug_mode)
        dr_fprintf(winafl_data.log, "Exception caught: %x\n", exception_code);

    if((exception_code == EXCEPTION_ACCESS_VIOLATION) ||
       (exception_code == EXCEPTION_ILLEGAL_INSTRUCTION) ||
       (exception_code == EXCEPTION_PRIV_INSTRUCTION) ||
       (exception_code == EXCEPTION_INT_DIVIDE_BY_ZERO) ||
       (exception_code == STATUS_HEAP_CORRUPTION) ||
       (exception_code == EXCEPTION_STACK_OVERFLOW) ||
       (exception_code == STATUS_STACK_BUFFER_OVERRUN) ||
       (exception_code == STATUS_FATAL_APP_EXIT)) {
            if(options.debug_mode) {
                dr_fprintf(winafl_data.log, "crashed\n");
            } else {
				WriteCommandToPipe('C');
            }
            dr_exit_process(1);
    }
    return true;
}

static void event_thread_init(void *drcontext)
{
  void **thread_data;

  thread_data = (void **)dr_thread_alloc(drcontext, 2 * sizeof(void *));
  thread_data[0] = 0;
  if(options.thread_coverage) {
    thread_data[1] = winafl_data.fake_afl_area;
  } else {
    thread_data[1] = winafl_data.afl_area;
  }
  drmgr_set_tls_field(drcontext, winafl_tls_field, thread_data);

  dr_fprintf(winafl_data.log, "event_thread_init");

}

static void event_thread_exit(void *drcontext)
{
  void *data = drmgr_get_tls_field(drcontext, winafl_tls_field);
  dr_thread_free(drcontext, data, 2 * sizeof(void *));
}



static dr_emit_flags_t
instrument_bb_coverage(void *drcontext, void *tag, instrlist_t *bb, instr_t *inst,
                      bool for_trace, bool translating, void *user_data)
{
    static bool debug_information_output = false;
    app_pc start_pc;
    module_entry_t **mod_entry_cache;
    module_entry_t *mod_entry;
    const char *module_name;
    uint offset;
    target_module_t *target_modules;
    bool should_instrument;
    unsigned char *afl_map;
	dr_emit_flags_t ret;
	//int npc;

	start_pc = dr_fragment_app_pc(tag);

	npc = (int)instr_get_app_pc(inst) ;


	if (enable_trace){
		if (npc > 0x400000 && npc < 0x400000 + 1024*1024){
			dr_fprintf(winafl_data.log, "bb 0x%x\n", npc);

			// Cfile:CFile
			/*
			if (npc >= 0x4030e0 && npc < 0x403430){
				if (!start_skip){
					dr_fprintf(winafl_data.log, "start_skip 0x%x\n", npc);
					start_skip = 1;
				}
			}

			if (npc > 0x403430 ||  npc < 0x4030e0){
				
					start_skip = 0;
				
			}
			*/



		}
	}


	/*
	if ( npc > 0x404a60 && npc < 0x405100){
		dr_fprintf(winafl_data.log, "'bb 0x%x\n", npc);

		
		if (npc == 0x404ee1) // second path, only skip parsefile CString and ~CString
			
			start_skip = 1;

		if (npc == 0x404e18) // first path, only skip ~CString in ParseFile
			start_skip = 1;
		



	}*/

	/*
	if (npc >= 0x787E248A && npc < 0x787E2670 )
		dr_fprintf(winafl_data.log, "'CFile::Open bb 0x%x\n", npc);

	if (npc >= 0x7DD74074 && npc < 0x7DD740BF)
		dr_fprintf(winafl_data.log, "'CreateFileW bb 0x%x\n", npc);

	if (npc >= 0x787E2906 && npc <= 0x787E2946)
		dr_fprintf(winafl_data.log, "'CFile::GetLength bb 0x%x\n", npc);
	*/


    if (!drmgr_is_first_instr(drcontext, inst))
        return DR_EMIT_DEFAULT;



    mod_entry_cache = winafl_data.cache;
    mod_entry = module_table_lookup(mod_entry_cache,
                                                NUM_THREAD_MODULE_CACHE,
                                                module_table, start_pc);

    if (mod_entry == NULL || mod_entry->data == NULL) return DR_EMIT_DEFAULT;

    module_name = dr_module_preferred_name(mod_entry->data);

    should_instrument = false;
    target_modules = options.target_modules;
    while(target_modules) {
        if(_stricmp(module_name, target_modules->module_name) == 0) {
            should_instrument = true;
            if(options.debug_mode && debug_information_output == false) {
                dr_fprintf(winafl_data.log, "Instrumenting %s with the 'bb' mode\n", module_name);
                debug_information_output = true;
            }
            break;
        }
        target_modules = target_modules->next;
    }
    //if(!should_instrument) return DR_EMIT_DEFAULT | DR_EMIT_PERSISTABLE;
	if(!should_instrument) return static_cast<dr_emit_flags_t>(DR_EMIT_DEFAULT | DR_EMIT_PERSISTABLE);

    offset = (uint)(start_pc - mod_entry->data->start);
    offset &= MAP_SIZE - 1;

    afl_map = winafl_data.afl_area;

    drreg_reserve_aflags(drcontext, bb, inst);

    if(options.thread_coverage || options.dr_persist_cache) {
      reg_id_t reg;
      opnd_t opnd1, opnd2;
      instr_t *new_instr;

      drreg_reserve_register(drcontext, bb, inst, NULL, &reg);

      drmgr_insert_read_tls_field(drcontext, winafl_tls_field, bb, inst, reg);

      opnd1 = opnd_create_reg(reg);
      opnd2 = OPND_CREATE_MEMPTR(reg, sizeof(void *));
      new_instr = INSTR_CREATE_mov_ld(drcontext, opnd1, opnd2);
      instrlist_meta_preinsert(bb, inst, new_instr);

      opnd1 = OPND_CREATE_MEM8(reg, offset);
      new_instr = INSTR_CREATE_inc(drcontext, opnd1);
      instrlist_meta_preinsert(bb, inst, new_instr);

      drreg_unreserve_register(drcontext, bb, inst, reg);

	  //ret = DR_EMIT_DEFAULT | DR_EMIT_PERSISTABLE;
	  ret = static_cast<dr_emit_flags_t>(DR_EMIT_DEFAULT | DR_EMIT_PERSISTABLE);

	} else {

      instrlist_meta_preinsert(bb, inst,
          INSTR_CREATE_inc(drcontext, OPND_CREATE_ABSMEM
          (&(afl_map[offset]), OPSZ_1)));

	  ret = DR_EMIT_DEFAULT;
    }

    drreg_unreserve_aflags(drcontext, bb, inst);

    return ret;
}

static dr_emit_flags_t
instrument_edge_coverage(void *drcontext, void *tag, instrlist_t *bb, instr_t *inst,
                      bool for_trace, bool translating, void *user_data)
{
    static bool debug_information_output = false;
    app_pc start_pc;
    module_entry_t **mod_entry_cache;
    module_entry_t *mod_entry;
    reg_id_t reg, reg2, reg3;
    opnd_t opnd1, opnd2;
    instr_t *new_instr;
    const char *module_name;
    uint offset;
    target_module_t *target_modules;
    bool should_instrument;
	dr_emit_flags_t ret;

    if (!drmgr_is_first_instr(drcontext, inst))
        return DR_EMIT_DEFAULT;

    start_pc = dr_fragment_app_pc(tag);

    mod_entry_cache = winafl_data.cache;
    mod_entry = module_table_lookup(mod_entry_cache,
                                                NUM_THREAD_MODULE_CACHE,
                                                module_table, start_pc);

     if (mod_entry == NULL || mod_entry->data == NULL) return DR_EMIT_DEFAULT;

    module_name = dr_module_preferred_name(mod_entry->data);

    should_instrument = false;
    target_modules = options.target_modules;
    while(target_modules) {
        if(_stricmp(module_name, target_modules->module_name) == 0) {
            should_instrument = true;
            if(options.debug_mode && debug_information_output == false) {
                dr_fprintf(winafl_data.log, "Instrumenting %s with the 'edge' mode\n", module_name);
                debug_information_output = true;
            }
            break;
        }
        target_modules = target_modules->next;
    }

    //if(!should_instrument) return DR_EMIT_DEFAULT | DR_EMIT_PERSISTABLE;
	if(!should_instrument) return static_cast<dr_emit_flags_t>(DR_EMIT_DEFAULT | DR_EMIT_PERSISTABLE);


    offset = (uint)(start_pc - mod_entry->data->start);
    offset &= MAP_SIZE - 1;

    drreg_reserve_aflags(drcontext, bb, inst);
    drreg_reserve_register(drcontext, bb, inst, NULL, &reg);
    drreg_reserve_register(drcontext, bb, inst, NULL, &reg2);
    drreg_reserve_register(drcontext, bb, inst, NULL, &reg3);

    //reg2 stores AFL area, reg 3 stores previous offset

    //load the pointer to previous offset in reg3
    drmgr_insert_read_tls_field(drcontext, winafl_tls_field, bb, inst, reg3);

    //load address of shm into reg2
    if(options.thread_coverage || options.dr_persist_cache) {
      opnd1 = opnd_create_reg(reg2);
      opnd2 = OPND_CREATE_MEMPTR(reg3, sizeof(void *));
      new_instr = INSTR_CREATE_mov_ld(drcontext, opnd1, opnd2);
      instrlist_meta_preinsert(bb, inst, new_instr);

	  //ret = DR_EMIT_DEFAULT | DR_EMIT_PERSISTABLE
	  ret = static_cast<dr_emit_flags_t>(DR_EMIT_DEFAULT | DR_EMIT_PERSISTABLE);

	} else {
      opnd1 = opnd_create_reg(reg2);
      opnd2 = OPND_CREATE_INTPTR((uint64)winafl_data.afl_area);
      new_instr = INSTR_CREATE_mov_imm(drcontext, opnd1, opnd2);
      instrlist_meta_preinsert(bb, inst, new_instr);

	  ret = DR_EMIT_DEFAULT;
    }

    //load previous offset into register
    opnd1 = opnd_create_reg(reg);
    opnd2 = OPND_CREATE_MEMPTR(reg3, 0);
    new_instr = INSTR_CREATE_mov_ld(drcontext, opnd1, opnd2);
    instrlist_meta_preinsert(bb, inst, new_instr);

    //xor register with the new offset
    opnd1 = opnd_create_reg(reg);
    opnd2 = OPND_CREATE_INT32(offset);
    new_instr = INSTR_CREATE_xor(drcontext, opnd1, opnd2);
    instrlist_meta_preinsert(bb, inst, new_instr);

    //increase the counter at reg + reg2
    opnd1 = opnd_create_base_disp(reg2, reg, 1, 0, OPSZ_1);
    new_instr = INSTR_CREATE_inc(drcontext, opnd1);
    instrlist_meta_preinsert(bb, inst, new_instr);

    //store the new value
    offset = (offset >> 1)&(MAP_SIZE - 1);
    opnd1 = OPND_CREATE_MEMPTR(reg3, 0);
    opnd2 = OPND_CREATE_INT32(offset);
    new_instr = INSTR_CREATE_mov_st(drcontext, opnd1, opnd2);
    instrlist_meta_preinsert(bb, inst, new_instr);

    drreg_unreserve_register(drcontext, bb, inst, reg3);
    drreg_unreserve_register(drcontext, bb, inst, reg2);
    drreg_unreserve_register(drcontext, bb, inst, reg);
    drreg_unreserve_aflags(drcontext, bb, inst);

    return ret;
}

static void
pre_loop_start_handler(void *wrapcxt, INOUT void **user_data)
{
	char command;
	void *drcontext = drwrap_get_drcontext(wrapcxt);

	if (!options.debug_mode || options.true_flag) {
		//let server know we finished a cycle, redundunt on first cycle.
		WriteCommandToPipe('K');

		if (fuzz_target.iteration == options.fuzz_iterations) {
			dr_exit_process(0);
		}
		fuzz_target.iteration++;

		//let server know we are starting a new cycle
		WriteCommandToPipe('P'); 

		//wait for server acknowledgement for cycle start
		command = ReadCommandFromPipe(); 

		if (command != 'F') {
			if (command == 'Q') {
				dr_exit_process(0);
			}
			else {
				char errorMessage[] = "unrecognized command received over pipe: ";
				errorMessage[sizeof(errorMessage)-2] = command;
				DR_ASSERT_MSG(false, errorMessage);
			}
		}
	}
	else {
		debug_data.pre_hanlder_called++;
		dr_fprintf(winafl_data.log, "In pre_loop_start_handler\n");
	}

	memset(winafl_data.afl_area, 0, MAP_SIZE);

	if (options.coverage_kind == COVERAGE_EDGE || options.thread_coverage) {
		void **thread_data = (void **)drmgr_get_tls_field(drcontext, winafl_tls_field);
		thread_data[0] = 0;
		thread_data[1] = winafl_data.afl_area;
	}
}

static void
pre_fuzz_handler(void *wrapcxt, INOUT void **user_data)
{
    char command = 0;
    int i;
    void *drcontext;

    app_pc target_to_fuzz = drwrap_get_func(wrapcxt);
    dr_mcontext_t *mc = drwrap_get_mcontext_ex(wrapcxt, DR_MC_ALL);
    drcontext = drwrap_get_drcontext(wrapcxt);


	fuzz_target.eax = mc->eax;
	fuzz_target.ebx = mc->ebx;
	fuzz_target.ecx = mc->ecx;
	fuzz_target.edx = mc->edx;
	fuzz_target.esi = mc->esi;
	fuzz_target.edi = mc->edi;
	fuzz_target.ebp = mc->ebp;
	fuzz_target.esp = mc->esp;



    fuzz_target.xsp = mc->xsp;
    fuzz_target.func_pc = target_to_fuzz;

    if(!options.debug_mode || options.true_flag) {
		WriteCommandToPipe('P');
		command = ReadCommandFromPipe();

        if(command != 'F') {
            if(command == 'Q') {
                dr_exit_process(0);
            } else {
                DR_ASSERT_MSG(false, "unrecognized command received over pipe");
            }
        }
    } else {
        debug_data.pre_hanlder_called++;
		dr_fprintf(winafl_data.log, "In pre_fuzz_handler fuzz_target:0x%x\n", target_to_fuzz);
    }

    //save or restore arguments
    if (!options.no_loop) {
        if (fuzz_target.iteration == 0) {
            for (i = 0; i < options.num_fuz_args; i++)
                options.func_args[i] = drwrap_get_arg(wrapcxt, i);
        } else {
            for (i = 0; i < options.num_fuz_args; i++)
                drwrap_set_arg(wrapcxt, i, options.func_args[i]);
        }
    }

    memset(winafl_data.afl_area, 0, MAP_SIZE);

    if(options.coverage_kind == COVERAGE_EDGE || options.thread_coverage) {
        void **thread_data = (void **)drmgr_get_tls_field(drcontext, winafl_tls_field);
        thread_data[0] = 0;
        thread_data[1] = winafl_data.afl_area;
    }
}

static void
post_fuzz_handler(void *wrapcxt, void *user_data)
{
    dr_mcontext_t *mc;
    //mc = drwrap_get_mcontext(wrapcxt);

	mc = drwrap_get_mcontext_ex(wrapcxt, DR_MC_ALL);


    if(!options.debug_mode || options.true_flag) {
		WriteCommandToPipe('K');
    } else {
        debug_data.post_handler_called++;
		dr_fprintf(winafl_data.log, "In post_fuzz_handler fuzz_target:0x%x\n", mc->pc);
    }

    /* We don't need to reload context in case of network-based fuzzing. */
    if (options.no_loop)
        return;

    fuzz_target.iteration++;
    if(fuzz_target.iteration == options.fuzz_iterations) {
        dr_exit_process(0);
    }

	mc->eax = fuzz_target.eax;
	mc->ebx = fuzz_target.ebx;
	mc->ecx = fuzz_target.ecx;
	mc->edx = fuzz_target.edx;
	mc->esi = fuzz_target.esi;
	mc->edi = fuzz_target.edi;
	mc->ebp = fuzz_target.ebp;
	mc->esp = fuzz_target.esp;

    mc->xsp = fuzz_target.xsp;
    mc->pc = fuzz_target.func_pc;
	drwrap_redirect_execution(wrapcxt);
}

static void
createfilew_interceptor(void *wrapcxt, INOUT void **user_data)
{

    wchar_t *filenamew = (wchar_t *)drwrap_get_arg(wrapcxt, 0);
	dr_fprintf(winafl_data.log, "In OpenFileW, reading %ls\n", filenamew);
	/*
	if (filenamew == NULL) return;

	int len = wcslen(filenamew);
	if (len < 4) return;

	if (filenamew[len - 1] == 'l' && filenamew[len - 2] == 'l' &&
		filenamew[len - 3] == 'd' && *(filenamew + len - 4) == '.') {
		dr_fprintf(winafl_data.log, "dll: %ls\n", filenamew);
	}
	else {
		dr_fprintf(winafl_data.log, "NON dll:%ls\n", filenamew);
		//wchar_t *newpath = "\?\C:\fuzz\application\windeploy\poc.txt";
		
		filenamew[len - 7] = 'p';
		filenamew[len - 6] = 'o';
		filenamew[len - 5] = 'c';
		filenamew[len - 4] = '.';
		filenamew[len - 3] = 't';
		filenamew[len - 2] = 'x';
		filenamew[len - 1] = 't';
		
		
		for (int i = 0; i < len; i++) {
			dr_fprintf(winafl_data.log, "%d:%c\n", i, filenamew[i]);

		}
		
	}*/
}

static void
createfilea_interceptor(void *wrapcxt, INOUT void **user_data)
{
    char *filename = (char *)drwrap_get_arg(wrapcxt, 0);
    if(options.debug_mode)
        dr_fprintf(winafl_data.log, "In OpenFileA, reading %s\n", filename);
}

static void
strncmp_interceptor(void *wrapcxt, INOUT void **user_data)
{
	const char *pepath = "c:\\fuzz_config\\pe.ini";

	const int buffer_len_max = 64;
	int copy_len = 0;
	char *src1 = (char *)drwrap_get_arg(wrapcxt, 0);
	char *src2 = (char *)drwrap_get_arg(wrapcxt, 1);
    int length = (int)drwrap_get_arg(wrapcxt, 2);

	char str1[buffer_len_max] = {0};
	char str2[buffer_len_max] = {0};

	if (buffer_len_max > strlen(src1))
		copy_len = strlen(src1);
	else copy_len = buffer_len_max;
	strncpy(str1, src1, copy_len+1);

	if (buffer_len_max > strlen(src2))
		copy_len = strlen(src2);
	else copy_len = buffer_len_max;
	strncpy(str2, src2, copy_len+1);

	read_afl_input();

	int valid1 = is_dictitem_valid(str1);
	int valid2 = is_dictitem_valid(str2);

	ini_t *peini = ini_load(pepath);
	int rdata_start = ini_get_int(peini, "", ".rdata_start");
	int rdata_end   = ini_get_int(peini, "", ".rdata_end");
	
	// firstly choose data from rdata as dictitem
	if (rdata_start < rdata_end && rdata_start > 0){
		if ((int)src1 > rdata_start && (int)src1 < rdata_end ){
			if (valid1){
				if (add_one_dictitem(str1)){
					if(options.debug_mode)
						dr_fprintf(winafl_data.log, "In %s, add one dictitem from rdata str1:%s\n", __FUNCTION__, str1);
					return;
				}
			}
		}
		
		if ((int)src2 > rdata_start && (int)src2 < rdata_end ){
			if (valid2) {
				if (add_one_dictitem(str2)) {
					if(options.debug_mode)
						dr_fprintf(winafl_data.log, "In %s, add one dictitem from rdata str2:%s\n", __FUNCTION__, str2);
		
					return;
				}
			}
		}
	}

	//dr_fprintf(winafl_data.log, "In %s, str1:%s\n", __FUNCTION__, str1);
	
	if (strstr(global_afl_input, str1) == 0)
		if (is_dictitem_valid(str1)){

			if (add_one_dictitem(str1))
				if(options.debug_mode)
					dr_fprintf(winafl_data.log, "In %s, add one dictitem str1:%s\n", __FUNCTION__, str1);
		}


	//dr_fprintf(winafl_data.log, "In %s, str2:%s\n", __FUNCTION__, str2);
	
	if (strstr(global_afl_input, str2) == 0)
		if (is_dictitem_valid(str2)){
			if (add_one_dictitem(str2))
				if(options.debug_mode)
					dr_fprintf(winafl_data.log, "In %s, add one dictitem str2:%s\n", __FUNCTION__, str2);
		}

}



// *************************Qt4****************************** //

typedef unsigned short ushort;
struct MyQStringData {
    long ref; //QBasicAtomicInt On Windows is long
    int alloc, size; // alloc == size
    ushort *data; // QT5: put that after the bit field to fill alignment gap; don't use sizeof any more then
    ushort clean : 1;
    ushort simpletext : 1;
    ushort righttoleft : 1;
    ushort asciiCache : 1;
    ushort capacity : 1;
    ushort reserved : 11;
    // ### Qt5: try to ensure that "array" is aligned to 16 bytes on both 32- and 64-bit
    ushort array[1];
};


struct MyQListData {
    long ref;
    int alloc, begin, end;
    uint sharable : 1;
    void *array[1];
};


static void
Qt4_QFileDialog_getOpenFileName_interceptor(void *wrapcxt, INOUT void **user_data)
{
	int *retval;
	int stdcall_args_size = 4 * 0;
	dr_mcontext_t *mc;

	/*
	QString str = "c:\\out\\afl.input";
	int *pqstring = (int *)drwrap_get_arg(wrapcxt, 0);
	*pqstring = (int)&str;

	drwrap_skip_call(wrapcxt, (void *)retval, stdcall_args_size);

	return;
	*/
	

	wchar_t *unicode = L"c:\\out\\afl.input";
    unsigned int size = wcslen(unicode);


    MyQStringData *d = NULL;

	if (d == NULL){
		d = (MyQStringData*) malloc(sizeof(MyQStringData)+size*sizeof(ushort));

		d->ref = 2;
		d->alloc = d->size = size;
		d->clean = d->asciiCache = d->simpletext = d->righttoleft = d->capacity = 0;
		d->data = d->array;
		memcpy(d->array, unicode, size * sizeof(ushort));
		d->array[size] = '\0';
	}

	int *ret = (int *)malloc(sizeof(int));
	retval = (int *)malloc(sizeof(int));
	*ret = (int)d;
    *retval = (int )ret;


	int *pqstring = (int *)drwrap_get_arg(wrapcxt, 0);
	*pqstring = (int)d;
	pqstring = (int *)drwrap_get_arg(wrapcxt, 0);
	if (options.debug_mode)
		dr_fprintf(winafl_data.log, "In Qt4_QFileDialog_getOpenFileName_interceptor, return QString: 0x%0x 0x%0x\n", pqstring, *pqstring);


	drwrap_skip_call(wrapcxt, (void *)retval, stdcall_args_size);
	
	
}


wchar_t *unicode = L"c:\\out\\afl.input";
unsigned int size = wcslen(unicode);
MyQStringData *myqstring = NULL;

int list_size = 32;
MyQListData *qstrings = (MyQListData*)malloc(list_size);

// avoid memory leak
void init_global_data(){
	if (myqstring == NULL){
		myqstring = (MyQStringData*) malloc(sizeof(MyQStringData)+size*sizeof(ushort));

		myqstring->ref = 2;
		myqstring->alloc = myqstring->size = size;
		myqstring->clean = myqstring->asciiCache = 0;
		myqstring->simpletext = myqstring->righttoleft = myqstring->capacity = 0;
		myqstring->data = myqstring->array;
		memcpy(myqstring->array, unicode, size * sizeof(ushort));
		myqstring->array[size] = '\0';
	}

	memset(qstrings, 0xcd, list_size);
	qstrings->ref = 123456;
	qstrings->alloc = 1;
	qstrings->begin = 0;
	qstrings->end = 1;
	qstrings->array[0] = (void*)(int)myqstring;
}


static void
Qt4_QFileDialog_getOpenFileNames_interceptor(void *wrapcxt, INOUT void **user_data)
{
	int *retval;
	int stdcall_args_size = 4 * 0;
	dr_mcontext_t *mc;

	int *pqstrings = (int *)drwrap_get_arg(wrapcxt, 0);
	
	*pqstrings = (int)qstrings;


	if (options.debug_mode)
		dr_fprintf(winafl_data.log, "In Qt4_QFileDialog_getOpenFileNames_interceptor, \
									return QString[0]: 0x%0x 0x%0x\n", *pqstrings, *pqstrings+18);

	drwrap_skip_call(wrapcxt, (void *)retval, stdcall_args_size);
	
}


static void
Qt4_QString_free_interceptor(void *wrapcxt, INOUT void **user_data)
{
	int retval = 0;
	int stdcall_args_size = 4 * 1;

	if (options.debug_mode)
		dr_fprintf(winafl_data.log, "In Qt4_QString_free_interceptor\n");

	if (npc > 0x4034c0 && npc < 0x4036d0)
	drwrap_skip_call(wrapcxt, (void *)retval, stdcall_args_size);

}

static void
Qt4_QFile_QFile_interceptor(void *wrapcxt, INOUT void **user_data)
{
	int retval = 0;
	//int stdcall_args_size = 4 * 1;

	int *pQString;
	pQString = (int *)drwrap_get_arg(wrapcxt, 0);

	if (options.debug_mode)
		dr_fprintf(winafl_data.log, "In Qt4_QFile_QFile_interceptor, reading 0x%x, * 0x%x, %ls\n", pQString, *pQString, *pQString + 5 );


		 wchar_t *unicode = L"c:\\out\\afl.input";
     unsigned int size = wcslen(unicode);


	dr_fprintf(winafl_data.log, "In Qt4_QFile_QFile_interceptor, before modify reading 0x%x, * 0x%x\n", pQString, *pQString );

	if (start_skip){


		MyQStringData *d;
		d = (MyQStringData*) malloc(sizeof(MyQStringData)+size*sizeof(ushort));

		d->ref = 1;
		d->alloc = d->size = size;
		d->clean = d->asciiCache = d->simpletext = d->righttoleft = d->capacity = 0;
		d->data = d->array;
		memcpy(d->array, unicode, size * sizeof(ushort));
		d->array[size] = '\0';

		*pQString = (int)d;
		pQString = (int *)drwrap_get_arg(wrapcxt, 0);


		dr_fprintf(winafl_data.log, "In Qt4_QFile_QFile_interceptor, after modify reading 0x%x, * 0x%x\n", pQString, *pQString );

	}


	//drwrap_skip_call(wrapcxt, (void *)retval, stdcall_args_size);

}



static void
Qt4_QMessageBox_information_interceptor(void *wrapcxt, INOUT void **user_data){

	int retval = 0;
	int stdcall_args_size = 4 * 0;
	int StandardButtons = (int)drwrap_get_arg(wrapcxt, 3);



	drwrap_skip_call(wrapcxt, (void *)retval, stdcall_args_size);
}

static void
Qt4_QMessageBox_warnning_interceptor(void *wrapcxt, INOUT void **user_data){

	int retval = 0;
	int stdcall_args_size = 4 * 0;

	drwrap_skip_call(wrapcxt, (void *)retval, stdcall_args_size);
}

static void
Qt4_QInputDialog_getItem_interceptor(void *wrapcxt, INOUT void **user_data){

	int retval = 0;
	int stdcall_args_size = 4 * 0;

	int length = 0;
	int offset = 0;

	//(ret, this, tr("QInputDialog::getItem()"),tr("Season:"), items, 0, false, &ok)
	
	int *pQString = (int *)drwrap_get_arg(wrapcxt, 0);
	int **pitems = (int **)drwrap_get_arg(wrapcxt, 4);
	int editable = (int)drwrap_get_arg(wrapcxt, 6);
	
	//random choose one item
	length = (int)*(*pitems + 0xc) - (int)*(*pitems + 8);
	offset = rand() % length;

	//*pQString = *(*pitems + 0x14 + offset);

	if (options.debug_mode)
		dr_fprintf(winafl_data.log, "In Qt4_QInputDialog_getItem_interceptor,\
			items %x, %x, %x, %x, %x length %d, choosed %d \n",\
			pitems, *pitems, **pitems, *(*pitems + 0x0), *(*pitems + 0x4), length, offset);

	// TODO editable

	drwrap_skip_call(wrapcxt, (void *)retval, stdcall_args_size);
}

static void
Qt4_QInputDialog_getDouble_interceptor(void *wrapcxt, INOUT void **user_data){

	int retval = 0;
	int stdcall_args_size = 4 * 0;
	
	//char mydouble[8] = {0, 0, 0, 0,  0, 0x80, 0x28, 0x40}; // 12.25
	
	char mydouble[8] = {0}; 

	int copy_size = read_afl_input();
	if (copy_size > 8) copy_size = 8;

	memcpy(mydouble, global_afl_input, copy_size);
	
	__asm{
		fld qword ptr mydouble
	}

	if (options.debug_mode)
		dr_fprintf(winafl_data.log, "In %s, insert double\n", __FUNCTION__);
	
	drwrap_skip_call(wrapcxt, (void *)retval, stdcall_args_size);
}


static void
Qt4_QColorDialog_getColor_interceptor(void *wrapcxt, INOUT void **user_data){

	int retval = 0;
	int stdcall_args_size = 4 * 0;

	int *pQColor = (int *)drwrap_get_arg(wrapcxt, 0);

	//QColor alloc object on stack
	//01 00 00 00 FF FF 01 01    02 02 03 03 00 00 2F 00
	*pQColor = 0x1;
	*(pQColor + 1) = 0x0101ffff;
	*(pQColor + 2) = 0x03030202;
	*(pQColor + 3) = 0x00180000;

	if (options.debug_mode)
		dr_fprintf(winafl_data.log, "In Qt4_QColorDialog_getColor_interceptor, %08x %08x %08x %08x\n", pQColor, *pQColor, pQColor+4, *(pQColor+4));
			
	retval = (int)pQColor;

	drwrap_skip_call(wrapcxt, (void *)retval, stdcall_args_size);
}

static void
Qt4_QPrintDialog_exec_interceptor(void *wrapcxt, INOUT void **user_data){

	int retval = 0;
	int stdcall_args_size = 4 * 0;

	if (options.debug_mode)
		dr_fprintf(winafl_data.log, "In Qt4_QPrintDialog_exec_interceptor, eax=1\n");
			
	drwrap_skip_call(wrapcxt, (void *)retval, stdcall_args_size);
}


static void
Qt4_QDoubleSpinBox_value_interceptor(void *wrapcxt, INOUT void **user_data){

	int retval = 0;
	int stdcall_args_size = 4 * 0;

	char mydouble[8] = {0, 0, 0, 0,  0, 0x80, 0x28, 0x40}; // 12.25
	//char mydouble[8] = {0}; 

	read_afl_input();
	int copy_size = global_afl_input_size > 8 ? 8 : global_afl_input_size;
	memcpy(mydouble, global_afl_input, copy_size);

	__asm{
		fld qword ptr mydouble
	}

	if (options.debug_mode)
		dr_fprintf(winafl_data.log, "In %s, inject double\n", __FUNCTION__);
			
	drwrap_skip_call(wrapcxt, (void *)retval, stdcall_args_size);
}



static inline uint julianDayFromGregorianDate(int year, int month, int day)
{
    // Gregorian calendar starting from October 15, 1582
    // Algorithm from Henry F. Fliegel and Thomas C. Van Flandern
    return (1461 * (year + 4800 + (month - 14) / 12)) / 4
           + (367 * (month - 2 - 12 * ((month - 14) / 12))) / 12
           - (3 * ((year + 4900 + (month - 14) / 12) / 100)) / 4
           + day - 32075;
}

static uint julianDayFromDate(int year, int month, int day)
{
    if (year < 0)
        ++year;

    if (year > 1582 || (year == 1582 && (month > 10 || (month == 10 && day >= 15)))) {
        return julianDayFromGregorianDate(year, month, day);
    } else if (year < 1582 || (year == 1582 && (month < 10 || (month == 10 && day <= 4)))) {
        // Julian calendar until October 4, 1582
        // Algorithm from Frequently Asked Questions about Calendars by Claus Toendering
        int a = (14 - month) / 12;
        return (153 * (month + (12 * a) - 3) + 2) / 5
               + (1461 * (year + 4800 - a)) / 4
               + day - 32083;
    } else {
        // the day following October 4, 1582 is October 15, 1582
        return 0;
    }
}



enum {
	FIRST_YEAR = -4713,
    FIRST_MONTH = 1,
    FIRST_DAY = 2,  // ### Qt 5: make FIRST_DAY = 1, by support jd == 0 as valid
};
static const char monthDays[] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

bool QDateisLeapYear(int y)
{
    if (y < 1582) {
        if ( y < 1) {  // No year 0 in Julian calendar, so -1, -5, -9 etc are leap years
            ++y;
        }
        return y % 4 == 0;
    } else {
        return (y % 4 == 0 && y % 100 != 0) || y % 400 == 0;
    }
}

bool QDateisValid(int year, int month, int day)
{
    if (year < FIRST_YEAR
        || (year == FIRST_YEAR &&
            (month < FIRST_MONTH
             || (month == FIRST_MONTH && day < FIRST_DAY)))
        || year == 0) // there is no year 0 in the Julian calendar
        return false;

    // passage from Julian to Gregorian calendar
    if (year == 1582 && month == 10 && day > 4 && day < 15)
        return 0;

    return (day > 0 && month > 0 && month <= 12) &&
           (day <= monthDays[month] || (day == 29 && month == 2 && QDateisLeapYear(year)));
}

bool QTimeisValid(int h, int m, int s, int ms)
{
    return (uint)h < 24 && (uint)m < 60 && (uint)s < 60 && (uint)ms < 1000;
}

class QTime{
	int mds;
};

class QDate{
	uint jd;
};

class QDateTimePrivate{
	public:
		enum Spec { LocalUnknown = -1, LocalStandard = 0, LocalDST = 1, UTC = 2, OffsetFromUTC = 3};
		int ref;
		int date;
		int time;
		int spec;
		/*!
		  \internal
		  \since 4.4

		  The offset in seconds. Applies only when timeSpec() is OffsetFromUTC.
		 */
		int utcOffset;
};

class QDateTime{
public:
	QDateTimePrivate d;
};

#include <time.h>

typedef struct times
{
	int Year;
	int Mon;
	int Day;
	int Hour;
	int Min;
	int Second;
}Times;

Times stamp_to_standard(int stampTime){
	time_t tick = (time_t)stampTime;
	struct tm tm;
	char s[100];
	Times t;

	tm = *localtime(&tick);
	strftime(s, sizeof(s), "%Y-%m-%d %H:%M:%S", &tm);
	
	t.Year = atoi(s);
	t.Mon = atoi(s+5);
	t.Day = atoi(s+8);
	t.Hour = atoi(s+11);
	t.Min = atoi(s+14);
	t.Second = atoi(s+17);
	return t;

}


static void 
Qt4_QDateTimeEdit_date_interceptor(void *wrapcxt, INOUT void **user_data){
	int retval = 0;
	int stdcall_args_size = 4 * 1;

	char timestamp[4] = {0};

	int copy_size = read_afl_input();
	if (copy_size > 4) copy_size = 4;

	memcpy(timestamp, global_afl_input, copy_size);

	timestamp[3] &= 0x7f;

	Times t = stamp_to_standard(*(int *)timestamp);
	int date = julianDayFromDate(t.Year, t.Mon, t.Day);

	int *pdate  = (int *)drwrap_get_arg(wrapcxt, 0);
	*pdate = date;

	if (options.debug_mode)
		dr_fprintf(winafl_data.log, "In %s, inject date %x %d-%d-%d \n", __FUNCTION__, *(int *)timestamp, t.Year, t.Mon, t.Day);
			
	drwrap_skip_call(wrapcxt, (void *)retval, stdcall_args_size);
}

static void 
Qt4_QDateTimeEdit_time_interceptor(void *wrapcxt, INOUT void **user_data){
	int retval = 0;
	int stdcall_args_size = 4 * 1;

	char timestamp[4] = {0};

	read_afl_input();
	int copy_size = global_afl_input_size > 4 ? 4 : global_afl_input_size;
	memcpy(timestamp, global_afl_input, copy_size);
	timestamp[3] &= 0x7f;

	Times t = stamp_to_standard(*(int *)timestamp);
	int mytime = (t.Hour * 3600 + t.Min * 60 + t.Second)*1000;

	int *ptime  = (int *)drwrap_get_arg(wrapcxt, 0);
	*ptime = mytime;

	if (options.debug_mode)
		dr_fprintf(winafl_data.log, "In Qt4_QDateTimeEdit_time_interceptor, inject time: %02d:%02d:%02d\n", t.Hour, t.Min, t.Second);
			
	drwrap_skip_call(wrapcxt, (void *)retval, stdcall_args_size);
}



static void 
Qt4_QDateTimeEdit_dateTime_interceptor(void *wrapcxt, INOUT void **user_data){
	int retval = 0;
	int stdcall_args_size = 4 * 1;

	char timestamp[4] = {0};
	
	read_afl_input();
	int copy_size = global_afl_input_size > 4 ? 4 : global_afl_input_size;
	memcpy(timestamp, global_afl_input, copy_size);
	timestamp[3] &= 0x7f;

	Times t = stamp_to_standard(*(int *)timestamp);

	int date = julianDayFromDate(t.Year, t.Mon, t.Day);
	int mytime = (t.Hour * 3600 + t.Min * 60 + t.Second)*1000;

	int *datetime  = (int *)drwrap_get_arg(wrapcxt, 0);

	QDateTime *dt = new QDateTime();
	QDateTimePrivate dtp = QDateTimePrivate();
	dtp.ref = 123456;
	dtp.date = date;
	dtp.time = mytime;
	dtp.spec = -1;
	dtp.utcOffset = 0;
	dt->d = dtp;

	*datetime = (int)dt;

	if (options.debug_mode)
		dr_fprintf(winafl_data.log, \
		"In Qt4_QDateTimeEdit_dateTime_interceptor, inject datetime %x %04d%02d%02d %02d%02d%02d\n", \
		*datetime, t.Year, t.Mon, t.Day, t.Hour, t.Min, t.Second);
			
	drwrap_skip_call(wrapcxt, (void *)retval, stdcall_args_size);
}


int countryarr_count = 7;
wchar_t *countryarr[7] = {L"Egypt", L"France", L"Germany", L"India", L"Italy", L"Norway", L"Pakistan"};


static void 
Qt4_QComboBox_currentIndex_interceptor(void *wrapcxt, INOUT void **user_data){
	int retval = 0;
	int stdcall_args_size = 4 * 0;

	//dr_mcontext_t *mc = drwrap_get_mcontext_ex(wrapcxt, DR_MC_ALL);
	//dr_fprintf(winafl_data.log, "before eax %x ecx %x\n", mc->eax, mc->ecx);

	//int countaddr = 0x65317AF0;
	//addr = ini_get_int(interceptor_config, module_name, "QComboBox_count");
			
	//	to_wrap = (app_pc)(addr - (int)module_base + (int)info->start);
	/*

	int count = 0;

	__asm{
		lea ebx, count;
		// call count()
		mov eax, countaddr;	
		//call eax;
		mov [ebx], eax;
	}
	*/

	//mc = drwrap_get_mcontext_ex(wrapcxt, DR_MC_ALL);
	//dr_fprintf(winafl_data.log, "after eax %x\n", *(int*)mc->eax);

	//dr_fprintf(winafl_data.log, "count %x\n", count);

	retval = rand() % countryarr_count;

	if (options.debug_mode)
		dr_fprintf(winafl_data.log, "In %s\n", __FUNCTION__);
			
	drwrap_skip_call(wrapcxt, (void *)retval, stdcall_args_size);
}

static void 
Qt4_QComboBox_currentText_interceptor(void *wrapcxt, INOUT void **user_data){
	int retval = 0;
	int stdcall_args_size = 4 * 1;


	int *pQString = (int *)drwrap_get_arg(wrapcxt, 0);

	int index = rand() % countryarr_count;
	
	wchar_t *currentText = countryarr[index];

	unsigned int size = wcslen(currentText);

	MyQStringData *myqstring = NULL;

	myqstring = (MyQStringData*) malloc(sizeof(MyQStringData)+size*sizeof(ushort));

	myqstring->ref = 2;
	myqstring->alloc = myqstring->size = size;
	myqstring->clean = myqstring->asciiCache = 0;
	myqstring->simpletext = myqstring->righttoleft = myqstring->capacity = 0;
	myqstring->data = myqstring->array;
	memcpy(myqstring->array, currentText, size * sizeof(ushort));
	myqstring->array[size] = '\0';

	

	if (options.debug_mode){

		dr_fprintf(winafl_data.log, "In %s npc:%x\n", __FUNCTION__, npc);

		//004012AC
		if (npc == 0x4012ac && start_skip == 0 ){
			*pQString = (int)myqstring;

			start_skip = 1;
			drwrap_skip_call(wrapcxt, (void *)retval, stdcall_args_size);
		}
	}

	//drwrap_skip_call(wrapcxt, (void *)retval, stdcall_args_size);
}

static void
Qt4_QUdpSocket_hasPendingDatagrams_interceptor(void *wrapcxt, INOUT void **user_data){
	int retval = 0;
	int stdcall_args_size = 4 * 0;

	int size = get_afl_input_size();
	
	retval = 1 ? size > 0: 0;

	if (options.debug_mode)
		dr_fprintf(winafl_data.log, "In %s return %d \n", __FUNCTION__, retval);
	
	drwrap_skip_call(wrapcxt, (void *)retval, stdcall_args_size);
}

static void
Qt4_QUdpSocket_pendingDatagramSize_interceptor(void *wrapcxt, INOUT void **user_data){
	int retval = 0;
	int stdcall_args_size = 4 * 0;

	int size = get_afl_input_size();
	
	retval = size;

	if (options.debug_mode)
		dr_fprintf(winafl_data.log, "In %s \n", __FUNCTION__);

	drwrap_skip_call(wrapcxt, (void *)retval, stdcall_args_size);
}

static void
Qt4_QUdpSocket_readDatagram_interceptor(void *wrapcxt, INOUT void **user_data){
	int retval = 0;
	int stdcall_args_size = 4 * 5;

	int size = read_afl_input(afl_input_path);

	retval = size;
	//TODO:set edx = 0;

	char *data = (char *)drwrap_get_arg(wrapcxt, 0);

	memcpy(data, global_afl_input, size);

	empty_afl_input();


	if (options.debug_mode)
		dr_fprintf(winafl_data.log, "In %s read:%s \n", __FUNCTION__, data);


	drwrap_skip_call(wrapcxt, (void *)retval, stdcall_args_size);
}

static void
Qt4_QUdpSocket_bind_interceptor(void *wrapcxt, INOUT void **user_data){
	int retval = 0;
	int stdcall_args_size = 4 * 0;
	if (options.debug_mode)
		dr_fprintf(winafl_data.log, "In %s \n", __FUNCTION__);
	;//drwrap_skip_call(wrapcxt, (void *)retval, stdcall_args_size);
}

// *************************Qt5***************************** //
static void
Qt5_QFile_interceptor(void *wrapcxt, INOUT void **user_data)
{
	wchar_t *filenamew = (wchar_t *)drwrap_get_arg(wrapcxt, 0);
	if (options.debug_mode)
		dr_fprintf(winafl_data.log, "In Qt5_QFile_interceptor, reading %ls\n", filenamew);
}
static void
Qt5_getOpenFileName_interceptor(void *wrapcxt, INOUT void **user_data)
{
	char *filename = (char *)drwrap_get_arg(wrapcxt, 0);
	if (options.debug_mode)
		dr_fprintf(winafl_data.log, "In OpenFileA, reading %s\n", filename);
}


static void
verfierstopmessage_interceptor_pre(void *wrapctx, INOUT void **user_data)
{
    EXCEPTION_RECORD exception_record = { 0 };
    dr_exception_t dr_exception = { 0 };
    dr_exception.record = &exception_record;
    exception_record.ExceptionCode = STATUS_HEAP_CORRUPTION;

    onexception(NULL, &dr_exception);
}

static void
bind_interceptor(void *wrapcxt, INOUT void **user_data)
{
	void *retval = 0;
	int stdcall_args_size = 4 * 3;

    if (options.debug_mode){
        dr_fprintf(winafl_data.log, "In bind\n");
	}

	drwrap_skip_call(wrapcxt, retval, stdcall_args_size);
}

static void
recvfrom_interceptor(void *wrapcxt, INOUT void **user_data)
{
	void *retval;
	int stdcall_args_size = 4 * 6;

	char *str  = (char *)drwrap_get_arg(wrapcxt, 1);
	int buf_size = (int)drwrap_get_arg(wrapcxt, 2);
	int testcase_len = read_afl_input(afl_input_path);

	int copy_len = buf_size > testcase_len? testcase_len : buf_size;

	memcpy(str, global_afl_input, copy_len);

	retval = &copy_len;

    if (options.debug_mode){
        dr_fprintf(winafl_data.log, "In recvfrom, len:%d %s\n", copy_len, str);
	}
	drwrap_skip_call(wrapcxt, retval, stdcall_args_size);
}

static void
sendto_interceptor(void *wrapcxt, INOUT void **user_data){

	int len;
	void * retval;
	int stdcall_args_size = 4 * 6;

	if (options.debug_mode){
			dr_fprintf(winafl_data.log, "In sendto\n");
	}

	len = (int)drwrap_get_arg(wrapcxt, 2); 
	retval = (void*)len;

	drwrap_skip_call(wrapcxt, retval, stdcall_args_size);
}


static void
closesocket_interceptor(void *wrapcxt, INOUT void **user_data){
	void * retval = 0;
	int stdcall_args_size = 4 * 1;

	if (options.debug_mode){
			dr_fprintf(winafl_data.log, "In closesocket_interceptor\n");
	}
	drwrap_skip_call(wrapcxt, retval, stdcall_args_size);

}

static void
shutdown_interceptor(void *wrapcxt, INOUT void **user_data){
	void * retval = 0;
	int stdcall_args_size = 4 * 2;

	if (options.debug_mode){
			dr_fprintf(winafl_data.log, "In shutdown_interceptor\n");
	}
	drwrap_skip_call(wrapcxt, retval, stdcall_args_size);

}

static void
WSAStartup_interceptor(void *wrapcxt, INOUT void **user_data){
	void * retval = 0;
	int stdcall_args_size = 4 * 2;

	if (options.debug_mode){
			dr_fprintf(winafl_data.log, "In WSAStartup_interceptor\n");
	}
	drwrap_skip_call(wrapcxt, retval, stdcall_args_size);
}

static void
WSACleanup_interceptor(void *wrapcxt, INOUT void **user_data){
	void * retval = 0;
	int stdcall_args_size = 4 * 0;

	if (options.debug_mode){
			dr_fprintf(winafl_data.log, "In WSACleanup_interceptor\n");
	}
	drwrap_skip_call(wrapcxt, retval, stdcall_args_size);

}

static void
listen_interceptor(void *wrapcxt, INOUT void **user_data)
{
	void *retval = 0;
	int stdcall_args_size = 4 * 2;

    if (options.debug_mode){
        dr_fprintf(winafl_data.log, "In listen\n");
	}
	drwrap_skip_call(wrapcxt, retval, stdcall_args_size);

}

static void
accept_interceptor(void *wrapcxt, INOUT void **user_data)
{
	void *retval;
	int stdcall_args_size = 4 * 3;
	int ret = 90;
	retval = (void *)ret;

    if (options.debug_mode){
        dr_fprintf(winafl_data.log, "In accept\n");
	}
	drwrap_skip_call(wrapcxt, retval, stdcall_args_size);

}
static void
connect_interceptor(void *wrapcxt, INOUT void **user_data)
{
	void *retval = 0;
	int stdcall_args_size = 4 * 3;

    if (options.debug_mode){
        dr_fprintf(winafl_data.log, "In connect\n");
	}
	drwrap_skip_call(wrapcxt, retval, stdcall_args_size);

}
static void
recv_interceptor(void *wrapcxt, INOUT void **user_data)
{
	void *retval;
	int stdcall_args_size = 4 * 4;

	char *str  = (char *)drwrap_get_arg(wrapcxt, 1);
	int buf_size = (int)drwrap_get_arg(wrapcxt, 2);
	int testcase_len = read_afl_input(afl_input_path);
	int copy_len = buf_size > testcase_len? testcase_len : buf_size;

	memcpy(str, global_afl_input, copy_len);

	retval = &copy_len;

    if (options.debug_mode){
        dr_fprintf(winafl_data.log, "In recv, len:%d %s\n", copy_len, str);
	}
	drwrap_skip_call(wrapcxt, retval, stdcall_args_size);

}

static void
send_interceptor(void *wrapcxt, INOUT void **user_data)
{
	void *retval = 0;
	int stdcall_args_size = 4 * 4;
	int len = (int)drwrap_get_arg(wrapcxt, 2); // len
	retval = &len;
    if (options.debug_mode){
        dr_fprintf(winafl_data.log, "In send\n");
	}

	drwrap_skip_call(wrapcxt, retval, stdcall_args_size);

}

static void
CFile_Open_interceptor(void *wrapcxt, INOUT void **user_data)
{
	int OpenFlag = 0;
	int *OpenFlags;
	
	dr_mcontext_t *mc;

	dr_fprintf(winafl_data.log, "In CFile_Open_interceptor\n");
	/*
		enum OpenFlags {
		modeRead =         (int) 0x00000,
		modeWrite =        (int) 0x00001,
		modeReadWrite =    (int) 0x00002,
		shareCompat =      (int) 0x00000,
		shareExclusive =   (int) 0x00010,
		shareDenyWrite =   (int) 0x00020,
		shareDenyRead =    (int) 0x00030,
		shareDenyNone =    (int) 0x00040,
		modeNoInherit =    (int) 0x00080,
		modeCreate =       (int) 0x01000,
		modeNoTruncate =   (int) 0x02000,
		typeText =         (int) 0x04000, // typeText and typeBinary are
		typeBinary =       (int) 0x08000, // used in derived classes only
		osNoBuffer =       (int) 0x10000,
		osWriteThrough =   (int) 0x20000,
		osRandomAccess =   (int) 0x40000,
		osSequentialScan = (int) 0x80000,
		};
		*/

	//return ;

	//filenamew = (int *)drwrap_get_arg(wrapcxt, 1);


	OpenFlags = (int *)drwrap_get_arg(wrapcxt, 2);
	
	mc = drwrap_get_mcontext(wrapcxt);
	OpenFlag = *(int *)(mc->esp + 8); 

	dr_fprintf(winafl_data.log, "In Cfile::open flags:%x\n", OpenFlag);

    if (options.debug_mode)
	{

		*OpenFlags = 0x00040;
		*(int *)(mc->esp + 8) = 0x00040;
	
		//drwrap_set_arg(wrapcxt, 2, (void*)OpenFlags);


		OpenFlag = (int )drwrap_get_arg(wrapcxt, 2);

		dr_fprintf(winafl_data.log, "In Cfile::open flags:%x\n", OpenFlag);

	}
	
}

static void
messagebox_interceptor(void *wrapcxt, INOUT void **user_data)
{
	void * retval;
	int ret = 1;
	size_t stdcall_args_size;

	if (options.debug_mode){
		dr_fprintf(winafl_data.log, "In Messsagebox, skip it\n");
	}

	retval = (void *)ret;
	stdcall_args_size = 4*4;

	drwrap_skip_call(wrapcxt, retval, stdcall_args_size);
}


static void
SendDlgItemMessageW_interceptor(void *wrapcxt, INOUT void **user_data){
	// TODO: blacklist or whitelist
	// WinUser.h	WM_VSCROLL 0x115

	void *retval = 0;
	int ret = 1;
	size_t stdcall_args_size = 4*5;

	// SendDlgItemMessageW(HWND hDlg, int nIDDlgItem, UINT Msg, WPARAM wParam, LPARAM lParam)
	int nMessage = (int )drwrap_get_arg(wrapcxt, 2);

	if (options.debug_mode)
		dr_fprintf(winafl_data.log, "In SendDlgItemMessageW_interceptor, Message: 0x%x\n", nMessage);

	if (nMessage == 0x115){

		drwrap_skip_call(wrapcxt, retval, stdcall_args_size);
		
	}
}


static void
CFileDialog_doModal_interceptor(void *wrapcxt, INOUT void **user_data)
{

		void * retval;
		int ret = 1;
		size_t stdcall_args_size;
		

	if (options.debug_mode){

		dr_fprintf(winafl_data.log, "in CFileDialog, skip it\n");


		retval = (void *)ret;
		stdcall_args_size = 0;

		drwrap_skip_call(wrapcxt, retval, stdcall_args_size);

	}

}

static void
CFileDialog_GetPathName_interceptor(void *wrapcxt, INOUT void **user_data)
{

		void * retval;
		int ret = 1;
		size_t stdcall_args_size;
		wchar_t *lpszFilePath;
		//int *lpszFilePath;
		//CString *p_FilePath;
		int *p_FilePath;

		//dr_mcontext_t *mc;
		CString newfilepath;
		CString fp;

		//wchar_t *lpszFilePath_obj; 
		
		//newfilepath = _T("c:\\out\\s.cur_input");


	if (options.debug_mode){
				
		stdcall_args_size = 4*1;

		lpszFilePath = (wchar_t *)malloc(128);

		memset(lpszFilePath, 0, 128);

		//wcscpy(lpszFilePath, (wchar_t *)("c:\\out\\s.cur_input"));
		//wcscpy(lpszFilePath, _T("c:\\out\\s.cur_input"));
		wcscpy(lpszFilePath , L"c:\\out\\afl.input");
		

		p_FilePath = (int *)drwrap_get_arg(wrapcxt, 0);

		*p_FilePath = (int)lpszFilePath;


		dr_fprintf(winafl_data.log,  "In GetPathName :%x %x\n",p_FilePath, *p_FilePath);


		//drwrap_set_arg(wrapcxt, 0, p_FilePath);
		retval = 0;


		/*
		mc = drwrap_get_mcontext(wrapcxt);
		mc->eax = (reg_t) p_FilePath;

		drwrap_set_mcontext(mc);

		*/

		drwrap_skip_call(wrapcxt, retval, stdcall_args_size);

	}

}

static void
CFile_GetLength_interceptor(void *wrapcxt, INOUT void **user_data)
{
		void * retval;
		int ret = 1;
		size_t stdcall_args_size;
		dr_mcontext_t * mc;
		
	if (options.debug_mode){
				
		stdcall_args_size = 0;

		retval = (void *)3;
		
		mc = drwrap_get_mcontext(wrapcxt);
		mc->eax = 6;
		
		
		drwrap_set_mcontext(mc);

		drwrap_skip_call(wrapcxt, retval, stdcall_args_size);
	}
}


static void
CString_Construct_interceptor(void *wrapcxt, INOUT void **user_data){
	void * retval;
	int ret = 0;
	// size_t stdcall_args_size=4*2; // ~Cstring stdcall_args_size = -4;
	size_t stdcall_args_size=4*1;
	app_pc pc;
		
	if (options.debug_mode){

		pc =  drwrap_get_func(wrapcxt);

		retval = (void *)ret;

		// modify second param on stack   
		drwrap_set_arg(wrapcxt, 1, drwrap_get_arg(wrapcxt, 2));


		if ( npc > 0x404e30 && npc < 0x404f30 ){
		//if ( start_skip ){
			dr_fprintf(winafl_data.log, "in MyFileOpen CStringT, skip it 0x%x\n", npc);

			drwrap_skip_call(wrapcxt, retval, stdcall_args_size);
		}

	}
}


static void
CString_Destruct_interceptor(void *wrapcxt, INOUT void **user_data){

	void * retval;
	int ret = 1;
	size_t stdcall_args_size=0;

	app_pc pc = 0;

	retval = (void *)ret;
	//pc = drwrap_get_func(wrapcxt);

	//if ( npc > 0x404e30 && npc < 0x404f30){
	//if ( start_skip ){

	if (options.debug_mode){
		dr_fprintf(winafl_data.log, "in ~CStringT, skip it 0x%x\n", npc);
	}

	drwrap_skip_call(wrapcxt, retval, stdcall_args_size);

}


static void
skip_destructor_cstringt_interceptor2(void *wrapcxt, INOUT void **user_data){
	
	void * retval;
	int ret = 1;
	size_t stdcall_args_size=4*0;
		

	if (options.debug_mode){

		dr_fprintf(winafl_data.log, "in ~CStringT call, skip it\n");

		retval = (void *)ret;


		drwrap_skip_call(wrapcxt, retval, stdcall_args_size);

	}
}

static void
CFile_Destruct_interceptor(void *wrapcxt, INOUT void **user_data){
	
	void * retval;
	int ret = 1;
	size_t stdcall_args_size=0;

	app_pc pc = 0;

	if (options.debug_mode){

		retval = (void *)ret;

		//pc =  drwrap_get_func(wrapcxt);

		//if ( npc > 0x404e30 && npc < 0x404f30){
		if ( start_skip ){

			dr_fprintf(winafl_data.log, "in MyFileOpen ~CFile, skip it 0x%x\n", npc);

			drwrap_skip_call(wrapcxt, retval, stdcall_args_size);
		}

		// free afl_input
	}
}


const wchar_t *GetWC(const char *c)
{
    const size_t cSize = strlen(c)+1;
    wchar_t* wc = new wchar_t[cSize];
    mbstowcs (wc, c, cSize);

    return wc;
}

static void
CWND_GetWindowTextW_interceptor(void *wrapcxt, INOUT void **user_data){
	void * retval;
	int ret = 1;
	int *cstring_stack_addr; // cstring on heap, it's address on stack, stack addr is cstring_stack_addr
	//wchar_t **string_addr;
	dr_mcontext_t *mc;

	size_t stdcall_args_size=4;
	app_pc pc = 0;

	char str[4096] = {0};
	
	mfc_ui_input_index %= mfc_ui_input_count;

	//TODO: need a input info table
	/*
	int mfc_ui_input_length_table[3] = {1, 1, 1};
	int start_pos = 0;
	int i = 0;
	for (;i < mfc_ui_input_index; i++){
		start_pos += mfc_ui_input_length_table[i];
	}
	*/

	const char *testcase_path;
	// TODO: need a input info table ?

	//options.par_id == mfc_ui_input_index;

	if (mfc_ui_input_index == 0){
		testcase_path = "c:\\out\\testcase01.txt";
	}else if (mfc_ui_input_index == 1){
		testcase_path = "c:\\out\\testcase03.txt";
	}else if (mfc_ui_input_index == 2){ 
		testcase_path = "c:\\out\\testcase02.txt";
	}

	read_afl_input(testcase_path);

	/*
	if (mfc_ui_input_index == 0){
		memcpy(str, global_afl_input, 1);
	}else if (mfc_ui_input_index == 1){
		memcpy(str, global_afl_input+2, 1);
	}else if (mfc_ui_input_index == 2){ 
		memcpy(str, global_afl_input+1, 1);
	}
*/
	mfc_ui_input_index++;

	

	/*
	lpszEditContent = (wchar_t *)malloc(128);
	memset(lpszEditContent, 0, 128);
	wcscpy(lpszEditContent , GetWC(str));
	*/

	//MessageBoxW(NULL, GetWC(str), L"afl.input", 0);
	//dr_fprintf(winafl_data.log, "in cstring_stack_addr cur.input: %s\n", str);


	mc = drwrap_get_mcontext(wrapcxt);
	//cstring_stack_addr = (int *)*(int*)(mc->esp + 4); // local var point to cstring
	cstring_stack_addr = (int *)drwrap_get_arg(wrapcxt, 0); // the same

	//wcscpy((wchar_t*)*cstring_stack_addr , lpszEditContent);
	//wcscpy((wchar_t*)*cstring_stack_addr , GetWC(str));
	wcscpy((wchar_t*)*cstring_stack_addr , GetWC(global_afl_input));

	dr_fprintf(winafl_data.log, "in CWND_GetWindowText_interceptor input %d:%s:%s \n", \
				mfc_ui_input_index, testcase_path, global_afl_input);

	drwrap_skip_call(wrapcxt, retval, stdcall_args_size);
}


static void
CWND_SetWindowTextW_interceptor(void *wrapcxt, INOUT void **user_data){
	void * retval = 0;
	size_t stdcall_args_size=4;
	int length = 0;
	int CFG_MAX_EDIT_LENGTH = 1024; // TODO

	const LPCWSTR buffer = (LPCWSTR) drwrap_get_arg(wrapcxt, 0); // TODO ansi
	if (buffer)
		length = wcslen(buffer);

	if (options.debug_mode)
		dr_fprintf(winafl_data.log, "in CWND_SetWindowTextW_interceptor, buffer length: %d skip it\n", length);

	if (length > CFG_MAX_EDIT_LENGTH)
		drwrap_skip_call(wrapcxt, retval, stdcall_args_size);
}

// socket

static void
AfxSocketInit_interceptor(void *wrapcxt, INOUT void **user_data){
	int retval = 1;
	size_t stdcall_args_size= 4 * 1;

	if (options.debug_mode)
		dr_fprintf(winafl_data.log, "in AfxSocketInit_interceptor\n");

	drwrap_skip_call(wrapcxt, (void *)retval, stdcall_args_size);
}

static void
CAsyncSocket_Create_interceptor(void *wrapcxt, INOUT void **user_data){
	int retval = 1;
	size_t stdcall_args_size= 4 * 4;

	if (options.debug_mode)
		dr_fprintf(winafl_data.log, "in CAsyncSocket_Create_interceptor\n");

	drwrap_skip_call(wrapcxt, (void *)retval, stdcall_args_size);
}

static void
CAsyncSocket_Socket_interceptor(void *wrapcxt, INOUT void **user_data){
	int retval = 1;
	size_t stdcall_args_size= 4 * 4;

	if (options.debug_mode)
		dr_fprintf(winafl_data.log, "in CAsyncSocket_CSocket_interceptor\n");

	drwrap_skip_call(wrapcxt, (void *)retval, stdcall_args_size);
}


static void
CAsyncSocket_Bind_interceptor(void *wrapcxt, INOUT void **user_data){
	int retval = 1;
	size_t stdcall_args_size= 4 * 2;

	if (options.debug_mode)
		dr_fprintf(winafl_data.log, "in CAsyncSocket_Bind_interceptor\n");

	drwrap_skip_call(wrapcxt, (void *)retval, stdcall_args_size);
}

static void
CAsyncSocket_Listen_interceptor(void *wrapcxt, INOUT void **user_data){
	int retval = 1;
	size_t stdcall_args_size= 4 * 1;

	if (options.debug_mode)
		dr_fprintf(winafl_data.log, "in CAsyncSocket_Listen_interceptor\n");

	drwrap_skip_call(wrapcxt, (void *)retval, stdcall_args_size);
}

static void
CAsyncSocket_Connect_interceptor(void *wrapcxt, INOUT void **user_data){
	int retval = 1;
	size_t stdcall_args_size= 4 * 2;

	if (options.debug_mode)
		dr_fprintf(winafl_data.log, "in CAsyncSocket_Connect_interceptor\n");

	drwrap_skip_call(wrapcxt, (void *)retval, stdcall_args_size);
}

static void
CAsyncSocket_Accept_interceptor(void *wrapcxt, INOUT void **user_data){
	int retval = 1;
	size_t stdcall_args_size= 4 * 3;

	if (options.debug_mode)
		dr_fprintf(winafl_data.log, "in CAsyncSocket_Accept_interceptor\n");

	drwrap_skip_call(wrapcxt, (void *)retval, stdcall_args_size);
}

static void
CAsyncSocket_Receive_interceptor(void *wrapcxt, INOUT void **user_data){
	int retval = 1;
	size_t stdcall_args_size= 4 * 3;
	char *buf = (char *) drwrap_get_arg(wrapcxt, 0);
	int buf_size = (int) drwrap_get_arg(wrapcxt, 1);

	int testcase_len = read_afl_input(afl_input_path);
	int copy_len = buf_size > testcase_len? testcase_len : buf_size;

	memcpy(buf, global_afl_input, copy_len);

	retval = copy_len;

	if (options.debug_mode)
		dr_fprintf(winafl_data.log, "in CAsyncSocket_Receive_interceptor recv %d\n", retval);

	drwrap_skip_call(wrapcxt, (void *)retval, stdcall_args_size);
}

static void
CAsyncSocket_Send_interceptor(void *wrapcxt, INOUT void **user_data){
	int retval = 1;
	size_t stdcall_args_size= 4 * 3;
	retval = (int) drwrap_get_arg(wrapcxt, 1);

	if (options.debug_mode)
		dr_fprintf(winafl_data.log, "in CAsyncSocket_Send_interceptor, send %d\n", retval);

	drwrap_skip_call(wrapcxt, (void *)retval, stdcall_args_size);
}

static void
CAsyncSocket_Sendto_interceptor(void *wrapcxt, INOUT void **user_data){
	int retval = 0;
	size_t stdcall_args_size= 4 * 5;
	retval = (int) drwrap_get_arg(wrapcxt, 1);

	if (options.debug_mode)
		dr_fprintf(winafl_data.log, "in CAsyncSocket_Sendto_interceptor, send %d\n", retval);

	drwrap_skip_call(wrapcxt, (void *)retval, stdcall_args_size);
}

static void
CAsyncSocket_RecvFrom_interceptor(void *wrapcxt, INOUT void **user_data){
	int retval = 0;
	size_t stdcall_args_size= 4 * 5;
	char *buf = (char *) drwrap_get_arg(wrapcxt, 0);
	int buf_size = (int) drwrap_get_arg(wrapcxt, 1);

	int testcase_len = read_afl_input(afl_input_path);
	int copy_len = buf_size > testcase_len? testcase_len : buf_size;

	memcpy(buf, global_afl_input, copy_len);

	retval = copy_len;

	if (options.debug_mode)
		dr_fprintf(winafl_data.log, "in CAsyncSocket_RecvFrom_interceptor, recv %d\n", copy_len);

	drwrap_skip_call(wrapcxt, (void *)retval, stdcall_args_size);
}

static void
CAsyncSocket_Attach_interceptor(void *wrapcxt, INOUT void **user_data){
	int retval = 1;
	size_t stdcall_args_size= 4 * 2;

	if (options.debug_mode)
		dr_fprintf(winafl_data.log, "in CAsyncSocket_Attach_interceptor\n");

	drwrap_skip_call(wrapcxt, (void *)retval, stdcall_args_size);
}

static void
CAsyncSocket_Detach_interceptor(void *wrapcxt, INOUT void **user_data){
	int retval = 1;
	size_t stdcall_args_size= 4 * 0;

	if (options.debug_mode)
		dr_fprintf(winafl_data.log, "in CAsyncSocket_Detach_interceptor\n");

	drwrap_skip_call(wrapcxt, (void *)retval, stdcall_args_size);
}

static void
CAsyncSocket_Close_interceptor(void *wrapcxt, INOUT void **user_data){
	int retval = 1;
	size_t stdcall_args_size= 4 * 0;

	if (options.debug_mode)
		dr_fprintf(winafl_data.log, "in CAsyncSocket_Close_interceptor\n");

	drwrap_skip_call(wrapcxt, (void *)retval, stdcall_args_size);
}

static void
CSocket_Close_interceptor(void *wrapcxt, INOUT void **user_data){
	int retval = 1;
	size_t stdcall_args_size= 4 * 0;

	if (options.debug_mode)
		dr_fprintf(winafl_data.log, "in CSocket_Close_interceptor\n");

	drwrap_skip_call(wrapcxt, (void *)retval, stdcall_args_size);
}


static void
isprocessorfeaturepresent_interceptor_pre(void *wrapcxt, INOUT void **user_data)
{
    DWORD feature = (DWORD)drwrap_get_arg(wrapcxt, 0);
    *user_data = (void*)feature;
}

static void
isprocessorfeaturepresent_interceptor_post(void *wrapcxt, void *user_data)
{
    DWORD feature = (DWORD)user_data;
    if(feature == PF_FASTFAIL_AVAILABLE) {
        if(options.debug_mode) {
            dr_fprintf(winafl_data.log, "About to make IsProcessorFeaturePresent(%d) returns 0\n", feature);
        }

        // Make the software thinks that _fastfail() is not supported.
        drwrap_set_retval(wrapcxt, (void*)0);
    }
}

static void
unhandledexceptionfilter_interceptor_pre(void *wrapcxt, INOUT void **user_data)
{
    PEXCEPTION_POINTERS exception = (PEXCEPTION_POINTERS)drwrap_get_arg(wrapcxt, 0);
    dr_exception_t dr_exception = { 0 };

    // Fake an exception
    dr_exception.record = exception->ExceptionRecord;
    onexception(NULL, &dr_exception);
}

static void
event_module_unload(void *drcontext, const module_data_t *info)
{
    module_table_unload(module_table, info);
}


static void
event_module_load(void *drcontext, const module_data_t *info, bool loaded)
{
	bool result;
	int addr, module_base;
	app_pc to_wrap;
	app_pc module_entry;
    const char *module_name = info->names.exe_name;
	module_entry = info->entry_point;
    
    if (module_name == NULL) {
        // In case exe_name is not defined, we will fall back on the preferred name.
        module_name = dr_module_preferred_name(info);
    }

	module_base = ini_get_int(interceptor_config, module_name, "module_base");

    if(options.debug_mode)
        dr_fprintf(winafl_data.log, "Module loaded, name: %s start: 0x%x, base: 0x%x\n", module_name, info->start, module_entry);

    if(options.fuzz_module[0]) {

		
        if(strcmp(module_name, options.fuzz_module) == 0) {
            if(options.fuzz_offset) {
                to_wrap = info->start + options.fuzz_offset;
            } else {
                //first try exported symbols
                to_wrap = (app_pc)dr_get_proc_address(info->handle, options.fuzz_method);
                if(!to_wrap) {
                    //if that fails, try with the symbol access library
#ifdef USE_DRSYMS
                    drsym_init(0);
                    drsym_lookup_symbol(info->full_path, options.fuzz_method, (size_t *)(&to_wrap), 0);
                    drsym_exit();
#endif
                    DR_ASSERT_MSG(to_wrap, "Can't find specified method in fuzz_module");                
                    to_wrap += (size_t)info->start;
                }
            }
			if (options.persistence_mode == native_mode)
			{
				drwrap_wrap_ex(to_wrap, pre_fuzz_handler, post_fuzz_handler, NULL, options.callconv);
			}
			if (options.persistence_mode == in_app)
			{
				drwrap_wrap_ex(to_wrap, pre_loop_start_handler, NULL, NULL, options.callconv);
			}
        }

        if (strcmp(module_name, "WS2_32.dll") == 0) {

			addr = ini_get_int(interceptor_config, module_name, "WSAStartup");

			if (!to_wrap){
				to_wrap = (app_pc)dr_get_proc_address(info->handle, "WSAStartup");
				result = drwrap_wrap(to_wrap, WSAStartup_interceptor, NULL);
			}

			addr = ini_get_int(interceptor_config, module_name, "bind");
			if (addr) {
				to_wrap = (app_pc)dr_get_proc_address(info->handle, "bind");
				result = drwrap_wrap(to_wrap, bind_interceptor, NULL); 
			}
			
			addr = ini_get_int(interceptor_config, module_name, "listen");
			if (addr) {
				to_wrap = (app_pc)dr_get_proc_address(info->handle, "listen");
				result = drwrap_wrap(to_wrap, listen_interceptor, NULL); 
			}

			addr = ini_get_int(interceptor_config, module_name, "connect");
			if (addr) {
				to_wrap = (app_pc)dr_get_proc_address(info->handle, "connect");
				result = drwrap_wrap(to_wrap, connect_interceptor, NULL); 
			}

			addr = ini_get_int(interceptor_config, module_name, "accept");
			if (addr) {
				to_wrap = (app_pc)dr_get_proc_address(info->handle, "accept");
				result = drwrap_wrap(to_wrap, accept_interceptor, NULL); 
			}

			addr = ini_get_int(interceptor_config, module_name, "recv");
			if (addr) {
				to_wrap = (app_pc)dr_get_proc_address(info->handle, "recv");
				result = drwrap_wrap(to_wrap, recv_interceptor, NULL); 
			}

			addr = ini_get_int(interceptor_config, module_name, "send");
			if (addr) {
				to_wrap = (app_pc)dr_get_proc_address(info->handle, "send");
				result = drwrap_wrap(to_wrap, send_interceptor, NULL);
			}

			addr = ini_get_int(interceptor_config, module_name, "sendto");
			if (addr) {
				to_wrap = (app_pc)dr_get_proc_address(info->handle, "sendto");
				result = drwrap_wrap(to_wrap, sendto_interceptor, NULL); 
			}

			addr = ini_get_int(interceptor_config, module_name, "recvfrom");
			if (addr) {
				to_wrap = (app_pc)dr_get_proc_address(info->handle, "recvfrom");
				result = drwrap_wrap(to_wrap, recvfrom_interceptor, NULL); 
			}

			addr = ini_get_int(interceptor_config, module_name, "closesocket");
			if (addr) {
				to_wrap = (app_pc)dr_get_proc_address(info->handle, "closesocket");
				result = drwrap_wrap(to_wrap, closesocket_interceptor, NULL);
			}

			addr = ini_get_int(interceptor_config, module_name, "shutdown");
			if (addr) {
				to_wrap = (app_pc)dr_get_proc_address(info->handle, "shutdown");
				result = drwrap_wrap(to_wrap, shutdown_interceptor, NULL);
			}
			//WSACleanup cost too much time
			addr = ini_get_int(interceptor_config, module_name, "WSACleanup");
			if (addr) {
				to_wrap = (app_pc)dr_get_proc_address(info->handle, "WSACleanup");
				result = drwrap_wrap(to_wrap, WSACleanup_interceptor, NULL);
			}

        }

        if(strcmp(module_name, "KERNEL32.dll") == 0) {
			addr = ini_get_int(interceptor_config, module_name, "CreateFileW");
			if (addr) {
				to_wrap = (app_pc)dr_get_proc_address(info->handle, "CreateFileW");
				result = drwrap_wrap(to_wrap, createfilew_interceptor, NULL);
			}

			addr = ini_get_int(interceptor_config, module_name, "CreateFileA");
			if (addr) {
				to_wrap = (app_pc)dr_get_proc_address(info->handle, "CreateFileA");
				result = drwrap_wrap(to_wrap, createfilea_interceptor, NULL);
			}

        }

		if (strcmp(module_name, "MSVCR100.dll") == 0) {
			addr = ini_get_int(interceptor_config, module_name, "strncmp");
			if (addr) {
				to_wrap = (app_pc)dr_get_proc_address(info->handle, "strncmp");
				result = drwrap_wrap(to_wrap, strncmp_interceptor, NULL); 
			}
		}



		if (strcmp(module_name, "mfc100u.dll") == 0) {

			dr_fprintf(winafl_data.log, "Module loaded, name: %s module_base: 0x%x start: 0x%x\n", module_name, module_base, info->start);

			addr = ini_get_int(interceptor_config, module_name, "CFileDialog_doModal");
			if (addr) {
				to_wrap = (app_pc)(addr - (int)module_base + (int)info->start);
				result = drwrap_wrap(to_wrap, CFileDialog_doModal_interceptor, NULL);
			}

			addr = ini_get_int(interceptor_config, module_name, "CFileDialog_GetPathName");
			if (addr) {
				to_wrap = (app_pc)(addr - (int)module_base + (int)info->start);
				result = drwrap_wrap(to_wrap, CFileDialog_GetPathName_interceptor, NULL);
			}

			addr = ini_get_int(interceptor_config, module_name, "CFileDialog_GetPathName");
			if (addr) {
				to_wrap = (app_pc)(addr - (int)module_base + (int)info->start);
				result = drwrap_wrap(to_wrap, CFileDialog_GetPathName_interceptor, NULL);
			}

			addr = ini_get_int(interceptor_config, module_name, "CString_Construct");
			if (addr) {
				to_wrap = (app_pc)(addr - (int)module_base + (int)info->start);
				result = drwrap_wrap(to_wrap, CString_Construct_interceptor, NULL);
			}

			addr = ini_get_int(interceptor_config, module_name, "CString_Destruct");
			if (addr) {
				to_wrap = (app_pc)(addr - (int)module_base + (int)info->start);
				result = drwrap_wrap(to_wrap, CString_Destruct_interceptor, NULL);
			}

			addr = ini_get_int(interceptor_config, module_name, "CFile_Open");
			if (addr) {
				to_wrap = (app_pc)(addr - (int)module_base + (int)info->start);
				result = drwrap_wrap(to_wrap, CFile_Open_interceptor, NULL);
			}
				
			addr = ini_get_int(interceptor_config, module_name, "CFile_Destruct");
			if (addr) {
				to_wrap = (app_pc)(addr - (int)module_base + (int)info->start);
				result = drwrap_wrap(to_wrap, CFile_Destruct_interceptor, NULL);
			}

			addr = ini_get_int(interceptor_config, module_name, "CFile_GetLength");
			if (addr) {
				to_wrap = (app_pc)(addr - (int)module_base + (int)info->start);
				result = drwrap_wrap(to_wrap, CFile_Destruct_interceptor, NULL);
			}

			addr = ini_get_int(interceptor_config, module_name, "CWND_GetWindowTextW");
			if (addr) {
				to_wrap = (app_pc)(addr - (int)module_base + (int)info->start);
				to_wrap = (app_pc)(addr - (int)module_base + (int)info->start);
				result = drwrap_wrap(to_wrap, CWND_GetWindowTextW_interceptor, NULL);
			}

			addr = ini_get_int(interceptor_config, module_name, "CWND_SetWindowTextW");
			if (addr) {
				to_wrap = (app_pc)(addr - (int)module_base + (int)info->start);
				result = drwrap_wrap(to_wrap,CWND_SetWindowTextW_interceptor, NULL);
			}

			addr = ini_get_int(interceptor_config, module_name, "AfxSocketInit");
			if (addr) {
				to_wrap = (app_pc)(addr - (int)module_base + (int)info->start);
				result = drwrap_wrap(to_wrap, AfxSocketInit_interceptor, NULL);
			}

			addr = ini_get_int(interceptor_config, module_name, "CAsyncSocket_Socket");
			if (addr) {
				to_wrap = (app_pc)(addr - (int)module_base + (int)info->start);
				result = drwrap_wrap(to_wrap, CAsyncSocket_Socket_interceptor, NULL);
			}
			
			addr = ini_get_int(interceptor_config, module_name, "CAsyncSocket_Create");
			if (addr) {
				to_wrap = (app_pc)(addr - (int)module_base + (int)info->start);
				result = drwrap_wrap(to_wrap, CAsyncSocket_Create_interceptor, NULL);
			}
			
			addr = ini_get_int(interceptor_config, module_name, "CAsyncSocket_Bind");
			if (addr) {
				to_wrap = (app_pc)(addr - (int)module_base + (int)info->start);
				result = drwrap_wrap(to_wrap, CAsyncSocket_Bind_interceptor, NULL);
			}

			addr = ini_get_int(interceptor_config, module_name, "CAsyncSocket_Listen");
			if (addr) {
				to_wrap = (app_pc)(addr - (int)module_base + (int)info->start);
				result = drwrap_wrap(to_wrap, CAsyncSocket_Listen_interceptor, NULL);
			}

			addr = ini_get_int(interceptor_config, module_name, "CAsyncSocket_Connect");
			if (addr) {
				to_wrap = (app_pc)(addr - (int)module_base + (int)info->start);
				result = drwrap_wrap(to_wrap, CAsyncSocket_Connect_interceptor, NULL);
			}

			addr = ini_get_int(interceptor_config, module_name, "CAsyncSocket_Accept");
			if (addr) {
				to_wrap = (app_pc)(addr - (int)module_base + (int)info->start);
				result = drwrap_wrap(to_wrap, CAsyncSocket_Accept_interceptor, NULL);
			}
			
			addr = ini_get_int(interceptor_config, module_name, "CAsyncSocket_Send");
			if (addr) {
				to_wrap = (app_pc)(addr - (int)module_base + (int)info->start);
				result = drwrap_wrap(to_wrap, CAsyncSocket_Send_interceptor, NULL);
			}

			addr = ini_get_int(interceptor_config, module_name, "CAsyncSocket_Sendto");
			if (addr) {
				to_wrap = (app_pc)(addr - (int)module_base + (int)info->start);
				result = drwrap_wrap(to_wrap, CAsyncSocket_Sendto_interceptor, NULL);
			}

			addr = ini_get_int(interceptor_config, module_name, "CAsyncSocket_Receive");
			if (addr) {
				to_wrap = (app_pc)(addr - (int)module_base + (int)info->start);
				result = drwrap_wrap(to_wrap, CAsyncSocket_Receive_interceptor, NULL);
			}

			// Attach -> AttachHandle, AsyncSelect
			addr = ini_get_int(interceptor_config, module_name, "CAsyncSocket_Attach");
			if (addr) {
				to_wrap = (app_pc)(addr - (int)module_base + (int)info->start);
				result = drwrap_wrap(to_wrap, CAsyncSocket_Attach_interceptor, NULL);
			}

			addr = ini_get_int(interceptor_config, module_name, "CAsyncSocket_Detach");
			if (addr) {
				to_wrap = (app_pc)(addr - (int)module_base + (int)info->start);
				result = drwrap_wrap(to_wrap, CAsyncSocket_Detach_interceptor, NULL);
			}

			addr = ini_get_int(interceptor_config, module_name, "CAsyncSocket_Close");
			if (addr) {
				to_wrap = (app_pc)(addr - (int)module_base + (int)info->start);
				result = drwrap_wrap(to_wrap, CAsyncSocket_Close_interceptor, NULL);
			}

			addr = ini_get_int(interceptor_config, module_name, "CSocket_Close");
			if (addr) {
				to_wrap = (app_pc)(addr - (int)module_base + (int)info->start);
				result = drwrap_wrap(to_wrap, CSocket_Close_interceptor, NULL);
			}
			
		}

		if (strcmp(module_name, "USER32.dll") == 0) {

			addr = ini_get_int(interceptor_config, module_name, "MessageBoxW");
			if (addr) {
				to_wrap = (app_pc)dr_get_proc_address(info->handle, "MessageBoxW");
				result = drwrap_wrap(to_wrap, messagebox_interceptor, NULL);
			}

			addr = ini_get_int(interceptor_config, module_name, "SendDlgItemMessageW");
			if (addr) {
				to_wrap = (app_pc)dr_get_proc_address(info->handle, "SendDlgItemMessageW");
				result = drwrap_wrap(to_wrap, SendDlgItemMessageW_interceptor, NULL);
			}
		}

		if (strcmp(module_name, "QtGui4.dll") == 0) {
			
			addr = ini_get_int(interceptor_config, module_name, "QFileDialog_getOpenFileName");
			if (addr) {
				to_wrap = (app_pc)(addr - (int)module_base + (int)info->start);
				dr_fprintf(winafl_data.log, "QFileDialog_getOpenFileName 0x%x\n", to_wrap);
				result = drwrap_wrap(to_wrap, Qt4_QFileDialog_getOpenFileName_interceptor, NULL);
			}

			addr = ini_get_int(interceptor_config, module_name, "QFileDialog_getOpenFileNames");
			if (addr) {
				to_wrap = (app_pc)(addr - (int)module_base + (int)info->start);
				result = drwrap_wrap(to_wrap, Qt4_QFileDialog_getOpenFileNames_interceptor, NULL);
			}

			addr = ini_get_int(interceptor_config, module_name, "QMessageBox_information");
			if (addr) {
				to_wrap = (app_pc)(addr - (int)module_base + (int)info->start);
				result = drwrap_wrap(to_wrap, Qt4_QMessageBox_information_interceptor, NULL);
			}

			addr = ini_get_int(interceptor_config, module_name, "QMessageBox_warnning");
			if (addr) {
				to_wrap = (app_pc)(addr - (int)module_base + (int)info->start);
				result = drwrap_wrap(to_wrap, Qt4_QMessageBox_warnning_interceptor, NULL);
			}

			//QInputDialog
			addr = ini_get_int(interceptor_config, module_name, "QInputDialog_getItem");
			if (addr) {
				to_wrap = (app_pc)(addr - (int)module_base + (int)info->start);
				result = drwrap_wrap(to_wrap, Qt4_QInputDialog_getItem_interceptor, NULL);
			}

			//Other Dialogs
			addr = ini_get_int(interceptor_config, module_name, "QColorDialog_getColor");
			if (addr) {
				to_wrap = (app_pc)(addr - (int)module_base + (int)info->start);
				result = drwrap_wrap(to_wrap, Qt4_QColorDialog_getColor_interceptor, NULL);
			}

			// print
			addr = ini_get_int(interceptor_config, module_name, "QPrintDialog_exec");
			if (addr) {
				to_wrap = (app_pc)(addr - (int)module_base + (int)info->start);
				result = drwrap_wrap(to_wrap, Qt4_QPrintDialog_exec_interceptor, NULL);
			}

			addr = ini_get_int(interceptor_config, module_name, "QInputDialog_getDouble");
			if (addr) {
				to_wrap = (app_pc)(addr - (int)module_base + (int)info->start);
				result = drwrap_wrap(to_wrap, Qt4_QInputDialog_getDouble_interceptor, NULL);
			}
			//double
			addr = ini_get_int(interceptor_config, module_name, "QDoubleSpinBox_value");
			if (addr) {
				to_wrap = (app_pc)(addr - (int)module_base + (int)info->start);
				result = drwrap_wrap(to_wrap, Qt4_QDoubleSpinBox_value_interceptor, NULL);
			}

			//datetime
			addr = ini_get_int(interceptor_config, module_name, "QDateTimeEdit_date");
			if (addr) {
				to_wrap = (app_pc)(addr - (int)module_base + (int)info->start);
				result = drwrap_wrap(to_wrap, Qt4_QDateTimeEdit_date_interceptor, NULL);
			}
			addr = ini_get_int(interceptor_config, module_name, "QDateTimeEdit_time");
			if (addr) {
				to_wrap = (app_pc)(addr - (int)module_base + (int)info->start);
				result = drwrap_wrap(to_wrap, Qt4_QDateTimeEdit_time_interceptor, NULL);
			}
			addr = ini_get_int(interceptor_config, module_name, "QDateTimeEdit_dateTime");
			if (addr) {
				to_wrap = (app_pc)(addr - (int)module_base + (int)info->start);
				result = drwrap_wrap(to_wrap, Qt4_QDateTimeEdit_dateTime_interceptor, NULL);
			}

			//list
			addr = ini_get_int(interceptor_config, module_name, "QComboBox_currentText");
			if (addr) {
				to_wrap = (app_pc)(addr - (int)module_base + (int)info->start);
				result = drwrap_wrap(to_wrap, Qt4_QComboBox_currentText_interceptor, NULL);
			}
			addr = ini_get_int(interceptor_config, module_name, "QComboBox_currentIndex");
			if (addr) {
				to_wrap = (app_pc)(addr - (int)module_base + (int)info->start);
				result = drwrap_wrap(to_wrap, Qt4_QComboBox_currentIndex_interceptor, NULL);
			}

			//socket
			addr = ini_get_int(interceptor_config, module_name, "QUdpSocket_hasPendingDatagrams");
			if (addr) {
				to_wrap = (app_pc)(addr - (int)module_base + (int)info->start);
				result = drwrap_wrap(to_wrap, Qt4_QUdpSocket_hasPendingDatagrams_interceptor, NULL);
			}
			addr = ini_get_int(interceptor_config, module_name, "QUdpSocket_pendingDatagramSize");
			if (addr) {
				to_wrap = (app_pc)(addr - (int)module_base + (int)info->start);
				result = drwrap_wrap(to_wrap, Qt4_QUdpSocket_pendingDatagramSize_interceptor, NULL);
			}
			addr = ini_get_int(interceptor_config, module_name, "QUdpSocket_readDatagram");
			if (addr) {
				to_wrap = (app_pc)(addr - (int)module_base + (int)info->start);
				result = drwrap_wrap(to_wrap, Qt4_QUdpSocket_readDatagram_interceptor, NULL);
			}
			addr = ini_get_int(interceptor_config, module_name, "QUdpSocket_bind");
			if (addr) {
				to_wrap = (app_pc)(addr - (int)module_base + (int)info->start);
				result = drwrap_wrap(to_wrap, Qt4_QUdpSocket_bind_interceptor, NULL);
			}

		}

		if (strcmp(module_name, "QtCore4.dll") == 0) {
			

			addr = ini_get_int(interceptor_config, module_name, "QString_free");
			if (addr) {
				to_wrap = (app_pc)(addr - (int)module_base + (int)info->start);
				dr_fprintf(winafl_data.log, "QString_free 0x%x\n", to_wrap);
				result = drwrap_wrap(to_wrap, Qt4_QString_free_interceptor, NULL);
			}


			addr = ini_get_int(interceptor_config, module_name, "Qfile_QFile");
			if (addr) {
				to_wrap = (app_pc)(addr - (int)module_base + (int)info->start);
				dr_fprintf(winafl_data.log, "Qfile_QFile 0x%x\n", to_wrap);
				result = drwrap_wrap(to_wrap, Qt4_QFile_QFile_interceptor, NULL);
			}

		}
		/*
		if (strcmp(module_name, "Qt5Core.dll") == 0) {
			to_wrap = (app_pc)dr_get_proc_address(info->handle, "??0QFile@@QAE@ABVQString@@@Z");
			drwrap_wrap(to_wrap, Qt5_QFile_interceptor, NULL);
			dr_fprintf(winafl_data.log, "wrap, QFile: %x\n", to_wrap);
		}
		
		//QFileDialog::getOpenFileName(QWidget *,QString const &,QString const &,QString const &,QString *,QFlags<QFileDialog::Option>)	Qt5Widgets
		if (options.debug_mode && (strcmp(module_name, "Qt5Widgets.dll") == 0)) {
			to_wrap = (app_pc)dr_get_proc_address(info->handle, "?getOpenFileName@QFileDialog@@SA?AVQString@@PAVQWidget@@ABV2@11PAV2@V?$QFlags@W4Option@QFileDialog@@@@@Z");
			drwrap_wrap(to_wrap, Qt5_getOpenFileName_interceptor, NULL);
			dr_fprintf(winafl_data.log, "wrap, getOpenFileName: %x\n", to_wrap);
		}
		*/


        if(_stricmp(module_name, "kernelbase.dll") == 0) {
            // Since Win8, software can use _fastfail() to trigger an exception that cannot be caught.
            // This is a problem for winafl as it also means DR won't be able to see it. Good thing is that
            // usually those routines (__report_gsfailure for example) accounts for platforms that don't
            // have support for fastfail. In those cases, they craft an exception record and pass it
            // to UnhandledExceptionFilter.
            //
            // To work around this we set up two hooks:
            //   (1) IsProcessorFeaturePresent(PF_FASTFAIL_AVAILABLE): to lie and pretend that the
            //       platform doesn't support fastfail.
            //   (2) UnhandledExceptionFilter: to intercept the exception record and forward it
            //       to winafl's exception handler.
            to_wrap = (app_pc)dr_get_proc_address(info->handle, "IsProcessorFeaturePresent");
            drwrap_wrap(to_wrap, isprocessorfeaturepresent_interceptor_pre, isprocessorfeaturepresent_interceptor_post);
            to_wrap = (app_pc)dr_get_proc_address(info->handle, "UnhandledExceptionFilter");
            drwrap_wrap(to_wrap, unhandledexceptionfilter_interceptor_pre, NULL);
        }
    }

    if (_stricmp(module_name, "verifier.dll") == 0) {
        to_wrap = (app_pc)dr_get_proc_address(info->handle, "VerifierStopMessage");
        drwrap_wrap(to_wrap, verfierstopmessage_interceptor_pre, NULL);
    }
	 
    module_table_load(module_table, info);
}



static void
event_exit(void)
{
    if(options.debug_mode) {
        if(debug_data.pre_hanlder_called == 0) {
            dr_fprintf(winafl_data.log, "WARNING: Target function was never called. Incorrect target_offset?\n");
        } else if(debug_data.post_handler_called == 0) {
            dr_fprintf(winafl_data.log, "WARNING: Post-fuzz handler was never reached. Did the target function return normally?\n");
        } else {
            dr_fprintf(winafl_data.log, "Everything appears to be running normally.\n");
        }

        dr_fprintf(winafl_data.log, "Coverage map follows:\n");
        //dump_winafl_data();
        dr_close_file(winafl_data.log);
    }

    /* destroy module table */
    module_table_destroy(module_table);

    drx_exit();
    drmgr_exit();
}

static void
event_init(void)
{
    char buf[MAXIMUM_PATH];

    module_table = module_table_create();

    memset(winafl_data.cache, 0, sizeof(winafl_data.cache));
    memset(winafl_data.afl_area, 0, MAP_SIZE);

    if(options.debug_mode) {
        debug_data.pre_hanlder_called = 0;
        debug_data.post_handler_called = 0;

        winafl_data.log =
            drx_open_unique_appid_file(options.logdir, dr_get_process_id(),
                                   "afl", "proc.log",
                                   DR_FILE_ALLOW_LARGE,
                                   buf, BUFFER_SIZE_ELEMENTS(buf));
        if (winafl_data.log != INVALID_FILE) {
            dr_log(NULL, LOG_ALL, 1, "winafl: log file is %s\n", buf);
            NOTIFY(1, "<created log file %s>\n", buf);
        }
    }

    fuzz_target.iteration = 0;
}


static void
setup_pipe() {
    pipe = CreateFile(
         options.pipe_name,   // pipe name
         GENERIC_READ |  // read and write access
         GENERIC_WRITE,
         0,              // no sharing
         NULL,           // default security attributes
         OPEN_EXISTING,  // opens existing pipe
         0,              // default attributes
         NULL);          // no template file

    if (pipe == INVALID_HANDLE_VALUE) DR_ASSERT_MSG(false, "error connecting to pipe");
}

static void
setup_shmem() {
   HANDLE map_file;

   map_file = OpenFileMapping(
                   FILE_MAP_ALL_ACCESS,   // read/write access
                   FALSE,                 // do not inherit the name
                   options.shm_name);            // name of mapping object

   if (map_file == NULL) DR_ASSERT_MSG(false, "error accesing shared memory");

   winafl_data.afl_area = (unsigned char *) MapViewOfFile(map_file, // handle to map object
               FILE_MAP_ALL_ACCESS,  // read/write permission
               0,
               0,
               MAP_SIZE);

   if (winafl_data.afl_area == NULL) DR_ASSERT_MSG(false, "error accesing shared memory");
}

static void
options_init(client_id_t id, int argc, const char *argv[])
{
    int i;
    const char *token;
	const char* mode;
    target_module_t *target_modules;
    /* default values */
	options.persistence_mode = native_mode;
    options.nudge_kills = true;
    options.debug_mode = false;
    options.thread_coverage = false;
    options.coverage_kind = COVERAGE_BB;
    options.target_modules = NULL;
    options.fuzz_module[0] = 0;
    options.fuzz_method[0] = 0;
    options.fuzz_offset = 0;
    options.fuzz_iterations = 1000;
    options.no_loop = false;
    options.func_args = NULL;
    options.num_fuz_args = 0;
    options.callconv = DRWRAP_CALLCONV_DEFAULT;
	options.dr_persist_cache = false;
    dr_snprintf(options.logdir, BUFFER_SIZE_ELEMENTS(options.logdir), ".");

	//MARK
	options.par_id = 0;
    strcpy(options.pipe_name, "\\\\.\\pipe\\afl_pipe_default");
    strcpy(options.shm_name, "afl_shm_default");
	strcpy(options.interceptor_config_path, interceptor_config_path);

	options.true_flag = false;

	//ADD
	strncpy(options.testcase_path, afl_input_path, BUFFER_SIZE_ELEMENTS(options.testcase_path));

    for (i = 1/*skip client*/; i < argc; i++) {
        token = argv[i];
        if (strcmp(token, "-no_nudge_kills") == 0)
            options.nudge_kills = false;
        else if (strcmp(token, "-nudge_kills") == 0)
            options.nudge_kills = true;
        else if (strcmp(token, "-thread_coverage") == 0)
            options.thread_coverage = true;
        else if (strcmp(token, "-debug") == 0)
            options.debug_mode = true;
        else if (strcmp(token, "-logdir") == 0) {
            USAGE_CHECK((i + 1) < argc, "missing logdir path");
            strncpy(options.logdir, argv[++i], BUFFER_SIZE_ELEMENTS(options.logdir));
        }
        else if (strcmp(token, "-fuzzer_id") == 0) {
            USAGE_CHECK((i + 1) < argc, "missing fuzzer id");
            strcpy(options.pipe_name, "\\\\.\\pipe\\afl_pipe_");
            strcpy(options.shm_name, "afl_shm_");
            strcat(options.pipe_name, argv[i+1]);
            strcat(options.shm_name, argv[i+1]);
            i++;
        }
        else if (strcmp(token, "-covtype") == 0) {
            USAGE_CHECK((i + 1) < argc, "missing coverage type");
            token = argv[++i];
            if(strcmp(token, "bb")==0) options.coverage_kind = COVERAGE_BB;
            else if (strcmp(token, "edge")==0) options.coverage_kind = COVERAGE_EDGE;
            else USAGE_CHECK(false, "invalid coverage type");
        }
        else if (strcmp(token, "-coverage_module") == 0) {
            USAGE_CHECK((i + 1) < argc, "missing module");
            target_modules = options.target_modules;
            options.target_modules = (target_module_t *)dr_global_alloc(sizeof(target_module_t));
            options.target_modules->next = target_modules;
            strncpy(options.target_modules->module_name, argv[++i], BUFFER_SIZE_ELEMENTS(options.target_modules->module_name));
        }
        else if (strcmp(token, "-target_module") == 0) {
            USAGE_CHECK((i + 1) < argc, "missing module");
            strncpy(options.fuzz_module, argv[++i], BUFFER_SIZE_ELEMENTS(options.fuzz_module));
        }
        else if (strcmp(token, "-target_method") == 0) {
            USAGE_CHECK((i + 1) < argc, "missing method");
            strncpy(options.fuzz_method, argv[++i], BUFFER_SIZE_ELEMENTS(options.fuzz_method));
        }
        else if (strcmp(token, "-fuzz_iterations") == 0) {
            USAGE_CHECK((i + 1) < argc, "missing number of iterations");
            options.fuzz_iterations = atoi(argv[++i]);
        }
        else if (strcmp(token, "-nargs") == 0) {
            USAGE_CHECK((i + 1) < argc, "missing number of arguments");
            options.num_fuz_args = atoi(argv[++i]);
        }
        else if (strcmp(token, "-target_offset") == 0) {
            USAGE_CHECK((i + 1) < argc, "missing offset");
            options.fuzz_offset = strtoul(argv[++i], NULL, 0);
        }
        else if (strcmp(token, "-verbose") == 0) {
            USAGE_CHECK((i + 1) < argc, "missing -verbose number");
            token = argv[++i];
            if (dr_sscanf(token, "%u", &verbose) != 1) {
                USAGE_CHECK(false, "invalid -verbose number");
            }
        }
        else if (strcmp(token, "-call_convention") == 0) {
            USAGE_CHECK((i + 1) < argc, "missing calling convention");
            ++i;
            if (strcmp(argv[i], "stdcall") == 0)
                options.callconv = DRWRAP_CALLCONV_CDECL;
            else if (strcmp(argv[i], "fastcall") == 0)
                options.callconv = DRWRAP_CALLCONV_FASTCALL;
            else if (strcmp(argv[i], "thiscall") == 0)
                options.callconv = DRWRAP_CALLCONV_THISCALL;
            else if (strcmp(argv[i], "ms64") == 0)
                options.callconv = DRWRAP_CALLCONV_MICROSOFT_X64;
            else
                NOTIFY(0, "Unknown calling convention, using default value instead.\n");
		}
		else if (strcmp(token, "-no_loop") == 0) {
			options.no_loop = true;
		}
		else if (strcmp(token, "-drpersist") == 0) {
			options.dr_persist_cache = true;
		}
		else if (strcmp(token, "-persistence_mode") == 0) {
			USAGE_CHECK((i + 1) < argc, "missing mode arg: '-fuzz_mode' arg");
			mode = argv[++i];
			if (strcmp(mode, "in_app") == 0)
			{
				options.persistence_mode = in_app;
			}
			else
			{
				options.persistence_mode = native_mode;
			}
		}
		//MARK add parellel args
        else if (strcmp(token, "-testcase") == 0) {
            USAGE_CHECK((i + 1) < argc, "missing testcase");
			strncpy(options.testcase_path, argv[++i], BUFFER_SIZE_ELEMENTS(options.testcase_path));
        }
		else if (strcmp(token, "-par_id") == 0) {
			USAGE_CHECK((i + 1) < argc, "missing parllel id");
			options.par_id = atoi(argv[++i]);
		}
		else if (strcmp(token, "-interceptors") == 0) {
			USAGE_CHECK((i + 1) < argc, "missing interceptors config file path");
			strncpy(options.interceptor_config_path, argv[++i], BUFFER_SIZE_ELEMENTS(options.interceptor_config_path));
		}
		else if (strcmp(token, "-true_flag") == 0) {
			options.true_flag = true;
		}
        else {
            NOTIFY(0, "UNRECOGNIZED OPTION: \"%s\"\n", token);
            USAGE_CHECK(false, "invalid option");
        }
    }

    if(options.fuzz_module[0] && (options.fuzz_offset == 0) && (options.fuzz_method[0] == 0)) {
       USAGE_CHECK(false, "If fuzz_module is specified, then either fuzz_method or fuzz_offset must be as well");
    }

    if(options.num_fuz_args) {
        options.func_args = (void **)dr_global_alloc(options.num_fuz_args * sizeof(void *));
    }
}

DR_EXPORT void
dr_client_main(client_id_t id, int argc, const char *argv[])
{
    drreg_options_t ops = {sizeof(ops), 2 /*max slots needed: aflags*/, false};

    dr_set_client_name("WinAFL", "");

	init_global_data();
	
    drmgr_init();
    drx_init();
    drreg_init(&ops);
    drwrap_init();

    options_init(id, argc, argv);


    dr_register_exit_event(event_exit);

    drmgr_register_exception_event(onexception);

    if(options.coverage_kind == COVERAGE_BB) {
        drmgr_register_bb_instrumentation_event(NULL, instrument_bb_coverage, NULL);
    } else if(options.coverage_kind == COVERAGE_EDGE) {
        drmgr_register_bb_instrumentation_event(NULL, instrument_edge_coverage, NULL);
    }

    drmgr_register_module_load_event(event_module_load);
    drmgr_register_module_unload_event(event_module_unload);
    dr_register_nudge_event(event_nudge, id);

    client_id = id;

    if (options.nudge_kills)
        drx_register_soft_kills(event_soft_kill);

    if(options.thread_coverage) {
        winafl_data.fake_afl_area = (unsigned char *)dr_global_alloc(MAP_SIZE);
    }

    if(!options.debug_mode || options.true_flag) {
        setup_pipe();
        setup_shmem();
    } else {
        winafl_data.afl_area = (unsigned char *)dr_global_alloc(MAP_SIZE);
    }

    if(options.coverage_kind == COVERAGE_EDGE || options.thread_coverage || options.dr_persist_cache) {
        winafl_tls_field = drmgr_register_tls_field();
        if(winafl_tls_field == -1) {
            DR_ASSERT_MSG(false, "error reserving TLS field");
        }
        drmgr_register_thread_init_event(event_thread_init);
        drmgr_register_thread_exit_event(event_thread_exit);
    }

    event_init();

	interceptor_config = ini_load(options.interceptor_config_path);
	if (!interceptor_config){
		dr_fprintf(winafl_data.log, "error open interceptor_config_path\n");
		DR_ASSERT_MSG(false, "error open interceptor_config_path");
	}
}
