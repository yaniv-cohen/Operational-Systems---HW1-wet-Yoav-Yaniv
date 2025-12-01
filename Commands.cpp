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
#include <unistd.h>

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

void printAllArgs(char **args)
{
    int i = 0;
    while (args[i] != nullptr)
    {
        std::cout << "arg " << i << ": " << args[i] << std::endl;
        i++;
    }
}

string _ltrim(const std::string &s)
{
    size_t start = s.find_first_not_of(WHITESPACE);
    return (start == std::string::npos) ? "" : s.substr(start);
}

string _rtrim(const std::string &s)
{
    size_t end = s.find_last_not_of(WHITESPACE);
    return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

string _trim(const std::string &s)
{
    return _rtrim(_ltrim(s));
}

int _parseCommandLine(const char *cmd_line, char **args)
{
    FUNC_ENTRY()
    int i = 0;
    std::istringstream iss(_trim(string(cmd_line)));
    for (std::string s; iss >> s;)
    {
        args[i] = (char *)malloc(s.length() + 1);
        memset(args[i], 0, s.length() + 1);
        strcpy(args[i], s.c_str());
        args[++i] = nullptr;
    }
    return i;
    FUNC_EXIT()
}

bool _isBackgroundCommand(const char *cmd_line)
{
    const string str(cmd_line);
    return str[str.find_last_not_of(WHITESPACE)] == '&';
}

void _removeBackgroundSign(char *cmd_line)
{
    const string str(cmd_line);
    // find last character other than spaces
    size_t idx = str.find_last_not_of(WHITESPACE);
    // if all characters are spaces then return
    if (idx == string::npos)
    {
        return;
    }
    // if the command line does not end with & then return
    if (cmd_line[idx] != '&')
    {
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
Command *SmallShell::CreateCommand(const char *cmd_line_raw)
{
    char cmd_line[COMMAND_MAX_LENGTH];
    strcpy(cmd_line, cmd_line_raw);

    if( strcmp(cmd_line, WHITESPACE.c_str())  ==0 ){
        printf("cmd_line empty %s\n", cmd_line);
        return new DoNothingCommand(cmd_line);
    }


    string trimmed = _trim(string(cmd_line));

    bool isBackground = _isBackgroundCommand(cmd_line);
    printf("isBackground = %d\n", isBackground);
    string firstWord = trimmed.substr(0, trimmed.find_first_of(" \n"));
    printf("pre remove: %s\n", cmd_line);
    _removeBackgroundSign(cmd_line);
    printf("post remove: %s\n", cmd_line);

    // TODO: check for aliases here, find way to remove & with function above

    printf("first word: %s\n", firstWord.c_str());
    // single word commands
    if (firstWord == "pwd")
    {
        return new GetCurrDirCommand(cmd_line);
    } // done
    else if (firstWord == "showpid")
    {
        return new ShowPidCommand(cmd_line);
    } // done
    else if (firstWord == "jobs")
    {
        return new JobsCommand(nullptr, &jobsList);
    }
    //    else if (firstWord == "sysinfo")
    //    {
    //        return new SysInfoCommand();
    //    }

    // multi-word commands
    else if (firstWord == "chprompt")
    {
        return new SetPromptCommand(cmd_line);
    } // done
    else if (firstWord == "cd")
    {
        return new ChangeDirCommand(cmd_line, this->previousUsedPath);
    } // done

    else if (firstWord == "fg")
    {
        return new ForegroundCommand(cmd_line, &this->jobsList);
    }

    // else if (firstWordNoAmp.compare("kill") == 0)
    // {
    //     // with given id
    //   //TODO:
    //     return new KillCommand(restOfWords.c_str(), &this->jobsList);
    // }
    // else if (firstWord.compare("quit") == 0)
    // {
    //     // TODO implement
    //     return new QuitCommand(restOfWords, &this->jobsList);
    // }

    // not built in command
    // not special command

    else
    {
        std::cout << "not matched any built in command\n" <<std::endl;
        if (string(cmd_line).find("*") == string::npos &&
            string(cmd_line).find("?") == string::npos)
        {

            std::cout << "smash: simpleExternal command" << std::endl;
            return new SimpleExternalCommand(cmd_line, isBackground);
        }
        else
        {
            std::cout << "smash: complexExternal command" << std::endl;

            return new ComplexExternalCommand(cmd_line, isBackground);
        }
    }
    return nullptr;
}

void GetCurrDirCommand::execute()
{
    printf("in get curr dir\n");
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != nullptr)
    {
        std::cout << cwd << std::endl;
    }
    else
    {
        perror("smash error: getcwd failed");
    }
}
void SmallShell::executeCommand(const char *cmd_line)
{
    // TODO: Add your implementation here
    // for example:

    std::cout << "smash: cmd_line " << cmd_line << std::endl;
    Command *cmd = CreateCommand(cmd_line);
    std::cout << "smash: executing command: " << cmd_line << std::endl;
    cmd->execute();
    // Please note that you must fork smash process for some commands (e.g., external commands....)
}

SetPromptCommand::SetPromptCommand(const char *cmdLine) : BuiltInCommand(cmdLine)
{
    char *args[COMMAND_MAX_ARGS];
    int i = _parseCommandLine(cmdLine, args);
    if (i == 1)
    { // no arguments (just command)
        char name[] = "smash";
        setCmdLine(name);
    }
    else
    { // ignore arguments aside from first
        setCmdLine(args[1]);
    }
}

void SetPromptCommand::execute()
{
    SmallShell &smash = SmallShell::getInstance();
    smash.setPrompt(getCmdLine());
}

ChangeDirCommand::ChangeDirCommand(const char *cmdLine, const char *previousUsed) : BuiltInCommand(cmdLine)
{

    // char* newTargetPath;
    // char* previousUsedPath;
    newTargetPath = new char[PATH_MAX];
    previousUsedPath = new char[PATH_MAX];

    char *args[COMMAND_MAX_ARGS];
    std::cout << "|" << cmdLine << std::endl;
    int i = _parseCommandLine(cmdLine, args);

    printAllArgs(args);
    if (i < 2)
    {
        std::cout << "no arguments to cd\n";
        getcwd(newTargetPath, PATH_MAX);
        if (previousUsed != nullptr)
        {

            strcpy(previousUsedPath, previousUsed);
        }
        else
        {
            std::cout << "no arguments to cd4\n";

            previousUsedPath = nullptr;
        }
    }
    if (i > 2)
    {
        throw runtime_error("smash error: cd: too many arguments");
    }
    else if (i == 2)
    {
        if (strcmp("-", args[1]) == 0 && previousUsed == nullptr)
        {
            throw runtime_error("smash error: cd: OLDPWD not set");
        }
        else if (strcmp("-", args[1]) == 0)
        { // set prev to current
            getcwd(previousUsedPath, PATH_MAX);
            strcpy(newTargetPath, previousUsed);
        }
        else
        {
            getcwd(previousUsedPath, PATH_MAX);
            strcpy(newTargetPath, args[1]);
        }
    }
}

void ChangeDirCommand::execute()
{

    std::cout << "newTargetPath:" << newTargetPath << strlen(newTargetPath) << std::endl;
    for (size_t i = 0; i < strlen(newTargetPath); i++)
    {
        if (newTargetPath[i] == '\n')
        {
            newTargetPath[i] = '\0';
            break;
        }
    }

    if (chdir(newTargetPath) == 0)
    {
        std::cout << "smash: about to change directory to " << newTargetPath << std::endl;
        SmallShell &smash = SmallShell::getInstance();
        std::cout << "smash: previous used path is " << previousUsedPath << std::endl;
        smash.setPreviousUsedPath(previousUsedPath);
    }
    else
    {
        // TODO: catch this exception with "smash error: cd: invalid path"
        throw runtime_error("smash error: cd: invalid path");
    }
}

ForegroundCommand::ForegroundCommand(const char *cmd_line, JobsList *jobsList) : BuiltInCommand(cmd_line)
{
    this->jobsList = jobsList;
    char *args[COMMAND_MAX_ARGS];
    int i = _parseCommandLine(cmd_line, args);

    if (i == 1)
    { // no arguments
        targetJobId = jobsList->getLastJob()->jobId;
    }
    else if (i == 2)
    { // 1 argument
        targetJobId = stoi(args[1]);
    }
    else
    {
        throw runtime_error("smash error: fg: invalid arguments");
    }
}

void ForegroundCommand::execute()
{
    if (jobsList->jobs.empty())
    {
        throw runtime_error("smash error: fg: jobs list is empty");
    }

    try
    {
        jobsList->jobs.at(targetJobId);
    }
    catch (...)
    {
        throw runtime_error("smash error: fg: job-id " + to_string(targetJobId) + " does not exist");
    }

    // job exists
    int pid = fork();
    if (pid == 0)
    {
        std::cout << "[" << getpid() << "] " << jobsList->jobs[targetJobId].cmd->getCmdLine() << std::endl;
        execv(jobsList->jobs[targetJobId].cmd->getCmdLine(), nullptr);
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
            jobsList->removeJobById(targetJobId);
        }
        else
        {
            // something else
            throw runtime_error("smash error: fg: execv failed");
        }
    }
    else
    {
        throw runtime_error("smash error: fg: fork failed");
    }
}

void ComplexExternalCommand::execute()
{
    std::cout << "Executing complex external command: " << getCmdLine() << std::endl;
    if(!isBG){
           //foreground
           int pid = fork();
           if(pid==0){
                //child
                char* const argv[] = {"/bin/bash", "-c",const_cast<char*>(getCmdLine()), NULL};
                execv(argv[0], argv ); 
                perror("smash error: excecution failed");
           }
           else if(pid>0){
                //parent
                waitpid(pid, nullptr, 0);
                std::cout << "finished waiting"<<std::endl;
           }
           else{
                perror("smash error: fork failed");
           }
       }
       else
       {
           //background
           int pid = fork();
           if(pid==0){
                //child
                char* const argv[] = {"/bin/bash", "-c",const_cast<char*>(getCmdLine()), NULL};
                execv(argv[0], argv ); 
                perror("smash error: excecution failed");
           }
           else if(pid>0){
                //parent
                // waitpid(pid, nullptr, 0);
                std::cout << "not waiting"<<std::endl;
                SmallShell::getInstance().getJobsList().addJob(this, false);
                std::cout << "added to joblist"<<std::endl;
                SmallShell::getInstance().getJobsList().printJobsList();
           }
           else{
                perror("smash error: fork failed");
           }
       }
}


void SimpleExternalCommand::execute()
{
    std::cout << "Executing SimpleExternalCommand: " << getCmdLine() << std::endl;
    if(!isBG){
           //foreground
           int pid = fork();
           if(pid==0){
                //child
                char *args[COMMAND_MAX_ARGS];
                _parseCommandLine(getCmdLine(), args);
                printAllArgs(args);
                // char* const argv[] = {args , NULL};
                execvp(args[0], args ); 
                perror("smash error: excecution failed");
           }
           else if(pid>0){
                //parent
                waitpid(pid, nullptr, 0);
                std::cout << "finished waiting"<<std::endl;
           }
           else{
                perror("smash error: fork failed");
           }
       }
       else
       {
           //background
           int pid = fork();
           if(pid==0){
                //child
                char *args[COMMAND_MAX_ARGS];
                _parseCommandLine(getCmdLine(), args);
                printAllArgs(args);
                // char* const argv[] = {args , NULL};
                execvp(args[0], args ); 
                perror("smash error: excecution failed");
           }
           else if(pid>0){
                //parent
                // waitpid(pid, nullptr, 0);
                std::cout << "not waiting"<<std::endl;
                SmallShell::getInstance().getJobsList().addJob(this, false);
                std::cout << "added to joblist"<<std::endl;
                SmallShell::getInstance().getJobsList().printJobsList();
           }
           else{
                perror("smash error: fork failed");
           }
       }
}