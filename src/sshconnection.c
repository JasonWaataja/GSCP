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

int read_from_ssh(ssh_path_info *info, char **data, size_t *mem_size)
{
  *data = NULL;

  if (!is_valid_ssh_path(info))
    {
      return 0;
    }

  if (info->on_lhost)
    {
      return 1;
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
      sin.sin_addr.s_addr = info->con->hostaddr;
      /*sin.sin_addr.s_addr = inet_addr("127.0.0.1");*/
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

      ssize_t bytes_read;
      while (got < fileinfo.st_size)
        {
          char mem_buf[SSH_READ_SIZE];
          int read_size = sizeof(mem_buf);
          if ((fileinfo.st_size - got) < read_size)
            read_size = (size_t) (fileinfo.st_size - got);

          bytes_read = libssh2_channel_read(channel, mem_buf, read_size);
          /* Note, excludes if it read 0. It doesn't do anything.  */
          if (bytes_read > 0)
            {
              size_t old_size = *mem_size;
              *mem_size += bytes_read;
              if (old_size == 0)
                {
                  *data = (char *) malloc(sizeof(char) * bytes_read);
                  if (!*data)
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
                  if (!realloc(*data, *mem_size))
                    {
                      free(*data);
                      *data = NULL;
                      libssh2_channel_free(channel);
                      libssh2_session_disconnect(session,
                                                 "Error: Failed to allocate data.");
                      libssh2_session_free(session);
                      close_socket(sock);
                      return 0;
                    }
                }
              memcpy(*data + old_size, mem_buf, bytes_read);
            }
          else if (bytes_read < 0)
            {
              free(*data);
              *data = NULL;
              fprintf(stderr, "libssh2_channel_read() failed: %d\n", result);
              libssh2_channel_free(channel);
              libssh2_session_disconnect(session, "Error");
              libssh2_session_free(session);
              close_socket(sock);
              return 0;
            }
          got += bytes_read;
        }
      libssh2_channel_free(channel);
      channel = NULL;

      libssh2_session_disconnect(session, "Normal Shutdown");
      libssh2_session_free(session);
      close_socket(sock);
      return 1;
    }
}
int write_to_ssh(ssh_path_info *info, const char *data, size_t mem_size)
{
  if (!is_valid_ssh_path(info))
    {
      return 0;
    }
  if (info->on_lhost)
    {
      return 1;
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
      int sock = socket(AF_INET, SOCK_STREAM, 0);
      struct sockaddr_in sin;
      sin.sin_family = AF_INET;
      sin.sin_port = htons(info->con->port);
      sin.sin_addr.s_addr = info->con->hostaddr;
      if (connect(sock, (struct sockaddr*)(&sin), sizeof(struct sockaddr_in)) != 0)
        {
          fprintf(stderr, "failed to connect!\n");
          return 0;
        }
      LIBSSH2_SESSION *session = libssh2_session_init();
      if (!session)
        {
          return 0;
        }

      result = libssh2_session_handshake(session, sock);
      if (result)
        {
          fprintf(stderr, "Failure establishing SSH session: %d\n", result);
          close_socket(sock);
          return 0;
        }
      const char *finger_temp;
      finger_temp = libssh2_hostkey_hash(session, LIBSSH2_HOSTKEY_HASH_SHA1);
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
      /*channel = libssh2_scp_send64(session, info->path, fileinfo.st_mode & 0777,*/
                                   /*(unsigned long)fileinfo.st_size, 0, 0);*/
      channel = libssh2_scp_send64(session, info->path, 0777,
                                   mem_size, 0, 0);
      /*channel = libssh2_scp_send(session, info->path, fileinfo.st_mode & 0777,*/
                                   /*(unsigned long)fileinfo.st_size);*/
      /*channel = libssh2_scp_send(session, info->path, fileinfo.st_mode & 0777,*/
                                   /*(unsigned long)fileinfo.st_size);*/
      if (!channel)
        {
          char *errmsg;
          int errlen;
          int err = libssh2_session_last_error(session, &errmsg, &errlen, 0);
          fprintf(stderr, "Unable to open a session: (%d) %s\n", err, errmsg);
          libssh2_session_disconnect(session, "Error");
          libssh2_session_free(session);
          close_socket(sock);
        }
      size_t current_pos = 0;
      size_t write_size;
      ssize_t bytes_written;
      do
        {
          write_size = ((mem_size - current_pos >= SSH_WRITE_SIZE) ? SSH_WRITE_SIZE
                                                                   : (mem_size - current_pos));
          printf("%zu\n", write_size);
          bytes_written = libssh2_channel_write(channel, data + current_pos, write_size);
          printf("%zu bytes written.\n", bytes_written);
          if (bytes_written < 0)
            {
              printf("Error sending data.\n");

              fprintf(stderr, "Sending EOF\n");
              libssh2_channel_send_eof(channel);

              fprintf(stderr, "Waiting for EOF\n");
              libssh2_channel_wait_eof(channel);

              fprintf(stderr, "Waiting for channel to close\n");
              libssh2_channel_wait_closed(channel);

              libssh2_channel_free(channel);
              channel = NULL;
              libssh2_session_disconnect(session, "Error");
              libssh2_session_free(session);
              close_socket(sock);
              return 0;
            }
          current_pos += bytes_written;
        }
      while (current_pos < mem_size);
      printf("Done Writing");

      fprintf(stderr, "Sending EOF\n");
      libssh2_channel_send_eof(channel);

      fprintf(stderr, "Waiting for EOF\n");
      libssh2_channel_wait_eof(channel);

      fprintf(stderr, "Waiting for channel to close\n");
      libssh2_channel_wait_closed(channel);

      libssh2_channel_free(channel);
      channel = NULL;
      libssh2_session_disconnect(session, "Shutting Down");
      libssh2_session_free(session);
      close_socket(sock);
      
      return 1;
    }
}

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
  con->password = (char *) malloc(sizeof(char) * (strlen(password) + 1));
  if (con->username == NULL || con->password == NULL)
    {
      fprintf(stderr, "Error creating connection.\n");
      free(con->username);
      free(con->password);
      free(con);
      return NULL;
    }

  strcpy(con->username, username);
  strcpy(con->password, password);
  con->port = SSH_DEFAULT_PORT;
  return con;
}

void ssh_connection_free(ssh_connection *con)
{
  free(con->username);
  free(con->password);
  free(con);
}
