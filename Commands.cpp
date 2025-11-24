#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include "Commands.h"

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
    std::istringstream iss(_trim(string(cmd_line)).c_str());
    for (std::string s; iss >> s;)
    {
        args[i] = (char *)malloc(s.length() + 1);
        memset(args[i], 0, s.length() + 1);
        strcpy(args[i], s.c_str());
        args[++i] = NULL;
    }
    return i;
    FUNC_EXIT()
}

bool _isBackgroundComamnd(const char *cmd_line)
{
    const string str(cmd_line);
    return str[str.find_last_not_of(WHITESPACE)] == '&';
}

void _removeBackgroundSign(char *cmd_line)
{
    const string str(cmd_line);
    // find last character other than spaces
    unsigned int idx = str.find_last_not_of(WHITESPACE);
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
    cmd_line[idx] = ' ';
    // truncate the command line string up to the last non-space character
    cmd_line[str.find_last_not_of(WHITESPACE, idx) + 1] = 0;
}

// TODO: Add your implementation for classes in Commands.h

SmallShell::SmallShell()
{
    // TODO: add your implementation
}

SmallShell::~SmallShell()
{
    // TODO: add your implementation
}

/**
 * Creates and returns a pointer to Command class which matches the given command line (cmd_line)
 */
Command *SmallShell::CreateCommand(const char *cmd_line)
{
    // For example:

    string cmd_s = _trim(string(cmd_line));
    string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));

    // TODO: check for aliases here

    string firstWordNoAmp = _removeBackgroundSign(firstWord);
    // single word commands
    // TODO: handle & at end of line
    if (firstWordNoAmp.compare("pwd") == 0)
    {
        return new GetCurrDirCommand();
    }
    else if (firstWordNoAmp.compare("showpid") == 0)
    {
        return new ShowPidCommand();
    }
    else if (firstWordNoAmp.compare("jobs") == 0)
    {
        return new JobsCommand(jobsList);
    }
    else if (firstWordNoAmp.compare("sysinfo") == 0)
    {
        return new SysInfoCommand();
    }

    string restOfWords = cmd_s.substr(cmd_s.find_first_of(" "), cmd_s.find_last_of(" \n"));
    // multi word commands
    if (firstWordNoAmp.compare("chprompt") == 0)
    {
        return new SetPromptCommand(restOfWords.c_str());
    }
    else if (firstWordNoAmp.compare("cd") == 0)
    {
        return new ChangeDirCommand(restOfWords.c_str(), &this->previousUsedPath);
    }

    else if (firstWordNoAmp.compare("fg") == 0)
    {
        return new ForegroundCommand(restOfWords.c_str(), &this->jobsList);
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

    // else {
    //   return new ExternalCommand(cmd_line);
    // }

    return nullptr;
}

void SmallShell::executeCommand(const char *cmd_line)
{
    // TODO: Add your implementation here
    // for example:
    Command *cmd = CreateCommand(cmd_line);
    cmd->execute();
    // Please note that you must fork smash process for some commands (e.g., external commands....)
}

void GetCurrDirCommand::execute()
{
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
void ShowPidCommand::execute()
{
    std::cout << "smash pid is " << getpid() << std::endl;
}

SetPromptCommand::SetPromptCommand(const string &givenPrompt)
{
    this->givenPrompt = givenPrompt;
}
SetPromptCommand::SetPromptCommand()
{
    this->givenPrompt = "smash";
}

void SetPromptCommand::execute()
{
    SmallShell &smash = SmallShell::getInstance();
    smash.setPreviousUsedPath(getcwd());
    smash.setPompt(givenPrompt);
}
