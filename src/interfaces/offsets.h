/*
 * offsets.h
 *
 *  Created on: 18.12.2013
 *      Author: lutz
 */

#ifndef OFFSETS_H_
#define OFFSETS_H_


#include "motorIDs.h"

/**
 * apply offsets on given values for motorIDs
 */

/*
 * add ofsets (to motors)
 */
bool addOffsets(const motorID_t *ids, uint8_t cnt, uint16_t **values);

/*
 * remove offsets (from motors)
 */
bool removeOffsets(const motorID_t *ids, uint8_t cnt, uint16_t **values);


#endif /* OFFSETS_H_ */
