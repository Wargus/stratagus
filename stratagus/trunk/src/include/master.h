#ifndef __MASTER_H__
#define __MASTER_H__

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--	Defines
----------------------------------------------------------------------------*/

#define MASTER_HOST "freecraft.dyndns.org"
#define MASTER_PORT 8123

/*----------------------------------------------------------------------------
--	Declarations
----------------------------------------------------------------------------*/

extern char MasterTempString[50];
extern int PublicMasterAnnounce;
extern unsigned long LastTimeAnnounced;
extern int master_port;
extern char *master_host;

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

extern int MasterInit(void);
extern int MasterSend(const void *buf, int len);
extern void MasterLoop(unsigned long ticks);
extern void MasterSendInfo(void);
extern void MasterStopAnnounced(void);
extern void MasterSendAnnounce(void);
extern void MasterProcessGetServerData(const char* msg, size_t length, unsigned long host, int port);

//@}

#endif	// !__MASTER_H__
