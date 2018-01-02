/**
 * Copyright © 2017 epsilonRT. All rights reserved.
 *
 * This software is governed by the CeCILL license under French law and
 * abiding by the rules of distribution of free software.  You can  use,
 * modify and/ or redistribute the software under the terms of the CeCILL
 * license as circulated by CEA, CNRS and INRIA at the following URL
 * <http://www.cecill.info>.
 *
 * The fact that you are presently reading this means that you have had
 * knowledge of the CeCILL license and that you accept its terms.
 *
 * @file
 * @brief
 */
/* ========================================================================== */
#include "ad7124.h"

using namespace Ad7124;

// -----------------------------------------------------------------------------
//
//                            Ad7124Chip class
//
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
Ad7124Chip::Ad7124Chip () {

}

// -----------------------------------------------------------------------------
Ad7124Chip::~Ad7124Chip() {

}

// -----------------------------------------------------------------------------
int
Ad7124Chip::init (int slave_select) {
  int ret;

  Ad7124Register::fillAllRegsWithDefault (reg);
  ret = d.init (slave_select, reg);
  if (ret < 0) {
    return ret;
  }

  return setAdcControl (StandbyMode, LowPower, false);
}

// -----------------------------------------------------------------------------
int
Ad7124Chip::reset() {

  return d.reset();
}

// -----------------------------------------------------------------------------
int
Ad7124Chip::status() {

  return getRegister (AD7124_Status);
}

// -----------------------------------------------------------------------------
int
Ad7124Chip::currentChannel() {
  int ret = status();

  if (ret < 0) {

    return ret;
  }
  return (uint8_t) (ret & AD7124_STATUS_REG_CH_ACTIVE (15));
}

// -----------------------------------------------------------------------------
int
Ad7124Chip::setChannel (uint8_t ch, uint8_t cfg, InputSel ainp,
                        InputSel ainm, bool enable) {

  if ( (ch < 16) && (cfg < 8)) {
    Ad7124Register * r;

    ch += AD7124_Channel_0;
    r = &reg[ch];

    r->value = AD7124_CH_MAP_REG_SETUP (cfg) |
               AD7124_CH_MAP_REG_AINP (ainp) |
               AD7124_CH_MAP_REG_AINM (ainm) |
               (enable ? AD7124_CH_MAP_REG_CH_ENABLE : 0);

    return writeRegister ( (RegisterId) ch);
  }
  return -1;
}

// -----------------------------------------------------------------------------
int
Ad7124Chip::enableChannel (uint8_t ch, bool enable) {

  if (ch < 16) {
    Ad7124Register * r;
    int ret;

    ch += AD7124_Channel_0;
    r = &reg[ch];

    ret = readRegister ( (RegisterId) ch);
    if (ret < 0) {
      return ret;
    }

    if (enable) {

      r->value |= AD7124_CH_MAP_REG_CH_ENABLE;

    }
    else {

      r->value &= ~AD7124_CH_MAP_REG_CH_ENABLE;
    }

    return writeRegister ( (RegisterId) ch);
  }
  return -1;
}

// -----------------------------------------------------------------------------
int
Ad7124Chip::channelConfig (uint8_t ch) {

  if (ch < 16) {
    Ad7124Register * r;
    uint8_t cfg;

    ch += AD7124_Channel_0;
    r = &reg[ch];

    cfg = (r->value >> 12) & 0x07;
    return cfg;
  }
  return -1;
}

// -----------------------------------------------------------------------------
int
Ad7124Chip::setConfig (uint8_t cfg, RefSel ref, PgaSel pga,
                       bool bipolar, BurnoutCurrent burnout) {

  if (cfg < 8) {
    Ad7124Register * r;

    cfg += AD7124_Config_0;
    r = &reg[cfg];

    r->value =    AD7124_CFG_REG_REF_SEL (ref) |
                  AD7124_CFG_REG_PGA (pga) |
                  (bipolar ? AD7124_CFG_REG_BIPOLAR : 0) |
                  AD7124_CFG_REG_BURNOUT (burnout) |
                  AD7124_CFG_REG_REF_BUFP | AD7124_CFG_REG_REF_BUFM |
                  AD7124_CFG_REG_AIN_BUFP | AD7124_CFG_REG_AINN_BUFM;
    return writeRegister ( (RegisterId) cfg);
  }
  return -1;
}

// -----------------------------------------------------------------------------
int
Ad7124Chip::setConfigFilter (uint8_t cfg, FilterType filter, PostFilterType postfilter, uint16_t fs, bool rej60, bool single) {

  if (cfg < 8) {
    Ad7124Register * r;

    cfg += AD7124_Filter_0;
    r = &reg[cfg];

    r->value = AD7124_FILT_REG_FILTER ( (uint32_t) filter) |
               AD7124_FILT_REG_POST_FILTER ( (uint32_t) postfilter) |
               AD7124_FILT_REG_FS (fs)    |
               (rej60 ? AD7124_FILT_REG_REJ60 : 0) |
               (single ? AD7124_FILT_REG_SINGLE_CYCLE : 0);
    return writeRegister ( (RegisterId) cfg);
  }
  return -1;
}

// -----------------------------------------------------------------------------
int
Ad7124Chip::setConfigOffset (uint8_t cfg, uint32_t value) {

  if (cfg < 8) {

    cfg += AD7124_Offset_0;
    return setRegister ( (RegisterId) cfg, value);
  }
  return -1;
}

