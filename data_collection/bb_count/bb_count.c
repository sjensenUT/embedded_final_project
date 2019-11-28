 
/* bb_count.c
 *
 * Counts how many times each basic block is executed.
 */

#include <stddef.h> /* for offsetof */
#include "dr_api.h"
#include "drmgr.h"
#include "drreg.h"
#include "drx.h"
#include "string.h"

#ifdef WINDOWS
#    define DISPLAY_STRING(msg) dr_messagebox(msg)
#else
#    define DISPLAY_STRING(msg) dr_printf("%s\n", msg);
#endif

#define NULL_TERMINATE(buf) buf[(sizeof(buf) / sizeof(buf[0])) - 1] = '\0'

// There can't possibly be more than 100 thousand basic blocks in any given
// benchmark, right?
#define MAX_BBS     100000
#define MAX_BB_SIZE 4000

// Store data as follows: two equal-size arrays,
// one with char*s and one with numbers
static char  bb_strs[MAX_BBS][MAX_BB_SIZE*2];
static int   bb_cnts[MAX_BBS];

// Keeps track of the next available index in the arrays
static unsigned int nextIndex = 0;


static void
event_exit(void)
{
    char msg[1024];

    int j;
    for (j = 0; j < nextIndex; j++)
    {
      /*int len;
      len = dr_snprintf(msg, sizeof(msg) / sizeof(msg[0]),
            "BB: %s, Count: %d, iter: %d", bb_strs[j], bb_cnts[j], j);

      DR_ASSERT(len > 0);
      NULL_TERMINATE(msg);
      DISPLAY_STRING(msg);*/
      printf("BB: %s, Count: %d, iter: %d\n", bb_strs[j], bb_cnts[j], j);
    }

    dr_snprintf(msg, sizeof(msg) / sizeof(msg[0]),
        "Counted executions for %d basic blocks\n", nextIndex);
    NULL_TERMINATE(msg);
    DISPLAY_STRING(msg);

    drx_exit();
    drreg_exit();
    drmgr_exit();
}


// This is the function that gets called at the beginning of each basic block.
// All it does is increment its BB's count by 1.
static void increment(uint bb_index)
{
  bb_cnts[bb_index] = bb_cnts[bb_index] + 1;
}


static dr_emit_flags_t
event_app_instruction(void *drcontext, void *tag, instrlist_t *bb, instr_t *inst,
                      bool for_trace, bool translating, void *user_data)
{

    /* By default drmgr enables auto-predication, which predicates all instructions with
     * the predicate of the current instruction on ARM.
     * We disable it here because we want to unconditionally execute the following
     * instrumentation.
     */
    drmgr_disable_auto_predication(drcontext, bb);
    if (!drmgr_is_first_instr(drcontext, inst))
        return DR_EMIT_DEFAULT;

    dr_printf("in dynamorio_basic_block(tag=" PFX ")\n", tag);
    
    // Create a copy of the bb and delete the last instruction
    // Because of how DynamoRIO works, it includes the control transfer instruction
    // at the end. We don't want it in our BB's so remove it
    instrlist_t* bb_copy = instrlist_clone(drcontext, bb);
    instrlist_remove(bb_copy, instrlist_last(bb_copy));
    //instrlist_disassemble(drcontext, tag, bb_copy, STDOUT);

    // Skip BB's that are now empty when removing the last instruction.
    if (instrlist_first(bb_copy) == instrlist_last(bb_copy)) return DR_EMIT_DEFAULT;    

    // Get the hex representation and copy it to the list
    // cow
    instr_t *instr, *next;
    char* needle = bb_strs[nextIndex];
    for (instr = instrlist_first(bb_copy);
         instr != NULL;
         instr = next) {
      next = instr_get_next(instr);
      int ilen = instr_length(drcontext, instr);
      int n;
      for (n = 0; n < ilen; n++) {
        sprintf(needle, "%02x", instr_get_raw_byte(instr, n));
        needle += 2;
      }
    }
    //dr_printf("Saved BB: %s\n", bb_strs[nextIndex]);

    instrlist_clear_and_destroy(drcontext, bb_copy);

    /*byte* e = instrlist_encode(drcontext, bb_copy, buf, true);
    if (e == NULL)
    {
      dr_printf("Uh-oh! BB encoding failed!\n");
      dr_abort();
    }


    // Convert the byte array to a string
    byte* s;
    for(s = buf; s < e; s++)
    {
      sprintf(bb_strs[nextIndex] + ((s - buf)*2), "%02X", *s);
    }*/

    // Initialize the count to 0
    bb_cnts[nextIndex] = 0;

    // And now do the instrumentation. Insert a clean call to the function that will
    // increase the count for this basic block.
    dr_insert_clean_call(drcontext, bb, instrlist_first_app(bb), (void *)increment,
                         false /* save fpstate */, 1, OPND_CREATE_INT32(nextIndex));  

    // Increment the counter
    nextIndex++;

    if (nextIndex == MAX_BBS)
    {
      dr_printf("Uh-oh! Encountered more BBs than expected! Increase MAX_BBS");
      dr_abort();
    }

    return DR_EMIT_DEFAULT;
}


DR_EXPORT void
dr_client_main(client_id_t id, int argc, const char *argv[])
{
    drreg_options_t ops = { sizeof(ops), 1 /*max slots needed: aflags*/, false };
    dr_set_client_name("bb_count", "");

    if (!drmgr_init() || !drx_init() || drreg_init(&ops) != DRREG_SUCCESS)
        DR_ASSERT(false);

    /* register events */
    dr_register_exit_event(event_exit);
    if (!drmgr_register_bb_instrumentation_event(NULL, event_app_instruction, NULL))
        DR_ASSERT(false);

    /* make it easy to tell, by looking at log file, which client executed */
    dr_log(NULL, DR_LOG_ALL, 1, "Client 'bbcount' initializing\n");
    /* also give notification to stderr */
    if (dr_is_notify_on()) {
#    ifdef WINDOWS
        /* ask for best-effort printing to cmd window.  must be called at init. */
        dr_enable_console_printing();
#    endif
        dr_fprintf(STDERR, "Client bb_count is running\n");
    }
}
