#include "register.h"

uint16_t Register::getFull(){
  return value;
}

uint8_t Register::getHigh(){
  return (value >> 8);
}

uint8_t Register::getLow(){
  return value;
}

Register Register::setHigh(uint8_t byte){
  value = (byte << 8) | (value & 0x00FF);
  return *this;
}

Register Register::setLow(uint8_t byte){
  value = byte | (value & 0xFF00);
  return *this;
}

Register Register::setFull(uint16_t word){
  value = word;
  return *this;
}

Register Register::setFull(Register r){
  value = r.value;
  return *this;
}

Register Register::operator++(int){
  value++;
  return *this;
}

Register Register::incHigh(){
  value += 0x0100;
  return *this;
}

Register Register::incLow(){
  this->setLow(value+1);
  return *this;
}
