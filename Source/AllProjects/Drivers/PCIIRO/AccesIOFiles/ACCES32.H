// ACCES32.h : Header file for ACCES32.dll
//
//For Microsoft Visual C++, use VCACCES.lib
//For Borland C++ Builder, use CBACCES.lib

#ifdef __cplusplus
extern "C" {
#endif

//8-bit
__declspec(dllimport) unsigned short InPortB(unsigned long Port);
__declspec(dllimport) unsigned short OutPortB(unsigned long Port, unsigned char Value);
//16-bit
__declspec(dllimport) unsigned short InPort(unsigned long Port);
__declspec(dllimport) unsigned short OutPort(unsigned long Port, unsigned short Value);
//32-bit
__declspec(dllimport) unsigned long InPortL(unsigned long Port);
__declspec(dllimport) unsigned short OutPortL(unsigned long Port, unsigned long Value);
__declspec(dllimport) unsigned long InPortDWord(unsigned long Port);
__declspec(dllimport) unsigned short OutPortDWord(unsigned long Port, unsigned long Value);

#ifdef __cplusplus
}
#endif

struct TPCI_COMMON_CONFIG
{
    unsigned short VendorID;
    unsigned short DeviceID;
    unsigned short Command;
    unsigned short Status;
    unsigned char RevisionID;
    unsigned char ProgIf;
    unsigned char SubClass;
    unsigned char BaseClass;
    unsigned char CacheLineSize;
    unsigned char LatencyTimer;
    unsigned char HeaderType;
    unsigned char BIST;
    unsigned long BaseAddresses[6];
    unsigned long Reserved1[2];
    unsigned long RomBaseAddress;
    unsigned long Reserved2[2];
    unsigned char InterruptLine;
    unsigned char InterruptPin;
    unsigned char MinimumGrant;
    unsigned char MaximumLatency;
};