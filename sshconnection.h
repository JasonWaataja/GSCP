#ifndef SSHCONNECTION_H
#define SSHCONNECTION_H

struct _ssh_connection
{
  unsigned long hostaddr;
  int auth_pw;
  char *fingerprint;
  char *username;
  char *password;
};

typedef _ssh_connection ssh_connection;

struct _ssh_path_info
{
  /* boolean is on local host */
  int localhost;
  char *path;
  ssh_connection *con;
};

typedef _ssh_path_info ssh_path_info;

int parse_ssh_path(const char *ssh_path, ssh_path_info *info);

int gscp(ssh_path_info *source, ssh_path_info *dest);

#endif /* SSHCONNECTION_H */ 
