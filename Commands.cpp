#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cstring>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <sys/utsname.h>
#include <sys/sysinfo.h>
#include <limits.h>
#include <iomanip>
#include "Commands.h"
#include "signals.h"
#include <signal.h>
#include <utility>
#include <fstream>

using namespace std;

const std::string WHITESPACE = " \n\r\t\f\v";

#if 0
#define FUNC_ENTRY() \
    cout << __PRETTY_FUNCTION__ << " --> " << endl;

#define FUNC_EXIT() \
    cout << __PRETTY_FUNCTION__ << " <-- " << endl;
#else
#define FUNC_ENTRY()
#define FUNC_EXIT()
#endif

void printAllArgs(char** args) {
    int i = 0;
    while (args[i] != nullptr) {
        std::cout << "arg " << i << ": " << args[i] << std::endl;
        i++;
    }
}

string _ltrim(const std::string& s) {
    size_t start = s.find_first_not_of(WHITESPACE);
    return (start == std::string::npos) ? "" : s.substr(start);
}

string _rtrim(const std::string& s) {
    size_t end = s.find_last_not_of(WHITESPACE);
    return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

string _trim(const std::string& s) {
    return _rtrim(_ltrim(s));
}

int _parseCommandLine(const char* cmd_line, char** args) {
    FUNC_ENTRY()
    int i = 0;
    std::istringstream iss(_trim(string(cmd_line)));
    for (std::string s; iss >> s;) {
        args[i] = (char*) malloc(s.length() + 1);
        memset(args[i], 0, s.length() + 1);
        strcpy(args[i], s.c_str());
        args[++i] = nullptr;
    }
    return i;
    FUNC_EXIT()
}

bool _isBackgroundCommand(const char* cmd_line) {
    const string str(cmd_line);
    return str[str.find_last_not_of(WHITESPACE)] == '&';
}

void _removeBackgroundSign(char* cmd_line) {
    const string str(cmd_line);
    // find last character other than spaces
    size_t idx = str.find_last_not_of(WHITESPACE);
    // if all characters are spaces then return
    if (idx == string::npos) {
        return;
    }
    // if the command line does not end with & then return
    if (cmd_line[idx] != '&') {
        return;
    }
    // replace the & (background sign) with space and then remove all tailing spaces.
    cmd_line[idx] = '\0';
    // truncate the command line string up to the last non-space character
    cmd_line[str.find_last_not_of(WHITESPACE, idx) + 1] = 0;
}

// TODO: Add your implementation for classes in Commands.h

SmallShell::SmallShell() = default;
//{
//    // TODO: add your implementation
//}

SmallShell::~SmallShell() = default;
//{
//    // TODO: add your implementation
//}

/**
 * Creates and returns a pointer to Command class which matches the given command line (cmd_line)
 */
Command* SmallShell::CreateCommand(const char* cmd_line_raw) {
    
    // 1. Always clean up finished background jobs first.
    jobsList.removeFinishedJobs();
    
    // 2. Create the primary mutable command line buffer.
    char cmd_line[COMMAND_MAX_LENGTH];
    // Copy the original raw input into the mutable buffer.
    strncpy(cmd_line, cmd_line_raw, COMMAND_MAX_LENGTH - 1);
    cmd_line[COMMAND_MAX_LENGTH - 1] = '\0';
    
    // 3. Trim the mutable string and find the first word.
    string current_cmd_str = _trim(string(cmd_line));
    size_t firstSpacePos = current_cmd_str.find_first_of(WHITESPACE);
    string firstWord = current_cmd_str.substr(0, firstSpacePos);
    // 4. Check for Alias Substitution
    // Note: The 'aliases' map is assumed to be a member of SmallShell.
    auto alias_it = aliases.find(firstWord);
    //TODO: handle hi&
    if (alias_it != aliases.end()) {
        // Alias found!
        string alias_value = alias_it->second; // e.g., "ls -l"
        string remaining_args;                // The arguments after the alias name
        
        if (firstSpacePos != string::npos) {
            // Get the rest of the command line (e.g., " /etc")
            remaining_args = current_cmd_str.substr(firstSpacePos);
        }
        
        // Construct the new command line string: "ls -l /etc"
        string new_cmd_line_str = alias_value + remaining_args;
        
        // Overwrite the mutable buffer with the substituted command.
        strncpy(cmd_line, new_cmd_line_str.c_str(), COMMAND_MAX_LENGTH - 1);
        cmd_line[COMMAND_MAX_LENGTH - 1] = '\0';
        
        // RE-PROCESS: After substitution, re-read the first word and the trimmed string
        // from the new content for command identification later.
        current_cmd_str = _trim(string(cmd_line));
        firstSpacePos = current_cmd_str.find_first_of(WHITESPACE);
        firstWord = current_cmd_str.substr(0, firstSpacePos);
    }
    
    // 5. Check for Background Command (uses the final, potentially substituted, cmd)
    bool isBackground = _isBackgroundCommand(cmd_line);
//    printf("isBackground = %d\n", isBackground);
    
    // 6. Remove the background sign (modifies cmd_line in place)
    _removeBackgroundSign(cmd_line);
//    printf("post remove: %s\n", cmd_line);

//    printf("first word: %s\n", firstWord.c_str());

// Find the position of the redirection character
    size_t redirect_pos = string(cmd_line).find_first_of('>');
    // Check the position directly against std::string::npos
    if (redirect_pos != std::string::npos) {
        // Redirection character was found!
        
        // Optional: Add logic here to ensure it's not inside quotes if required by spec.
        
        // For now, assume a found '>' means redirection must be handled
        return new RedirectionCommand(cmd_line);
    }
    
    //PIPE
    size_t pipe_pos = string(cmd_line).find_first_of('|');
    // Check the position directly against std::string::npos
    if (pipe_pos != std::string::npos) {
        return new PipeCommand(cmd_line);
    }

//    std::cout << "find_first_of " << redirect_pos << std::endl;
    // --- Remainder of Command Identification Logic ---
    // single word commands
    if (firstWord == "pwd") {
        return new GetCurrDirCommand(cmd_line);
    } // done
    else if (firstWord == "showpid") {
        return new ShowPidCommand(cmd_line);
    } // done
    else if (firstWord == "jobs") {
        return new JobsCommand(cmd_line, &jobsList);
    } else if (firstWord == "sysinfo") {
        return new SysInfoCommand(cmd_line);
    }
        
        // multi-word commands
    else if (firstWord == "chprompt") {
        return new SetPromptCommand(cmd_line);
    } // done
    else if (firstWord == "cd") {
        return new ChangeDirCommand(cmd_line);
    } // done
    
    else if (firstWord == "fg") {
        return new ForegroundCommand(cmd_line, &this->jobsList);
    } else if (firstWord == "quit") {
        return new QuitCommand(cmd_line, &this->jobsList);
    } else if (firstWord == "kill") {
        return new KillCommand(cmd_line, &this->jobsList);
    } else if (firstWord == "alias") {
        return new AliasCommand(cmd_line, &aliases);
    } else if (firstWord == "unalias") {
        return new UnAliasCommand(cmd_line, &aliases);
    } else if (firstWord == "unsetenv") {
        return new UnSetEnvCommand(cmd_line);
    } else if (firstWord == "du") {
        return new DiskUsageCommand(cmd_line);
    } else if (firstWord == "whoami") {
        return new WhoAmICommand(cmd_line);
    }
        // not built in command
        // not special command
    
    else {
//        std::cout << "not matched any built in command\n" << std::endl;
        if (string(cmd_line).find("*") == string::npos &&
            string(cmd_line).find("?") == string::npos) {
            
            return new SimpleExternalCommand(cmd_line, isBackground);
        } else {
            
            return new ComplexExternalCommand(cmd_line, isBackground);
        }
    }
    return nullptr;
}

Command::Command(const char* cmd_line) {
    strcpy(this->cmd_line, cmd_line);
    numArgs = _parseCommandLine(cmd_line, args);
};

void GetCurrDirCommand::execute() {
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != nullptr) {
        std::cout << cwd << std::endl;
    } else {
        perror("smash error: getcwd failed");
    }
}

