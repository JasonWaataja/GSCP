#ifndef SSHCONNECTION_H
#define SSHCONNECTION_H

#include <stdint.h>
#include <stdlib.h>

#define SSH_DEFAULT_PORT 22

struct _ssh_connection
{
  unsigned long hostaddr;
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

int is_valid_ssh_path(ssh_path_info *info);
int is_valid_ssh_connection(ssh_connection *con);

int read_from_ssh(ssh_path_info *info, uint8_t *data, size_t *mem_size);
int write_to_ssh(ssh_path_info *info, const uint8_t *data, size_t *mem_size);

int parse_ssh_path(const char *ssh_path, ssh_path_info *info);

int gscp(ssh_path_info *source, ssh_path_info *dest);

#endif /* SSHCONNECTION_H */ 
