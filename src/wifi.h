void wifi_init(void);
void setINA219_1(float voltage, float current, float wattage);
void setINA219_2(float voltage, float current, float wattage);
int counter;
static const char *TAG;
struct INA219;
void wifi_init_sta(void);
