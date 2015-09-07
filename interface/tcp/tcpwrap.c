/****************************************************************************/
/* TestFarm                                                                 */
/* TCP/IP server wrapper                                                    */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 17-AUG-1999                                                    */
/****************************************************************************/

/*
    This file is part of TestFarm,
    the Test Automation Tool for Embedded Software.
    Please visit http://www.testfarm.org.

    TestFarm is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    TestFarm is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with TestFarm.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "tcp.h"

int respawn = 0;
int debug = 0;
int port = 2000;
int stdin_only = 0;

void version(void)
{
  fprintf(stderr, "TestFarm - TCP/IP wrapper " VERSION " (" __DATE__ ")\n");
}


void usage(void)
{
  version();
  fprintf(stderr, "Usage: tcpwrap [-port <number>] [-respawn] [-stdin] [-debug] [-version] command [arguments ...]\n");
  exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
  int argc2;
  char **argv2;
  int lsock, ssock;
  struct sockaddr_in ilocal;
  struct sockaddr_in iremote;
  socklen_t size;
  pid_t pid;
  int status;

  /* Retrieve arguments arguments */
  argc2 = argc - 1;
  argv2 = argv + 1;
  while ( (*argv2 != NULL) && (**argv2 == '-') ) {
    if ( strcmp(*argv2, "-port") == 0 ) {
      argc2--;
      argv2++;
      port = atoi(*argv2);
    }
    else if ( strcmp(*argv2, "-respawn") == 0 ) {
      respawn = 1;
    }
    else if ( strcmp(*argv2, "-stdin") == 0 ) {
      stdin_only = 1;
    }
    else if ( strcmp(*argv2, "-debug") == 0 ) {
      version();
      debug = 1;
    }
    else if ( strcmp(*argv2, "-version") == 0 ) {
      version();
      exit(EXIT_SUCCESS);
    }
    else {
      usage();
    }

    argc2--;
    argv2++;
  }

  if ( argc2 < 1 ) {
    usage();
  }

  /* Create network socket */
  if ( (lsock = socket(AF_INET, SOCK_STREAM, 0)) == -1 ) {
    perror("socket");
    exit(EXIT_FAILURE);
  }

  /* Bind network socket */
  ilocal.sin_family = AF_INET;
  ilocal.sin_addr.s_addr = htonl(INADDR_ANY);
  ilocal.sin_port = htons(port);

  if ( bind(lsock, (struct sockaddr *) &ilocal, sizeof(ilocal)) == -1 ) { 
    perror("bind");
    close(lsock);
    exit(EXIT_FAILURE);
  }

  /* Listen to network connection */
  if ( listen(lsock, 1) == -1 ) {
    perror("listen");
    close(lsock);
    exit(EXIT_FAILURE);
  }

  while ( 1 ) {
    if ( debug )
      fprintf(stderr, "Listening from port %d...\n", port);

    /* Accept incoming connection */
    size = sizeof(iremote);
    ssock = accept(lsock, (struct sockaddr *) &iremote, &size);

    if ( debug )
      fprintf(stderr, "Connected to %s\n", tcpaddr(NULL, &iremote));

    /* Spawn server application */
    switch ( (pid = fork()) ) {
    case 0 :
      /* Show exec info */
      if ( debug ) {
        int i;

        fprintf(stderr, "Server exec (pid=%d):", (int) getpid());
        for (i = 0; i < argc2; i++)
          fprintf(stderr, " %s", argv2[i]);
        fprintf(stderr, "\n");
      }

      /* Branch network connection to standard i/o */
      dup2(ssock, STDIN_FILENO);
      if ( ! stdin_only )
        dup2(ssock, STDOUT_FILENO);
      close(ssock);
      close(lsock);

      /* Perform exec (returnless call if success) */
      execvp(*argv2, argv2);

      /* Return from exec: something went wrong */
      perror("execvp");
      shutdown(STDIN_FILENO, 2);
      exit(EXIT_FAILURE);
      break;

    case -1 :
      perror("fork");
      shutdown(ssock, 2);
      close(lsock);
      exit(EXIT_FAILURE);
      break;

    default :
      if ( debug )
        fprintf(stderr, "Server forked with pid %d\nWaiting for server termination...\n", (int) pid);

      waitpid(pid, &status, 0);

      if ( debug )
        fprintf(stderr, "Server exited with code %d\n", WEXITSTATUS(status));

      shutdown(ssock, 2);
      close(ssock);

      if ( !respawn ) {
        close(lsock);
        exit(EXIT_SUCCESS);
      }
      break;
    }
  }

  return 1; /* Dummy */
}
