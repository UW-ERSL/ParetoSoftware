// This program fetches the MAC address of the localhost by fetching the 
// information through GetAdapatersInfo.  It does not rely on the NETBIOS
// protocol and the ethernet adapter need not be connect to a network.
//
// Supported in Windows NT/2000/XP
// Supported in Windows 95/98/Me
//
// Supports multiple NIC cards on a PC.


#include <stdio.h>
#include <time.h>
#include <string.h>
#include "MacAddresses.txt"
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <Windows.h>
#include <Iphlpapi.h>
#pragma comment(lib, "iphlpapi.lib")
#pragma warning(disable:4996)
#endif

char thisComputerMacAddresses[12][14];


/**********************************************************************/
void getDateMonthYear(int dateMonthYear[3])
{
	time_t t = time(0);   // get time now
    struct tm * now = localtime( & t );
	dateMonthYear[2] = (now->tm_year + 1900);
	dateMonthYear[1] = (now->tm_mon + 1);
	dateMonthYear[0] =  now->tm_mday;
	
}
#ifdef _WIN32
/**********************************************************************/
 bool GetMACaddress(char str[])
{
	IP_ADAPTER_INFO AdapterInfo[16];			// Allocate information for up to 16 NICs
	DWORD dwBufLen = sizeof(AdapterInfo);		// Save the memory size of buffer

	DWORD dwStatus = GetAdaptersInfo(			// Call GetAdapterInfo
		AdapterInfo,							// [out] buffer to receive data
		&dwBufLen);								// [in] size of receive data buffer
	if(dwStatus != ERROR_SUCCESS)
		return false;// Verify return value is valid, no buffer overflow

	PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo;// Contains pointer to current adapter info
	bool found = false;
	while (pAdapterInfo){
		for (int i =0; i < 6; i++) {
			sprintf(&(str[2*i]),"%02X",pAdapterInfo->Address[i]);
			if (strncmp(&(str[2*i]),"00",2) !=0) found = true;
		}
		if (found) return true;
		pAdapterInfo = pAdapterInfo->Next;
	}
	if (found) return true;
	return false;
}
 /**********************************************************************/
 int GetAllMACaddresses(char str[12][14])
{
	IP_ADAPTER_INFO AdapterInfo[16];			// Allocate information for up to 16 NICs
	DWORD dwBufLen = sizeof(AdapterInfo);		// Save the memory size of buffer

	DWORD dwStatus = GetAdaptersInfo(			// Call GetAdapterInfo
		AdapterInfo,							// [out] buffer to receive data
		&dwBufLen);								// [in] size of receive data buffer
	if(dwStatus != ERROR_SUCCESS)
		return false;// Verify return value is valid, no buffer overflow

	PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo;// Contains pointer to current adapter info
	bool found = false;
	int nAddresses = 0;
	while (pAdapterInfo){
		found = false;
		for (int i =0; i < 6; i++) {
			sprintf(&(str[nAddresses][2*i]),"%02X",pAdapterInfo->Address[i]);
			if (strncmp(&(str[nAddresses][2*i]),"00",2) !=0) found = true;
		}
		if (found) nAddresses++;
		if (nAddresses == 12) break;//don't exceed memory
		pAdapterInfo = pAdapterInfo->Next;
	}
	return nAddresses;
}

/**********************************************************************/
bool checkUserMacAddress(void)
{
	int compareSerial;
	int nMacAddresses;
	int expiryDateMonthYear[3],currentDateMonthYear[3];
	getDateMonthYear(currentDateMonthYear);

	nMacAddresses = GetAllMACaddresses(thisComputerMacAddresses);
	for (int i = 0; i < nMacAddresses; i++) {
		for (int j = 0; j < NUM_PARETO_USERS; j++) {
			compareSerial = strncmp(thisComputerMacAddresses[i],allowedMacAddresses[j],12);
			sscanf(&(allowedMacAddresses[j][13]),"%d %d %d \n",&(expiryDateMonthYear[0]),&(expiryDateMonthYear[1]),&(expiryDateMonthYear[2]));// expiry date
			if (compareSerial == 0) {
				if (currentDateMonthYear[2] < expiryDateMonthYear[2])
					compareSerial = 0;// OK
				else if (currentDateMonthYear[2] > expiryDateMonthYear[2])
					compareSerial = 1;// failed
				else { // year matches, check month
					if (currentDateMonthYear[1] < expiryDateMonthYear[1])
						compareSerial = 0;// OK
					else if (currentDateMonthYear[1] > expiryDateMonthYear[1])
						compareSerial = 1;// failed
					else {// year and month match, check date
						if (currentDateMonthYear[0] < expiryDateMonthYear[0]){
							compareSerial = 0;// OK
							printf("Your Pareto license will expiry in %d days \n",expiryDateMonthYear[0]-currentDateMonthYear[0]);
						}
						else if  (currentDateMonthYear[0] > expiryDateMonthYear[0])
							compareSerial = 1;// failed
					}
				}	
				if (compareSerial == 1)
					printf("Your Pareto license has expired\n");
			}
			if (compareSerial == 0) return true;
		}
	}
	printf("Invalid Permanent License: %s \n",thisComputerMacAddresses[0]);

	return false;
}
#endif
/**********************************************************************/
bool checkTemporaryLicense(char temporaryLicense[])
{
	int compareSerial;
	int expiryDateMonthYear[3],currentDateMonthYear[3];
	getDateMonthYear(currentDateMonthYear);

	for (int j = 0; j < NUM_TEMP_PARETO_LICENSES; j++) {
		compareSerial = strncmp(temporaryLicense,allowedTempParetoLicenses[j],12);
		sscanf(&(allowedTempParetoLicenses[j][13]),"%d %d %d \n",&(expiryDateMonthYear[0]),&(expiryDateMonthYear[1]),&(expiryDateMonthYear[2]));// expiry date
		if (compareSerial == 0) {
			if (currentDateMonthYear[2] < expiryDateMonthYear[2])
				compareSerial = 0;// OK
			else if (currentDateMonthYear[2] > expiryDateMonthYear[2])
				compareSerial = 1;// failed
			else { // year matches, check month
				if (currentDateMonthYear[1] < expiryDateMonthYear[1])
					compareSerial = 0;// OK
				else if (currentDateMonthYear[1] > expiryDateMonthYear[1])
					compareSerial = 1;// failed
				else {// year and month match, check date
					if (currentDateMonthYear[0] < expiryDateMonthYear[0]){
						compareSerial = 0;// OK
						printf("Your Pareto license will expiry in %d days \n",expiryDateMonthYear[0]-currentDateMonthYear[0]);
					}
					else if  (currentDateMonthYear[0] > expiryDateMonthYear[0])
						compareSerial = 1;// failed
				}
			}	
			if (compareSerial == 1)
				printf("Your Pareto license has expired\n");
		}
		if (compareSerial == 0) return true;
	}

	printf("Invalid Temporary License: %s \n",temporaryLicense);

	return false;
}
