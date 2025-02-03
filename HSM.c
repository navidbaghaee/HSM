/** @file HSM.c
 * 	@brief This file includes the HSM functions and initial a basic HSM.
 * 	@author Navid Baghaee
 * 	@date 04/08/2023
 */
#include "HSM.h"

static HSM_Event const initEvt = { HSM_SIG_INIT, 0};
static HSM_Event const entryEvt = { HSM_SIG_ENTRY, 0};
static HSM_Event const exitEvt = { HSM_SIG_EXIT, 0};

void __Transition(HSM_Fsm *me, const HSM_State * const target);
void __Dispatch(HSM_Fsm *me, const HSM_Event * const e, const HSM_State * const superstateToStopBefore, kal_bool ascendingOrder);


void HSM_FsmCtor(HSM_Fsm *me, const HSM_State * const initialState, HSM_State_fn const fnGenericEvtHandler){
    me->pState__ = (HSM_State*)initialState;
    me->pTransitionTarget = NULL;
    me->pLatestEvent = NULL;
    me->fnGenericEvtHandler = fnGenericEvtHandler;
}

void HSM_FsmInit(HSM_Fsm *me){
    __Dispatch(me, (HSM_Event*)&initEvt, me->pState__->superState, KAL_FALSE); // Only dispatch in the target state. No superstates
    __Dispatch(me, (HSM_Event*)&entryEvt, NULL, KAL_TRUE);
}

// Finds the "Lowest Common Ancestor" between two states
const HSM_State * __lowestCommonSuperstate(const HSM_State* const state1, const HSM_State* const state2){
    const HSM_State* pState1Superstate;
    const HSM_State* pState2Superstate; 
    for(pState1Superstate = state1; pState1Superstate != NULL; pState1Superstate = pState1Superstate->superState){
        for(pState2Superstate = state2; pState2Superstate != NULL; pState2Superstate = pState2Superstate->superState){
            if(pState1Superstate == pState2Superstate){
                return pState1Superstate;
            }
        }
    }
    return NULL;
}

void __Transition(HSM_Fsm *me, const HSM_State * const target)
{
    const HSM_State* lowestCommonSuperstate = __lowestCommonSuperstate(me->pState__, (HSM_State *)target);
    /* exit the source and its superstates, stopping before reaching a superstate in common with the target */
    __Dispatch(me, (HSM_Event*)&exitEvt, lowestCommonSuperstate, KAL_FALSE);
    me->pState__ = (HSM_State*)target;
    /* enter starting at the lowest common state and working down to the target state*/
    __Dispatch(me, (HSM_Event*)&entryEvt,lowestCommonSuperstate, KAL_TRUE);
}

void HSM_Transition(HSM_Fsm *me, const HSM_State * const target)
{
    if(me->pLatestEvent
      && (me->pLatestEvent->sig != HSM_SIG_ENTRY)
      && (me->pLatestEvent->sig != HSM_SIG_EXIT)
    ){
      // Save the transition target.  The transition will be made upon completion of the active event handling.
      me->pTransitionTarget = (HSM_State*) target;
    }
}

void HSM_Dispatch(HSM_Fsm *me, HSM_Event *e){
  __Dispatch(me, e, NULL, KAL_FALSE);
}

void __Dispatch(HSM_Fsm *me, const HSM_Event * const e, const HSM_State * const superstateToStopBefore, kal_bool descending){

    static const HSM_State *stateList[HSM_MAX_HIERARCHY_DEPTH] = {NULL};
    int8_t stateListIndex = 0;
    const HSM_State * pState;
    uint8_t retVal;

    me->pLatestEvent = e;

    if(me->fnGenericEvtHandler){
        (me->fnGenericEvtHandler)(me, e);
    }

    // We dispatch starting with the active state, and then move up through super states, until we've reached a common superstate
    for(pState = me->pState__; ((pState != NULL) && (pState != superstateToStopBefore)); pState = pState->superState){
        if(descending){
            stateList[stateListIndex] = pState;
            stateListIndex++;
            continue;
        }
        retVal = (pState->handlerFn)(me, e);
        if( retVal == HSM_SUPPRESS_SUPERSTATES){
            break;
        }else if( retVal == HSM_SUPPRESS_IMMEDIATE_SUPERSTATE){
            if(pState->superState != NULL){
                pState = pState->superState; // Skip over the next
            }
            continue;
        }
    }

    if(descending){
        stateListIndex--;
        for(; stateListIndex>=0; stateListIndex--){
            (stateList[stateListIndex]->handlerFn)(me, e);
        }
    }

    // Check if we should perform a transition
    // Because transitions are can only be executed outside of Entry and Exit events,
    // this HSM_Dispatch() function is restricted to be, at max, recursively executed only once by the __Transition() call
    if(me->pTransitionTarget != NULL){
        const HSM_State * pTransitionTarget = me->pTransitionTarget;
        me->pTransitionTarget = NULL;
        __Transition(me, pTransitionTarget);
    }
}

kal_bool HSM_StateIsActive(HSM_Fsm *me, const HSM_State *pState){
    // Iterate trough the active state and its superstates to see if the requested pState matches any of them
    const HSM_State* pStateSuperstate;
    for(pStateSuperstate = me->pState__; pStateSuperstate != NULL; pStateSuperstate = pStateSuperstate->superState){
        if(pStateSuperstate == pState){
            return KAL_TRUE;
        }
    }
    return KAL_FALSE;
}
