#ifndef TDS
#define TDS

struct TdsData {
  uint16_t values[3];
};

TdsData tdsSensorRead();

#endif
