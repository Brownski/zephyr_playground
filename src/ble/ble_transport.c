#include <zephyr/kernel.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>
#include <active_object.h>

#define STACKSIZE 1024
K_THREAD_STACK_DEFINE(buttonAO_stack, STACKSIZE);

static char my_msgq_buffer[10 * sizeof(Event)];
static struct k_msgq my_msgq;
/* This file will contain an active object and state machine to receive message fragments and reassemble them
   The active object will then post the assembled message to any subscribed active object               */

zephyr_ao bluetoothTransportAO;

static void bluetoothTransportIdle_handler(zephyr_ao const *me, const uint8_t signal)
{
   switch (signal)
   {
   case BUTTON_PRESSED:
      printk("Starting bluetooth low energy scan\n");
      start_scan();
      // Transition to the SCANNING state
      break;
   default:
      break;
   }
}

static void bluetoothTransportScanning_handler(zephyr_ao const *me, const uint8_t signal)
{
   switch (signal)
   {
   case BUTTON_PRESSED:
      printk("Stopping bluetooth low energy scan\n");
      stop_scan();
      // Transition to the IDLE state
      break;
   default:
      break;
   }
}

int initialise_and_start_ble_transport_obj(void)
{
   zephyrAO_constructor(&bluetoothTransportAO, bluetoothTransportIdle_handler);
   zephyrAO_start(&bluetoothTransportAO, my_msgq_buffer, );
}