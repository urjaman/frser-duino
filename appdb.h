struct command_t {
	PGM_P name;
	void(*function)(void);
};

extern const struct command_t appdb[] PROGMEM;

void *find_appdb(unsigned char* cmd);
void echo_cmd(void);
void help_cmd(void);
void flash_readsect_cmd(void);
void flash_idchip_cmd(void);
void calc_cmd(void);

