#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <linux/input.h>
#include <fcntl.h>
#include <unistd.h>

#define CTRL_REG1 0x20
#define CTRL_REG2 0x21
#define I2C_DEV "/dev/i2c-1"  // 실제 I2C 장치 경로로 변경 필요

/* 뮤텍스 */
static pthread_mutex_t pressure_lock;
static pthread_mutex_t temperature_lock;

static int is_run = 1;

int pressure_fd, temperature_fd;

void setLSM9DS1LED(int led_num, int on_off) {
    // 실제 LED 제어 구현 필요
    printf("LED %d %s\n", led_num, on_off ? "ON" : "OFF");
}

void wiringPiI2CWriteReg8(int fd, int reg, int value) {
    // 실제 I2C 레지스터 쓰기 구현 필요
}

void getPressure(int fd, double *temp, double *pressure) {
    *temp = 25.0;
    *pressure = 1013.25;
}

void getTemperature(int fd, double *temperature, double *humidity) {
    *temperature = 24.5;
    *humidity = 60.0;
}

void delay(int ms) {
    usleep(ms * 1000);
}

/* 압력 센서 스레드 */
void *pressureFunction(void *arg) {
    double t_c = 0.0, pressure = 0.0;

    while (is_run) {
        if (pthread_mutex_trylock(&pressure_lock) != EBUSY) {
            wiringPiI2CWriteReg8(pressure_fd, CTRL_REG1, 0x00);
            wiringPiI2CWriteReg8(pressure_fd, CTRL_REG1, 0x84);
            wiringPiI2CWriteReg8(pressure_fd, CTRL_REG2, 0x01);

            getPressure(pressure_fd, &t_c, &pressure);
            printf("Temperature(from LPS25H) = %.2f°C\n", t_c);
            printf("Pressure = %.0f hPa\n", pressure);

            pthread_mutex_unlock(&pressure_lock);
        }
        delay(1000);
    }
    return NULL;
}

/* 온습도 센서 스레드 */
void *temperatureFunction(void *arg) {
    double temperature, humidity;

    while (is_run) {
        if (pthread_mutex_trylock(&temperature_lock) != EBUSY) {
            wiringPiI2CWriteReg8(temperature_fd, CTRL_REG1, 0x00);
            wiringPiI2CWriteReg8(temperature_fd, CTRL_REG1, 0x84);
            wiringPiI2CWriteReg8(temperature_fd, CTRL_REG2, 0x01);

            getTemperature(temperature_fd, &temperature, &humidity);
            printf("Temperature(from HTS221) = %.2f°C\n", temperature);
            printf("Humidity = %.0f%% rH\n", humidity);

            pthread_mutex_unlock(&temperature_lock);
        }
        delay(1000);
    }
    return NULL;
}

/* 조이스틱 스레드 (LED 연동 포함) */
void *joystickFunction(void *arg) {
    int fd;
    struct input_event ie;
    const char *joy_dev = "/dev/input/event10";  // 실제 장치 경로로 변경

    if ((fd = open(joy_dev, O_RDONLY)) == -1) {
        perror("opening device");
        return NULL;
    }

    while (is_run) {
        if (read(fd, &ie, sizeof(struct input_event)) > 0) {
            if (ie.type == EV_KEY && ie.value == 1) {  // 키 눌림
                switch (ie.code) {
                    case KEY_UP:
                        printf("Up\n");
                        setLSM9DS1LED(1, 1);
                        break;
                    case KEY_DOWN:
                        printf("Down\n");
                        setLSM9DS1LED(2, 1);
                        break;
                    case KEY_LEFT:
                        printf("Left\n");
                        setLSM9DS1LED(3, 1);
                        break;
                    case KEY_RIGHT:
                        printf("Right\n");
                        setLSM9DS1LED(4, 1);
                        break;
                    case KEY_ENTER:
                        printf("Enter\n");
                        is_run = 0;
                        break;
                    default:
                        printf("Unknown key code: %d\n", ie.code);
                        break;
                }
            }
        }
        delay(100);
    }

    close(fd);
    return NULL;
}

/* 메인 함수 */
int main(int argc, char **argv) {
    pthread_t ptPressure, ptTemperature, ptJoystick;

    pthread_mutex_init(&pressure_lock, NULL);
    pthread_mutex_init(&temperature_lock, NULL);

    if ((pressure_fd = open(I2C_DEV, O_RDWR)) < 0) {
        perror("Failed to open pressure sensor device");
        return -1;
    }
    if ((temperature_fd = open(I2C_DEV, O_RDWR)) < 0) {
        perror("Failed to open temperature sensor device");
        close(pressure_fd);
        return -1;
    }

    pthread_create(&ptPressure, NULL, pressureFunction, NULL);
    pthread_create(&ptTemperature, NULL, temperatureFunction, NULL);
    pthread_create(&ptJoystick, NULL, joystickFunction, NULL);

    printf("q : Quit\n");
    while (is_run) {
        delay(100);
    }

    pthread_join(ptPressure, NULL);
    pthread_join(ptTemperature, NULL);
    pthread_join(ptJoystick, NULL);

    wiringPiI2CWriteReg8(pressure_fd, CTRL_REG1, 0x00);
    close(pressure_fd);

    wiringPiI2CWriteReg8(temperature_fd, CTRL_REG1, 0x00);
    close(temperature_fd);

    pthread_mutex_destroy(&pressure_lock);
    pthread_mutex_destroy(&temperature_lock);

    printf("Good Bye!\n");
    return 0;
}

