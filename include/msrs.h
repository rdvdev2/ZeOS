#ifndef MSRS_H
#define MSRS_H

#define SYSENTER_CS_MSR 0x174
#define SYSENTER_ESP_MSR 0x175
#define SYSENTER_EIP_MSR 0x176

void writeMSR(unsigned int msr, unsigned long long value);

#endif
