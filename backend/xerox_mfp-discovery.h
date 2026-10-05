/* Native Samsung SNMP discovery. Licensed under GPL + SANE exception. */
#ifndef XEROX_MFP_DISCOVERY_H
#define XEROX_MFP_DISCOVERY_H

#include "sane/sane.h"

SANE_Status xerox_mfp_discover (const char *interface,
                              SANE_Status (*attach) (SANE_String_Const));

#endif
