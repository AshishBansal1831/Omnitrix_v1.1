#ifndef _COMMAND_HANDLER_H_

#ifdef __cplusplus
extern "C" {
#endif
#define CMD_BUFFER_SIZE 550

uint16_t make_packet(uint8_t *payload, uint16_t length);
uint8_t check_cmd(const uint8_t Recv);
#ifdef __cplusplus
}
#endif
#endif /* _COMMAND_HANDLER_H_ */