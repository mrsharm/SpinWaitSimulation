#include <handleapi.h>
#include <cassert>
#include <synchapi.h>
#include <windows.h>

// WindowsEvent is an implementation of GCEvent that forwards
// directly to Win32 APIs.

//#define INVALID_HANDLE_VALUE ((void*)-1)
typedef unsigned int uint;

class EventImpl
{
private:
    HANDLE m_hEvent;

public:
    EventImpl() : m_hEvent(INVALID_HANDLE_VALUE) {}

    bool IsValid() const
    {
        return m_hEvent != INVALID_HANDLE_VALUE;
    }

    void Set()
    {
        assert(IsValid());
        BOOL result = SetEvent(m_hEvent);
        assert(result && "SetEvent failed");
    }

    void Reset()
    {
        assert(IsValid());
        BOOL result = ResetEvent(m_hEvent);
        assert(result && "ResetEvent failed");
    }

    uint Wait(uint timeout, bool alertable)
    {
        UNREFERENCED_PARAMETER(alertable);
        assert(IsValid());

        return WaitForSingleObject(m_hEvent, timeout);
    }

    void CloseEvent()
    {
        assert(IsValid());
        BOOL result = CloseHandle(m_hEvent);
        assert(result && "CloseHandle failed");
        m_hEvent = INVALID_HANDLE_VALUE;
    }

    bool CreateAutoEvent(bool initialState)
    {
        m_hEvent = CreateEvent(nullptr, false, initialState, nullptr);
        return IsValid();
    }

    bool CreateManualEvent(bool initialState)
    {
        m_hEvent = CreateEvent(nullptr, true, initialState, nullptr);
        return IsValid();
    }
};
