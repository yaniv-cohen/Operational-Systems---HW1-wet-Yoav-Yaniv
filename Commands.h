// Ver: 04-11-2025
#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <vector>
#include <map>
#include <string.h>
#include <cstring>
#include <unordered_set>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#define COMMAND_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)
using namespace std;

class Command {
    char cmd_line[COMMAND_MAX_LENGTH];
public:
    char* args[COMMAND_MAX_ARGS + 1];
    int numArgs;
    Command(const char* cmd_line);
    
    virtual ~Command();
    
    virtual void execute() = 0;

    const char* getCmdLine() const {
        return cmd_line;
    }
    
    void setCmdLine(char* newLine) {
        strcpy(this->cmd_line, newLine);
    }
};

class BuiltInCommand : public Command {
public:
    BuiltInCommand(const char* cmd_line) : Command(cmd_line) {};
    
    virtual ~BuiltInCommand() = default;
};

class ExternalCommand : public Command {

public:
    bool isBG;
    
    ExternalCommand(const char* cmd_line, bool isBG) : Command(cmd_line),
                                                       isBG(isBG) {};
    virtual ~ExternalCommand() = default;
};

class SimpleExternalCommand : public ExternalCommand {

public:
    SimpleExternalCommand(const char* cmd_line, bool isBG) : ExternalCommand(
            cmd_line, isBG) {};
    
    virtual ~SimpleExternalCommand() = default;
    
    void execute() override;
};

class ComplexExternalCommand : public ExternalCommand {

public:
    ComplexExternalCommand(const char* cmd_line, bool isBG) : ExternalCommand(
            cmd_line,isBG) {};
    
    virtual ~ComplexExternalCommand() = default;
    
    void execute() override;
};

class RedirectionCommand : public Command {
    // TODO: Add your data members
    string innerCommand;
    string outerFile;
    bool isAppend;

public:
    explicit RedirectionCommand(const char* cmdLine);
    
    virtual ~RedirectionCommand() {
    }
    
    void execute() override;
    
};

//
class PipeCommand : public Command {
    // TODO: Add your data members
    string command1Line;
    string command2Line;
    bool isNormalPipe;
public:
    PipeCommand(const char* cmd_line);
    
    virtual ~PipeCommand() {
    }
    
    void execute() override;
};
//
#include <limits.h>
#include <memory>

class DiskUsageCommand : public Command {
    char path[PATH_MAX];
public:
    DiskUsageCommand(const char* cmd_line);
    
    virtual ~DiskUsageCommand() {
    }
    
    void execute() override;
};

class WhoAmICommand : public Command {

public:
    WhoAmICommand(const char *cmdLine) : Command(cmdLine){};
    
    virtual ~WhoAmICommand() {
    };
    
    void execute() override;
};

//
// class USBInfoCommand : public Command
//{
//    // TODO: Add your data members **BONUS: 10 Points**
// public:
//    USBInfoCommand(const char *cmd_line);
//
//    virtual ~USBInfoCommand()
//    {
//    }
//
//    void execute() override;
//};

class DoNothingCommand : public BuiltInCommand {
public:
    DoNothingCommand(const char* cmdLine) : BuiltInCommand(cmdLine) {};
    
    virtual ~DoNothingCommand() = default;
    
    void execute() override {};
};

class ChangeDirCommand : public BuiltInCommand {
public:
    string newTargetPath;

    ChangeDirCommand(const char* cmdLine);
    
    virtual ~ChangeDirCommand() = default;
    
    void execute() override;
}; // done

class GetCurrDirCommand : public BuiltInCommand {
public:
    GetCurrDirCommand(const char* cmdLine) : BuiltInCommand(cmdLine) {};
    
    virtual ~GetCurrDirCommand() = default;
    
    void execute() override;
}; // done

class ShowPidCommand : public BuiltInCommand {
public:
    ShowPidCommand(const char* cmdLine) : BuiltInCommand(cmdLine) {};
    
    virtual ~ShowPidCommand() = default;
    
    void execute() override {
        std::cout << "smash pid is " << getpid() << std::endl;
    }
}; // done

class SetPromptCommand : public BuiltInCommand {

public:
    SetPromptCommand(const char* cmdLine);
    
    void execute() override;
    
}; // done

class JobsList {
public:
    class JobEntry {
    public:
        int jobId = 1;
        std::unique_ptr<Command> cmd = nullptr;
        int pid = 0;
        
        JobEntry() = default;
        
        JobEntry(int jobId, Command* cmd, int pid)
                : jobId(jobId), cmd(cmd), pid(pid) {}
        
        JobEntry(JobEntry&&) = default;
        JobEntry& operator=(JobEntry&&) = default;
        
        JobEntry(const JobEntry&) = delete;
        JobEntry& operator=(const JobEntry&) = delete;
        
        ~JobEntry() = default;
    };

    map<int, JobEntry> jobs;

public:
    JobsList() = default;
    JobsList(const JobsList&) = delete;
    JobsList& operator=(const JobsList&) = delete;
    
    ~JobsList() = default;
    
    void removeJobById(const int jobId){
        jobs.erase(jobId);
    }
    
