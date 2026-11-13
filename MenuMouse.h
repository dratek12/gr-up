#pragma once

// SDL calls must run on the window thread, never on the Present/worker thread.
class MenuMouse {
public:
    static bool AllowsAutomation(bool overlayOpen,bool focused,bool relative){return !overlayOpen&&focused&&relative;}
    struct Api {
        void* (*focus)() = nullptr;
        bool (*relative)(void*) = nullptr;
        bool (*setRelative)(void*, bool) = nullptr;
        bool (*grab)(void*) = nullptr;
        bool (*setGrab)(void*, bool) = nullptr;
        unsigned (*drain)(float*, float*) = nullptr;
    } api;
    bool Open() {
        if (window) return true;
        if (!api.focus || !api.relative || !api.setRelative || !api.grab || !api.setGrab || !api.drain)
            return false;
        window = api.focus();
        if (!window) return false;
        wasRelative = api.relative(window);
        wasGrabbed = api.grab(window);
        if (!Enforce()) { Close(); return false; }
        return true;
    }
    bool Enforce() {
        if (!window) return false;
        bool ok = true;
        if (api.relative(window)) ok = api.setRelative(window, false);
        if (api.grab(window)) ok = api.setGrab(window, false) && ok;
        return ok;
    }
    void Close() {
        if (!window) return;
        // Do not replay motion collected over the menu when gameplay resumes.
        float x{}, y{};
        api.drain(&x, &y);
        api.setRelative(window, wasRelative);
        api.setGrab(window, wasGrabbed);
        window = nullptr;
    }
private:
    void* window = nullptr;
    bool wasRelative = false, wasGrabbed = false;
};
