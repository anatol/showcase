struct showcase_page {
    unsigned int wait_time;
    const char *url;
};

struct showcase_config {
    unsigned int wait_time;
    unsigned int pages_num;
    struct showcase_page *pages;
};

void config_parse(void);
void config_free(void);

extern struct showcase_config config;