// -----------------------------------------------------------------------------
int
Ad7124Chip::setConfigGain (uint8_t cfg, uint32_t value) {

  if (cfg < 8) {

    cfg += AD7124_Gain_0;
    return setRegister ( (RegisterId) cfg, value);
  }
  return -1;
}

// -----------------------------------------------------------------------------
int
Ad7124Chip::setAdcControl (OperatingMode mode,
                           PowerMode power_mode,
                           bool ref_en, ClkSel clk_sel) {
  Ad7124Register * r;

  r = &reg[AD7124_ADC_Control];
  r->value = AD7124_ADC_CTRL_REG_MODE (mode) |
             AD7124_ADC_CTRL_REG_POWER_MODE (power_mode) |
             AD7124_ADC_CTRL_REG_CLK_SEL (clk_sel) |
             (ref_en ? AD7124_ADC_CTRL_REG_REF_EN : 0) |
             AD7124_ADC_CTRL_REG_DOUT_RDY_DEL;

  return writeRegister (AD7124_ADC_Control);
}

// -----------------------------------------------------------------------------
int
Ad7124Chip::setMode (OperatingMode mode) {
  Ad7124Register * r;

  r = &reg[AD7124_ADC_Control];
  r->value &= ~AD7124_ADC_CTRL_REG_MODE (0x0F); // clear mode
  r->value |= AD7124_ADC_CTRL_REG_MODE (mode);
  return writeRegister (AD7124_ADC_Control);
}

// -----------------------------------------------------------------------------
int
Ad7124Chip::waitEndOfConversion (uint32_t timeout_ms) {

  return d.waitForConvReady (timeout_ms);
}

// -----------------------------------------------------------------------------
long
Ad7124Chip::getData() {
  int32_t value;
  int ret;

  ret = d.readData (&value);
  if (ret < 0) {
    return ret;
  }
  return (long) value;
}

// -----------------------------------------------------------------------------
int
Ad7124Chip::startSingleConversion (uint8_t ch) {

  if (ch < 16) {
    int ret;

    ret = enableChannel (ch, true);
    if (ret < 0) {
      return ret;
    }
    return setMode (SingleConvMode);
  }
  return -1;
}

// -----------------------------------------------------------------------------
long
Ad7124Chip::read (uint8_t ch) {
  int ret;
  uint8_t cur_ch = currentChannel();

  if (ch != cur_ch) {

    // disable previous channel if different
    ret = enableChannel (cur_ch, false);
    if (ret < 0) {
      return ret;
    }
  }

  ret = startSingleConversion (ch);
  if (ret < 0) {

    return ret;
  }

  ret = waitEndOfConversion (timeout());
  if (ret < 0) {
    return ret;
  }

  return getData();
}

// -----------------------------------------------------------------------------
int
Ad7124Chip::internalCalibration (uint8_t ch) {
  int ret;
  uint8_t cfg;

  ret = setMode (StandbyMode);
  if (ret < 0) {
    return ret;
  }

  for (uint8_t c = 0; c < 16; c++) {

    // disable all channels
    ret = enableChannel (c, false);
    if (ret < 0) {
      return ret;
    }
  }

  ret = channelConfig (ch);
  if (ret < 0) {

    return ret;
  }
  cfg = (uint8_t) ret;
  ret = setConfigOffset (cfg, 0x800000);

  ret = enableChannel (ch, true);
  if (ret < 0) {
    return ret;
  }
  
  ret = setMode(InternalGainCalibrationMode);
  ret = waitEndOfConversion (timeout());
  if (ret < 0) {
    return ret;
  }
  
  ret = setMode(InternalOffsetCalibrationMode);
  ret = waitEndOfConversion (timeout());
  if (ret < 0) {
    return ret;
  }

  return enableChannel (ch, false);
}

// -----------------------------------------------------------------------------
long
Ad7124Chip::getRegister (RegisterId id) {

  if ( (id >= AD7124_Status) && (id < AD7124_REG_NO))  {
    int ret;

    ret = readRegister (id);
    if (ret < 0) {

      return ret;
    }
    return reg[id].value;
  }
  return -1;
}

// -----------------------------------------------------------------------------
int
Ad7124Chip::setRegister (RegisterId id, long value) {

  if ( (id >= AD7124_Status) && (id < AD7124_REG_NO))  {

    reg[id].value = value;
    return writeRegister (id);
  }
  return -1;
}

// -----------------------------------------------------------------------------
double
Ad7124Chip::toVoltage (long value, int gain, double vref, bool bipolar) {
  double voltage = (double) value;

  if (bipolar) {

    voltage = voltage / (double) 0x7FFFFFUL - 1;
  }
  else {

    voltage = voltage / (double) 0xFFFFFFUL;
  }

  voltage = voltage * vref / (double) gain;
  return voltage;
}

// -----------------------------------------------------------------------------
//                         >---- Protected ----<
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
int
Ad7124Chip::readRegister (RegisterId id) {

  return d.readRegister (&reg[id]);
}

// -----------------------------------------------------------------------------
int
Ad7124Chip::writeRegister (RegisterId id) {

  return d.writeRegister (reg[id]);
}

/* ========================================================================== */