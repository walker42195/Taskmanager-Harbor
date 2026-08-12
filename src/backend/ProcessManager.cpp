#include "ProcessManager.hpp"
#include <sys/types.h>
#include <signal.h>
#include <sys/resource.h>
#include <cerrno>
#include <cstdlib>
#include <string>

namespace Harbor {

bool ProcessManager::sendSignal(int pid, int signalNumber) {
    if (pid <= 1) return false; // Protection for PID 0 and systemd (PID 1)
    
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
}

bool ProcessManager::terminateProcess(int pid) {
    return sendSignal(pid, SIGTERM);
}

bool ProcessManager::killProcess(int pid) {
    return sendSignal(pid, SIGKILL);
}

bool ProcessManager::pauseProcess(int pid) {
    return sendSignal(pid, SIGSTOP);
}

bool ProcessManager::resumeProcess(int pid) {
    return sendSignal(pid, SIGCONT);
}

bool ProcessManager::setPriority(int pid, int niceValue) {
    if (pid <= 1) return false;
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
}

} // namespace Harbor
