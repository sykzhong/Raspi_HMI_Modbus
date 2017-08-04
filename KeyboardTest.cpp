#include<stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define TTY_PATH            "/dev/tty"
#define STTY_US             "stty raw -echo -F "
#define STTY_DEF            "stty -raw echo -F "

static int get_char();

static int get_char()
{
    fd_set rfds;
    struct timeval tv;
    int ch = 0;

    FD_ZERO(&rfds);
    FD_SET(0, &rfds);
    tv.tv_sec = 1;
    tv.tv_usec = 0; //设置等待超时时间

    //检测键盘是否有输入

    printf("sykdebug: enter get_char()\n\r");
    while(1)
    {
        // printf("sykdebug: enter loop\n");
        if (select(1, &rfds, NULL, NULL, &tv) > 0)
        {
            printf("sykdebug:get in\n");
            ch = getchar(); 
            if(ch)
            {
                printf("key = %d(%c)\n\r", ch, ch);
                switch (ch)
                {
                    case 3 ://ctrl+c
                        {system(STTY_DEF TTY_PATH);return 0;}
                    case '0':
                        printf("0\n\r");break;
                    case '1':
                        printf("1\n\r");break;
                    case '2':
                        printf("2\n\r");break;
                    case '3':
                        printf("3\n\r");break;
                }
            }
        }
    }
    return ch;
}

int main()
{
    int ch = 0;
    system(STTY_US TTY_PATH);

    // ch = get_char();

    fd_set rfds;
    struct timeval tv;

    FD_ZERO(&rfds);
    FD_SET(0, &rfds);
    tv.tv_sec = 5;
    tv.tv_usec = 0; //设置等待超时时间

    if(select(1, &rfds, NULL, NULL, &tv) > 0)
    {
        ch = getchar();
        printf("ch = %c\n\r", ch);
    }

    // ch = select(1, &rfds, NULL, NULL, &tv);
    // if (ch == -1)  
    //   perror("select()");  
    // else if (ch)  
    //   printf("Data is available now.\n");  
    // /* FD_ISSET(0, &rfds) will be true. */  
    // else  
    //   printf("No data within five seconds.\n");  

    printf("sykdebug: main end\n\r");
    system(STTY_DEF TTY_PATH);
    return 0;
    // while(1)
    // {
    //     ch = get_char();
    //     if (ch)
    //     {
    //         printf("key = %d(%c)\n\r", ch, ch);
    //         switch (ch)
    //         {
    //             case 3 ://ctrl+c
    //                 {system(STTY_DEF TTY_PATH);return 0;}
    //             case '0':
    //                 printf("0\n\r");break;
    //             case '1':
    //                 printf("1\n\r");break;
    //             case '2':
    //                 printf("2\n\r");break;
    //             case '3':
    //                 printf("3\n\r");break;
    //         }
    //     }           
    // }
}