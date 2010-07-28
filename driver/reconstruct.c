/**
 * Module that reconstructs the splitted data given in input.
 * We use a global circular array to store every possible "conversation".
 * This array contains the chunks we're temporary building with
 * some additional informations.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "reconstruct.h"
#include "chunker.h"
#include "tunnel.h"
#include "compress.h"
#include "../shared/structs.h"

#define POS(x) (x % MAX_RECONSTRUCTABLE)

typedef long unsigned bitmask_t;

// FIXME: some types are not big enough handling big numbers
typedef struct {
    int seq_no;
    // bitmaks of chunks still missing
    bitmask_t missing_bitmask;
    // That is the max size of the theoretically completed packet
    stream_t chunks[MAX_FRAME_SIZE];
    int tot_size;
} packet_t;

// Statistic variables
unsigned long started_pkts = 0;
unsigned long finished_pkts = 0;

// just using a send function would be fine
//static void (*send_back)(ipv6Packet *completed);
static packet_t temp_packets[MAX_RECONSTRUCTABLE];
static void (*send_back)(payload_t completed);

/** 
 * @param seq_no sequential number to look for
 * 
 * @return NULL if not found, the pointer if found
 *         It can only returns null if that seq_no has been already overwritten
 */
packet_t *get_packet(int seq_no) {
    packet_t *found = &temp_packets[POS(seq_no)];
    if (found->seq_no == seq_no) {
        return found;
    }
    return NULL;
}

void fake_reconstruct_done(payload_t complete) {
    (void) complete;
    printf("packet completed, not doing anything\n");
}

int is_completed(packet_t *pkt) {
    return (pkt->missing_bitmask == 0);
}

int check_if_same_chunk(packet_t *pkt1, packet_t *pkt2, int size) {
    return memcmp((void *) pkt1, (void *) pkt2, size);
}

// TODO: change name or change what is done inside here
void send_if_completed(packet_t *pkt) {
    // now we check if everything if the packet is completed and sends it back
    if (is_completed(pkt)) {
        if (DEBUG)
            printf("packet seqno=%d completed, tot_size=%d\n", pkt->seq_no, pkt->tot_size);
        if(DEBUG)
            finished_pkts++;
        
        payload_t payload = {
            .len = pkt->tot_size,
            .stream = pkt->chunks
        };

#if COMPRESSION_ENABLED
        stream_t compr_data[MAX_FRAME_SIZE];
        payload_t compressed = {
            .len = MAX_FRAME_SIZE,
            .stream = compr_data
        };
    
        // we'll overwrite it when done
        payloadDecompress(payload, &compressed);
        // TODO: is the rest of the memory lost maybe?
        // should we alloc - memcpy - free instead?
        copyPayload(&compressed, &payload);
#endif

        if (send_back) 
            send_back(payload);
        else 
            printf("WARNING: no callback function registered for completed chunks.\n");
    }
}


/** 
 * initializing a temporary reconstruction packet
 * resetting also the memory
 * 
 * @param pkt 
 */
void init_temp_packet(packet_t* const pkt) {
  *pkt = (packet_t){
    .seq_no = -1,
    .missing_bitmask = -1ul,
    .tot_size = 0
  };

  memset((void*)(pkt->chunks), 0, MAX_FRAME_SIZE * sizeof(stream_t));
}

/** 
 * Initializing the reconstruction module, setting to a default value
 * the values of the packet
 * 
 * @param callback callback function which will be called when the packet is completed
 */
void initReconstruction(void (*callback)(payload_t completed)) {
    if (DEBUG)
        printf("initializing the reconstruction\n");

    send_back = callback;
    if (!callback) {
        send_back = fake_reconstruct_done;
        printf("WARNING: installing useless callback for completed packets.\n");
    }

    for (int i = 0; i < MAX_RECONSTRUCTABLE; i++) {
        // using a [] would be better?
        init_temp_packet(temp_packets + i);
    }
}

/** 
 * Add a new chunk to the list of temp
 * 
 * @param data 
 */
void addChunk(payload_t data) {
    // TODO: add another check of the length of the data given in
    assert(data.len <= sizeof(ipv6Packet));
    assert(data.len >= sizeof(struct ipv6PacketHeader));

    ipv6Packet *original = malloc(sizeof(ipv6Packet));
    memcpy(original, data.stream, sizeof(ipv6Packet));

    int seq_no = getSeqNo(original);
    int ord_no = getOrdNo(original);
    
    // just for readability
    packet_t *pkt = &temp_packets[POS(seq_no)];
    
    if (pkt->seq_no != seq_no) {
        if (DEBUG)
            printf("overwriting or creating new packet at position %d\n", POS(seq_no));
        
        if (DEBUG)
            started_pkts++;

        // resetting to the initial configuration
        pkt->missing_bitmask = (1ul << getParts(original)) - 1;
        pkt->seq_no = seq_no;
        pkt->tot_size = 0;
    }

    if (DEBUG)
        printf("adding chunk (seq_no: %d, ord_no: %d, parts: %d, missing bitmask: %lu)\n", seq_no, ord_no, getParts(original), pkt->missing_bitmask);

    // getting the real data of the packets
    int size = getSize(original, data.len);
    pkt->tot_size += size;

    memcpy(pkt->chunks + (MAX_CARRIED * ord_no), original->payload, size);
    
    // remove the arrived packet from the bitmask
    unsigned new_bm = (pkt->missing_bitmask) & ~(1ul << ord_no);
    if (new_bm == pkt->missing_bitmask) {
        printf("adding twice the same chunk!!!!\n");
        // the other one is at position POS(seq_no)
        if (check_if_same_chunk(pkt, &temp_packets[POS(seq_no)], sizeof(packet_t))) {
        // check if really the same one
            printf("REALLY THE SAME CHUNK!!!!\n");
        }
        
    } else  {
        // don't really need to even check for completion if it's a duplicate chunk
        pkt->missing_bitmask = new_bm;
        send_if_completed(pkt);
    }
    free(original);
}

stream_t *getChunks(int seq_no) {
    packet_t *pkt = get_packet(seq_no);
    if (pkt)
        return pkt->chunks;

    return NULL;
}

/** 
 * Prints some statistical information about how many packets were completed.
 * 
 */
void printStatistics(void){
    printf("%lu packets were recognized and %lu were really sent (%f%%).\n", 
           started_pkts, finished_pkts, ((finished_pkts*100.0)/started_pkts));
}
