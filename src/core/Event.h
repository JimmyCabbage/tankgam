#pragma once

#include <cstdint>
#include <queue>

enum class KeyPressType : uint32_t
{
    None = 0,
    LeftArrow = 256,
    RightArrow,
    UpArrow,
    DownArrow,
    Space = ' ',
    Backspace = '\b',
    Delete =  '\x7F',
    Tab = '\t',
    Escape = '\x1B',
    Return = '\r',
    Period = '.',
    N0 = '0',
    N1 = '1',
    N2 = '2',
    N3 = '3',
    N4 = '4',
    N5 = '5',
    N6 = '6',
    N7 = '7',
    N8 = '8',
    N9 = '9',
    A = 'A',
    B = 'B',
    C = 'C',
    D = 'D',
    E = 'E',
    F = 'F',
    G = 'G',
    H = 'H',
    I = 'I',
    J = 'J',
    K = 'K',
    L = 'L',
    M = 'M',
    N = 'N',
    O = 'O',
    P = 'P',
    Q = 'Q',
    R = 'R',
    S = 'S',
    T = 'T',
    U = 'U',
    V = 'V',
    W = 'W',
    X = 'X',
    Y = 'Y',
    Z = 'Z',
};

enum class EventType : uint16_t
{
    KeyDown,
    KeyUp,
    MouseMove,
    WindowResize,
    WindowHide,
    WindowShow
};

struct Event
{
    EventType type;
    uint32_t data1;
    uint32_t data2;
};

class EventQueue
{
public:
    EventQueue();
    ~EventQueue();
    
    EventQueue(const EventQueue&) = delete;
    EventQueue& operator=(const EventQueue&) = delete;
    
    void pushEvent(Event ev);
    
    bool popEvent(Event& outEv);
    
private:
    std::queue<Event> events;
};
