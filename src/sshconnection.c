/*  This file is part of GSCP.

    GSCP is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    GSCP is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with GSCP.  If not, see <http://www.gnu.org/licenses/>.  */

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

#include <gtk/gtk.h>


void
popup_message (GtkWindow *parent, const gchar *message)
{
  /* Create a modal dialog with an ok button and the given message.  */
  GtkWidget *dialog;
  dialog = gtk_message_dialog_new(parent,
                                  GTK_DIALOG_MODAL,
                                  GTK_MESSAGE_INFO,
                                  GTK_BUTTONS_OK,
                                  "%s",
                                  message);
  g_signal_connect_swapped (dialog,
                            "response",
                            G_CALLBACK (gtk_widget_destroy),
                            dialog);
  gtk_dialog_run(GTK_DIALOG(dialog));
}

void error_message (const char *message, int make_popup, GtkWindow *parent)
{
  /* Print to stderr.  */
  fprintf(stderr, "%s\n", message);
  /* If make_popup is true, create a popup with popup_message ().  */
  if (make_popup)
    {
      popup_message(parent, message);
    }
}

int read_from_ssh (ssh_path_info *info, char **data, size_t *mem_size)
{
  /* Set the contents of data to NULL.  This is so that if the function
     exists on an error, that it won't contain anything.  */
  *data = NULL;

  if (!is_valid_ssh_path(info))
    {
      fprintf(stderr, "Error: Not a valid ssh path\n");
      return 0;
    }

  /* If it's on the localhost, then create a new file object and read it
     into data.  */
  if (info->on_lhost)
    {
      FILE *local_file;
      local_file = fopen(info->path, "rb");
      if (!local_file)
        {
          fprintf(stderr, "Error opening file %s\n", info->path);
          return 0;
        }
      fseek(local_file, 0, SEEK_END);
      *mem_size = ftell(local_file);
      *data = (char *) malloc(sizeof(char) * (*mem_size));
      if (!(*data))
        {
          fprintf(stderr, "Error allocating memory for file.\n");
          return 0;
        }
      rewind(local_file);
      int result = fread((*data), sizeof(char), *mem_size, local_file);
      if (result != *mem_size)
        {
          fprintf(stderr, "Error reading file %s\n", info->path);
          free(*data);
          *data = NULL;
          fclose(local_file);
          return 0;
        }
      fclose(local_file);
      return 1;
    }
  /* If it's over ssh, read from ssh.  */
  else
    {
      int result;
      LIBSSH2_SESSION *session;
      int sock;
      result = ssh_connection_session_open(info->con, &sock, &session);
      if (!result)
        {
          return 0;
        }
      LIBSSH2_CHANNEL *channel;
      libssh2_struct_stat fileinfo;
      channel = libssh2_scp_recv2(session, info->path, &fileinfo);

      if (!channel)
        {
          fprintf(stderr, "unable to open a channel: %d\n",
                  libssh2_session_last_errno(session));
          /*libssh2_session_disconnect(session, "Error: Couldn't open channel.");*/
          /*libssh2_session_free(session);*/
          /*close_socket(sock);*/
          ssh_connection_session_close(&session, sock, "Error: Couldn't open channel.");
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
                      /*libssh2_session_disconnect(session,*/
                      /*"Error: Failed to allocate data.");*/
                      /*libssh2_session_free(session);*/
                      /*close_socket(sock);*/
                      ssh_connection_session_close(&session, sock, "Error, failed to allocate data");
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
                      /*libssh2_session_disconnect(session,*/
                      /*"Error: Failed to allocate data.");*/
                      /*libssh2_session_free(session);*/
                      /*close_socket(sock);*/
                      ssh_connection_session_close(&session, sock, "Error, failed to allocate data");
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
              /*libssh2_session_disconnect(session, "Error");*/
              /*libssh2_session_free(session);*/
              /*close_socket(sock);*/
              ssh_connection_session_close(&session, sock, "Error");
              return 0;
            }
          got += bytes_read;
        }
      libssh2_channel_free(channel);
      channel = NULL;

      ssh_connection_session_close(&session, sock, "Normal shutdown");
      /*libssh2_session_disconnect(session, "Normal Shutdown");*/
      /*libssh2_session_free(session);*/
      /*close_socket(sock);*/
      return 1;
    }
}
int write_to_ssh(ssh_path_info *info, const char *data, size_t mem_size)
{
  if (!is_valid_ssh_path(info))
    {
      return 0;
    }
  /* If it's on the local host, write to the file using the c
     standard library.  */
  if (info->on_lhost)
    {
      FILE *local_file;
      local_file = fopen(info->path, "wb");
      if (!local_file)
        {
          fprintf(stderr, "Error opening file %s\n", info->path);
          return 0;
        }
      for (size_t write_pos = 0; write_pos < mem_size; write_pos += FILE_WRITE_SIZE)
        {
          size_t write_size = (mem_size - write_pos >= FILE_WRITE_SIZE) ? FILE_WRITE_SIZE : (mem_size - write_pos);
          size_t result = fwrite(data + write_pos, sizeof(char), write_size, local_file);
          if (result != write_size)
            {
              fprintf(stderr, "Error writing to file %s\n", info->path);
              fclose(local_file);
              local_file = NULL;
              return 0;
            }
        }
      fclose(local_file);
      local_file = NULL;
      return 1;
    }
  /* If it's over ssh, write to ssh.  */
  else
    {
      int result;
      int sock;
      LIBSSH2_SESSION *session;
      result = ssh_connection_session_open(info->con, &sock, &session);
      LIBSSH2_CHANNEL *channel;
      libssh2_struct_stat fileinfo;
      channel = libssh2_scp_send64(session, info->path, 0777,
                                   mem_size, 0, 0);
      if (!channel)
        {
          char *errmsg;
          int errlen;
          int err = libssh2_session_last_error(session, &errmsg, &errlen, 0);
          fprintf(stderr, "Unable to open a session: (%d) %s\n", err, errmsg);
          ssh_connection_session_close(&session, sock, "Error, unable to open a session");
        }
      size_t current_pos = 0;
      size_t write_size;
      ssize_t bytes_written;
      do
        {
          write_size = ((mem_size - current_pos >= SSH_WRITE_SIZE) ? SSH_WRITE_SIZE
                        : (mem_size - current_pos));
          bytes_written = libssh2_channel_write(channel, data + current_pos, write_size);
          if (bytes_written < 0)
            {
              fprintf(stderr, "Error sending data.\n");

              ssh_channel_send_send_eof(channel);

              fprintf(stderr, "Waiting for channel to close\n");
              libssh2_channel_wait_closed(channel);

              libssh2_channel_free(channel);
              channel = NULL;
              ssh_connection_session_close(&session, sock, "Error");
              return 0;
            }
          current_pos += bytes_written;
        }
      while (current_pos < mem_size);
      ssh_channel_send_send_eof(channel);

      libssh2_channel_free(channel);
      channel = NULL;
      ssh_connection_session_close(&session, sock, "Shutting down");

      return 1;
    }
}

