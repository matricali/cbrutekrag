#include <libssh/libssh.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char** argv)
{
    ssh_session my_ssh_session;
    int verbosity = SSH_LOG_PROTOCOL;
    int port = 22;

    my_ssh_session = ssh_new();

    if (my_ssh_session == NULL) {
        exit(-1);
    }

    ssh_options_set(my_ssh_session, SSH_OPTIONS_HOST, "localhost");
    ssh_options_set(my_ssh_session, SSH_OPTIONS_LOG_VERBOSITY, &verbosity);
    ssh_options_set(my_ssh_session, SSH_OPTIONS_PORT, &port);

    int r;
    r = ssh_connect(my_ssh_session);
    if (r != SSH_OK) {
        fprintf(
            stderr,
            "Error connecting to localhost: %s\n",
            ssh_get_error(my_ssh_session)
        );
        exit(-1);
    }


    ssh_free(my_ssh_session);
}
