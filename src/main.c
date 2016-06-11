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

#include "sshconnection.h"

int main(int argc, char *argv[])
{
  libssh2_init(0);

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

  ssh_path_info src;
  src.on_lhost = 1;
  src.path = "/home/jason/src.txt";
  /*src.path = "/home/jason/Videos/Demonstration.mkv";*/
  src.con = NULL;
  ssh_path_info dest;
  dest.on_lhost = 0;
  dest.path = "/home/jason/dest.txt";
  char password[1024];
  printf("What's the password?\n");
  fgets(password, sizeof(password), stdin);
  *(strchr(password, '\n')) = '\0';
  dest.con = ssh_connection_new("jason", password);
  dest.con->auth_pw = 1;
  /*dest.con->hostaddr = inet_addr("127.0.0.1");*/
  dest.con->hostaddr = inet_addr("192.168.5.31");
  dest.con->port = 22;

  /*gscp(&src, &dest);*/
  gscp(&dest, &src);

  ssh_connection_free(dest.con);

  libssh2_exit();


    /*unsigned long hostaddr;*/
    /*int sock, i, auth_pw = 1;*/
    /*struct sockaddr_in sin;*/
    /*const char *fingerprint;*/
    /*LIBSSH2_SESSION *session;*/
    /*LIBSSH2_CHANNEL *channel;*/
    /*const char *username="username";*/
    /*const char *password="password";*/
    /*const char *scppath="/tmp/TEST";*/
    /*libssh2_struct_stat fileinfo;*/
    /*int rc;*/
    /*libssh2_struct_stat_size got = 0;*/

/*#ifdef WIN32*/
    /*WSADATA wsadata;*/
    /*int err;*/

    /*err = WSAStartup(MAKEWORD(2,0), &wsadata);*/
    /*if (err != 0) {*/
        /*fprintf(stderr, "WSAStartup failed with error: %d\n", err);*/
        /*return 1;*/
    /*}*/
/*#endif*/

    /*if (argc > 1) {*/
        /*hostaddr = inet_addr(argv[1]);*/
    /*} else {*/
        /*hostaddr = htonl(0x7F000001);*/
    /*}*/
    /*if (argc > 2) {*/
        /*username = argv[2];*/
    /*}*/
    /*if (argc > 3) {*/
        /*password = argv[3];*/
    /*}*/
    /*if (argc > 4) {*/
        /*scppath = argv[4];*/
    /*}*/

    /*rc = libssh2_init (0);*/
    /*if (rc != 0) {*/
        /*fprintf (stderr, "libssh2 initialization failed (%d)\n", rc);*/
        /*return 1;*/
    /*}*/

    /* Ultra basic "connect to port 22 on localhost"
     * Your code is responsible for creating the socket establishing the
     * connection
     */
    /*sock = socket(AF_INET, SOCK_STREAM, 0);*/

    /*sin.sin_family = AF_INET;*/
    /*sin.sin_port = htons(22);*/
    /*sin.sin_addr.s_addr = hostaddr;*/
    /*if (connect(sock, (struct sockaddr*)(&sin),*/
            /*sizeof(struct sockaddr_in)) != 0) {*/
        /*fprintf(stderr, "failed to connect!\n");*/
        /*return -1;*/
    /*}*/

    /* Create a session instance
     */
    /*session = libssh2_session_init();*/
    /*if(!session)*/
        /*return -1;*/

    /* ... start it up. This will trade welcome banners, exchange keys,
     * and setup crypto, compression, and MAC layers
     */
    /*rc = libssh2_session_handshake(session, sock);*/
    /*if(rc) {*/
        /*fprintf(stderr, "Failure establishing SSH session: %d\n", rc);*/
        /*return -1;*/
    /*}*/

    /* At this point we havn't yet authenticated.  The first thing to do
     * is check the hostkey's fingerprint against our known hosts Your app
     * may have it hard coded, may go to a file, may present it to the
     * user, that's your call
     */
    /*fingerprint = libssh2_hostkey_hash(session, LIBSSH2_HOSTKEY_HASH_SHA1);*/
    /*fprintf(stderr, "Fingerprint: ");*/
    /*for(i = 0; i < 20; i++) {*/
        /*fprintf(stderr, "%02X ", (unsigned char)fingerprint[i]);*/
    /*}*/
    /*fprintf(stderr, "\n");*/

    /*if (auth_pw) {*/
        /*[> We could authenticate via password <]*/
        /*if (libssh2_userauth_password(session, username, password)) {*/
            /*fprintf(stderr, "Authentication by password failed.\n");*/
            /*goto shutdown;*/
        /*}*/
    /*} else {*/
        /*[> Or by public key <]*/
        /*if (libssh2_userauth_publickey_fromfile(session, username,*/
                            /*"/home/username/.ssh/id_rsa.pub",*/
                            /*"/home/username/.ssh/id_rsa",*/
                            /*password)) {*/
            /*fprintf(stderr, "\tAuthentication by public key failed\n");*/
            /*goto shutdown;*/
        /*}*/
    /*}*/

    /*[> Request a file via SCP <]*/
    /*channel = libssh2_scp_recv2(session, scppath, &fileinfo);*/

    /*if (!channel) {*/
        /*fprintf(stderr, "Unable to open a session: %d\n",*/
                /*libssh2_session_last_errno(session));*/
        /*goto shutdown;*/
    /*}*/


    /*while(got < fileinfo.st_size) {*/
        /*char mem[1024];*/
        /*int amount=sizeof(mem);*/

        /*if((fileinfo.st_size -got) < amount) {*/
            /*amount = (int)(fileinfo.st_size -got);*/
        /*}*/

        /*rc = libssh2_channel_read(channel, mem, amount);*/
        /*if(rc > 0) {*/
            /*write(1, mem, rc);*/
        /*}*/
        /*else if(rc < 0) {*/
            /*fprintf(stderr, "libssh2_channel_read() failed: %d\n", rc);*/
            /*break;*/
        /*}*/
        /*got += rc;*/
    /*}*/

    /*libssh2_channel_free(channel);*/
    /*channel = NULL;*/

 /*shutdown:*/

    /*libssh2_session_disconnect(session, "Normal Shutdown, Thank you for playing");*/
    /*libssh2_session_free(session);*/

/*#ifdef WIN32*/
    /*closesocket(sock);*/
/*#else*/
    /*close(sock);*/
/*#endif*/
    /*fprintf(stderr, "all done\n");*/

    /*libssh2_exit();*/

    /*return 0;*/
}
