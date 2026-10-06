#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "TimeParser.h"

// time format: HHMMSS (6 characters)
int time_parse(char *time) 
{
	if (time == NULL)
	{
		return TIME_ARRAY_ERROR;
	}
	if (strlen(time) != 6)
	{
		return TIME_LEN_ERROR;
	}
	for (int i = 0; i < 6; i++)
    {
        if (!isdigit(time[i]))
        {
            return TIME_VALUE_ERROR;
        }
    }

	int values[3];
	values[2] = atoi(time+4); // seconds
	time[4] = 0;
	values[1] = atoi(time+2); // minutes
	time[2] = 0;
	values[0] = atoi(time); // hours

	// how many seconds, default returns error

	// TODO: Check that string is not null

	
	// Parse values from time string
	// For example: 124033 -> 12hour 40min 33sec

	if (values[0] < 0 || values[0] > 23)
	{
		return TIME_VALUE_ERROR;
	}
	if (values[1] < 0 || values[1] > 59)
	{
		return TIME_VALUE_ERROR;
	}
	if (values[2] < 0 || values[2] > 59)
	{
		return TIME_VALUE_ERROR;
	}
	
	int seconds = ((values[0] * 3600) + values[1] * 60) + values[2];
	return seconds;
}


/*
#define TIME_LEN_ERROR      -1
#define TIME_ARRAY_ERROR    -2
#define TIME_VALUE_ERROR    -3
*/

// Parse values from time string
// For example: 124033 -> 12hour 40min 33sec
// Now you have:
	// values[0] hour
	// values[1] minute
	// values[2] second
	// TODO: Add more features to get more points
	// TODO: Add boundary check time values: below zero or above limit not allowed
	// limits are 59 for minutes, 23 for hours, etc

	// TODO: Calculate return value from the parsed minutes and seconds
	// Otherwise error will be returned!
	// seconds = ...