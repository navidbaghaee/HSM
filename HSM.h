/** @file HSM.c
 * 	@brief This file includes the HSM functions and initial a basic HSM.
 * 	@author Navid Baghaee
 * 	@date 04/08/2023
 */

#ifndef HSM_H
#define HSM_H

#include "stdint.h"
#include "stack_ltlcom.h" //for --> LOACAL_PARA_HDR
#include "kal_general_types.h"
//#include "stdbool.h"

#define HSM_CONCAT_2(p1, p2)      HSM_CONCAT_2_(p1, p2)
#define HSM_CONCAT_2_(p1, p2)     p1##p2

#ifndef HSM_MAX_HIERARCHY_DEPTH
#define HSM_MAX_HIERARCHY_DEPTH 5
#endif

#ifndef NULL
#define NULL 0
#endif


typedef enum
{
    HSM_CONTINUE = 0,
    HSM_SUPPRESS_SUPERSTATES,  // Bypass all superstates' event handling
    HSM_SUPPRESS_IMMEDIATE_SUPERSTATE  // Bypass only the immediate superstate's event handling
} HSM_return_t;

typedef struct HSM_Fsm HSM_Fsm;
typedef struct HSM_State HSM_State;
typedef struct HSM_Event HSM_Event;
typedef HSM_return_t (*HSM_State_fn)(HSM_Fsm *, HSM_Event const *);
typedef short HSM_Signal;

enum {
    HSM_SIG_INIT = 1,
    HSM_SIG_ENTRY,
    HSM_SIG_EXIT,
    HSM_SIG_USER_START        /*!< first signal for the user applications */
};


struct HSM_Event{
    HSM_Signal sig;
    unsigned int zone_number;
};

struct HSM_State
{
    HSM_State_fn const handlerFn;
    const HSM_State * const superState;
    int identifier;       // For debugging
};

struct HSM_Fsm
{
   const HSM_State *pState__; /* the active state */
   const HSM_State *pTransitionTarget;      // State to transition to after dispatching current event
   const HSM_Event *pLatestEvent;           // Most recent event to be dispatched, or that's in the process of being dispatched
   HSM_State_fn fnGenericEvtHandler;  // A generic event handler function that's called on every event.  This can be used for debuggings or logging.
};




#define HSM_STATE_DEF(debug_id, varName) \
    static HSM_return_t HSM_CONCAT_2(varName,_fxn) (HSM_Fsm * me, HSM_Event const * pEvent); \
    static const HSM_State HSM_CONCAT_2(varName,_data) = { HSM_CONCAT_2(varName,_fxn), NULL, debug_id}; \
    static const HSM_State  * const varName = &HSM_CONCAT_2(varName,_data)

#define HSM_SUBSTATE_DEF(debug_id, varName, superstate) \
    static HSM_return_t HSM_CONCAT_2(varName,_fxn) (HSM_Fsm * me, HSM_Event const * pEvent); \
    static const HSM_State HSM_CONCAT_2(varName,_data) = { HSM_CONCAT_2(varName,_fxn), &HSM_CONCAT_2(superstate,_data), debug_id}; \
    static const HSM_State  * const varName = &HSM_CONCAT_2(varName,_data)


void HSM_FsmCtor(HSM_Fsm *me, const HSM_State * const initialState, HSM_State_fn const fnGenericEvtHandler);
void HSM_FsmInit(HSM_Fsm *me);
// Call HSM_Transition() to register a state transition to be performed AFTER the active state
// AND its superstates have completed dispatching the current event.
// Only call HSM_Transition() in the context of a HSM_State's event handler function.
// This operation will be ignored if called during an entry or exit event.
void HSM_Transition(HSM_Fsm *me, const HSM_State * const target);
// HSM_Dispatch() should be called from a single location within the application's scheduler (event queue). 
// Do not call HSM_Dispatch from within a state function
void HSM_Dispatch(HSM_Fsm *me, HSM_Event *e);
// Call this to confirm if the state machine is in a state.
// It applies to superstates as well.  For example, if state B is the active state and it's a substate of A,
// calling this function will return true if either A or B are passed in as the pState parameter.
kal_bool HSM_StateIsActive(HSM_Fsm *me, const HSM_State *pState);

#endif