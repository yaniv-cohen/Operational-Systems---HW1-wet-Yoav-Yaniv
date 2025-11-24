// Ver: 04-11-2025
#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <vector>
#include <map>
#include <string.h>
#include <unistd.h>
#define COMMAND_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)
using namespace std;

class Command
{
    // TODO: Add your data members
    char *cmd_line;

public:
    Command(const char *cmd_line)
    {
        this->cmd_line = strdup(cmd_line);
    };

    virtual ~Command() = default;

    virtual void execute() = 0;

    // virtual void prepare();
    // virtual void cleanup();
    //  TODO: Add your extra methods if needed
    const char *getCmdLine() const
    {
        return cmd_line;
    }
    void setCmdLine(char* newLine){
        cmd_line = newLine;
    }
};

class BuiltInCommand : public Command
{
public:
    BuiltInCommand(const char *cmd_line) : Command(cmd_line){};

    virtual ~BuiltInCommand() = default;
//    {
//    }
};
//
//class ExternalCommand : public Command
//{
//public:
//    ExternalCommand(const char *cmd_line);
//
//    virtual ~ExternalCommand()
//    {
//    }
//
//    void execute() override;
//};
//
//class RedirectionCommand : public Command
//{
//    // TODO: Add your data members
//public:
//    explicit RedirectionCommand(const char *cmd_line);
//
//    virtual ~RedirectionCommand()
//    {
//    }
//
//    void execute() override;
//};
//
//class PipeCommand : public Command
//{
//    // TODO: Add your data members
//public:
//    PipeCommand(const char *cmd_line);
//
//    virtual ~PipeCommand()
//    {
//    }
//
//    void execute() override;
//};
//
//class DiskUsageCommand : public Command
//{
//public:
//    DiskUsageCommand(const char *cmd_line);
//
//    virtual ~DiskUsageCommand()
//    {
//    }
//
//    void execute() override;
//};
//
//class WhoAmICommand : public Command
//{
//public:
//    WhoAmICommand(const char *cmd_line);
//
//    virtual ~WhoAmICommand()
//    {
//    }
//
//    void execute() override;
//};
//
//class USBInfoCommand : public Command
//{
//    // TODO: Add your data members **BONUS: 10 Points**
//public:
//    USBInfoCommand(const char *cmd_line);
//
//    virtual ~USBInfoCommand()
//    {
//    }
//
//    void execute() override;
//};

class ChangeDirCommand : public BuiltInCommand
{
    // TODO: Add your data members public:
    char* newTargetPath;
    char* previousUsedPath;

public:
    ChangeDirCommand(const char *cmdLine, const char *previousUsed);


    virtual ~ChangeDirCommand() = default;

    void execute() override;
}; //done

class GetCurrDirCommand : public BuiltInCommand
{
public:
    GetCurrDirCommand(const char *cmdLine) : BuiltInCommand(cmdLine){};

    virtual ~GetCurrDirCommand() = default;

    void execute() override;
}; //done

class ShowPidCommand : public BuiltInCommand
{
public:
    ShowPidCommand(const char *cmdLine) : BuiltInCommand(cmdLine){};

    virtual ~ShowPidCommand() = default;

    void execute() override{
        std::cout << "smash pid is " << getpid() << std::endl;
    }
}; //done

class SetPromptCommand : public BuiltInCommand
{

public:
    SetPromptCommand(const char *cmdLine);

    void execute() override;
}; //done

class JobsList;

//class QuitCommand : public BuiltInCommand
//{
//    // TODO: Add your data members public:
//    QuitCommand(string &restOfWord, JobsList *jobs);
//
//    virtual ~QuitCommand()
//    {
//    }
//
//    void execute() override;
//};

class JobsList
{
public:
    class JobEntry
    {
        // TODO: Add your data members
    public:
//         int pid = 0;
         int jobId = 1;
        bool isFinished = false;
        Command *cmd = nullptr;

