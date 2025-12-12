#include <iostream>
#include <unistd.h>
//#include <sys/wait.h>
#include <signal.h>
#include "Commands.h"
#include "signals.h"

int main(int argc, char *argv[]) {
    if (signal(SIGINT, ctrlCHandler) == SIG_ERR) {
        perror("smash error: failed to set ctrl-C handler");
    }
    SmallShell &smash = SmallShell::getInstance();
    while (true) {
        std::cout << smash.getPrompt() << "> "<< flush;
        std::string cmd_line;
        if (!std::getline(std::cin, cmd_line)) {
            // EOF reached (end of input file or Ctrl+D)
            break;
        }
        try {
            smash.executeCommand(cmd_line.c_str());
            
        }catch(const std::exception & e ){
            cerr << e.what();
        }
    }
    return 0;
}