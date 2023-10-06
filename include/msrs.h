#ifndef MSRS_H
#define MSRS_H

const unsigned int SYSENTER_CS_MSR = 0x174;
const unsigned int SYSENTER_ESP_MSR = 0x175;
const unsigned int SYSENTER_EIP_MSR = 0x176;

void writeMSR(unsigned int msr, unsigned long value);

#endif
