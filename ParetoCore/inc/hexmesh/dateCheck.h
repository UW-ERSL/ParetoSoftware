#ifndef DATECHECK_H
#define DATECHECK_H

#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#define DAY (15) // 1-31
#define MONTH (1) // 0-11
#define YEAR (2013)

bool dateValid(int expiryDay,int expiryMonth, int expiryYear ) {
	// Licensing date
	tm licenseDate;
	licenseDate.tm_sec = 0;
	licenseDate.tm_min = 0;
	licenseDate.tm_hour = 0;
	licenseDate.tm_mday = expiryDay; // 1-31
	licenseDate.tm_mon = expiryMonth; // 0-11
	licenseDate.tm_year = expiryYear - 1900; // Years since 1900.
	licenseDate.tm_isdst = 0;
	if (difftime(mktime(&licenseDate), time(NULL)) < 0.0) 
		return false;
	else
		return true;
};
 

#endif // #ifndef DATECHECK_H
