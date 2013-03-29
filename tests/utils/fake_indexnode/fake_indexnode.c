/* TODO: Listen on the indexnode port and respond to some of the common
 * indexnode commands, like /browse and /download. This will probably involve a
 * re-write in node.js or something
 */

#include <assert.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>


static const char fixed1[] = "fs2protocol-0.13:1337:-479049884424373567";
static const char fixed2[] = "fs2protocol-0.11:4114:-55378008";
static const char autop[] = "fs2protocol-0.13:autoindexnode:920926720:-479049884424373567";


int main( int argc, char **argv )
{
    int s = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
    struct sockaddr_in addr;


    assert(s != -1);

    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(42444);

    // Can't broadcast without root?
    // Can't use INADDR_LOOPBACK without root? is it a "network" address e.g.
    // 127.0.0.0 ?
    errno = 0;
    if (inet_aton("127.0.0.1", &addr.sin_addr)==0)
    {
        perror("inet_aton");
    }

    while( 1 )
    {
        errno = 0;
        if( sendto( s, fixed1, strlen(fixed1), 0, (struct sockaddr *)&addr, sizeof(addr) ) != strlen( fixed1 ) )
        {
            perror("sendto");
        }

        errno = 0;
        if( sendto( s, fixed2, strlen(fixed2), 0, (struct sockaddr *)&addr, sizeof(addr) ) != strlen( fixed2 ) )
        {
            perror("sendto");
        }

        errno = 0;
        if( sendto( s, autop, strlen(autop), 0, (struct sockaddr *)&addr, sizeof(addr) ) != strlen( autop ) )
        {
            perror("sendto");
        }

        printf("sent\n");
        sleep( 1 );
    }
}