        JobEntry() = default;
        JobEntry(int jobId, Command* cmd, bool isFinished) :
        jobId(jobId), cmd(cmd), isFinished(isFinished){};
    };

    // TODO: Add your data members
    map<int, JobEntry> jobs;

public:
    JobsList()= default;

    ~JobsList(){
        for (auto &[id, job] : jobs)
        {
            delete job.cmd;
        }
    };

    void addJob(Command *cmd, bool isFinished = false)
    {
        int newJobId = -1;
        auto it = jobs.begin();
        for (int i = 1; i <= 100 ; i++, it++){
            if(it == jobs.end() || i != it->first){
                newJobId = i;
                break;
            }
        }       //find first open jobId. maximum of 100 jobs (page 2 under assumptions)
        if(newJobId == -1){
            throw runtime_error("Jobs full"); //ask what error to put for full jobs list
        }
        jobs[newJobId] = JobEntry(newJobId, cmd, isFinished);
    };

    void printJobsList()
    {
        removeFinishedJobs();

        for (const auto &[id, job] : jobs)
        {
            std::cout << "[" << id << "] " << job.cmd->getCmdLine() << std::endl;
        }
    };

    // void killAllJobs(){};

    void removeFinishedJobs()
    {
        for (auto &[id, job] : jobs)
        {
            if (job.isFinished == 1)
            {
                removeJobById(id);
            }
        }
    };

    JobEntry *getJobById(int jobId)
    {
        return &jobs.at(jobId);
    };

    void removeJobById(int jobId)
    {
        jobs.erase(jobId);
    };

    JobEntry *getLastJob()
    {
        return &prev(jobs.end())->second;
    };

//    JobEntry *getLastStoppedJob(int *jobId);

    // TODO: Add extra methods or modify exisitng ones as needed
};

class JobsCommand : public BuiltInCommand
{
    // TODO: Add your data members
    JobsList *jobsList;

public:
    JobsCommand(const char *cmdLine, JobsList *jobs) : BuiltInCommand(cmdLine), jobsList(jobs) {};

    virtual ~JobsCommand() = default;

    void execute() override
    {
        jobsList->printJobsList();
    };
}; //done

//class KillCommand : public BuiltInCommand
//{
//    // TODO: Add your data members
//    int signal;
//    int jobId;
//
//public:
//    KillCommand(string &restOfWord, JobsList *jobs)
//    {
//        if (restOfWords.length() == 0 || restOfWords[0] != '-')
//        {
//            throw std::exception("smash error: kill: invalid arguments");
//        }
//
//        int spaceCount = 0;
//        for (int i = 1; i < restOfWords.length(); i++)
//        {
//            if (restOfWords[i] == ' ')
//            {
//                spaceCount++;
//                if (spaceCount > 1)
//                {
//                    throw std::exception("smash error: fg: invalid arguments");
//                }
//            }
//            else if (restOfWords[i] < '0' || restOfWords[i] > '9')
//            {
//                throw std::exception("smash error: fg: invalid arguments");
//            }
//        }
//
//        if (spaceCount != 1)
//        {
//            throw std::exception("smash error: fg: invalid arguments");
//        }
//        int argCount = sscanf(restOfWord, "-%d %d", &signal, &jobId);
//        if (argCount != 2)
//        {
//            throw std::exception("smash error: kill: invalid arguments");
//        }
//    };
//
//    virtual ~KillCommand()
//    {
//    }
//
//    void execute() override
//    {
//        if (JobsList->contains(jobId) == 0)
//        {
//            throw std::exception("smash error: kill: job-id " + jobId + " does not exist");
//        }
//        //
//        int pid = JobsList->getJobById(jobId)->cmd;
//        if (kill(pid, signal) == -1)
//        {
//            throw std::exception("smash error: kill: kill failed");
//        }
//
//        // TODO: actually kill the process
//    };
//};

