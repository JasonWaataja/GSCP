#include "sshconnection.h"

#include <libssh2.h>
#include "libssh2_config.h"
#include <string.h>
#ifdef HAVE_WINSOCK2_H
# include <winsock2.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif
#ifdef HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif
# ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_ARPA_INET_H
# include <arpa/inet.h>
#endif
#ifdef HAVE_SYS_TIME_H
# include <sys/time.h>
#endif

#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <ctype.h>

int read_from_ssh(ssh_path_info *info, uint8_t *data, size_t *mem_size)
{
  if (!is_valid_ssh_path(info))
    {
      exit(EXIT_FAILURE);
    }

  LIBSSH2_CHANNEL *channel;
  libssh2_struct_stat_size fileinfo;
  int result;
  libssh2_struct_stat_size got = 0;
#ifdef WIN32
  WSADATA wsadata;
  int err;

  err = WSAStartup(MAKEWORD(2,0), &wsadata);
  if (err != 0) {
      fprintf(stderr, "WSAStartup failed with error: %d\n", err);
      /*return 1;*/
      exit(EXIT_FAILURE);
  }
#endif

  /* Right now I'm assuming that libssh2 has been init with libssh2_init(0).  */

  int sock = socket(AF_INET, SOCK_STREAM, 0);
  struct sockaddr_in sin;
  sin.sin_family = AF_INET;
  sin.sin_port = htons(info->con->port);
  sin.sin_addr.s_addr = info->con->port;
  if (connect(sock, (struct sockaddr*)(&sin), sizeof(struct sockaddr_in)) != 0)
    {
      fprintf(stderr, "failed to connect!\n");
      return 0;
    }

  LIBSSH2_SESSION *session = libssh2_session_init();
  if (!session)
    return 0;

  result = libssh2_session_handshake(session, sock);
  if (result)
    {
      fprintf(stderr, "Failure establishing SSH session: %d\n", result);
      return 0;
    }
  const char *finger_temp;
  finger_temp = libssh2_hostkey_hash(session, LIBSSH2_HOSTKEY_HASH_SHA1);
  /* Check to see if it's NULL.  */
  /*size_t finger_len = strlen(finger_temp);*/
  /*info->con->fingerprint = malloc(sizeof(char) * (finger_len + 1));*/
  /*strcpy(info->con->fingerprint, finger_temp);*/



  return 1;
}
int write_to_ssh(ssh_path_info *info, const uint8_t *data, size_t *mem_size);

int parse_ssh_path(const char *ssh_path, ssh_path_info *info)
{
  return 1;
}

int gscp(ssh_path_info *source, ssh_path_info *info)
{
  return 1;
}

int is_valid_ssh_path(ssh_path_info *info)
{
  if (info == NULL)
    return 0;

  if (info->path == NULL)
    return 0;

  if (!is_valid_ssh_connection(info->con))
    return 0;

  return 1;
}

int is_valid_ssh_connection(ssh_connection *con)
{
  if (con == NULL)
    return 0;

  if (con->username == NULL)
    return 0;

  return 1;
}
