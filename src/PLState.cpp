#include "PLState.h"

const char* plStateToString(PLConnectionState state) {
  switch (state) {
    case PL_STATE_BOOTING:          return "BOOTING";
    case PL_STATE_WIFI_CONNECTING:  return "WIFI_CONNECTING";
    case PL_STATE_LOCAL_CONNECTING: return "LOCAL_CONNECTING";
    case PL_STATE_LOCAL_CONNECTED:  return "LOCAL_CONNECTED";
    case PL_STATE_CLOUD_CONNECTING: return "CLOUD_CONNECTING";
    case PL_STATE_CLOUD_CONNECTED:  return "CLOUD_CONNECTED";
    case PL_STATE_RECONNECTING:     return "RECONNECTING";
    case PL_STATE_OFFLINE_IDLE:     return "OFFLINE_IDLE";
    case PL_STATE_ERROR:            return "ERROR";
    default:                        return "UNKNOWN";
  }
}
