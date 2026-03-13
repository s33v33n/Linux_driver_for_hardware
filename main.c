#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
 
#define MAX_BUFFER 1024
#define SYSFS_FILE_DSCZMI_IN            "/sys/kernel/my_sysfs/dsczmi"     
#define SYSFS_FILE_DTCZMI_CTRL          "/sys/kernel/my_sysfs/dtczmi"
#define SYSFS_FILE_DCCZMI_STATE         "/sys/kernel/my_sysfs/dcczmi"
#define SYSFS_FILE_DRCZMI_RESULT        "/sys/kernel/my_sysfs/drczmi"
//#define SYSFS_FILE_TEST                 "/sys/kernel/my_sysfs/test"
 

 
int main(void)
{
    char buffer[MAX_BUFFER];

    int fd_in = open(SYSFS_FILE_DSCZMI_IN, O_WRONLY); // otwarcie pliku dsczmi (IN) sterownika
    if (fd_in < 0) {
        printf("Open %s - error: %d\n", SYSFS_FILE_DSCZMI_IN, errno);
        exit(1);
    }

    int fd_ctrl = open(SYSFS_FILE_DTCZMI_CTRL, O_WRONLY); // otwarcie pliku dtczmi (CTRL) sterownika
    if (fd_ctrl < 0) {
        printf("Open %s - error: %d\n", SYSFS_FILE_DTCZMI_CTRL, errno);
        close(fd_in);
        exit(1);
    }

    int fd_state = open(SYSFS_FILE_DCCZMI_STATE, O_RDONLY); // otwarcie pliku dcczmi (STATE) sterownika 
    if (fd_state < 0) {
        printf("Open %s - error: %d\n", SYSFS_FILE_DCCZMI_STATE, errno);
        close(fd_in);
        close(fd_ctrl);
        exit(1);
    }

    int fd_result = open(SYSFS_FILE_DRCZMI_RESULT, O_RDONLY); // otwarcie pliku drczmi (RESULT) sterownika 
    if (fd_result < 0) {
        printf("Open %s - error: %d\n", SYSFS_FILE_DRCZMI_RESULT, errno);
        close(fd_in);
        close(fd_ctrl);
        close(fd_state);
        exit(1);
    }

    /*int fd_test = open(SYSFS_FILE_TEST, O_RDWR); // otwarcie pliku test (TEST) sterownika 
    if (fd_result < 0) {
        printf("Open %s - error: %d\n", SYSFS_FILE_TEST, errno);
        close(fd_in);
        close(fd_ctrl);
        close(fd_state);
        close(fd_result);
        exit(1);
    }*/
 
    int i, n;
    char option;
    
    while (1) {
        printf("\n1. Data input (IN)\n");
        printf("2. Count CRC (CTRL)\n");
        printf("3. Read actual module state (STATE)\n");
        printf("4. Read CRC (RESULT)\n");
        printf("5. App exit\n");
        //printf("0. Test (TEST)\n");
        printf("Choice: ");
        scanf("%d", &option);
        getchar(); // usuniecie '\n'

        switch (option) {
           case 1: 
                printf("data 8-bit in HEX: ");      // IN
                if (scanf("%x", &i) != 1) {
                    printf("Wrong format.\n");
                    while (getchar() != '\n'); // czyszczenie bufora 
                    break;
                }
                getchar(); // usuniecie ostatniego znaku z bufora 

                snprintf(buffer, MAX_BUFFER, "%#x", i); 
                lseek(fd_in, 0L, SEEK_SET);
                n = write(fd_in, buffer, strlen(buffer));
                if (n != strlen(buffer)) {
                    perror("Can't write to file");
                } 
                break;

            case 2: 
                printf("Enter commad to calculate CRC (only 1 available (0 or 0x0)): ");  // CTRL
                if (scanf("%x", &i) != 1) {
                    printf("Wrong format.\n");
                    while (getchar() != '\n'); 
                    break;
                }
                getchar();  

                snprintf(buffer, MAX_BUFFER, "%#x", i);         //treść do przekazania sterownikowi
                lseek(fd_ctrl, 0L, SEEK_SET);                   //przesunięcie się na początek pliku
                n = write(fd_ctrl, buffer, strlen(buffer));     //właściwe przekazanie danych
                if (n != strlen(buffer)) {
                    perror("Can't write to file");
                } 
                break;

            case 3:                                             // STATE
                lseek(fd_state, 0L, SEEK_SET);                  //przesunięcie się na początek pliku                      
                n = read(fd_state, buffer, MAX_BUFFER);         //odczyt danych ze sterownika
                if (n > 0) {
                    buffer[n] = '\0';  
                    printf("Module state: %s\n", buffer);
                } else {
                    perror("Error");
                }
                break;

            case 4:                                     // RESULT
                lseek(fd_result, 0L, SEEK_SET);                        
                n = read(fd_result, buffer, MAX_BUFFER);
                if (n > 0) {
                    buffer[n] = '\0';  
                    printf("CRC16CDMA2000: %s\n", buffer);
                } else {
                    perror("Error");
                }
                break;

            case 5:
                printf("App is closing...\n");
                close(fd_in);
                close(fd_ctrl);
                close(fd_state);
                close(fd_result);
                //close(fd_test);
                return 0;

            
            /*case 0: // Test
                lseek(fd_test, 0L, SEEK_SET);  
                int n = read(fd_test, buffer, MAX_BUFFER);
                if (n > 0) {
                    buffer[n] = '\0';  
                    printf("Wynik: %s\n", buffer);
                } else {
                    perror("Error");
                }
                break;
                break;*/

            default:
                printf("Forbidden option\n");
        }
    }
    
    return 0;
}

