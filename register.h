#include <cstdint>

class Register {
public:
  Register(uint16_t initial): value(initial){}

  uint16_t getFull();
  uint8_t getHigh();
  uint8_t getLow();

  Register setHigh(uint8_t byte);
  Register setLow(uint8_t byte);
  Register setFull(uint16_t word);
  Register setFull(Register r);

  Register operator++(int);

  Register incHigh();
  Register incLow();

private:
  uint16_t value;
};
