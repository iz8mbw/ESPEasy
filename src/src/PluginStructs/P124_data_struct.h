#ifndef PLUGINSTRUCTS_P124_DATA_STRUCT_H
#define PLUGINSTRUCTS_P124_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P124

# include <multi_channel_relay.h>

// # define P124_DEBUG_LOG // Enable for some (extra) logging

# define P124_CONFIG_RELAY_COUNT  PCONFIG(0)
# define P124_CONFIG_FLAGS        PCONFIG_LONG(0)

# define P124_FLAGS_INIT_RELAYS    8 // 0..7 hold the on/off state of each relay

struct P124_data_struct : public PluginTaskData_base {
public:

  P124_data_struct(uint8_t relayCount);

  P124_data_struct() = delete;
  ~P124_data_struct();

  bool isInitialized() {
    return relay != nullptr;
  }

  uint8_t getChannelState();
  uint8_t getFirmwareVersion();
  bool    channelCtrl(uint8_t state);
  bool    turn_on_channel(uint8_t channel);
  bool    turn_off_channel(uint8_t channel);

private:

  bool validChannel(uint channel) {
    return channel > 0 && channel <= _relayCount;
  }

  Multi_Channel_Relay *relay = nullptr;

  uint8_t _relayCount;
};
#endif // ifdef USES_P124
#endif // ifndef PLUGINSTRUCTS_P124_DATA_STRUCT_H
