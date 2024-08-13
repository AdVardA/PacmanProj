#include <iostream>

#include <unistd.h>

#include <cstdlib>

#include <string>

#include <thread>

using namespace std;

class Error_
{
public:
    Error_()
    {
        int fd[2];
        if (pipe(fd) == -1)
        {
            perror("Pipe failed");
        }

        pid_t pid = fork();

        if (pid < 0)
        {
            perror("Fork failed");
        }
        else if (pid > 0)
        {
            close(fd[1]);
            writer_fd = fd[0];
        }
        else
        {
            close(fd[0]);

            if (dup2(fd[1], STDOUT_FILENO) == -1)
            {
                perror("Dup2 failed");
            }

            close(fd[1]);

            thread xterm_thread([this]()
                                {
                if (execlp("xterm", "xterm", "-e", "bash", "-c", "echo 'Error detected:'; sleep 5; exit", (char *)NULL) == -1)
                {
                    perror("Execlp failed");
                } });

            xterm_thread.detach();
        }
    }

    ~Error_()
    {
        close(writer_fd);
    }

    void show_error(const char *message)
    {
        string error_message = "Error(" + this->name + "): " + string(message) + "\n";
        if (write(this->writer_fd, error_message.c_str(), error_message.size()) == -1)
        {
            perror("Write failed");
        }
    }

    string name;

private:
    int writer_fd;
};
