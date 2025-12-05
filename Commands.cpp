#include <unistd.h>
#include <string.h>
#include <cstring>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <limits.h>
#include <iomanip>
#include "Commands.h"
#include <signal.h>
#include <utility>

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
    for (char c : firstWord) {
        if(!isalnum(c) && c!= '_')
        {
        
        }
    }
    // 4. Check for Alias Substitution
    // Note: The 'aliases' map is assumed to be a member of SmallShell.
    auto alias_it = aliases.find(firstWord);
    
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
    }
        //    else if (firstWord == "sysinfo")
        //    {
        //        return new SysInfoCommand();
        //    }
        
        // multi-word commands
    else if (firstWord == "chprompt") {
        return new SetPromptCommand(cmd_line);
    } // done
    else if (firstWord == "cd") {
        return new ChangeDirCommand(cmd_line, this->previousUsedPath);
    } // done
    
    else if (firstWord == "fg") {
        return new ForegroundCommand(cmd_line, &this->jobsList);
    } else if (firstWord == "quit") {
        return new QuitCommand(cmd_line, &this->jobsList);
    } else if (firstWord == "kill") {
        return new KillCommand(cmd_line, &this->jobsList);
    } else if (firstWord == "alias") {
        return new AliasCommand(cmd_line, &aliases);
    }else if (firstWord == "unalias") {
        return new UnAliasCommand(cmd_line, &aliases);
    }
    else if (firstWord == "unsetenv") {
        return new UnSetEnvCommand(cmd_line);
    }
        // not built in command
        // not special command
    
    else {
        std::cout << "not matched any built in command\n" << std::endl;
        if (string(cmd_line).find("*") == string::npos &&
            string(cmd_line).find("?") == string::npos) {
            
            std::cout << "smash: simpleExternal command" << std::endl;
            return new SimpleExternalCommand(cmd_line, isBackground);
        } else {
            std::cout << "smash: complexExternal command" << std::endl;
            
            return new ComplexExternalCommand(cmd_line, isBackground);
        }
    }
    return nullptr;
}

void GetCurrDirCommand::execute() {
    printf("in get curr dir\n");
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != nullptr) {
        std::cout << cwd << std::endl;
    } else {
        perror("smash error: getcwd failed");
    }
}

void SmallShell::executeCommand(const char* cmd_line) {
    // TODO: Add your implementation here
    // for example:
    
    std::cout << "smash: cmd_line " << cmd_line << std::endl;
    Command* cmd = CreateCommand(cmd_line);
    std::cout << "smash: executing command: " << cmd_line << std::endl;
    cmd->execute();
    // Please note that you must fork smash process for some commands (e.g., external commands....)
}

