#include <zephyr/kernel.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>
#include <active_object.h>

static void bluetoothTransportAO_Idle(zephyr_ao const *me, const Event evt);
static void bluetoothTransportAO_Scanning(zephyr_ao const *me, const Event evt);
static void bluetoothTransportAO_Connected(zephyr_ao const *me, const Event evt);

#define STACKSIZE 1024
  
K_THREAD_STACK_DEFINE(bluetoothTransportAO_stack, STACKSIZE);

static char my_msgq_buffer[10 * sizeof(Event)];
/* This file will contain an active object and state machine to receive message fragments and reassemble them
   The active object will then post the assembled message to any subscribed active object               */

zephyr_ao bluetoothTransportAO;

static void bluetoothTransportAO_Idle(zephyr_ao const *me, const Event evt)
{
   switch (evt.signal)
   {
   case BUTTON1_PRESSED:
      printk("Bluetooth transport AO Idle..\n");
      //start_scan();
      AO_TRANSITION(bluetoothTransportAO, Scanning);
      // Transition to the SCANNING state
      break;
   default:
      break;
   }
}

static void bluetoothTransportAO_Scanning(zephyr_ao const *me, const Event evt)
{
   switch (evt.signal)
   {
   case BUTTON2_PRESSED:
      printk("Stopping bluetooth low energy scan\n");
      //stop_scan();
      AO_TRANSITION(bluetoothTransportAO, Idle);
      // Transition to the IDLE state
      break;
   case BLE_ADV_PACKET_RECEIVED:
      printk("Got AD Packet from BT Stack:\n");
      //stop_scan();
      //Here we can check if the Service UUID is there, if it is connect.
      AO_TRANSITION(bluetoothTransportAO, Connected);
      // Transition to the IDLE state
      break;
   default:
      break;
   }
}

static void bluetoothTransportAO_Connected(zephyr_ao const *me, const Event evt)
{
   switch (evt.signal)
   {
   case BUTTON3_PRESSED:
      printk("Stopping bluetooth low energy scan\n");
      //stop_scan();
      // Transition to the IDLE state
      break;
   default:
      break;
   }
}

int initialise_and_start_ble_transport_obj(void)
{
   zephyrAO_constructor(&bluetoothTransportAO, &bluetoothTransportAO_Idle);
   zephyrAO_start(&bluetoothTransportAO, my_msgq_buffer, bluetoothTransportAO_stack);
   return 0;
}