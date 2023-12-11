/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

 /* Zpracovali: Vojtěch Gerö, Tereza Vrbová
 * Deska: DISCO-F746NG
 */

#include "mbed.h"
#include "lcd.h"
#include <stack>

#define SECOND 500ms
#define NRCARS 6

Semaphore semaphore_gate(4);
Mutex get_places_mutex, display_lock;
stack<int> free_park_places;
uint32_t colors[] = {LCD_COLOR_BLUE, LCD_COLOR_WHITE, LCD_COLOR_RED, LCD_COLOR_GREEN, LCD_COLOR_YELLOW, LCD_COLOR_ORANGE, LCD_COLOR_GRAY};
Thread cars[NRCARS];

typedef struct {
    std::uint32_t color;
    int park_time;
    int drive_time;
} PARAMS_CAR;

PARAMS_CAR params_car[NRCARS];

void wait_for_semaphore() {
    semaphore_gate.acquire();
}

void release_semaphore() {
    semaphore_gate.release();
}

int get_place() {
    get_places_mutex.lock();
    int parking_place = free_park_places.top();
    free_park_places.pop();
    get_places_mutex.unlock();
    return parking_place;
}

void park_for(int times, int place, uint32_t color) {
    display_lock.lock();
    color_segment(place, color);
    display_lock.unlock();
    ThisThread::sleep_for(times * SECOND);
}

void drive_for(int times) {
    ThisThread::sleep_for(times * SECOND);
}

void free_parking_place(int park_place) {
    display_lock.lock();
    color_segment(park_place, LCD_COLOR_BLACK);
    display_lock.unlock();

    get_places_mutex.lock();
    free_park_places.push(park_place);
    get_places_mutex.unlock();
}

void init() {
    init_display();
    free_park_places.push(0);
    free_park_places.push(1);
    free_park_places.push(2);
    free_park_places.push(3);
}

void set_auto(PARAMS_CAR *param) {
    PARAMS_CAR p = *param;
    while (true) {
        wait_for_semaphore();
        int place = get_place();
        park_for(p.park_time, place, p.color);
        free_parking_place(place);
        release_semaphore();
        drive_for(p.drive_time);
    }
}

int main()
{
    init();

    for (int i=0; i < NRCARS; i++) {
        params_car[i].color = colors[i];
        params_car[i].park_time = 1 + i;
        params_car[i].drive_time = 1 + i;
        cars[i].start(callback([=] {set_auto(&params_car[i]);}));
    }
    
    while(1) {

    }

    return 0;
}
