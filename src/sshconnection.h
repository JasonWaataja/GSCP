#ifndef SSHCONNECTION_H
#define SSHCONNECTION_H

#include <stdint.h>
#include <stdlib.h>
#include <libssh2.h>
#include <gtk/gtk.h>

#define SSH_DEFAULT_PORT 22

#define SSH_READ_SIZE 32000
#define SSH_WRITE_SIZE 32000

#define FILE_READ_SIZE 4000
#define FILE_WRITE_SIZE 4000

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

int ssh_connection_session_open(ssh_connection *con, int *sock, LIBSSH2_SESSION **session);
int ssh_connection_session_close(LIBSSH2_SESSION **session, int sock, const char *message);
int ssh_channel_send_send_eof(LIBSSH2_CHANNEL *channel);

int is_valid_ssh_path(ssh_path_info *info);
int is_valid_ssh_connection(ssh_connection *con);

void close_socket(int socket);
//void close_libssh2_session(LIBSSH2_SESSION *session, const char *close_messg);
//void close_libssh2_channel(LIBSSH2_CHANNEL *channel);

int read_from_ssh(ssh_path_info *info, char **data, size_t *mem_size);
int write_to_ssh(ssh_path_info *info, const char *data, size_t mem_size);

int parse_ssh_path(const char *ssh_path, ssh_path_info *info);

int gscp(ssh_path_info *src, ssh_path_info *dest, GtkProgressBar *progress_bar, int make_popups, GtkWindow *parent);

int get_ssh_path_info_from_user(ssh_path_info *info);

void popup_message(GtkWindow *parent, const gchar *message);
void error_message(const char *message, int make_popup, GtkWindow *parent);

#endif /* SSHCONNECTION_H */ 
