// Ver: 04-11-2025
#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <vector>

#define COMMAND_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)

class Command
{
    // TODO: Add your data members
    char cmd_line[COMMAND_MAX_LENGTH];

public:
    Command(const char *cmd_line)
    {
        strcpy(this->cmd_line, cmd_line);
    };

    virtual ~Command();

    virtual void execute() = 0;

    // virtual void prepare();
    // virtual void cleanup();
    //  TODO: Add your extra methods if needed
    const char *getCmdLine() const
    {
        return cmd_line;
    }
};

class BuiltInCommand : public Command
{
public:
    BuiltInCommand(const char *cmd_line);

    virtual ~BuiltInCommand()
    {
    }
};

class ExternalCommand : public Command
{
public:
    ExternalCommand(const char *cmd_line);

    virtual ~ExternalCommand()
    {
    }

    void execute() override;
};

class RedirectionCommand : public Command
{
    // TODO: Add your data members
public:
    explicit RedirectionCommand(const char *cmd_line);

    virtual ~RedirectionCommand()
    {
    }

    void execute() override;
};

class PipeCommand : public Command
{
    // TODO: Add your data members
public:
    PipeCommand(const char *cmd_line);

    virtual ~PipeCommand()
    {
    }

    void execute() override;
};

class DiskUsageCommand : public Command
{
public:
    DiskUsageCommand(const char *cmd_line);

    virtual ~DiskUsageCommand()
    {
    }

    void execute() override;
};

class WhoAmICommand : public Command
{
public:
    WhoAmICommand(const char *cmd_line);

    virtual ~WhoAmICommand()
    {
    }

    void execute() override;
};

class USBInfoCommand : public Command
{
    // TODO: Add your data members **BONUS: 10 Points**
public:
    USBInfoCommand(const char *cmd_line);

    virtual ~USBInfoCommand()
    {
    }

    void execute() override;
};

class ChangeDirCommand : public BuiltInCommand
{
    // TODO: Add your data members public:
    char newTargetPath[MAX_READ_LOCKS]; // on execute go here
public:
    ChangeDirCommand(const char *restOfWords)
    {
        // TODO: parse restOfWords to get the target path, and handle spaces
        newTargetPath = restOfWords; // after "cd "
    };

    ChangeDirCommand()
    {
        SmallShell &smash = SmallShell::getInstance();
        newTargetPath = smash.getPreviousUsedPath();
    };

    virtual ~ChangeDirCommand()
    {
    }

    void execute() override
    {
        SmallShell &smash = SmallShell::getInstance();
        if (newTargetPath == nullptr)
        {
            // TODO: catch this exception
            throw std::exception("smash error: cd: OLDPWD not set");
        }
        smash.updatePreviousUsedPath();
        chdir(newTargetPath);
    }
};

class GetCurrDirCommand : public BuiltInCommand
{
public:
    GetCurrDirCommand(const char *cmd_line);

    virtual ~GetCurrDirCommand()
    {
    }

    void execute() override;
};

class ShowPidCommand : public BuiltInCommand
{
public:
    ShowPidCommand(const char *cmd_line);

    virtual ~ShowPidCommand()
    {
    }

    void execute() override;
};

class SetPromptCommand : public BuiltInCommand
{
    string givenPrompt;

public:
    SetPromptCommand(const string &givenPrompt);
    SetPromptCommand();

    virtual ~ShowPidCommand()
    {
    }

    void execute() override;
};
class JobsList;

class QuitCommand : public BuiltInCommand
{
    // TODO: Add your data members public:
    QuitCommand(const char *cmd_line, JobsList *jobs);

    virtual ~QuitCommand()
    {
    }

    void execute() override;
};

class JobsList
{
public:
    class JobEntry
    {
        // TODO: Add your data members
        // int pid;
        int jobId;
        Command *cmd;
        bool isStopped;
    };