int parse_ssh_path (const char *ssh_path, ssh_path_info *info)
{
  /* This functin isn't written so fail.  */
  return 0;
}

int gscp (ssh_path_info *src, ssh_path_info *dest, GtkProgressBar *progress_bar, int make_popups, GtkWindow *parent)
{
  /* If either of the paths and connections are invalis, then fail.  */
  if (!is_valid_ssh_path(src))
    {
      error_message("Error, invalid source path", make_popups, parent);
      return 0;
    }
  if (!is_valid_ssh_path(dest))
    {
      error_message("Error, invalid destination path", make_popups, parent);
      return 0;
    }

  /*if (progress_bar)*/
    /*{*/
      /*gtk_widget_set_visible(GTK_WIDGET(progress_bar), TRUE);*/
    /*}*/

  

  /* For this next section, it has a temporary buffer. Data is read correctly
     from src, then storred in mem_buf.  mem_buf is then read and written to
     dest.  */

  /* The memory buffer to temporarily store data being transferred.  */
  char *mem_buf;
  /* Thes size of mem_buf.  */
  size_t mem_size;
  /* The position in mem_buf that is currently being written to.  */
  size_t read_pos = 0;
  /* The position in mem_buf that is currently being read from.  */
  size_t write_pos = 0;
  /* The size of the file, in bytes.  */
  libssh2_struct_stat_size read_file_size;
  /* The amount of memory in bytes that has been read from src.  */
  size_t mem_read = 0; /* Note, this is read as "red" because it's past tense.  */
  /* The amount of memory in bytes that has been written to dest.  */
  size_t mem_written = 0;

  /* Either the ssh or file objects are used based on whether or not it's local.  */
  LIBSSH2_SESSION *read_session;
  LIBSSH2_CHANNEL *read_channel;
  int read_sock;
  LIBSSH2_SESSION *write_session;
  LIBSSH2_CHANNEL *write_channel;
  int write_sock;
  FILE *read_file;
  FILE *write_file;

  /* The amount of bytes that should be read every time. It's different
     based on wheter or not the file is local or over ssh.  */
  size_t read_size;
  /* The amount of bytes that should be written every time. It's different
     based on wheter or not the file is local or over ssh.  */
  size_t write_size;
  /* Set read_size correctly based on whether it's a local file or over ssh.  */
  if (src->on_lhost)
    read_size = FILE_READ_SIZE;
  else
    read_size = SSH_READ_SIZE;
  /* Do the same for the write size.  */
  if (dest->on_lhost)
    write_size = FILE_WRITE_SIZE;
  else
    write_size = SSH_WRITE_SIZE;

  /* Set mem_size to the greatr of read_size and write_size.  This is
     so that the buffer can accommodate both sizes.  */
  mem_size = (read_size > write_size) ? read_size : write_size;

  /* Allocate mem_buff.  */
  mem_buf = (char *) malloc(sizeof(char) * (mem_size));
  if (!mem_buf)
    {
      error_message("Error allocating memory buffer", make_popups, parent);
      return 0;
    }

  /* If src is on the localhost, open using file io.  Else, open it over ssh.  */
  if (src->on_lhost)
    {
      read_file = fopen(src->path, "rb");
      if (!read_file)
        {
          fprintf(stderr, "Error opening file %s for reading\n", src->path);
          if (make_popups)
            popup_message(parent, "Error opening file for reading");
          free(mem_buf);
          mem_buf = NULL;
          return 0;
        }
      fseek(read_file, 0, SEEK_END);
      read_file_size = ftell(read_file);
      fseek(read_file, 0, SEEK_SET);
    }
  else
    {
      int result = ssh_connection_session_open(src->con, &read_sock, &read_session);
      if (!result)
        {
          if (make_popups)
            {
              GtkWidget *dialog = gtk_message_dialog_new(parent,
                                                                GTK_DIALOG_MODAL,
                                                                GTK_MESSAGE_INFO,
                                                                GTK_BUTTONS_OK,
                                                                "Error connecting to source on address %d on port %i",
                                                                src->con->hostaddr,
                                                                src->con->port);
              g_signal_connect_swapped(G_OBJECT(dialog), "response", G_CALLBACK(gtk_widget_destroy), dialog);
              gtk_dialog_run(GTK_DIALOG(dialog));
            }

          free(mem_buf);
          mem_buf = NULL;
          return 0;
        }
      libssh2_struct_stat fileinfo;
      read_channel = libssh2_scp_recv2(read_session, src->path, &fileinfo);
      if (!read_channel)
        {
          free(mem_buf);
          mem_buf = NULL;
          fprintf(stderr, "Error opening remote file %s", src->path);
          if (make_popups)
            popup_message(parent, "Error opening remote file for reading");
          ssh_connection_session_close(&read_session, read_sock, "Error opening file for writing");
          return 0;
        }
      read_file_size = fileinfo.st_size;
      /*printf("fileinfo.st_size %ld\n", fileinfo.st_size);*/
    }
  /* If dest is on the localhost, open using file io.  Else, open it over ssh.  */
  if (dest->on_lhost)
    {
      write_file = fopen(dest->path, "wb");
      if (!write_file)
        {
          free(mem_buf);
          mem_buf = NULL;
          fprintf(stderr, "Error opening file %s for writing\n", dest->path);
          if (make_popups)
            popup_message(parent, "Error opening file for writing");
          if (src->on_lhost)
            {
              fclose(read_file);
              read_file = 0;
            }
          else
            ssh_connection_session_close(&read_session, read_sock, "Error");
          return 0;
        }
    }
  else
    {
      int result = ssh_connection_session_open(dest->con, &write_sock, &write_session);
      if (!result)
        {
          free(mem_buf);
          mem_buf = NULL;
          if (make_popups)
            {
              GtkWidget *dialog = gtk_message_dialog_new(parent,
                                                                GTK_DIALOG_MODAL,
                                                                GTK_MESSAGE_INFO,
                                                                GTK_BUTTONS_OK,
                                                                "Error connecting to destination on address %d on port %i",
                                                                dest->con->hostaddr,
                                                                dest->con->port);
              g_signal_connect_swapped(G_OBJECT(dialog), "response", G_CALLBACK(gtk_widget_destroy), dialog);
              gtk_dialog_run(GTK_DIALOG(dialog));
            }

          if (src->on_lhost)
            {
              fclose(read_file);
              read_file = 0;
            }
          else
            {
              libssh2_channel_free(read_channel);
              ssh_connection_session_close(&read_session, read_sock, "Error");
            }
          return 0;
        }
      write_channel = libssh2_scp_send64(write_session, dest->path, 0777, read_file_size, 0, 0);
      if (!write_channel)
        {
          fprintf(stderr, "Error opening remote file %s", src->path);
          if (make_popups)
            popup_message(parent, "Error opening remote file for writing");
          free(mem_buf);
          mem_buf = NULL;
          if (src->on_lhost)
            {
              fclose(read_file);
              read_file = 0;
            }
          else
            {
              libssh2_channel_free(read_channel);
              ssh_connection_session_close(&read_session, read_sock, "Error");
            }

          ssh_connection_session_close(&write_session, write_sock, "Error opening file for writing");
          return 0;
        }
    }

  int result;
  /* Repeat until all of the data has been read from src.  */
  while (mem_read < read_file_size)
    {
      /* Now, read data from src into mem_buff.  Stop when either
         the buffer is full or the end of the file has been reached.  */
      while (read_pos < mem_size && mem_read < read_file_size)
        {
          size_t remaining_bytes_buff = mem_size - read_pos;
          size_t remaining_bytes_file = read_file_size - mem_read;
          /* Set the amount of bytes to read to be the least of these three values:
             the remaining memory in the buffer, the remaining bytes in the file, or
             the maximum amount of data to be read every time.  */
          size_t bytes_to_read = (remaining_bytes_buff < read_size) ? remaining_bytes_buff : read_size;
          bytes_to_read = (remaining_bytes_file < bytes_to_read) ? remaining_bytes_file : bytes_to_read;
          /*printf("bytes_to_read %zu\n", bytes_to_read);*/
          int bytes_read;
          if (src->on_lhost)
            {
              bytes_read = fread(mem_buf + read_pos, sizeof(char), bytes_to_read, read_file);
            }
          else
            {
              bytes_read = libssh2_channel_read(read_channel, mem_buf + read_pos, bytes_to_read);
            }
          /* The return value of both functions is the amount of bytes read so if it's less than zero
             there was an error.  */
          if (bytes_read < 0)
            {
              error_message("Error reading file", make_popups, parent);
              free(mem_buf);
              mem_buf = NULL;
              if (src->on_lhost)
                {
                  fclose(read_file);
                  read_file = NULL;
                }
              else
                {
                  libssh2_channel_free(read_channel);
                  ssh_connection_session_close(&read_session, read_sock, "Error");
                }

              if (dest->on_lhost)
                {
                  fclose(write_file);
                  write_file = NULL;
                }
              else
                {
                  ssh_channel_send_send_eof(write_channel);
                  libssh2_channel_free(write_channel);
                  ssh_connection_session_close(&write_session, write_sock, "Error opening file for writing");
                }
              return 0;
            }
          /* Make sure that the amount of bytes read and the amount of total
             memory read is updates.  */
          read_pos += bytes_read;
          mem_read += bytes_read;
        }
      /* Now that a block of memory has been read, write all of it to
         dest. Stop when either as much data has been written as has been read
         or the amount of data is the amount as the total size of the total.  */
      while (write_pos < read_pos && mem_written < read_file_size)
        {
          size_t remaining_bytes_buff = read_pos - write_pos;
          size_t remaining_bytes_file = read_file_size - mem_written;
          /* Set the amount of bytes to write to be the least of thes three:
             the amount of bytes between the current position and the amount of data
             read, the amount of bytes left in the file, or the maximum amount to write
             at once.  */
          size_t bytes_to_write = (remaining_bytes_buff < write_size) ? remaining_bytes_buff : write_size;
          bytes_to_write = (remaining_bytes_file < bytes_to_write) ? remaining_bytes_file : bytes_to_write;
          int bytes_written;
          if (dest->on_lhost)
            {
              bytes_written = fwrite(mem_buf + write_pos, sizeof(char), bytes_to_write, write_file);
            }
          else
            {
              bytes_written = libssh2_channel_write(write_channel, mem_buf + write_pos, bytes_to_write);
            }
          if (bytes_written != bytes_to_write)
            {
              error_message("Error writing to file", make_popups, parent);
              free(mem_buf);
              mem_buf = NULL;
              if (src->on_lhost)
                {
                  fclose(read_file);
                  read_file = NULL;
                }
              else
                {
                  libssh2_channel_free(read_channel);
                  ssh_connection_session_close(&read_session, read_sock, "Error");
                }

              if (dest->on_lhost)
                {
                  fclose(write_file);
                  write_file = NULL;
                }
              else
                {
                  ssh_channel_send_send_eof(write_channel);
                  libssh2_channel_free(write_channel);
                  ssh_connection_session_close(&write_session, write_sock, "Error opening file for writing");
                }
              return 0;
            }
          /* Make sure that the current write position and the total amount written
             are updates.  */
          write_pos += bytes_written;
          mem_written += bytes_written;
          if (progress_bar)
            {
              double progress_fraction = (double) mem_written / read_file_size;
              /*printf("%f\n", progress_fraction);*/
              /*printf("mem written %zu read file size %zu fraction %f\n", mem_written, read_file_size, progress_fraction);*/
              gtk_progress_bar_set_fraction(progress_bar, progress_fraction);
            }
        }
      /* Reset the read and write positions so that they will start over.  */
      read_pos = 0;
      write_pos = 0;
    }
  /* Do the cleanup.  */
  free(mem_buf);
  mem_buf = NULL;
  if (src->on_lhost)
    {
      fclose(read_file);
      read_file = NULL;
    }
  else
    {
      libssh2_channel_free(read_channel);
      ssh_connection_session_close(&read_session, read_sock, "Normal shutdown");
    }

  if (dest->on_lhost)
    {
      fclose(write_file);
      write_file = NULL;
    }
  else
    {
      ssh_channel_send_send_eof(write_channel);
      libssh2_channel_free(write_channel);
      ssh_connection_session_close(&write_session, write_sock, "Error opening file for writing");
    }
  return 1;
}