void SmallShell::executeCommand(const char* cmd_line) {
    // TODO: Add your implementation here
    Command* cmd = CreateCommand(cmd_line);
    cmd->execute();
    
//        delete cmd;
}

SetPromptCommand::SetPromptCommand(const char* cmdLine) : BuiltInCommand(
        cmdLine) {
    char* args[COMMAND_MAX_ARGS];
    int numArgs = _parseCommandLine(cmdLine, args);
    if (numArgs == 1) { // no arguments (just command)
        char name[] = "smash";
        setCmdLine(name);
    } else { // ignore arguments aside from first
        setCmdLine(args[1]);
    }
}

void SetPromptCommand::execute() {
    SmallShell& smash = SmallShell::getInstance();
    smash.setPrompt(getCmdLine());
}

ChangeDirCommand::ChangeDirCommand(const char* cmdLine) : BuiltInCommand(
        cmdLine) {
    
    int numArgs = _parseCommandLine(cmdLine, args);
    
    auto& smash = SmallShell::getInstance();
    if (numArgs < 2) {
        perror("no arguments to cd\n");
    }
    if (numArgs > 2) {
        perror("smash error: cd: too many arguments");
    } else if (numArgs == 2) {
        
        if (strcmp("-", args[1]) == 0 &&
            smash.getPreviousUsedPath() == "\n") {
            perror("smash error: cd: OLDPWD not set");
        } else if (strcmp("-", args[1]) == 0) { // set prev to current
//            cout<< "move to prev:" <<smash.getPreviousUsedPath()<<endl;
            newTargetPath = smash.getPreviousUsedPath();
        } else {
            newTargetPath = args[1];
        }
        
        char p[PATH_MAX];
        getcwd(p, PATH_MAX);
//        cout <<"set prev to : "<<p<<endl;
        smash.setPreviousUsedPathChar(p);
    }
}