    void addJob(Command* cmd, int pid);
    
    void printJobsList() {
        for (const auto& entry : jobs) {
            cout << "[" << entry.second.jobId << "] " << entry.second.cmd->getCmdLine() << endl;
        }
    };
    
    void removeFinishedJobs();
    
    JobsList::JobEntry* getJobById(int jobId) {
        return &jobs.at(jobId);
    };
    
    JobsList::JobEntry* getLastJob() {
        return &prev(jobs.end())->second;
    };
}; //done

class JobsCommand : public BuiltInCommand {
    // TODO: Add your data members
    JobsList* jobsList;

public:
    JobsCommand(const char* cmdLine, JobsList* jobs) : BuiltInCommand(cmdLine),
                                                       jobsList(jobs) {};
    
    virtual ~JobsCommand() = default;
    
    void execute() override {
        jobsList->printJobsList();
    };
}; // done

class KillCommand : public BuiltInCommand {
    // TODO: Add your data members
    int targetJobId;
    int signalNumber;
    JobsList* jobsList;
public:
    KillCommand(const char* cmdLine, JobsList* jobsList);
    
    virtual ~KillCommand() {
    };
    
    void execute() override {
        // 1. Find and validate the job
        JobsList::JobEntry* targetJob = jobsList
                ->getJobById(targetJobId); // You'll need this method
        
        if (targetJob == nullptr) {
            // This check must happen AFTER successful parsing
            throw runtime_error(
                    "smash error: kill: job-id " + std::to_string(targetJobId) +
                    " does not exist");
        }
        
        pid_t pid = targetJob->pid;
        
        // 2. Send the signal
        if (kill(pid, signalNumber) == -1) {
            // Check for kill failure (e.g., permission denied)
            perror("smash error: kill failed");
            return;
        }
        
        // 3. Output confirmation
        cout << "signal number " << signalNumber << " was sent to pid " << pid
             << endl;
    };
};

class ForegroundCommand : public BuiltInCommand {
    // TODO: Add your data members
    int targetJobId;
    JobsList* jobsList;

public:
    ForegroundCommand(const char* cmd_line, JobsList* jobsList);
    
    virtual ~ForegroundCommand() {
    }
    
    void execute() override;
};

class QuitCommand : public BuiltInCommand {
    JobsList* jl;
public:
    QuitCommand(const char* cmd_line, JobsList* jobsList) : BuiltInCommand(
            cmd_line),jl(jobsList) {};
    
    virtual ~QuitCommand() {
    }
    
    void execute() override;
};

class AliasCommand : public BuiltInCommand {
    map<string, string>* aliasesMap;
    string newAliasName;
    string newAliasCommand;
    bool emptyAlias;
public:
    AliasCommand(const char* cmd_line, map<string, string>* aliasesMap);
    
    virtual ~AliasCommand() {
    }
    
    int isValidAliasName(string& s);
    void execute() override;
};

//
class UnAliasCommand : public BuiltInCommand {
    map<string, string>* aliasesMap;
public:
    UnAliasCommand(const char* cmd_line, map<string, string>* aliasesMap);
    
    virtual ~UnAliasCommand() {
    }
    
    void execute() override {};
};

class UnSetEnvCommand : public BuiltInCommand {
public:
    UnSetEnvCommand(const char* cmd_line);
    
    virtual ~UnSetEnvCommand() {
    }
    bool checkVarExistsInProc( const string& s );
    void removeFromEnviron( const string& s );
    void execute() override;
};

//
class SysInfoCommand : public BuiltInCommand {
public:
    SysInfoCommand(const char* cmdLine) : BuiltInCommand(cmdLine) {};
    
    virtual ~SysInfoCommand() {
    }
    
    void execute() override;
    
};

class SmallShell {
private:
    // TODO: Add your data members
    string previousUsedPath = "\n";
    string prompt = "smash";
    JobsList jobsList;
    map<string, string> aliases;
    
    pid_t fgPid = -1;
    
    SmallShell();

public:
    Command* CreateCommand(const char* cmd_line);
    
    SmallShell(SmallShell const&) = delete;     // disable copy ctor
    void operator=(SmallShell const&) = delete; // disable = operator
    static SmallShell& getInstance()             // make SmallShell singleton
    {
        static SmallShell instance; // Guaranteed to be destroyed.
        // Instantiated on first use.
        return instance;
    }
    
    ~SmallShell();
    
    void executeCommand(const char* cmd_line);
    
    void setPrompt(const string& newPrompt) {
        prompt = newPrompt;
    }
    
    string getPrompt() {
        return prompt;
    }
    
    void setPreviousUsedPathString(string &previousUsed) {
        previousUsedPath = previousUsed;
    }
    
    void setPreviousUsedPathChar(const char* arr) {
        previousUsedPath = arr;
    }
    
    string getPreviousUsedPath() {
        return previousUsedPath;
    }
    
    JobsList& getJobsList() {
        return jobsList;
    }
    
    pid_t getFgPid() const { return fgPid; };
    
    void setFgPid(pid_t newPid) { fgPid = newPid; }

    static bool isBuiltinCommand(const string &s);
};

#endif // SMASH_COMMAND_H_
