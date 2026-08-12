#include "ProcessManager.hpp"
#include <sys/types.h>
#include <signal.h>
#include <sys/resource.h>
#include <cerrno>

namespace Harbor {

bool ProcessManager::sendSignal(int pid, int signalNumber) {
    if (pid <= 1) return false; // Safety protection for PID 0 and init/systemd (PID 1)
    return (kill(pid, signalNumber) == 0);
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
    return (setpriority(PRIO_PROCESS, pid, niceValue) == 0);
}

} // namespace Harbor
