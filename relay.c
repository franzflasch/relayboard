/*
    This program is distributed under the GPL, version 2
    Original author: Zwiebel (http://forum.fhem.de/index.php/topic,12142.msg72162.html#msg72162)


    Parameters:

    -l		Shows all connected boards
    -o on|off|tog	Set state to on or off (or toggle)
    -p <port>|all	Select port (or all ports)
    -s <serial>	Device serial (find with -l)
    -v		Verbose output
*/


#include <stdio.h>
#include <unistd.h>
#include <ftdi.h>	/* the FTDI chip */
#include <string.h>	/* string.h um strings zu vergleichen */
#include <unistd.h>     /* for getopt */

/* Forward declarations */
int usage();
int isprint(int c);
int writeserial (struct ftdi_context ftdcont, unsigned char buffer[1], int vflag);


int main(int argc, char **argv)
{
    int c = 0;
    int lflag = 0;
    int vflag = 0;
    int hflag = 0;
    char *ovalue = "0";
    char *pvalue = "all";
    int pvalue_int = -1;
    char *serial = 0;

    struct ftdi_context ftdic;
    int f;
    unsigned char buf[1];
    unsigned char pins[1];

    /* Scan parameters with getopt */
    opterr = 0;
    while ((c = getopt (argc, argv, "hvlo:p:s:")) != -1)
        switch (c)
        {
        case 'h':
            hflag = 1;
            break;
        case 'v':
            vflag = 1;
            break;
        case 'l':
            lflag = 1;
            break;
        case 'o':
            ovalue = optarg;
            break;
        case 'p':
            pvalue = optarg;
            break;
        case 's':
            serial = optarg;
            break;
        case '?':
            if (optopt == 'o' || optopt == 'p' || optopt == 's')
                fprintf (stderr, "Option -%c requires an argument.\n", optopt);
            else if (isprint(optopt))
                fprintf (stderr, "Unknown option `-%c'.\n", optopt);
            else
                fprintf (stderr,"Unknown option character `\\x%x'.\n", optopt);
            return EXIT_FAILURE;
        default: abort ();
        }

    /* Show usage when help flag is activated */
    if (hflag)
    {
        usage();
        return EXIT_FAILURE;

    }


    /* List all devices, then stop the program */
    if (lflag == 1)
    {
        int ret, i;
        struct ftdi_device_list *devlist, *curdev;
        char manufacturer[128], description[128];
        char serial[128];
        ftdi_init(&ftdic);
        if((ret = ftdi_usb_find_all(&ftdic, &devlist, 0x0403, 0x6001)) < 0)
        {
            fprintf(stderr, "ftdi_usb_find_all failed: %d (%s)\n", ret, ftdi_get_error_string(&ftdic));
            return EXIT_FAILURE;
        }
        if(vflag)
            printf("Number of FTDI devices found: %d\n", ret);

        i = 0;
        for (curdev = devlist; curdev != NULL; i++)
        {
            printf("Device %d: ", i);
            if((ret = ftdi_usb_get_strings(&ftdic, curdev->dev, manufacturer, 128, description, 128, serial, 128)) < 0)
            {
                fprintf(stderr, "ftdi_usb_get_strings failed: %d (%s)\n", ret, ftdi_get_error_string(&ftdic));
                return EXIT_FAILURE;
            }
            if(vflag)
                printf("Manufacturer: %s, Description: %s, Serial: %s\n", manufacturer, description, serial);
            else
                printf("ID %s %s %s\n", serial, manufacturer, description);
            curdev = curdev->next;
        }

        ftdi_list_free(&devlist);
        ftdi_deinit(&ftdic);
        return EXIT_SUCCESS;
    }




    /* get current status of FTDI chip */
    if (ftdi_init(&ftdic) < 0)
    {
        fprintf(stderr, "ftdi_init failed\n");
        return EXIT_FAILURE;
    }

    /* TODO: Vendor id  und Product id nicht als Konstanten */
    f = ftdi_usb_open_desc(&ftdic, 0x0403, 0x6001, NULL, serial);
    if (f < 0)
    {
        fprintf(stderr, "unable to open ftdi device: %d (%s)\n", f, ftdi_get_error_string(&ftdic));
        return EXIT_FAILURE;
    }
    if(vflag)
    {
        printf("ftdi open succeeded: %d\n",f);
        printf("enabling bitbang mode\n");
    }

    ftdi_set_bitmode(&ftdic, 0xFF, BITMODE_BITBANG);
    f = ftdi_read_pins(&ftdic, pins);
    if (f < 0)
    {
        fprintf(stderr,"read pins failed, error %d (%s)\n",f, ftdi_get_error_string(&ftdic));
        ftdi_usb_close(&ftdic);
        ftdi_deinit(&ftdic);
        return EXIT_FAILURE;
    }
    else if(vflag)
    {
        printf("pins are: %02hhx\n", pins[0]);
    }


    /* calculate new status for bits */
    if (!strcmp(ovalue,"on")) /* set pins to on */
    {
        if (!strcmp(pvalue,"all"))
        {
            buf[0] = pins[0] | 0b11111111;
        }
        else
        {
            pvalue_int = atoi(pvalue);
            if(pvalue_int >= 8)
            {
                usage();
                ftdi_usb_close(&ftdic);
                ftdi_deinit(&ftdic);
                return EXIT_FAILURE;
            }
            buf[0] = pins[0] | (1 << pvalue_int);
        }
        writeserial(ftdic,buf,vflag);
    }
    if (!strcmp(ovalue,"off")) /* set pins to off */
    {
        if (!strcmp(pvalue,"all"))
        {
            buf[0] = pins[0] & 0b00000000;
        }
        else
        {
            pvalue_int = atoi(pvalue);
            if(pvalue_int >= 8)
            {
                usage();
                ftdi_usb_close(&ftdic);
                ftdi_deinit(&ftdic);
                return EXIT_FAILURE;
            }
            buf[0] = pins[0] & ~(1 << pvalue_int);
        }
        writeserial(ftdic,buf,vflag);
    }
    if(!strcmp(ovalue,"tog")) /* toggle pins if no o-parameter is given */
    {
        if (!strcmp(pvalue,"all"))
        {
            buf[0] = pins[0] ^ 0b11111111;
        }
        else
        {
            pvalue_int = atoi(pvalue);
            if(pvalue_int >= 8)
            {
                usage();
                ftdi_usb_close(&ftdic);
                ftdi_deinit(&ftdic);
                return EXIT_FAILURE;
            }
            buf[0] = pins[0] ^ (1 << pvalue_int);
        }
        writeserial(ftdic,buf,vflag);
    }


    f = ftdi_read_pins(&ftdic, pins);
    if (f < 0)
    {
        fprintf(stderr,"read pins failed, error %d (%s)\n",f, ftdi_get_error_string(&ftdic));
        return EXIT_FAILURE;
    }
    if(vflag){
        printf("pins are: %02hhx\n", pins[0]);
    }

    int i = 0;
    for(i=0;i<8;i++)
    {
        printf("relay[%d]: %s\n", i, (pins[0] & (1<<i)) ? "on" : "off");
    }

    ftdi_usb_close(&ftdic);
    ftdi_deinit(&ftdic);
    if (f < 0)
    {
        return EXIT_FAILURE;
    }
    else
    {
        return EXIT_SUCCESS;
    }
}

int usage ()
{
    fprintf(stderr,
"Usage:\n"
"relay [options]\n\n"
"Options:\n\n"
" -l      list relay boards\n"
" -o      device state to set can be (on) or (off) or (tog) for toggle\n"
" -p      port, can be a number (1-4) or (all)\n"
"         if no option is given, all pins are switched\n"
" -s      serial of relay board, find out with -l\n"
" -v      verbose output\n\n"
"If no options are given, the program only prints the state of the relay.\n\n"
"This program is used to control an FTDI based relayboard.\n"
"I tested it with an Denkovi Ltd. 4-Port USB Relais board.\n"
"I have no idea if other boards work, too.\n");

    return 0;

}

int writeserial (struct ftdi_context ftdcont, unsigned char buffer[1], int vflag)
{
    int f;

    if(vflag)
        printf("pins new are: %02hhx\n", buffer[0]);

    f = ftdi_write_data(&ftdcont, buffer, 1);
    if (f < 0)
    {
        fprintf(stderr,"write failed for 0x%x, error %d (%s)\n",buffer[0],f, ftdi_get_error_string(&ftdcont));
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;

}
