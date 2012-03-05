/*
 * KernelPacketizer.h
 *
 *  Created on: 2 Mar 2012
 *      Author: v1smoll
 */

#ifndef KERNELPACKETIZER_H_
#define KERNELPACKETIZER_H_


typedef unsigned int uint;

/*
 * performs in place packetization with the given smdWidth on the module located at fileName
 */
void packetizeAllKernelsInModule(const char * fileName, uint smdWidth);


#endif /* KERNELPACKETIZER_H_ */
