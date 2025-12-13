#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"

using namespace std;

void ctrlCHandler(int sig_num) {
    auto& smash = SmallShell::getInstance();
    pid_t targetPid = smash.getFgPid();
    cout << "smash: got ctrl-C" << endl;
    if (targetPid != -1) {
        int sigOut = kill(targetPid, SIGKILL);
        if (sigOut != 0) {
            return;
        }
        cout << "smash: process " << smash.getFgPid() << " was killed" << endl;
        smash.setFgPid(-1, -1);
    }
}
