#!/bin/bash

HOSTNAME="$(uname -srm | sed 's/\\/\\\\\\\\/g' | sed 's/"/\\\\"/g')"
DATE="$(date --rfc-3339 seconds)"

cat "$0" | sed '1,/<''<<START>>>/d' | sed -e "s/___SCRIPTFILE___/$0/g" -e "s/___DATE___/$DATE/g" -e "s/___HOSTNAME___/$HOSTNAME/g" > hostname.h

exit 0
<<<START>>>
/*******************************************************************************
 *    This file was autogenerated by ___SCRIPTFILE___ on ___DATE___.
 *    Do not modify this file!
 *******************************************************************************/
#ifndef _HOSTNAME_H
#define _HOSTNAME_H

#ifndef HOSTNAME
#define HOSTNAME "___HOSTNAME___"
#endif

#endif