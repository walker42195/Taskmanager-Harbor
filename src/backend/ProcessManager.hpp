#ifndef PROCESSMANAGER_HPP
#define PROCESSMANAGER_HPP

#include <csignal>
#include <string>

namespace Harbor {

class ProcessManager {
public:
    static bool sendSignal(int pid, int signalNumber);
    static bool terminateProcess(int pid); // SIGTERM
    static bool killProcess(int pid);      // SIGKILL
    static bool pauseProcess(int pid);     // SIGSTOP
    static bool resumeProcess(int pid);    // SIGCONT
    static bool setPriority(int pid, int niceValue);
};

} // namespace Harbor

#endif // PROCESSMANAGER_HPP
