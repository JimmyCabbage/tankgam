#include "Event.h"

EventQueue::EventQueue() = default;

EventQueue::~EventQueue() = default;

void EventQueue::pushEvent(Event ev)
{
    events.push(ev);
}

bool EventQueue::popEvent(Event& outEv)
{
    if (events.empty())
    {
        return false;
    }
    
    outEv = events.front();
    events.pop();
    
    return true;
}