SetPromptCommand::SetPromptCommand(const char* cmdLine) : BuiltInCommand(
        cmdLine) {
    char* args[COMMAND_MAX_ARGS];
    int i = _parseCommandLine(cmdLine, args);
    if (i == 1) { // no arguments (just command)
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

ChangeDirCommand::ChangeDirCommand(const char* cmdLine,
                                   const char* previousUsed) : BuiltInCommand(
        cmdLine) {
    
    // char* newTargetPath;
    // char* previousUsedPath;
    newTargetPath = new char[PATH_MAX];
    previousUsedPath = new char[PATH_MAX];
    
    char* args[COMMAND_MAX_ARGS];
    std::cout << "|" << cmdLine << std::endl;
    int i = _parseCommandLine(cmdLine, args);
    
    // printAllArgs(args);
    if (i < 2) {
        std::cout << "no arguments to cd\n";
        getcwd(newTargetPath, PATH_MAX);
        if (previousUsed != nullptr) {
            
            strcpy(previousUsedPath, previousUsed);
        } else {
            std::cout << "no arguments to cd4\n";
            
            previousUsedPath = nullptr;
        }
    }
    if (i > 2) {
        throw runtime_error("smash error: cd: too many arguments");
    } else if (i == 2) {
        if (strcmp("-", args[1]) == 0 && previousUsed == nullptr) {
            throw runtime_error("smash error: cd: OLDPWD not set");
        } else if (strcmp("-", args[1]) == 0) { // set prev to current
            getcwd(previousUsedPath, PATH_MAX);
            strcpy(newTargetPath, previousUsed);
        } else {
            getcwd(previousUsedPath, PATH_MAX);
            strcpy(newTargetPath, args[1]);
        }
    }
}

void ChangeDirCommand::execute() {
    
    std::cout << "newTargetPath:" << newTargetPath << strlen(newTargetPath)
              << std::endl;
    for (size_t i = 0; i < strlen(newTargetPath); i++) {
        if (newTargetPath[i] == '\n') {
            newTargetPath[i] = '\0';
            break;
        }
    }
    
    if (chdir(newTargetPath) == 0) {
        std::cout << "smash: about to change directory to " << newTargetPath
                  << std::endl;
        SmallShell& smash = SmallShell::getInstance();
        std::cout << "smash: previous used path is " << previousUsedPath
                  << std::endl;
        smash.setPreviousUsedPath(previousUsedPath);
    } else {
        // TODO: catch this exception with "smash error: cd: invalid path"
        throw runtime_error("smash error: cd: invalid path");
    }
}

ForegroundCommand::ForegroundCommand(const char* cmd_line, JobsList* jobsList)
        : BuiltInCommand(cmd_line), jobsList(jobsList) {
    
    char* args[COMMAND_MAX_ARGS];
    int num_args = _parseCommandLine(cmd_line, args);
    targetJobId = -1;
    
    if (num_args == 1) { // no arguments - use the last job
        // Check if the list is empty before attempting to access the last job
        if (jobsList->jobs
                .empty()) { // You must implement an isEmpty or similar check
            throw runtime_error("smash error: fg: jobs list is empty");
        }
        // Assuming getLastJob() returns a valid pointer to a JobEntry
        targetJobId = jobsList->getLastJob()->jobId;
        
    } else if (num_args == 2) { // 1 argument - job ID
        try {
            // Safety check for integer conversion
            targetJobId = std::stoi(args[1]);
        } catch (const std::exception& e) {
            throw runtime_error("smash error: fg: invalid job ID argument");
        }
        
    } else {
        throw runtime_error("smash error: fg: invalid arguments");
    }
    
}

void ForegroundCommand::execute() {
// 1. Find the job safely
        auto it = jobsList->jobs.find(targetJobId);
    
    if (it == jobsList->jobs.end()) {
        //no such id
        throw std::runtime_error("smash error: fg: job-id does not exist");
    }
    
    
    // Use a reference to the JobEntry
    auto& targetJob = it->second;
    
    // Check if the job is running or stopped (it shouldn't be 'finished' here)
    // You should probably check for a valid, non-negative PID.
    
    std::cout << (targetJob.cmd->getCmdLine()) << " " << targetJob.pid
              << std::endl;
    
    // Wait for the foreground process to finish
    // Note: If you want to handle signals/errors, you should use
    // WUNTRACED or other options and check the return value.
    // TODO: consider tcsetpgrp
    waitpid(targetJob.pid, nullptr, 0);
    
    // Remove the job from the list after it finishes
    // jobsList->removeJobById(targetJob.jobId);
}

QuitCommand::QuitCommand(const char* cmd_line, JobsList* jobsList) :
        BuiltInCommand(cmd_line) {
    
    char* args[COMMAND_MAX_ARGS];
    int num_args = _parseCommandLine(cmd_line, args);
    if (num_args <= 1) {
        doNothing = true;
        return;
    }
    doNothing = false;
    if (strcmp(args[1], "kill") == 0) {
        jl = jobsList;
        cout << "create quit command" << endl;
        jobsList->removeFinishedJobs();
        cout << "sending SIGKILL signal to " << jobsList->jobs.size()
             << " jobs: "
             << endl;
    }
}

void QuitCommand::execute() {
    
    cout << "quit execute" << endl;
    if (doNothing) {
        cout << "do nothing detected" << endl;
        return;
    }
    for (auto it = jl->jobs.begin();
         it != jl->jobs.end(); it++) {
        cout << it->second.pid << ": " << it->second.cmd->getCmdLine() << endl;
        if (kill(it->second.pid, 9) == -1) {
            // Handle error, though often ignored for SIGKILL on exit
            perror("smash error: kill failed");
        }
    }
    kill(getpid(), 9);
}

void ComplexExternalCommand::execute() {
    std::cout << "Executing complex external command: " << getCmdLine()
              << std::endl;
    if (!isBG) {
        //foreground
        int pid = fork();
        if (pid == 0) {
            //child
            char* const argv[] = {"/bin/bash", "-c",
                                  const_cast<char*>(getCmdLine()), NULL};
            execv(argv[0], argv);
            perror("smash error: excecution failed");
        } else if (pid > 0) {
            //parent
            waitpid(pid, nullptr, 0);
            std::cout << "finished waiting" << std::endl;
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
            std::cout << "not waiting" << std::endl;
            SmallShell::getInstance().getJobsList().addJob(this, pid, false);
            std::cout << "added to joblist" << std::endl;
            SmallShell::getInstance().getJobsList().printJobsList();
            return;
        } else {
            perror("smash error: fork failed");
        }
    }
}

void SimpleExternalCommand::execute() {
    std::cout << "Executing SimpleExternalCommand: " << getCmdLine()
              << std::endl;
    if (!isBG) {
        //foreground
        int pid = fork();
        if (pid == 0) {
            //child
            char* args[COMMAND_MAX_ARGS];
            _parseCommandLine(getCmdLine(), args);
            // printAllArgs(args);
            // char* const argv[] = {args , NULL};
            execvp(args[0], args);
            perror("smash error: excecution failed");
        } else if (pid > 0) {
            //parent
            waitpid(pid, nullptr, 0);
            std::cout << "finished waiting" << std::endl;
        } else {
            perror("smash error: fork failed");
        }
    } else {
        //background
        int pid = fork();
        if (pid == 0) {
            //child
            char* args[COMMAND_MAX_ARGS];
            _parseCommandLine(getCmdLine(), args);
            // printAllArgs(args);
            // char* const argv[] = {args , NULL};
            execvp(args[0], args);
            perror("smash error: excecution failed");
        } else if (pid > 0) {
            //parent
            // waitpid(pid, nullptr, 0);
            std::cout << "not waiting" << std::endl;
            SmallShell::getInstance().getJobsList().addJob(this, pid, false);
            std::cout << "added to joblist" << std::endl;
            // SmallShell::getInstance().getJobsList().printJobsList();
        } else {
            perror("smash error: fork failed");
        }
    }
}

void JobsList::removeFinishedJobs() {
    for (auto it = jobs.begin(); it != jobs.end();) {
        if (waitpid(it->second.pid, nullptr, WNOHANG) != 0) {
            it = jobs.erase(it);
        } else {
            ++it;
        }
    }
};

void JobsList::removeJobById(const int jobId) {
    auto it = jobs.find(jobId);
    jobs.erase(it);
};

KillCommand::KillCommand(const char* cmdLine,
                         JobsList* jobsList) : BuiltInCommand(cmdLine),
                                               jobsList(jobsList) {
    
    char* args[COMMAND_MAX_ARGS];
    int num_args = _parseCommandLine(cmdLine, args);
    
    // 1. Argument Count Check
    if (num_args != 3) {
        // The kill command requires exactly 3 tokens: "kill", "-<signum>", and "<job-id>"
        throw runtime_error("smash error: kill: invalid arguments");
    }
    
    // Initialize members
    targetJobId = -1;
    signalNumber = -1;
    
    // 2. Signal Number Parsing
    // Check if the signal argument starts with '-'
    if (stoi(args[1])>= 0  || args[1][0] != '-') {
        throw runtime_error("smash error: kill: invalid arguments");
    }
    
    // Extract the signal number (the string after '-')
    string signal_str = string(args[1]).substr(1);
    
    try {
        // Convert the extracted string to an integer
        signalNumber = std::stoi(signal_str);
    } catch (const std::exception& e) {
        // Handle non-integer signal argument
        throw runtime_error("smash error: kill: invalid signal number");
    }
    
    // 3. Job ID Parsing
    try {
        // Convert the job ID argument to an integer
        targetJobId = std::stoi(args[2]);
    } catch (const std::exception& e) {
        // Handle non-integer job ID argument
        throw runtime_error("smash error: kill: invalid job ID argument");
    }
    
    // TODO: delete args
    
}

AliasCommand::AliasCommand(const char* cmdLine,
                           map<string, string>* aliasesMap) : BuiltInCommand(
        cmdLine), aliasesMap(aliasesMap) { // Note: pass map by pointer/reference
    
    // Convert to std::string for easier manipulation
    string cmd = _trim(string(cmdLine));
    
    // Find the first space to isolate the command name ("alias")
    size_t firstSpace = cmd.find_first_of(WHITESPACE);
    
    // Check if the command contains anything after "alias"
    if (firstSpace == string::npos) {
        emptyAlias =true;
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
        throw runtime_error("smash error: alias: invalid arguments (missing '=')");
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
    if (commandDef.empty() || commandDef.front() != '\'' || commandDef.back() != '\'') {
        throw runtime_error("smash error: alias: command must be enclosed in single quotes");
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
    
    if(emptyAlias){
        for (auto &al: *aliasesMap) {
            cout << al.first <<"='"<< al.second <<"'" << endl;
        }
        return;
    }
    aliasesMap->insert(pair<string, string>(newAliasName, newAliasCommand));
    
};


UnAliasCommand::UnAliasCommand(const char *cmd_line,map<string, string>* aliasesMap):
        BuiltInCommand(cmd_line), aliasesMap(aliasesMap){
    
    char* args[COMMAND_MAX_ARGS];
    int num_args = _parseCommandLine(cmd_line, args);
    if(num_args==1){
        //empty command
        perror("smash error: unalias: not enough arguments");
    }
    
    for (int i = 1; i < num_args; ++i) {
        aliasesMap->erase(args[i]);
    }
    
}

UnSetEnvCommand::UnSetEnvCommand(const char* cmdLine):
BuiltInCommand(cmdLine){
    char* args[COMMAND_MAX_ARGS];
    int num_args = _parseCommandLine(cmdLine, args);
    if(num_args==1){
        perror("smash error: unsetenv: not enough arguments");
    }
    else{
        for (int i = 1; i < num_args; ++i) {
        envToUnset.insert(args[i]);
        }
    }
}

void UnSetEnvCommand::execute() {
    
    // Iterate over all requested variable names
    for (const std::string& varName : this->envToUnset) {
        
        // The environment variable format is "VARNAME=VALUE".
        std::string prefix = varName + "=";
        
        int i = 0;
        // Locate the pointer to the variable in the global environ array.
        while (environ[i] != nullptr) {
            
            // Check if the current entry starts with the target prefix.
            if (strncmp(environ[i], prefix.c_str(), prefix.length()) == 0) {
                
                // --- Variable found: Perform manual shift (Deletion) ---
                cout<<"Variable found "<< environ[i] << endl;
                int j = i;
                // Shift all subsequent pointers up by one to overwrite the found entry.
                do {
                    environ[j] = environ[j + 1];
                    j++;
                } while (environ[j] != nullptr);
                cout<<"now Variable found "<< environ[i] << endl;
                // We need to re-check the current index 'i' because it now holds the
                // next environment variable.
                i--;
                break;
            }
            i++; // Move to the next environment entry
        }
    }
}