void ChangeDirCommand::execute() {
    
    if (chdir(newTargetPath.c_str()) == 0) {
        SmallShell& smash = SmallShell::getInstance();
    } else {
        // TODO: catch this exception with "smash error: cd: invalid path"
        perror("smash error: cd: invalid path");
    }
}

ForegroundCommand::ForegroundCommand(const char* cmd_line, JobsList* jobsList)
        : BuiltInCommand(cmd_line), jobsList(jobsList) {
    
    char* args[COMMAND_MAX_ARGS];
    int numArgs = _parseCommandLine(cmd_line, args);
    targetJobId = -1;
    
    if (numArgs == 1) { // no arguments - use the last job
        // Check if the list is empty before attempting to access the last job
        if (jobsList->jobs
                .empty()) { // You must implement an isEmpty or similar check
            perror("smash error: fg: jobs list is empty");
        }
        // Assuming getLastJob() returns a valid pointer to a JobEntry
        targetJobId = jobsList->getLastJob()->jobId;
        
    } else if (numArgs == 2) { // 1 argument - job ID
        try {
            // Safety check for integer conversion
            targetJobId = std::stoi(args[1]);
        } catch (const std::exception& e) {
            perror("smash error: fg: invalid job ID argument");
        }
        
    } else {
        perror("smash error: fg: invalid arguments");
    }
    
}

void ForegroundCommand::execute() {
// Find job
    auto it = jobsList->jobs.find(targetJobId);
    
    if (it == jobsList->jobs.end()) {
        //no such id
        throw std::runtime_error("smash error: fg: job-id does not exist");
    }
    
    
    // Use a reference to the JobEntry
    auto& targetJob = it->second;
    
    // Check if the job is running or stopped (it shouldn't be 'finished' here)
    std::cout << (targetJob.cmd->getCmdLine()) << " " << targetJob.pid
              << std::endl;
    
    
    //set group leader for Ctr+C
    auto& smash = SmallShell::getInstance();
    smash.setFgPid(targetJob.pid);
    if (signal(SIGINT, ctrlCHandler) == SIG_ERR) {
        perror("smash error: failed to set ctrl-C handler");
    }
    // Wait for the foreground process to finish
    waitpid(targetJob.pid, nullptr, 0);
    smash.setFgPid(-1);
    
    // Remove the job from the list after it finishes
    jobsList->removeJobById(targetJob.jobId);
}

QuitCommand::QuitCommand(const char* cmd_line, JobsList* jobsList) :
        BuiltInCommand(cmd_line) {
    
    char* args[COMMAND_MAX_ARGS];
    int numArgs = _parseCommandLine(cmd_line, args);
    if (numArgs <= 1) {
        doNothing = true;
        return;
    }
    doNothing = false;
    if (strcmp(args[1], "kill") == 0) {
        jl = jobsList;
        jobsList->removeFinishedJobs();
        cout << "smash: sending SIGKILL signal to " << jobsList->jobs.size()
             << " jobs:" << flush;
    }
}

