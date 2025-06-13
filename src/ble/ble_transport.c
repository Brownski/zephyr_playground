#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>

/* This file will contain an active object and state machine to receive message fragments and reassemble them
   The active object will then post the assembled message to any subscribed active object               */