class ForegroundCommand : public BuiltInCommand
{
    // TODO: Add your data members
    int targetJobId;
    JobsList *jobsList;
public:
    ForegroundCommand(const char* cmd_line, JobsList *jobsList);

    virtual ~ForegroundCommand()
    {
    }

    void execute() override;
};

//class AliasCommand : public BuiltInCommand
//{
//public:
//    AliasCommand(const char *cmd_line);
//
//    virtual ~AliasCommand()
//    {
//    }
//
//    void execute() override;
//};
//
//class UnAliasCommand : public BuiltInCommand
//{
//public:
//    UnAliasCommand(const char *cmd_line);
//
//    virtual ~UnAliasCommand()
//    {
//    }
//
//    void execute() override;
//};
//
//class UnSetEnvCommand : public BuiltInCommand
//{
//public:
//    UnSetEnvCommand(const char *cmd_line);
//
//    virtual ~UnSetEnvCommand()
//    {
//    }
//
//    void execute() override;
//};
//
//class SysInfoCommand : public BuiltInCommand
//{
//public:
//    SysInfoCommand();
//
//    virtual ~SysInfoCommand()
//    {
//    }
//
//    void execute() override
//    {
//        struct utsname name;
//
//        // TODO: change to
//
//        // open /proc and read from there
//        if (uname(&name) == 0)
//        {
//            std::cout << "System: " << name.sysname << std::endl;
//            std::cout << "Hostname: " << name.nodename << std::endl;
//            std::cout << "Kernel: " << name.release << std::endl;
//            std::cout << "Architecture: " << name.machine << std::endl;
//        }
//        else
//        {
//            // Handle error: Could not get system information
//            std::cerr << "smash error: sysinfo: failed to retrieve basic info" << std::endl;
//        }
//
//        std::ifstream stat_file("/proc/stat");
//        std::string line;
//        long boot_time_sec = 0;
//
//        // Search for the "btime" line in /proc/stat
//        while (std::getline(stat_file, line))
//        {
//            if (line.substr(0, 6) == "btime ")
//            {
//                std::stringstream ss(line.substr(6));
//                ss >> boot_time_sec;
//                break;
//            }
//        }
//
//        if (boot_time_sec == 0)
//        {
//            return "N/A"; // Error reading file
//        }
//
//        // Convert seconds since epoch to a formatted string
//        std::time_t boot_time_t = boot_time_sec;
//        struct tm *tm_info = std::localtime(&boot_time_t);
//
//        char buffer[20];
//        // Format: YYYY-MM-DD HH:MM:SS
//        std::strftime(buffer, 20, "%Y-%m-%d %H:%M:%S", tm_info);
//
//        std::cout << "Boot Time: " << std::string(buffer) << std::endl;
//    }
//};

class SmallShell
{
private:
    // TODO: Add your data members

    char* previousUsedPath = "\n";

    string prompt = "smash";
    JobsList jobsList;
    SmallShell();

public:
    Command *CreateCommand(const char *cmd_line);

    SmallShell(SmallShell const &) = delete;     // disable copy ctor
    void operator=(SmallShell const &) = delete; // disable = operator
    static SmallShell &getInstance()             // make SmallShell singleton
    {
        static SmallShell instance; // Guaranteed to be destroyed.
        // Instantiated on first use.
        return instance;
    }

    ~SmallShell();

    void executeCommand(const char *cmd_line);

    void setPrompt(const string &newPrompt)
    {
        prompt = newPrompt;
    }
    string getPrompt(){
        return prompt;
    }
    void setPreviousUsedPath(char *previousUsed)
    {
        previousUsedPath = strdup(previousUsed);
    }
    char* getPreviousUsedPath()
    {
        return previousUsedPath;
    }
    JobsList &getJobsList()
    {
        return jobsList;
    }

    // TODO: add extra methods as needed
};

#endif // SMASH_COMMAND_H_
