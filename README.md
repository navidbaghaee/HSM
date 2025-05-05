# Hierarchical State Machine (HSM) Framework in C
![HSM](https://img.shields.io/badge/HSM%20-blue)
![License](https://img.shields.io/badge/License-MIT-green)

This repository provides a lightweight Hierarchical State Machine (HSM) implementation in C, allowing structured and scalable event-driven state management for embedded systems.

---

## Table of Contents

1. [Overview](#overview)
2. [Directory Structure](#directory-structure)
3. [Core Concepts](#core-concepts)
4. [API Reference](#api-reference)

   * [Fsm Constructor](#fsm-constructor)
   * [Initialization](#initialization)
   * [Dispatching Events](#dispatching-events)
   * [State Transitions](#state-transitions)
   * [State Query](#state-query)
5. [Usage Example](#usage-example)
6. [Error Handling & Return Codes](#error-handling--return-codes)
7. [Author & License](#author--license)

---

## Overview

The HSM framework implements a hierarchical finite state machine that supports:

* Nesting of states (superstates and substates)
* Automatic entry/exit event propagation
* Deferred transitions pending completion of event handling
* Generic event callbacks
* Querying active states

Designed for portability, it only depends on standard C types and can be integrated with any hardware abstraction.

---

## Directory Structure

```plaintext
src/
  ├── HSM.h        # Header declarations
  ├── HSM.c        # Implementation of HSM functions
examples/
  └── main.c       # Example usage of HSM framework
README.md         # This file
LICENSE           # License information
```

---

## Core Concepts

* **States**: Each state has a handler function (`handlerFn`) and an optional pointer to its `superState`, forming a hierarchy.
* **Events**: Defined by a signal (`sig`) and optional parameter (`parm`).
* **Entry/Exit Events**: Automatically generated (`HSM_SIG_ENTRY`, `HSM_SIG_EXIT`) when transitioning.
* **Initialization**: The FSM is initialized to an initial state, dispatching entry actions.
* **Dispatch**: Incoming events are offered to the active state and may propagate to superstates, unless suppressed.
* **Transitions**: Deferred until after event handling completes; the framework identifies the lowest common superstate to correctly exit and enter states.

---

## API Reference

### Fsm Constructor

```c
void HSM_FsmCtor(HSM_Fsm *me,
                 const HSM_State *initialState,
                 HSM_State_fn genericEvtHandler);
```

* Initializes the FSM context `me`.
* `initialState`: Pointer to the topmost starting state.
* `genericEvtHandler`: Optional callback invoked on every event.

### Initialization

```c
void HSM_FsmInit(HSM_Fsm *me);
```

* Performs the initial transition by dispatching `HSM_SIG_INIT` and `HSM_SIG_ENTRY` events.

### Dispatching Events

```c
void HSM_Dispatch(HSM_Fsm *me, HSM_Event *e);
```

* Delivers event `e` to the current active state and its ancestry.
* Handlers can return `HSM_SUPPRESS_SUPERSTATES` or `HSM_SUPPRESS_IMMEDIATE_SUPERSTATE` to control propagation.

### State Transitions

```c
void HSM_Transition(HSM_Fsm *me, const HSM_State *target);
```

* Requests a transition to `target` state after current event processing.
* Internally computes the lowest common superstate for proper exit/entry sequences.

### State Query

```c
kal_bool HSM_StateIsActive(HSM_Fsm *me, const HSM_State *pState);
```

* Returns `KAL_TRUE` if `pState` is the current active state or any of its superstates.

---

## Usage Example

```c
#include "HSM.h"

// State handler prototypes
static uint8_t Idle_state(FSM *me, const HSM_Event *e);
static uint8_t Active_state(FSM *me, const HSM_Event *e);

// State definitions
const HSM_State Idle = { Idle_state, NULL };
const HSM_State Active = { Active_state, &Idle };

// FSM instance
HSM_Fsm myFsm;

int main(void) {
    HSM_FsmCtor(&myFsm, &Idle, NULL);
    HSM_FsmInit(&myFsm);

    HSM_Event evt = { SIG_START, 0 };
    HSM_Dispatch(&myFsm, &evt);

    return 0;
}
```

In this example, the FSM starts in `Idle`, receives `SIG_START`, and may transition to `Active` based on the handler logic.

---

## Error Handling & Return Codes

State handlers should return one of the following to control dispatch behavior:

* `HSM_HANDLED` (0): Event was handled; continue propagating.
* `HSM_SUPPRESS_SUPERSTATES`: Stop further propagation to superstates.
* `HSM_SUPPRESS_IMMEDIATE_SUPERSTATE`: Skip only the immediate superstate, continue with higher levels.

No explicit error codes are returned by the HSM core functions; transitions are deferred and executed transparently.

---

## Author & License

* **Author**: Navid Baghaee
* **Date**: April 8, 2023
* **License**: MIT License. See [LICENSE](LICENSE) for details.
