/**
 * @file SimpleMoteApp.h
 * @author Marius Grysla
 * @date Fr 23. Juli 10:00:23 CEST 2010
 *
 * @brief This file carries configuration variables and structs needed in the
 *        SimpleMoteApp program.
 *
 **/

#ifndef SIMPLEMOTEAPP_H
#define SIMPLEMOTEAPP_H

#include "AM.h"

/*
 * General definitions.
 */
enum{
    MAX_MOTES = 16
};

/* 
 * AM types for communications.
 * Values are quite arbitrary right now.
 */
enum {
    AM_SIMPLE_RADIO = 15,
    AM_SIMPLE_SERIAL = 0
};

/*
 * Queue sizes
 */
enum {
    RADIO_QUEUE_SIZE = 10,
    SERIAL_QUEUE_SIZE = 10,
    PACKET_QUEUE_SIZE = 16
};

typedef nx_uint8_t nx_seq_no_t;
typedef nx_uint8_t nx_boolean;

typedef uint8_t seq_no_t;
typedef uint8_t boolean;

// WARNING must be the same as the one in structs.h
typedef nx_struct myPacketHeader {
    nx_am_addr_t sender;
    nx_am_addr_t destination;
    nx_seq_no_t seq_no;
    nx_uint8_t ord_no;
    // tells if the payload is compressed or not
    nx_boolean is_compressed;
    // how many chunks in total
    nx_uint8_t parts;
} myPacketHeader;


/** Routing part **/
/*
 * General definitions.
 */
enum {
  BEACON_TIMER_MILLI = 5120,
  MAX_NUM_RECORDS = 10,
  MAX_TIMEOUT = 15
};

/* 
 * Types of messages for the routing module
 */
enum {
  AM_BEACON = 0x10,
  AM_ROUTING_UPDATE = 0x11,
  AM_IP = 0x12
};

typedef nx_struct routing_record {
    nx_uint8_t node_id;
    nx_uint8_t metric;		
} routing_record_t;

typedef nx_struct routing_table {
    nx_uint8_t node_id;
    nx_am_addr_t node_addr;
    nx_uint8_t metric;
    nx_am_addr_t nexthop;
    nx_uint8_t timeout;
} routing_table_t;

typedef nx_struct routing_update {
    nx_uint8_t node_id;
    nx_uint8_t num_of_records;
    routing_record_t records[MAX_NUM_RECORDS];
} routing_update_t; 

typedef nx_struct beacons {
    nx_uint8_t node_id;
} beacons_t; 


typedef nx_struct ack {

} ack_t;

#endif