int is_valid_ssh_path(ssh_path_info *info)
{
  if (info == NULL)
    {
    return 0;
    }

  if (info->path == NULL)
    {
    return 0;
    }

  if (!info->on_lhost && !is_valid_ssh_connection(info->con))
    {
    return 0;
    }

  return 1;
}

int is_valid_ssh_connection(ssh_connection *con)
{
  if (con == NULL)
    {
    return 0;
    }

  if (con->username == NULL)
    {
    return 0;
    }

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
  if (con != NULL)
    {
      free(con->username);
      con->username = NULL;
      free(con->password);
      con->password = NULL;
    }
  free(con);
}

int get_ssh_path_info_from_user(ssh_path_info *info)
{
  /* I just realized I'll have to do use ssh_connection_new.  */
  char string_input[1024];
  int max_input_len;
  char input_char;
  printf("Is this file on the local machine?\n");
  scanf("%c", &input_char);
  if (input_char == 'y' || input_char == 'Y')
    {
      info->on_lhost = 1;
    }
  else
    {
      info->on_lhost = 0;
      printf("What's the ip address?\n");
      fgets(string_input, max_input_len, stdin);
      string_input[strchr(string_input, '\n') - string_input] = '\0';
      info->con->hostaddr = inet_addr(string_input);
      printf("What's the port?\n");
      int port_input;
      scanf("%i", &port_input);
      info->con->port = port_input;
      info->con->auth_pw = 1;
      printf("What's the username?\n");
      fgets(string_input, max_input_len, stdin);
      string_input[strchr(string_input, '\n') - string_input] = '\0';
      info->con->username = string_input;
      printf("What's the password?\n");
      fgets(string_input, max_input_len, stdin);
      string_input[strchr(string_input, '\n') - string_input] = '\0';
      info->con->password = string_input;
    }
  printf("What's the path?\n");
  fgets(string_input, max_input_len, stdin);
  string_input[strchr(string_input, '\n') - string_input] = '\0';

  return 0;
}

