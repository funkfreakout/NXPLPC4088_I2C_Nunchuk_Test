#ifndef SD_LPC4088_MCI_H
#define SD_LPC4088_MCI_H

#define MCI_CLOCK_ENABLE            (1<<8)
#define MCI_CLOCK_PWRSAVE           (1<<9)
#define MCI_CLOCK_BYPASS            (1<<10)
#define MCI_CLOCK_WIDEBUS           (1<<11)

#define MCI_COMMAND_RESPONSE        (1<<6)
#define MCI_COMMAND_LONGRESP        (1<<7)
#define MCI_COMMAND_INTERRUPT       (1<<8)
#define MCI_COMMAND_PENDING         (1<<9)
#define MCI_COMMAND_ENABLE          (1<<10)

#define MCI_STATUS_CMDCRCFAIL       (1<<0)
#define MCI_STATUS_CMDTIMEOUT       (1<<2)
#define MCI_STATUS_CMDRESPEND       (1<<6)
#define MCI_STATUS_CMDSENT          (1<<7)
#define MCI_STATUS_DATABLOCKEND     (1<<10)
#define MCI_STATUS_CMDACTIVE        (1<<11)
#define MCI_STATUS_TXFIFOHALFEMPTY  (1<<14)
#define MCI_STATUS_RXFIFOHALFFULL   (1<<15)

#define NO_RESPONSE 0
#define SHORT_RESPONSE 1
#define LONG_RESPONSE 2

#define GO_IDLE_STATE 0
#define APP_CMD 55
#define SD_SEND_OP_COND 41
#define ALL_SEND_CID 2
#define SEND_RELATIVE_ADDR 3
#define SEND_STATUS 13
#define SELECT_DESELECT_CARD 7
#define SET_BUS_WIDTH 6
#define SEND_IF_COND 8
 
#define	CMDREAD             17
#define	CMDREADMULTIPLE		18
#define	CMDWRITE            24
#define	CMDWRITEMULTIPLE    24
#define	CMDREADCSD          9
#define CMD55               55
#define ACMD23              23
#define CMDSTOPTRANSMISION  12

#define MCI_PRESCALE_MIN  1

#define OCR_VOLTAGE_WINDOW 0x00FF8000

#define OCR_HCS (1<<30)

unsigned int sd_init(void);
unsigned int sd_status(void);
unsigned int sd_read(unsigned char *buff, int sector, int count);
unsigned int sd_write(const unsigned char *buff, int sector, int count);
unsigned int sd_ioctl(int ctrl, void *buff);
unsigned int sd_getfattime(void);

void mci_set_speed(int speed);

unsigned int sd_command(unsigned int cmd, unsigned int resp_type, unsigned int arg, unsigned int* resp);

//#define SD_LPC4088_MCI_DMA 1
#define SD_LPC4088_MCI_DMA 0

#endif  /* SD_LPC4088_MCI_H */
