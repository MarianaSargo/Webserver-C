#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <arpa/inet.h>
#define BUFSIZE 8096


struct {
char *ext;
char *filetype;
} extensions [] = {
{"jpg", "image/jpeg"},
{"png", "image/png" },
{"html","text/html" },
{"js","text/js" },
{"css","test/css" },
{0,0} };


void error(const char *msg) {
perror(msg);
exit(1);
}

cost char page_404[] = "HTTP/1.1 200 OK\n"
                        "Date: Mon, 27 Jul 2009 12:28:53 GMT\n"
                        "Server: LUSIADA\n"
                        "Content-Length: 88\n"
                        "Content-Type: text/html\n"
                        "Connection: Closed\n"
                        "\n"
                        "\n"
                        "<html>\n"
                        "<body>\n"
                        "<h1>Hello, World!</h1>\n"
                        "</body>\n"
                        "</html>";



void web(int fd)
{
int j, file_fd, buflen, len;
long i, ret;
char * fstr;
static char buffer[BUFSIZE+1];

ret =read(fd,buffer,BUFSIZE);
if(ret == 0 || ret == -1) {
printf("failed to read browser request\n");
}
if(ret > 0 && ret < BUFSIZE)
buffer[ret]=0;
else buffer[0]=0;


for(i=0;i<ret;i++)
if(buffer[i] == '\r' || buffer[i] == '\n')
buffer[i]='*';
printf("request\n");

if( strncmp(buffer,"GET ",4) && strncmp(buffer,"get ",4) )
printf("Only simple GET operation supported\n");

for(i=4;i<BUFSIZE;i++) {
if(buffer[i] == ' ') {
buffer[i] = 0;
break;
}
}

for(j=0;j<i-1;j++)
if(buffer[j] == '.' && buffer[j+1] == '.')
printf("Parent directory (..) path names not supported\n");

if( !strncmp(&buffer[0],"GET /\0",6) || !strncmp(&buffer[0],"get /\0",6) )
(void)strcpy(buffer,"GET /home.html");

buflen=strlen(buffer);
fstr = (char *)0;
for(i=0;extensions[i].ext != 0;i++) {
len = strlen(extensions[i].ext);
if( !strncmp(&buffer[buflen-len], extensions[i].ext, len)) {
fstr =extensions[i].filetype;
break;
}
}
if(fstr == 0) printf("file extension type not supported\n");

printf("SEND\n");

if(( file_fd = open(&buffer[5])) == -1)
printf("failed to open file");


(void)sprintf(buffer,"HTTP/1.0 200 OK\r\nContent-Type: %s\r\n\r\n", fstr);
(void)write(fd,buffer,strlen(buffer));

while ( (ret = read(file_fd, buffer, BUFSIZE)) > 0 ) {
(void)write(fd,buffer,ret);
}

}

int main(int argc, char *argv[])
{
int sockfd;
int newsockfd;
int portno;
int pid;
int i;

socklen_t clilen;

char buffer[2048];

struct sockaddr_in serv_addr;
struct sockaddr_in cli_addr;

if( argc < 3 || argc > 3 || !strcmp(argv[1], "-?") ) {
(void)printf("usage: server [port] [server directory] &"
"\tExample: server 80 ./ &\n\n"
"\tOnly Supports:");
for(i=0;extensions[i].ext != 0;i++)
(void)printf(" %s",extensions[i].ext);

(void)printf("\n\tNot Supported: directories / /etc /bin /lib /tmp /usr /dev /sbin \n"
);
exit(0);
}
if( !strncmp(argv[2],"/" ,2 ) || !strncmp(argv[2],"/etc", 5 ) ||
!strncmp(argv[2],"/bin",5 ) || !strncmp(argv[2],"/lib", 5 ) ||
!strncmp(argv[2],"/tmp",5 ) || !strncmp(argv[2],"/usr", 5 ) ||
!strncmp(argv[2],"/dev",5 ) || !strncmp(argv[2],"/sbin",6) ){
(void)printf("ERROR: Bad top directory %s, see server -?\n",argv[2]);
exit(3);
}
if(chdir(argv[2]) == -1){
(void)printf("ERROR: Can't Change to directory %s\n",argv[2]);
exit(4);
}

ssize_t n;
if (argc < 2){
fprintf(stderr, "Error");
exit(1);
}

sockfd = socket(AF_INET, SOCK_STREAM, 0);
if (sockfd < 0) {
error("ERROR opening socket");
}

bzero((char *) &serv_addr, sizeof(serv_addr)); // Empty an char-array
char *arg_end;
portno = (int)strtol(argv[1], &arg_end, 10);

// Prepare the serv_address struct and bind the socket
serv_addr.sin_family = AF_INET;
serv_addr.sin_addr.s_addr = INADDR_ANY;
serv_addr.sin_port = htons(portno);

if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
error("ERROR on binding");
}
int flag = 1;

if (-1 == setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag))) {

printf("setsockopt fail");
return 0;



}
// We got bind, let's listen with a backlog of 5 connections
if (listen(sockfd, 5) < 0) {
error("ERROR in listening");
}

// The accept call below is blocking (waits for a connection)
clilen = sizeof(cli_addr);

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
while (1)
{
newsockfd = accept(sockfd,(struct sockaddr *) &cli_addr, &clilen);
if (newsockfd < 0)
{
error("ERROR on accept");
}
pid = fork();
if (pid < 0)
{
error("ERROR on fork");
}
if (pid == 0) // pai
{
close(newsockfd);
printf("pid_%d: conection accepted\n", pid);
continue;
}
else // filhos
{
web(newsockfd);
/*char *reply = "HTTP/1.1 200 OK\n"
"Date: Mon, 27 Jul 2009 12:28:53 GMT\n"
"Server: LUSIADA\n"
"Content-Length: 88\n"
"Content-Type: text/html\n"
"Connection: Closed\n"
"\n"
"\n"
"<html>\n"
"<body>\n"
"<h1>Hello, World!</h1>\n"
"</body>\n"
"</html>";



bzero(buffer, 2048); // Empty the buffer
n = read(newsockfd, buffer, 2047);



// Verify connection
if (n < 0)
{
error("ERROR reading from socket");
}
printf("pid_%d: ip: %s\n", pid, inet_ntoa(cli_addr.sin_addr));
printf("pid_%d: message: %s\n", pid, buffer);
send(newsockfd, reply, strlen(reply), 0);
close(newsockfd);
*/
}
}


#pragma clang diagnostic pop
// Clean up some stuff
close(newsockfd);
close(sockfd);
close(sockfd);
return 0;
}