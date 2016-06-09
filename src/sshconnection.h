#ifndef SSHCONNECTION_H
#define SSHCONNECTION_H

#include <stdint.h>
#include <stdlib.h>
#include <libssh2.h>

#define SSH_DEFAULT_PORT 22

#define SSH_READ_SIZE 1024
#define SSH_WRITE_SIZE 1024

#define FILE_WRITE_SIZE 2048
#define FILE_WRITE_SIZE 2048

struct _ssh_connection
{
  uint32_t hostaddr;
  unsigned int port;
  int auth_pw;
  //char *fingerprint;
  char *username;
  char *password;
};

typedef struct _ssh_connection ssh_connection;

struct _ssh_path_info
{
  /* boolean is on local host */
  int on_lhost;
  char *path;
  ssh_connection *con;
};

typedef struct _ssh_path_info ssh_path_info;

ssh_connection *ssh_connection_new(const char *username, const char *password);
void ssh_connection_free(ssh_connection *con);

int is_valid_ssh_path(ssh_path_info *info);
int is_valid_ssh_connection(ssh_connection *con);

void close_socket(int socket);
//void close_libssh2_session(LIBSSH2_SESSION *session, const char *close_messg);
//void close_libssh2_channel(LIBSSH2_CHANNEL *channel);

int read_from_ssh(ssh_path_info *info, char **data, size_t *mem_size);
int write_to_ssh(ssh_path_info *info, char *const *data, size_t *mem_size);

int parse_ssh_path(const char *ssh_path, ssh_path_info *info);

int gscp(ssh_path_info *source, ssh_path_info *dest);

#endif /* SSHCONNECTION_H */ 
