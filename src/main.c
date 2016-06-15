/*
 * Sample showing how to do a simple SCP transfer.
 */

#include "libssh2_config.h"
#include <libssh2.h>

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

#include "sshconnection.h"

GtkWidget *window;
GtkWidget *grid;

GtkWidget *src_on_lhost_checkbox;
GtkWidget *dest_on_lhost_checkbox;
GtkWidget *src_addr_entry;
GtkWidget *dest_addr_entry;
GtkWidget *src_prt_entry;
GtkWidget *dest_prt_entry;
GtkWidget *src_pswd_entry;
GtkWidget *dest_pswd_entry;

static void
src_on_lhost_checkbox_toggled()
{
}

static void
activate (GtkApplication* app,
          gpointer        user_data)
{
  window = gtk_application_window_new (app);
  gtk_window_set_title (GTK_WINDOW (window), "Window");
  /*gtk_window_set_default_size (GTK_WINDOW (window), 200, 200);*/


  grid = gtk_grid_new();
  gtk_container_add(GTK_CONTAINER(window), grid);
  src_on_lhost_checkbox = gtk_check_button_new_with_label("on local filesystem");
  gtk_grid_attach(GTK_GRID(grid), src_on_lhost_checkbox, 0, 0, 1, 1);
  dest_on_lhost_checkbox = gtk_check_button_new_with_label("on local filesystem");
  gtk_grid_attach(GTK_GRID(grid), dest_on_lhost_checkbox, 1, 0, 1, 1);
  src_addr_entry = gtk_entry_new();
  gtk_entry_set_placeholder_text(GTK_ENTRY(src_addr_entry), "address");
  gtk_grid_attach(GTK_GRID(grid), src_addr_entry, 0, 1, 1, 1);
  dest_addr_entry = gtk_entry_new();
  gtk_entry_set_placeholder_text(GTK_ENTRY(dest_addr_entry), "address");
  gtk_grid_attach(GTK_GRID(grid), dest_addr_entry, 1, 1, 1, 1);


  gtk_widget_show_all (window);
}

int main(int argc, char *argv[])
{
  /*libssh2_init(0);*/

  /*ssh_path_info inf;*/
  /*inf.on_lhost = 1;*/
  /*char path[] = "/home/jason/testfile.txt";*/
  /*inf.path = malloc(sizeof(path));*/
  /*strcpy(inf.path, path);*/
  /*inf.con = NULL;*/

  /*char *data;*/
  /*size_t mem_size;*/
  /*int result;*/
  /*[>result = read_from_ssh(&inf, &data, &mem_size);<]*/
  /*[>if (result)<]*/
    /*[>{<]*/
      /*[>for (int i = 0; i < mem_size; i++)<]*/
        /*[>putchar(data[i]);<]*/
    /*[>}<]*/
  /*char string_to_write[] = "Hello file";*/
  /*result = write_to_ssh(&inf, string_to_write, strlen(string_to_write));*/
  /*printf("result %i\n", result);*/
  /*free(inf.path);*/

  /*ssh_connection *con;*/
  /*char password[256];*/
  /*printf("Enter the password.\n");*/
  /*fgets(password, sizeof(password), stdin);*/
  /*size_t length = strlen(password);*/
  /*char *nl_pos;*/
  /*if ((nl_pos = strchr(password, '\n')))*/
      /**nl_pos = '\0';*/
  /*con = ssh_connection_new("jason", password);*/
  /*if (con)*/
    /*{*/
      /*con->auth_pw = 1;*/
      /*con->hostaddr = inet_addr("127.0.0.1");*/
      /*ssh_path_info inf;*/
      /*inf.con = con;*/
      /*inf.on_lhost = 0;*/
      /*[>[>char path[] = "/home/jason/Music/Danny/rip.flac";<]<]*/
      /*[>char path[] = "/home/jason/testfile2.txt";<]*/
      /*char path[] = "/home/jason/testfile.txt";*/
      /*inf.path = (char *) malloc(sizeof(path));*/
      /*strcpy(inf.path, path);*/
      /*char *data;*/
      /*size_t mem_size;*/
      /*[>char string_to_write[] = "Hello scp";<]*/
      /*[>mem_size = strlen(string_to_write);<]*/
      /*[>write_to_ssh(&inf, string_to_write, mem_size);<]*/
      /*read_from_ssh(&inf, &data, &mem_size);*/
      /*for (int i = 0; i < mem_size; i++)*/
        /*{*/
          /*putchar(data[i]);*/
        /*}*/
      /*putchar('\n');*/
      /*free(data);*/
      /*free(inf.path);*/
    /*}*/
  /*ssh_connection_free(con);*/

  /*ssh_path_info src;*/
  /*src.on_lhost = 1;*/
  /*src.path = "/home/jason/src.txt";*/
  /*[>src.path = "/home/jason/Videos/DemonstrationCompressed.mkv";<]*/
  /*src.con = NULL;*/
  /*ssh_path_info dest;*/
  /*dest.on_lhost = 0;*/
  /*dest.path = "/home/jason/dest.txt";*/
  /*char password[1024];*/
  /*printf("What's the password?\n");*/
  /*fgets(password, sizeof(password), stdin);*/
  /**(strchr(password, '\n')) = '\0';*/
  /*dest.con = ssh_connection_new("jason", password);*/
  /*dest.con->auth_pw = 1;*/
  /*[>dest.con->hostaddr = inet_addr("127.0.0.1");<]*/
  /*dest.con->hostaddr = inet_addr("98.225.21.217");*/
  /*dest.con->port = 8022;*/
  /*[>gscp(&src, &dest);<]*/
  /*gscp(&dest, &src);*/

  /*ssh_connection_free(dest.con);*/

  /*libssh2_exit();*/

  GtkApplication *app;
  int status;

  app = gtk_application_new ("com.waataja.gscp", G_APPLICATION_FLAGS_NONE);
  g_signal_connect (app, "activate", G_CALLBACK (activate), NULL);
  status = g_application_run (G_APPLICATION (app), argc, argv);
  g_object_unref (app);

  return status;
}
