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

int read_from_ssh(ssh_path_info *info, char *data, size_t *mem_size)
{

  if (!is_valid_ssh_path(info))
    {
      return 0;
    }

  if (info->on_lhost)
    {

    }
  else
    {

      int result;
#ifdef WIN32
      WSADATA wsadata;
      int err;

      err = WSAStartup(MAKEWORD(2,0), &wsadata);
      if (err != 0) {
          fprintf(stderr, "WSAStartup failed with error: %d\n", err);
          return 0;
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
          close_socket(sock);
          return 0;
        }
      const char *finger_temp;
      finger_temp = libssh2_hostkey_hash(session, LIBSSH2_HOSTKEY_HASH_SHA1);
      /* Check to see if it's NULL.  */
      /*size_t finger_len = strlen(finger_temp);*/
      /*info->con->fingerprint = malloc(sizeof(char) * (finger_len + 1));*/
      /*strcpy(info->con->fingerprint, finger_temp);*/

      int auth_success = libssh2_userauth_password(session, info->con->username, info->con->password);
      if (auth_success)
        {
          fprintf(stderr, "Authentication by password failed.\n");
          libssh2_session_disconnect(session,
                                     "Error: Failure authenticating.");
          libssh2_session_free(session);
          close_socket(sock);
          return 0;
        }
      LIBSSH2_CHANNEL *channel;
      libssh2_struct_stat fileinfo;
      channel = libssh2_scp_recv2(session, info->path, &fileinfo);

      if (!channel)
        {
          fprintf(stderr, "unable to open a session: %d\n",
                  libssh2_session_last_errno(session));
          libssh2_session_disconnect(session, "Error: Couldn't open channel.");
          libssh2_session_free(session);
          close_socket(sock);
          return(0);
        }

      libssh2_struct_stat_size got = 0;
      *mem_size = 0;

      while (got < fileinfo.st_size)
        {
          char mem_buf[1024];
          int read_size = sizeof(mem_buf);
          if ((fileinfo.st_size - got) < read_size)
            read_size = (int) (fileinfo.st_size - got);

          result = libssh2_channel_read(channel, mem_buf, read_size);
          if (result > 0)
            {
              size_t old_size = *mem_size;
              *mem_size += read_size;
              if (old_size == 0)
                {
                  data = (char *) malloc(sizeof(char) * read_size);
                  if (!data)
                    {
                      libssh2_channel_free(channel);
                      libssh2_session_disconnect(session,
                                                 "Error: Failed to allocate data.");
                      libssh2_session_free(session);
                      close_socket(sock);
                      return 0;
                    }
                }
              else
                {
                  if (!realloc(data, *mem_size))
                    {
                      libssh2_channel_free(channel);
                      libssh2_session_disconnect(session,
                                                 "Error: Failed to allocate data.");
                      libssh2_session_free(session);
                      close_socket(sock);
                      return 0;
                    }
                }
            }
          else if (result < 0)
            {
              fprintf(stderr, "libssh2_channel_read() failed: %d\n", result);
              libssh2_channel_free(channel);
              libssh2_session_disconnect(session, "Error");
              libssh2_session_free(session);
              close_socket(sock);
              return 0;
            }
          got += result;
        }
      libssh2_channel_free(channel);
      channel = NULL;

      libssh2_session_disconnect(session, "Normal Shutdown");
      libssh2_session_free(session);
      close_socket(sock);
      return 1;
    }
}
int write_to_ssh(ssh_path_info *info, const char *data, size_t *mem_size);

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

void close_socket(int socket)
{
#ifdef WIN32
  closesocket(socket);
#else
  close(socket);
#endif /* WIN32 */
}

ssh_connection *ssh_connection_new(const char *username, const char *password)
{
  ssh_connection *con = (ssh_connection *) malloc(sizeof(ssh_connection));
  if (!con)
    return con;

  con->username = (char *) malloc(sizeof(char) * (strlen(username) + 1));
  con->password = (char *) malloc(sizeof(char) * (strlen(username) + 1));
  if (con->username == NULL || con->password == NULL)
    {
      free(con->username);
      free(con->password);
      free(con);
      return NULL;
    }

  strcpy(con->username, username);
  strcpy(con->password, password);
  return con;
}

void ssh_connectin_free(ssh_connection *con)
{
  free(con->username);
  free(con->password);
  free(con);
}