void QuitCommand::execute() {
    
    if (doNothing) {
        //        cout << "do nothing detected" << endl;
        return;
    }
    bool first = true;
    for (auto it = jl->jobs.begin();
         it != jl->jobs.end(); it++) {
        if (first) {
            first = false;
            cout << endl;
        }
        cout << it->second.pid << ": " << it->second.cmd->getCmdLine() << endl;
        if (kill(it->second.pid, 9) == -1) {
            // Handle error, though often ignored for SIGKILL on exit
            perror("smash error: kill failed");
        }
    }
    kill(getpid(), 9);
}

void ComplexExternalCommand::execute() {
//    std::cout << "Executing complex external command: " << getCmdLine()
//              << std::endl;
    if (!isBG) {
        //foreground
        int pid = fork();
        if (pid == 0) {
            //child
            // change group ID:
            setpgrp();
            signal(SIGINT, SIG_DFL);
            
            char* const argv[] = {"/bin/bash", "-c",
                                  const_cast<char*>(getCmdLine()), NULL};
            execv(argv[0], argv);
            perror("smash error: excecution failed");
        } else if (pid > 0) {
            //parent
            auto& smash = SmallShell::getInstance();
            smash.setFgPid(pid);
            waitpid(pid, nullptr, 0);
            smash.setFgPid(-1);
        } else {
            perror("smash error: fork failed");
        }
    } else {
        //background
        int pid = fork();
        if (pid == 0) {
            //child
            char* argv[] = {"/bin/bash", "-c", const_cast<char*>(getCmdLine()),
                            NULL};
            execv(argv[0], argv);
            perror("smash error: excecution failed");
            return;
        } else if (pid > 0) {
            //parent
            // waitpid(pid, nullptr, 0);
//            std::cout << "not waiting" << std::endl;
            SmallShell::getInstance().getJobsList().addJob(this, pid, false);
//            std::cout << "added to joblist" << std::endl;
            SmallShell::getInstance().getJobsList().printJobsList();
            return;
        } else {
            perror("smash error: fork failed");
        }
    }
}

void SimpleExternalCommand::execute() {
//    std::cout << "Executing SimpleExternalCommand: " << getCmdLine()
//              << std::endl;
    if (!isBG) {
        //foreground
        int pid = fork();
        if (pid == 0) {
            //child
            setpgrp();
            signal(SIGINT, SIG_DFL);
            
            char* args[COMMAND_MAX_ARGS];
            int numArgs = _parseCommandLine(getCmdLine(), args);
            // printAllArgs(args);
            // char* const argv[] = {args , NULL};
            execvp(args[0], args);
            perror("smash error: excecution failed");
            _exit(1);
        } else if (pid > 0) {
            //parent
            auto& smash = SmallShell::getInstance();
            smash.setFgPid(pid);
            waitpid(pid, nullptr, 0);
            smash.setFgPid(-1);
        } else {
            perror("smash error: fork failed");
        }
    } else {
        //background
        int pid = fork();
        if (pid == 0) {
            //child
            char* args[COMMAND_MAX_ARGS];
            int numArgs = _parseCommandLine(getCmdLine(), args);
            // printAllArgs(args);
            // char* const argv[] = {args , NULL};
            execvp(args[0], args);
            perror("smash error: excecution failed");
            _exit(1);
        } else if (pid > 0) {
            //parent
            // waitpid(pid, nullptr, 0);
//            std::cout << "not waiting" << std::endl;
            SmallShell::getInstance().getJobsList().addJob(this, pid, false);
//            std::cout << "added to joblist" << std::endl;
            // SmallShell::getInstance().getJobsList().printJobsList();
        } else {
            perror("smash error: fork failed");
        }
    }
}

void JobsList::removeFinishedJobs() {
// We must use a safe iteration pattern that allows element removal.
    for (auto it = jobs.begin(); it != jobs.end(); ) {
        int status;
        
        // Use WNOHANG (Non-blocking) to check if the child has terminated
        pid_t result = waitpid(it->second.pid, &status, WNOHANG);
        
        if (result == it->second.pid) {
            delete it->second.cmd;
            it = jobs.erase(it);
        } else if (result == 0 || (result == -1 && errno == EINTR)) {
            ++it;
        } else if (result == -1 && errno == ECHILD) {
            // Process has been reaped by another function (unlikely but robust)
            // Clean up the map entry if the process is gone.
            delete it->second.cmd;
            it = jobs.erase(it);
        } else {
            ++it;
        }
    }
};

