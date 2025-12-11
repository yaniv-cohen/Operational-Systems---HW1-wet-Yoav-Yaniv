// Ver: 04-11-2025
#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <vector>
#include <map>
#include <string.h>
#include <cstring>
#include <unistd.h>
#include <unordered_set>
#include <sys/stat.h>
#include <fcntl.h>

#define COMMAND_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)
using namespace std;

class Command {
    char cmd_line[COMMAND_MAX_LENGTH];
    char *args[COMMAND_MAX_ARGS];
    int numArgs;
public:
    Command(const char* cmd_line);
    
    virtual ~Command() {
        for (int i = 0; i < numArgs; i++) {
                free(args[i]);
        }
    };
    
    virtual void execute() = 0;
    
    // virtual void prepare();
    // virtual void cleanup();
    //  TODO: Add your extra methods if needed
    const char* getCmdLine() const {
        return cmd_line;
    }
    
    void setCmdLine(char* newLine) {
        strcpy(this->cmd_line, newLine);
    }
};

class BuiltInCommand : public Command {
public:
    BuiltInCommand(const char* cmd_line) : Command(cmd_line) {
//        std::cout << "in BuiltInCommand constructor\n"
//                  << std::endl;
    };
    
    virtual ~BuiltInCommand() = default;
};

//
class ExternalCommand : public Command {

public:
    bool isBG;
    
    ExternalCommand(const char* cmd_line, bool isBG) : Command(cmd_line),
                                                       isBG(isBG) {
    };
    
    virtual ~ExternalCommand() = default;
};

class SimpleExternalCommand : public ExternalCommand {

public:
    SimpleExternalCommand(const char* cmd_line, bool isBG) : ExternalCommand(
            cmd_line, isBG) {
    };
    
    virtual ~SimpleExternalCommand() = default;
    
    void execute() override;
};

class ComplexExternalCommand : public ExternalCommand {

public:
    ComplexExternalCommand(const char* cmd_line, bool isBG) : ExternalCommand(
            cmd_line,
            isBG) {
//        std::cout << "in ComplexExternalCommand constructor BG=" << isBG
//                  << std::endl;
    };
    
    virtual ~ComplexExternalCommand() {
    }
    
    void execute() override;
};

//
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

class DiskUsageCommand : public Command {
    char path[PATH_MAX];
public:
    DiskUsageCommand(const char* cmd_line);
    
    virtual ~DiskUsageCommand() {
    }
    
    void execute() override;
};

class WhoAmICommand : public Command {
    char username[PATH_MAX];
    uid_t userId;
    gid_t groupId;
    char homeDir[PATH_MAX];
public:
    WhoAmICommand(const char* cmd_line);
    
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
    // TODO: Add your data members public:
    string newTargetPath;
    char **args;
public:
    ChangeDirCommand(const char* cmdLine);
    
    virtual ~ChangeDirCommand() {
        if (args) {
            free(args);
    };
    
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
        // TODO: Add your data members
    public:
        Command* cmd = nullptr;
        int jobId = 1;
        bool isFinished = false;
        int pid = 0;
        
        JobEntry() = default;
        
        JobEntry(int jobId, Command* cmd, bool isFinished, int pid)
                : jobId(jobId), cmd(cmd), isFinished(isFinished), pid(pid) {
        }
        
        JobEntry(JobEntry&&) = default;
        JobEntry& operator=(JobEntry&&) = default;
        
        JobEntry(const JobEntry&) = delete;
        JobEntry& operator=(const JobEntry&) = delete;
        
        
        ~JobEntry() {};
    };

// TODO: Add your data members
    map<int, JobEntry> jobs;

public:
    JobsList() = default;
    JobsList(const JobsList&) = delete;
    JobsList& operator=(const JobsList&) = delete;
    
    ~
    
    JobsList() {
        for (auto& pair: jobs) {
            delete pair.second.cmd;
        }
    }
    
    void removeJobById(const int jobId);
    
    void addJob(Command* cmd, int pid, bool isFinished = false) {
        int newJobId = -1;
        auto it = jobs.begin();
        for (int i = 1; i <= 100; i++, it++) {
            if (it == jobs.end() || i != it->first) {
                newJobId = i;
                break;
            }
        } // find first open jobId. maximum of 100 jobs (page 2 under assumptions)
        if (newJobId == -1) {
            throw runtime_error(
                    "Jobs full"); // ask what error to put for full jobs list
        }
//        jobs[newJobId] =  JobEntry(newJobId, cmd, isFinished, pid);
        jobs.insert(
                make_pair(newJobId,
                          JobsList::JobEntry(newJobId, cmd, isFinished, pid)
                )
        );
    };
    
    void printJobsList() {
        removeFinishedJobs();
        for (const auto& [id, job]: jobs) {
            std::cout << "[" << id << "] " << job.cmd->getCmdLine()
                      << std::endl;
        }
    };

// void killAllJobs(){};
    
    void removeFinishedJobs();
    
    JobsList::JobEntry* getJobById(int jobId) {
        return &jobs.at(jobId);
    };
    
    JobsList::JobEntry* getLastJob() {
        return &prev(jobs.end())->second;
    };
};

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
        
        // 4. Update job list state (for SIGKILL/SIGTERM)
        // A robust shell would clean up the job list if SIGKILL (9) or SIGTERM (15) was sent,
        // but typically cleanup is deferred to the removeFinishedJobs via waitpid in the main loop/signal handler.
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
    bool doNothing;
public:
    QuitCommand(const char* cmd_line, JobsList* jobsList);
    
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
    
    static bool isValidAliasName(string& s) {
        for (char c: s) {
            if ((c < 0 || c > 9) && (c < 'a' || c > 'z') &&
                (c < 'A' || c > 'Z') && c != '_')
                return false;
        }
        vector<string> illegals = {
                "chprompt",
                "showpid",
                "pwd",
                "cd",
                "jobs",
                "fg",
                "quit",
                "kill",
                "alias",
                "unalias",
                "unsetenv",
                "sysinfo",
                ""
        };
        for (auto i: illegals) {
            if (s.compare(i) == 0) {
//                cout << "illegal! " << i << endl;
                return false;
            }
        }
        return true;
    };
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
    unordered_set<string> envToUnset;
public:
    UnSetEnvCommand(const char* cmd_line);
    
    virtual ~UnSetEnvCommand() {
    }
    
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
    
    void setPreviousUsedPathString(string previousUsed) {
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
    
    pid_t getFgPid() { return fgPid; };
    
    pid_t setFgPid(pid_t newPid) { fgPid = newPid; };
    // TODO: add extra methods as needed
};

#endif // SMASH_COMMAND_H_
