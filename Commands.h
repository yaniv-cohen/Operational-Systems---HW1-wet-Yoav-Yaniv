// Ver: 04-11-2025
#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <vector>
#include <map>
#include <sys/utsname.h>

#define COMMAND_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)

class Command
{
    // TODO: Add your data members
    char *cmd_line;

public:
    Command(const char *cmd_line)
    {
        cmd_line = cmd_line;
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
    char newTargetPath[_MAX_PATH]; // on execute go here
    string *previousUsedPath;

public:
    ChangeDirCommand(const char *restOfWords, string *previousUsedPath);
    previousUsedPath(previousUsedPath)
    {
        string args[COMMAND_MAX_ARGS]{"\n"};
        _parseCommandLine(restOfWords, args);

        if (args[0] != "\n")
        {
            if (args[1].compare("\n") == 0)
            {
                throw std::exception("smash error: cd: too many arguments");
            }

            else if (args[1].compare("-") == 0)
            {
                newTargetPath = *previousUsedPath;
            }
            else
            {
                newTargetPath = args[1];
            }
        }
        // TODO: parse restOfWords to get the target path, and handle spaces
        // newTargetPath = restOfWords; // after "cd "

        // previousUsedPath = previousUsedPath;
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

        string pathBeforeChange = getcwd(*previousUsedPath, sizeof(*previousUsedPath));
        if (chdir(newTargetPath) == 0)
        {
            *previousUsedPath = pathBeforeChange;
        }
        else
        {
            throw std::exception("smash error: cd: invalid path");
        }
    }
};

class GetCurrDirCommand : public BuiltInCommand
{
public:
    GetCurrDirCommand();

    virtual ~GetCurrDirCommand()
    {
    }

    void execute() override;
};

class ShowPidCommand : public BuiltInCommand
{
public:
    ShowPidCommand();

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
    QuitCommand(string &restOfWord, JobsList *jobs);

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
        // int jobId;
        Command *cmd;
        bool isFinished;
    };

    // TODO: Add your data members
    std::map<size_t, JobEntry> jobs;
    // std::vector<JobEntry*> stopedJobs;
    std::vector<int> freedJobIds;
    int curMaxJobId = 1;

public:
    JobsList();

    ~JobsList();

    void addJob(Command *cmd, bool isFinished = false)
    {
        int newJobId;
        if (curMaxJobId < INT_MAX)
        {
            newJobId = curMaxJobId;
            curMaxJobId++;
        }
        // else if (freedJobIds.size() > 0)
        // {
        //     int newJobId = freedJobIds.back();
        //     freedJobIds.pop_back();
        // }

        // else
        // {
        //     throw std::exception("smash error: cannot add new job");
        // }

        JobEntry *newJob = JobEntry();
        newJob->cmd = cmd;
        newJob->isFinished = isFinished;

        jobs.insert(newJobId, newJob);
        // TODO: figure out job id assignment
    };

    void printJobsList()
    {
        removeFinishedJobs();

        for (const auto &[id, job] : jobs)
        {
            // TODO: SORT BY JOB ID, and
            std::cout << "[" << id << "] " << job.cmd->getCmdLine() < "\n" << std::endl;
            // TODO: make sure no excess \n
        }
    };

    void killAllJobs();

    void removeFinishedJobs()
    {
        for (auto &it : jobs)
        {
            if (it->key->job->isFinished == 1)
            {
                jobs.erase(it);
            }
        }
    };

    JobEntry *getJobById(int jobId)
    {
        // TODO: optimize search
        return jobs[jobId];
    };

    void removeJobById(int jobId)
    {
        jobs.erase(jobId);
    };

    JobEntry *getLastJob()
    {
        return jobs.end().key;
    };

    JobEntry *getLastStoppedJob(int *jobId);

    // TODO: Add extra methods or modify exisitng ones as needed
};

class JobsCommand : public BuiltInCommand
{
    // TODO: Add your data members
    JobsList *jobsList;

public:
    JobsCommand(JobsList *jobs)
    {
        jobsList = jobs;
    };

    virtual ~JobsCommand()
    {
    }

    void execute() override
    {
        jobsList.printJobsList();
    };
};