void JobsList::removeJobById(const int jobId) {
        auto it = jobs.find(jobId);
        if (it != jobs.end()) {
            delete it->second.cmd;
            jobs.erase(it);
        }
};

KillCommand::KillCommand(const char* cmdLine,
                         JobsList* jobsList) : BuiltInCommand(cmdLine),
                                               jobsList(jobsList) {
    
    char* args[COMMAND_MAX_ARGS];
    int numArgs = _parseCommandLine(cmdLine, args);
    
    // 1. Argument Count Check
    if (numArgs != 3) {
        // The kill command requires exactly 3 tokens: "kill", "-<signum>", and "<job-id>"
        throw runtime_error("smash error: kill: invalid arguments");
    }
    
    // Initialize members
    targetJobId = -1;
    signalNumber = -1;
    
    // 2. Signal Number Parsing
    // Check if the signal argument starts with '-'
    if (stoi(args[1]) >= 0 || args[1][0] != '-') {
        throw runtime_error("smash error: kill: invalid arguments");
    }
    
    // Extract the signal number (the string after '-')
    string signal_str = string(args[1]).substr(1);
    
    try {
        // Convert the extracted string to an integer
        signalNumber = std::stoi(signal_str);
    } catch (const std::exception& e) {
        // Handle non-integer signal argument
        perror("smash error: kill: invalid signal number");
    }
    
    // 3. Job ID Parsing
    try {
        // Convert the job ID argument to an integer
        targetJobId = std::stoi(args[2]);
    } catch (const std::exception& e) {
        // Handle non-integer job ID argument
        perror("smash error: kill: invalid job ID argument");
    }
    
    // TODO: delete args
    
}

AliasCommand::AliasCommand(const char* cmdLine,
                           map<string, string>* aliasesMap) : BuiltInCommand(
        cmdLine), aliasesMap(
        aliasesMap) { // Note: pass map by pointer/reference
    
    // Convert to std::string for easier manipulation
    string cmd = _trim(string(cmdLine));
    
    // Find the first space to isolate the command name ("alias")
    size_t firstSpace = cmd.find_first_of(WHITESPACE);
    
    // Check if the command contains anything after "alias"
    if (firstSpace == string::npos) {
        emptyAlias = true;
        // Case 1: "alias" (Only the command name)
        // This is not correct for the constructor; this logic should be in execute()
        // but for now, we set flags or store the map reference.
        return;
    }
    emptyAlias = false;
    // Find the first occurrence of '=' in the rest of the string
    size_t equalsPos = cmd.find('=', firstSpace);
    
    // Case 1: "alias" (handled above, but checking here for non-assignment)
    if (equalsPos == string::npos) {
        // If there are arguments but no '=', it's either an error or a request
        // to check a single alias (like 'alias l'), which your spec may not require.
        // For simplicity, we only implement assignment or list-all.
        throw runtime_error(
                "smash error: alias: invalid arguments (missing '=')");
    }
    
    // Case 2: "alias name='command'"
    
    // 1. Extract the Alias Name
    // The name is the substring between the first space and the '='
    string aliasName = cmd.substr(firstSpace, equalsPos - firstSpace);
    aliasName = _trim(aliasName); // Clean up whitespace around the name
    
    // 2. Extract the Command Definition
    // The command definition starts after the '='
    string commandDef = cmd.substr(equalsPos + 1);
    commandDef = _trim(commandDef); // Clean up whitespace around the command
    
    // 3. Validation and Cleaning (Checking quotes)
    if (commandDef.empty() || commandDef.front() != '\'' ||
        commandDef.back() != '\'') {
        throw runtime_error(
                "smash error: alias: command must be enclosed in single quotes");
    }
    
    // Store the cleaned alias name and the command content (inside the quotes)
    // The actual command is the substring between the quotes.
    newAliasName = aliasName;
    newAliasCommand = commandDef.substr(1, commandDef.length() - 2);
    
    // Check if the alias name itself is valid (e.g., not containing '=' or illegal characters)
    if (!isValidAliasName(newAliasName)) {
        perror("smash error: alias: invalid alias name");
    }
}

void AliasCommand::execute() {
    
    if (emptyAlias) {
        for (auto& al: *aliasesMap) {
            cout << al.first << "='" << al.second << "'" << endl;
        }
        return;
    }
    aliasesMap->insert(pair<string, string>(newAliasName, newAliasCommand));
    
};

