/*
 * halAudioClock.h
 *
 *  Created on: Jun 28, 2010
 *      Author: klabadi & dlopo
 */
#ifndef HAL_AUDIO_CLOCK_H_
#define HAL_AUDIO_CLOCK_H_

#include "types.h"
/*
 * @param value: new N value
 */
void halAudioClock_N(IM_UINT16 baseAddress, IM_UINT32 value);
/*
 * @param value: new CTS value or set to zero for automatic mode
 */
void halAudioClock_Cts(IM_UINT16 baseAddress, IM_UINT32 value);

void halAudioClock_F(IM_UINT16 baseAddress, IM_UINT8 value);

#endif /* HAL_AUDIO_CLOCK_H_ */
