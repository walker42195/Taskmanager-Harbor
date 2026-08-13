#include "ProcessManager.hpp"
#include <cerrno>
#include <cstdlib>
#include <string>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/types.h>
#include <signal.h>
#include <sys/resource.h>
#endif

namespace Harbor {

bool ProcessManager::sendSignal(int pid, int signalNumber) {
    if (pid <= 1) return false; // Protection for PID 0 and systemd (PID 1)

#ifdef _WIN32
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, static_cast<DWORD>(pid));
    if (hProcess != NULL) {
        BOOL result = TerminateProcess(hProcess, 1);
        CloseHandle(hProcess);
        return (result != FALSE);
    }
    return false;
#else
    if (kill(pid, signalNumber) == 0) {
        return true;
    }

    // If permission denied (EPERM), elevate via pkexec (graphical sudo/polkit prompt)
    if (errno == EPERM) {
        std::string cmd = "pkexec kill -" + std::to_string(signalNumber) + " " + std::to_string(pid);
        int res = std::system(cmd.c_str());
        return (res == 0);
    }

    return false;
#endif
}

bool ProcessManager::terminateProcess(int pid) {
#ifdef _WIN32
    return sendSignal(pid, 0);
#else
    return sendSignal(pid, SIGTERM);
#endif
}

bool ProcessManager::killProcess(int pid) {
#ifdef _WIN32
    return sendSignal(pid, 0);
#else
    return sendSignal(pid, SIGKILL);
#endif
}

bool ProcessManager::pauseProcess(int pid) {
#ifdef _WIN32
    return false;
#else
    return sendSignal(pid, SIGSTOP);
#endif
}

bool ProcessManager::resumeProcess(int pid) {
#ifdef _WIN32
    return false;
#else
    return sendSignal(pid, SIGCONT);
#endif
}

bool ProcessManager::setPriority(int pid, int niceValue) {
    if (pid <= 1) return false;

#ifdef _WIN32
    HANDLE hProcess = OpenProcess(PROCESS_SET_INFORMATION, FALSE, static_cast<DWORD>(pid));
    if (hProcess != NULL) {
        DWORD priorityClass = NORMAL_PRIORITY_CLASS;
        if (niceValue < -10) priorityClass = HIGH_PRIORITY_CLASS;
        else if (niceValue < 0) priorityClass = ABOVE_NORMAL_PRIORITY_CLASS;
        else if (niceValue > 10) priorityClass = IDLE_PRIORITY_CLASS;
        else if (niceValue > 0) priorityClass = BELOW_NORMAL_PRIORITY_CLASS;

        BOOL res = SetPriorityClass(hProcess, priorityClass);
        CloseHandle(hProcess);
        return (res != FALSE);
    }
    return false;
#else
    if (niceValue < -20) niceValue = -20;
    if (niceValue > 19) niceValue = 19;

    if (setpriority(PRIO_PROCESS, pid, niceValue) == 0) {
        return true;
    }

    if (errno == EPERM || errno == EACCES) {
        std::string cmd = "pkexec renice " + std::to_string(niceValue) + " -p " + std::to_string(pid);
        int res = std::system(cmd.c_str());
        return (res == 0);
    }

    return false;
#endif
}

} // namespace Harbor