UnAliasCommand::UnAliasCommand(const char* cmd_line,
                               map<string, string>* aliasesMap) :
        BuiltInCommand(cmd_line), aliasesMap(aliasesMap) {
    
    char* args[COMMAND_MAX_ARGS];
    int num_args = _parseCommandLine(cmd_line, args);
    if (num_args == 1) {
        //empty command
        perror("smash error: unalias: not enough arguments");
    }
    
    for (int i = 1; i < num_args; ++i) {
        aliasesMap->erase(args[i]);
    }
    
}

UnSetEnvCommand::UnSetEnvCommand(const char* cmdLine) :
        BuiltInCommand(cmdLine) {
    char* args[COMMAND_MAX_ARGS];
    int num_args = _parseCommandLine(cmdLine, args);
    if (num_args == 1) {
        perror("smash error: unsetenv: not enough arguments");
    } else {
        for (int i = 1; i < num_args; ++i) {
            envToUnset.insert(args[i]);
        }
    }
}

void UnSetEnvCommand::execute() {
    // TODO: fix this
    // Iterate over all requested variable names
    for (const std::string& varName: this->envToUnset) {
        
        // The environment variable format is "VARNAME=VALUE".
        std::string prefix = varName + "=";
        
        int i = 0;
        // Locate the pointer to the variable in the global environ array.
        while (environ[i] != nullptr) {
            
            // Check if the current entry starts with the target prefix.
            if (strncmp(environ[i], prefix.c_str(), prefix.length()) == 0) {
                
                // --- Variable found: Perform manual shift (Deletion) ---
                cout << "Variable found " << environ[i] << endl;
                int j = i;
                // Shift all subsequent pointers up by one to overwrite the found entry.
                do {
                    environ[j] = environ[j + 1];
                    j++;
                } while (environ[j] != nullptr);
                cout << "now Variable found " << environ[i] << endl;
                // We need to re-check the current index 'i' because it now holds the
                // next environment variable.
                i--;
                break;
            }
            i++; // Move to the next environment entry
        }
    }
}

void SysInfoCommand::execute() {
    
    struct utsname name;
    
    if (uname(&name) == 0) {
        printf("System: %s\n", name.sysname);
        printf("Hostname: %s\n", name.nodename);
        printf("Kernel: %s ", name.release);
        for (int i = 0; i < PATH_MAX; ++i) {
            if (name.version[i] == ' ' && name.version[i] == '\n' &&
                name.version[i] == '\0') {
                cout << name.version[i];
            } else {
                cout << endl;
                break;
            }
        }
        printf("Architecture: %s\n", name.machine);
    } else {
        // Use perror to print a descriptive error message if uname fails
        perror("Error calling uname");
        return; // Return a non-zero exit code to signal failure
    }
    
    std::ifstream stat_file("/proc/stat");
    std::string line;
    long boot_time_sec = 0;

// Search for the "btime" line in /proc/stat
    while (std::getline(stat_file, line)) {
        if (line.substr(0, 6) == "btime ") {
            std::stringstream ss(line.substr(6));
            ss >> boot_time_sec;
            break;
        }
    }
    
    if (boot_time_sec == 0) {
//            return "N/A"; // Error reading file
        perror("Error reading file");
    }

// Convert seconds since epoch to a formatted string
    std::time_t boot_time_t = boot_time_sec;
    struct tm* tm_info = std::localtime(&boot_time_t);
    
    char buffer[20];
// Format: YYYY-MM-DD HH:MM:SS
    std::strftime(buffer, 20, "%Y-%m-%d %H:%M:%S", tm_info);
    
    std::cout << "Boot Time: " << std::string(buffer) << std::endl;
}

RedirectionCommand::RedirectionCommand(const char* cmdLine) : Command(cmdLine) {
    string cmdS = string(cmdLine);
    int firstArrowIdx = cmdS.find_first_of('>');
    int lastArrowIdx = cmdS.find_last_of('>');
    if (firstArrowIdx == lastArrowIdx) {
        isAppend = false;
    } else if (firstArrowIdx + 1 == lastArrowIdx) {
        isAppend = true;
    } else {
        perror("smash error: invalid arguments");
    }
//    cout<<"firstArrowIdx " <<firstArrowIdx <<" lastArrowIdx " << lastArrowIdx
//    << "isAppend " << isAppend << endl;
    
    innerCommand = cmdS.substr(0, firstArrowIdx);
    outerFile = _trim(cmdS.substr(lastArrowIdx + 1));
//    cout << innerCommand << " into file: " << outerFile << endl;
}