class KillCommand : public BuiltInCommand
{
    // TODO: Add your data members
    int signal;
    int jobId;

public:
    KillCommand(string &restOfWord, JobsList *jobs)
    {
        if (restOfWords.length() == 0 || restOfWords[0] != '-')
        {
            throw std::exception("smash error: kill: invalid arguments");
        }

        int spaceCount = 0;
        for (int i = 1; i < restOfWords.length(); i++)
        {
            if (restOfWords[i] == ' ')
            {
                spaceCount++;
                if (spaceCount > 1)
                {
                    throw std::exception("smash error: fg: invalid arguments");
                }
            }
            else if (restOfWords[i] < '0' || restOfWords[i] > '9')
            {
                throw std::exception("smash error: fg: invalid arguments");
            }
        }

        if (spaceCount != 1)
        {
            throw std::exception("smash error: fg: invalid arguments");
        }
        int argCount = sscanf(restOfWord, "-%d %d", &signal, &jobId);
        if (argCount != 2)
        {
            throw std::exception("smash error: kill: invalid arguments");
        }
    };

    virtual ~KillCommand()
    {
    }

    void execute() override
    {
        if (JobsList->contains(jobId) == 0)
        {
            throw std::exception("smash error: kill: job-id " + jobId + " does not exist");
        }
        //
        int pid = JobsList->getJobById(jobId)->cmd;
        if (kill(pid, signal) == -1)
        {
            throw std::exception("smash error: kill: kill failed");
        }

        // TODO: actually kill the process
    };
};

class ForegroundCommand : public BuiltInCommand
{
    // TODO: Add your data members
    size_t targetJobId;
    JobsList *jobsList
public:
    ForegroundCommand(string arguments, JobsList *jobsList):jobsList(JobsList)
    {

        string args[COMMAND_MAX_ARGS]{"\n"};
        _parseCommandLine(restOfWords, args);
        // count arguments
        if (args[0] == "\n")
        {
            // no arguments
            targetJobId = jobs->getLastJob();
        }
        else if(args[2] != "\n")
        {

            // targetJobId = stoi(arguments);
        }
        else if(args[1] != "\n")
        {
            // targetJobId = stoi(arguments);
            targetJobId = stoi(args[1]);
        }
    };

    virtual ~ForegroundCommand()
    {
    }

    void execute() override
    {

        if (jobsList->jobs.size() == 0)
        {
            throw std::exception("smash error: fg: jobs list is empty");
        }

        if (jobsList->jobs.contains(targetJobId) == 0)
        {
            throw std::exception("smash error: fg: job-id " + targetJobId + " does not exist");
        }

        //job exists
        int pid = fork();
        if (pid == 0)
        {
            std::cout << "[" << getpid() << "] " << jobs[targetJobId]->cmd->getCmdLine() << std::endl;
            execv(jobs[targetJobId]->cmd->getCmdLine(), nullptr);
            perror("smash error: fg: execv failed");
            exit(EXIT_FAILURE);
        }
        else if (pid > 0)
        {
            int status;
            waitpid(pid, &status, 0);
            if (WIFEXITED(status))
            {
                // normal exit
                jobs->removeJobById(targetJobId);
            }
            else
            {
                // something else
                throw std::exception("smash error: fg: execv failed");
            }
        }
        else{
                throw std::exception("smash error: fg: fork failed");
        }
    };
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
    SysInfoCommand();

    virtual ~SysInfoCommand()
    {
    }

    void execute() override
    {
        struct utsname name;

        // TODO: change to

        // open /proc and read from there
        if (uname(&name) == 0)
        {
            std::cout << "System: " << name.sysname << std::endl;
            std::cout << "Hostname: " << name.nodename << std::endl;
            std::cout << "Kernel: " << name.release << std::endl;
            std::cout << "Architecture: " << name.machine << std::endl;
        }
        else
        {
            // Handle error: Could not get system information
            std::cerr << "smash error: sysinfo: failed to retrieve basic info" << std::endl;
        }

        std::ifstream stat_file("/proc/stat");
        std::string line;
        long boot_time_sec = 0;

        // Search for the "btime" line in /proc/stat
        while (std::getline(stat_file, line))
        {
            if (line.substr(0, 6) == "btime ")
            {
                std::stringstream ss(line.substr(6));
                ss >> boot_time_sec;
                break;
            }
        }

        if (boot_time_sec == 0)
        {
            return "N/A"; // Error reading file
        }

        // Convert seconds since epoch to a formatted string
        std::time_t boot_time_t = boot_time_sec;
        struct tm *tm_info = std::localtime(&boot_time_t);

        char buffer[20];
        // Format: YYYY-MM-DD HH:MM:SS
        std::strftime(buffer, 20, "%Y-%m-%d %H:%M:%S", tm_info);

        std::cout << "Boot Time: " << std::string(buffer) << std::endl;
    }
};

class SmallShell
{
private:
    // TODO: Add your data members

    char previousUsedPath[PATH_MAX] = nullptr;

    string promp = "smash";
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
    JobsList &getJobsList()
    {
        return jobsList;
    }
    void executeCommand(const char *cmd_line);

    // TODO: add extra methods as needed
};

#endif // SMASH_COMMAND_H_
