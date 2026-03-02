#pragma once
#include "Receiver.h"
#include "Event.h"
#include "Dispatcher.h"

/**
 * @brief Alias defining the event dispatcher used by the EventBus.
 *
 * This alias configures a `KGR::FNDispatcher` to route events from a
 * `Receiver` (the object that handles events) to an `Event` (the base type
 * of all events in the system).
 *
 * Template parameters:
 * - LhsType      : Receiver — the object receiving the event.
 * - RhsType      : Event — the polymorphic event being dispatched.
 * - ReturnType   : void — event handlers do not return a value.
 * - Args         : no additional arguments (empty list).
 *
 * This dispatcher allows registering functions of the form:
 * @code
 * void Handle(MyReceiver&, MyEvent&);
 * @endcode
 *
 * and automatically dispatches them based on the dynamic types of both
 * the receiver and the event.
 */
using EventDispatcher = KGR::FNDispatcher<
    Receiver,               ///< Left-hand side type (receiver)
    Event,                  ///< Right-hand side type (event)
    void,                   ///< Return type of handlers
    KGR::WhrapperTypes<>    ///< No extra arguments
>;