void RedirectionCommand::execute() {
    
    pid_t pid = fork();
    if (pid > 0) {
//parent
        waitpid(pid, nullptr, 0);
    } else if (pid == 0) {
//child
        
        int flags = O_WRONLY | O_CREAT;
        if (isAppend) {
            flags |= O_APPEND;
        } else {
            flags |= O_TRUNC;
        }
        int fd_out = open(outerFile.c_str(), flags, 0666);
        if (fd_out < 0) {
            perror("smash error: open failed");
            _exit(1);
        }

// 3. Redirect stdout (File Descriptor 1) to the file
        if (dup2(fd_out, 1) == -1) {
            perror("smash error: dup2 failed");
            close(fd_out);
            _exit(1);
        }
        
        Command* cmd = SmallShell::getInstance()
                .CreateCommand(innerCommand.c_str());

// Check if the command is a BuiltInCommand
        auto* built_in_cmd = dynamic_cast<BuiltInCommand*>(cmd);
        
        if (built_in_cmd) {
            // 2. Case: Built-in Command (e.g., showpid, cd, chprompt)
            // Run the built-in command directly in the child process.
            // Its output will be redirected via FD 1.
            cmd->execute();
            
            // IMPORTANT: After the built-in command finishes, the child process must terminate.
            delete cmd; // Clean up the Command object created by CreateCommand
            _exit(0);   // Exit the child process!
            
        } else {
            // 3. Case: External Command (or Complex Command)
            // Use execl to replace the process image. The existing Command object
            // (SimpleExternalCommand or ComplexExternalCommand) is not needed here;
            // we use /bin/bash -c for guaranteed execution.
            
            // Clean up the Command object before exec, though exec generally cleans up memory.
            delete cmd;
            
            if (execl("/bin/bash", "bash", "-c", innerCommand.c_str(),
                      nullptr) == -1) {
                perror("smash error: execl failed");
                _exit(1);
            }
        }
    } else {
//error
        perror("smash error: fork failed");
    }
}

PipeCommand::PipeCommand(const char* cmdLine) : Command(cmdLine) {
    std::string line_str(cmdLine);
    
    // Find the position of the first pipe character
    size_t firstPipePos = line_str.find('|');
    
    // Determine the pipe type: '|&' or '|'
    bool is_stderr_pipe = false;
    if (firstPipePos != std::string::npos &&
        line_str[firstPipePos + 1] == '&') {
        isNormalPipe = false;
    } else {
        isNormalPipe = true;
    }
    // Determine the split position (either '|' or '|&')
    size_t splitPos = firstPipePos + (is_stderr_pipe ? 2 : 1);
    
    // Extract and trim command1 (everything before the pipe symbol(s))
    this->command1Line = line_str.substr(0, firstPipePos);
    this->command1Line = _trim(this->command1Line); // Assumes _trim is global
    cout << "c1" << command1Line << endl;
    // Extract and trim command2 (everything after the pipe symbol(s))
    this->command2Line = line_str.substr(splitPos);
    this->command2Line = _trim(this->command2Line); // Assumes _trim is global
    cout << " into " << command2Line << endl;
};

