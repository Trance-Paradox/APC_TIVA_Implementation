#define PART_TM4C1294NCPDT

#include <stdint.h>
#include <stdbool.h>

#ifndef _DAC124S085_H_
#define _DAC124S085_H_

//*****************************************************************************
// The following are defines for different modes of the DAC.
//*****************************************************************************
#define DAC_ALL_LOW 0b0011000000000000    // Minimun output for all channels
#define DAC_POWER_DOWN 0b1111000000000000 // Power down all channels of DAC

/// @brief Provides initial constant values of the DAC.
/// @param ssi_base specifies the SSI module base address used to interface the DAC module.
#define DACInitValue(ssi_base)        \
    {                                 \
        .SSI.base = ssi_base,         \
        .channel[0].bits.addr = 0b00, \
        .channel[1].bits.addr = 0b01, \
        .channel[2].bits.addr = 0b10, \
        .channel[3].bits.addr = 0b11  \
    }

struct DAC124S085
{
    struct SSI
    {
        const uint32_t base; // SSI base address.
    } const SSI;

    union DAC_U
    {
        // Access individual sections.
        struct DacChannelData
        {
            uint16_t value : 12;     // DAC output value.
            uint16_t mode : 2;       // DAC update mode.
            const uint16_t addr : 2; // Register address bits.
        } bits;

        // Access total register.
        uint16_t total;
    } channel[4];
};

//*****************************************************************************
//! @brief Updates DAC struct with default register values and initializes
//! the SSI interface.
//!
//! @param dac specifies a pointer to the struct of DAC module.
//! @param ssi_base specifies the SSI module base address used to interface
//! the DAC module.
//!
//! @return None.
//*****************************************************************************
// void DACInitialize(struct DAC124S085 *dac, uint32_t ssi_base);

//*****************************************************************************
//! @brief Initializes and enables the SSI interface.
//!
//! @param dac specifies a pointer to the struct of DAC module.
//!
//! @return None.
//*****************************************************************************
void DACEnable(struct DAC124S085 *dac);

//*****************************************************************************
//! @brief Powers down the DAC module and disables the SSI inteface.
//!
//! @param dac specifies a pointer to the struct of DAC module.
//!
//! @return None.
//*****************************************************************************
void DACDisable(struct DAC124S085 *dac);

//*****************************************************************************
//! @brief Sets the value of given channel of the DAC but does not updates the
//! output the channel.
//!
//! @param dac specifies a pointer to the struct of DAC module.
//! @param channel_no specifies the channel of the DAC module.
//! @param uint12_val is the data to be put in given channel of the DAC.
//!
//! @def Sets given channel value of the DAC with the supplied data and
//! transmits the full data to the DAC module over SSI interface. This function
//! does not update the output of channels in the DAC module. It is a
//! blocking function and return only when the tranmission is over.
//!
//! @note Only lower 12 bits are taken as value of given channel of the DAC
//! to be updated. The upper 4 bits are discarded.
//!
//! @return None.
//*****************************************************************************
void DACSetChannel(struct DAC124S085 *dac, int channel_no, uint16_t uint12_val);

//*****************************************************************************
//! @brief Sets the value of given channel of the DAC and update the outputs
//! of all the channels.
//!
//! @param dac specifies a pointer to the struct of DAC module.
//! @param channel_no specifies the channel of the DAC module.
//! @param uint12_val is the data to be put in given channel of the DAC.
//!
//! @def Sets given channel value of the DAC with the supplied data and
//! transmits the full data to the DAC module over SSI interface. This function
//! updates the output for all the previously set values for all channels. It
//! is a blocking function and return only when the tranmission is over.
//!
//! @note Only lower 12 bits are taken as value of given channel of the DAC
//! to be updated. The upper 4 bits are discarded.
//!
//! @return None.
//*****************************************************************************
void DACUpdateChannel(struct DAC124S085 *dac, int channel_no, uint16_t uint12_val);

//*****************************************************************************
//! @brief Sets the value for all channels of the DAC and update the outputs
//! of all the channels.
//!
//! @param dac specifies a pointer to the struct of DAC module.
//! @param uint12_val is the data to be put in given channel of the DAC.
//!
//! @def Sets all channel value of the DAC with the supplied data and
//! transmits the full data to the DAC module over SSI interface. This function
//! updates the output of all channels in the DAC module. It is a blocking
//! function and return only when the tranmission is over.
//!
//! @note Only lower 12 bits are taken as value of given channel of the DAC
//! to be updated. The upper 4 bits are discarded.
//!
//! @return None.
//*****************************************************************************
void DACUpdateAll(struct DAC124S085 *dac, uint16_t uint12_val);

#endif