int ssh_connection_session_open(ssh_connection *con, int *sock, LIBSSH2_SESSION **session)
{
  *sock = 0;
  *session = NULL;

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

  *sock = socket(AF_INET, SOCK_STREAM, 0);
  struct sockaddr_in sin;
  sin.sin_family = AF_INET;
  sin.sin_port = htons(con->port);
  sin.sin_addr.s_addr = con->hostaddr;
  /*sin.sin_addr.s_addr = inet_addr("127.0.0.1");*/
  if (connect(*sock, (struct sockaddr *)(&sin), sizeof(struct sockaddr_in)) != 0)
    {
      fprintf(stderr, "Error, failed to connect to address %i on port %i\n", con->hostaddr, con->port);
      return 0;
    }

  *session = libssh2_session_init();
  if (!(*session))
    {
      fprintf(stderr, "Error, can't establish ssh session\n");
      close_socket(*sock);
      return 0;
    }
  result = libssh2_session_handshake(*session, *sock);
  if (result)
    {
      fprintf(stderr, "Error, failure with handshake\n");
      close_socket(*sock);
      return 0;
    }
  const char *finger_temp;
  finger_temp = libssh2_hostkey_hash(*session, LIBSSH2_HOSTKEY_HASH_SHA1);
  /* Check to see if it's NULL.  */
  /*size_t finger_len = strlen(finger_temp);*/
  /*info->con->fingerprint = malloc(sizeof(char) * (finger_len + 1));*/
  /*strcpy(info->con->fingerprint, finger_temp);*/
  int auth_success = libssh2_userauth_password(*session, con->username, con->password);
  if (auth_success)
    {
      fprintf(stderr, "Authentication by password failed.\n");
      libssh2_session_disconnect(*session,
                                 "Error: Failure authenticating.");
      libssh2_session_free(*session);
      close_socket(*sock);
      return 0;
    }
  return 1;
}

int ssh_connection_session_close(LIBSSH2_SESSION **session, int sock, const char *message)
{
  libssh2_session_disconnect(*session, message);
  libssh2_session_free(*session);
  close_socket(sock);
  return 1;
}

int ssh_channel_send_send_eof(LIBSSH2_CHANNEL *channel)
{
  libssh2_channel_send_eof(channel);
  libssh2_channel_wait_eof(channel);
  libssh2_channel_wait_closed(channel);
  return 1;
}
