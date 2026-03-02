#pragma once

/**
 * @brief Base class for all event receivers.
 *
 * Any object that wishes to receive events through the EventDispatcher
 * must inherit from this class. It provides a virtual destructor to
 * ensure proper cleanup when used polymorphically.
 */
struct Receiver
{
    virtual ~Receiver() = default;
};