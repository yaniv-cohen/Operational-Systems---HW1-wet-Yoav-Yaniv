#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"

using namespace std;

void ctrlCHandler(int sig_num) {
    // TODO: Add your implementation
    cout << "smash: got ctrl-C " << endl;
    auto& smash = SmallShell::getInstance();
    pid_t targetPid = smash.getFgPid();
    if (targetPid != -1) {
        int sigOut = kill(targetPid, SIGKILL);
        if (sigOut != 0) {
            return;
        }
        cout << "smash: process " <<
             targetPid << "  was killed" << endl;
        smash.setFgPid(-1);
    }
}
