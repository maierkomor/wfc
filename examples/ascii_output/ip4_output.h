#ifndef IP4_OUTPUT_H
#define IP4_OUTPUT_H

int parse_ip4(uint32_t *, const char *);
void uint32_to_ip4(std::ostream &, uint32_t);
void degC_to_ascii(std::ostream &o, float f);
void degC_to_json(std::ostream &o, float f);

#endif