    // TODO: Add your data members
    std::vector<JobEntry> jobs;
    // std::vector<JobEntry*> stopedJobs;
    std::vector<int> freedJobIds;
    int curMaxJobId = 1;

public:
    JobsList();

    ~JobsList();

    void addJob(Command *cmd, bool isStopped = false)
    {
        int newJobId = -1;
        if (freedJobIds.size() > 0)
        {
            int newJobId = freedJobIds.back();
            freedJobIds.pop_back();
        }
        else if (curMaxJobId < INT_MAX)
        {
            newJobId = curMaxJobId;
            curMaxJobId++;
        }
        else
        {
            throw std::exception("smash error: cannot add new job");
        }

        JobEntry newJob;
        newJob.cmd = cmd;
        newJob.jobId = newJobId;
        newJob.isStopped = isStopped;

        // if(isStopped)
        // {
        //     stopedJobs.push_back(&newJob);
        // }
        
        jobs.push_back(newJob);
    };

    void printJobsList(){
        for (auto job : jobs)
        {
            std::cout << "[" << job.jobId << "] " << job.cmd->getCmdLine() < "\n" << std::endl;
            //TODO: make sure no excess \n
        }
    };

    void killAllJobs();

    void removeFinishedJobs();

    JobEntry *getJobById(int jobId){
        //TODO: optimize search
        for (auto &job : jobs)
        {
            if (job.jobId == jobId)
            {
                return &job;
            }
        }
        return nullptr;
    };

    void removeJobById(int jobId){
        for (auto &  it = jobs.begin(); it != jobs.end(); ++it)
        {
            if (it->jobId == jobId)
            {
                freedJobIds.push_back(it->jobId);
                jobs.erase(it);
                return;
            }
        }
    };

    JobEntry *getLastJob(int *lastJobId);

    JobEntry *getLastStoppedJob(int *jobId);

    // TODO: Add extra methods or modify exisitng ones as needed
};

class JobsCommand : public BuiltInCommand
{
    // TODO: Add your data members
public:
    JobsCommand(const char *cmd_line, JobsList *jobs);

    virtual ~JobsCommand()
    {
    }

    void execute() override;
};

class KillCommand : public BuiltInCommand
{
    // TODO: Add your data members
public:
    KillCommand(const char *cmd_line, JobsList *jobs);

    virtual ~KillCommand()
    {
    }

    void execute() override;
};

class ForegroundCommand : public BuiltInCommand
{
    // TODO: Add your data members
public:
    ForegroundCommand(const char *cmd_line, JobsList *jobs);

    virtual ~ForegroundCommand()
    {
    }

    void execute() override;
};

class AliasCommand : public BuiltInCommand
{
public:
    AliasCommand(const char *cmd_line);

    virtual ~AliasCommand()
    {
    }

    void execute() override;
};

class UnAliasCommand : public BuiltInCommand
{
public:
    UnAliasCommand(const char *cmd_line);

    virtual ~UnAliasCommand()
    {
    }

    void execute() override;
};

class UnSetEnvCommand : public BuiltInCommand
{
public:
    UnSetEnvCommand(const char *cmd_line);

    virtual ~UnSetEnvCommand()
    {
    }

    void execute() override;
};

class SysInfoCommand : public BuiltInCommand
{
public:
    SysInfoCommand(const char *cmd_line);

    virtual ~SysInfoCommand()
    {
    }

    void execute() override;
};

class SmallShell
{
private:
    // TODO: Add your data members

    char previousUsedPath[PATH_MAX] = nullptr;

    string promp = "smash";

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
    void setPompt(const string &newPrompt)
    {
        promp = newPrompt;
    }
    void updatePreviousUsedPath()
    {
        getcwd(previousUsedPath, sizeof(previousUsedPath));
    }
    char *getPreviousUsedPath()
    {
        return previousUsedPath;
    }

    ~SmallShell();

    void executeCommand(const char *cmd_line);

    // TODO: add extra methods as needed
};

#endif // SMASH_COMMAND_H_