void PipeCommand::execute() {
    int pfd[2];
    if (pipe(pfd) == -1) {
        perror("smash error: pipe failed");
        return;
    }
    
    pid_t pid1 = fork(); // Fork Command 1 (Writer)
    
    if (pid1 == 0) {
        // --- CHILD 1 (Writer: command1) ---
        close(pfd[0]); // Close read end
        
        // Redirect stdout (1) or stderr (2) to the pipe's write end (pfd[1])
        int fd_to_redirect = isNormalPipe ? 2 : 1;
        if (dup2(pfd[1], fd_to_redirect) == -1) {
            perror("smash error: dup2 failed");
            _exit(1);
        }
        close(pfd[1]); // Close write end //MAYBE WRONG
        
        // Execute command1 (using /bin/bash -c for flexibility)
        // TODO: BuiltIn Command
        if (execl("/bin/bash", "bash", "-c", command1Line.c_str(), nullptr) ==
            -1) {
            perror("smash error: execl failed");
            _exit(1);
        }
    }
    
    // --- PARENT ---
    pid_t pid2 = fork(); // Fork Command 2 (Reader)
    
    if (pid2 == 0) {
        // --- CHILD 2 (Reader: command2) ---
        close(pfd[1]); // Close write end
        
        // Redirect stdin (0) to the pipe's read end (pfd[0])
        if (dup2(pfd[0], 0) == -1) {
            perror("smash error: dup2 failed");
            _exit(1);
        }
        close(pfd[0]); // Close read end
        
        // Execute command2
        if (execl("/bin/bash", "bash", "-c", command2Line.c_str(), nullptr) ==
            -1) {
            perror("smash error: execl failed");
            _exit(1);
        }
    } else {
        
        // --- PARENT (smash shell) ---
        // Close the parent's copies of the pipe FDs
        close(pfd[0]);
        close(pfd[1]);
        
        // Wait for both children to finish
        waitpid(pid1, nullptr, 0);
        waitpid(pid2, nullptr, 0);
    }
}

DiskUsageCommand::DiskUsageCommand(const char* cmdLine) : Command(cmdLine) {
    char* args[COMMAND_MAX_ARGS];
    int numArgs = _parseCommandLine(cmdLine, args);
    
    if (numArgs == 1) {
        if (getcwd(this->path, PATH_MAX) == nullptr) {
            throw std::runtime_error(
                    "smash error: du: cannot get current directory");
        }
    } else if (numArgs == 2) {
        // Safe copy: ensures content is copied to the class member array
        strncpy(this->path, args[1], PATH_MAX - 1);
        this->path[PATH_MAX - 1] = '\0'; // Ensure null termination
    } else {
        throw std::runtime_error("smash error: du: too many arguments");
    }
    
    for (int i = 0; i < numArgs; ++i) {
        free(args[i]);
    }
}

#include <sys/stat.h> // for stat() and struct stat
#include <dirent.h>  // for opendir(), readdir(), closedir()

int getKBDiskUsage(const std::string& path) {
    struct stat st;
    
    // 1. Get file status (size and type)
    if (stat(path.c_str(), &st) == -1) {
        // Use cerr for errors, as stdout might be redirected
        std::cerr << "smash error: stat failed for " << path << std::endl;
        return 0;
    }
    
    // Initialize total usage with the size of the current file/directory, rounded up to KB
    // (st.st_blocks * 512) is the accurate disk usage on many systems,
    // but st.st_size / 1024 is often used for simplicity in shell projects.
    int total_usage_kb = (st.st_size + 1023) / 1024;
    
    // 2. Check if it is a directory
    if (S_ISDIR(st.st_mode)) {
        //TODO: cant use open dir!
        DIR* dir = opendir(path.c_str());
        if (!dir) {
            std::cerr << "smash error: opendir failed for " << path
                      << std::endl;
            return total_usage_kb; // Return size of the directory entry itself
        }
        
        struct dirent* entry;
        //cant use readdir
        while ((entry = readdir(dir)) != nullptr) {
            std::string name = entry->d_name;
            
            // Skip '.' (current directory) and '..' (parent directory)
            if (name == "." || name == "..") {
                continue;
            }
            
            // Construct the full path for the recursive call
            std::string full_path = path + "/" + name;
            
            // Recursively calculate and add usage
            total_usage_kb += getKBDiskUsage(full_path);
        }
        //TODO: cnat use closedir
        closedir(dir);
    }
    
    // 3. Return the total size (current size + size of all contents)
    return total_usage_kb;
};

void DiskUsageCommand::execute() {
    
    // 2. Start the recursive calculation
    int total_kb = getKBDiskUsage(string(path));
    
    // 3. Print the result
    std::cout << "Total disk usage: " << total_kb << " KB" << std::endl;
}

#include <sys/types.h>
#include <pwd.h>

WhoAmICommand::WhoAmICommand(const char* cmdLine) : Command(cmdLine) {
// 1. Get the real user ID and group ID
    userId = geteuid();
    struct passwd* data = getpwuid(userId);
    if (data == nullptr) {
        perror("Smash getpwuid failed");
    }
    strcpy(username, data->pw_name);
    groupId = data->pw_uid;
    strcpy(homeDir, data->pw_dir);
}

void WhoAmICommand::execute() {
    printf("%lld\n%lld\n%s %s\n", userId, groupId,
           username, homeDir);
}