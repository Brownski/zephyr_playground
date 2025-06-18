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
static void start_scan(void);
static void device_found(const bt_addr_le_t *addr, int8_t rssi, uint8_t type,
                         struct net_buf_simple *ad);
void mtu_updated(struct bt_conn *conn, uint16_t tx, uint16_t rx);

#define STACKSIZE 1024

K_THREAD_STACK_DEFINE(bluetoothTransportAO_stack, STACKSIZE);

static char my_msgq_buffer[10 * sizeof(Event)];
/* This file will contain an active object and state machine to receive message fragments and reassemble them
   The active object will then post the assembled message to any subscribed active object               */

zephyr_ao bluetoothTransportAO;

static struct bt_gatt_cb gatt_callbacks = {
    .att_mtu_updated = mtu_updated};

static void start_scan(void)
{
   int err;

   err = bt_le_scan_start(BT_LE_SCAN_ACTIVE, device_found);
   if (err)
   {
      printk("%s: Scanning failed to start (err %d)\n", __func__,
             err);
      return;
   }

   printk("%s: Scanning successfully started\n", __func__);
}

static void bluetoothTransportAO_Idle(zephyr_ao const *me, const Event evt)
{
   switch (evt.signal)
   {
   case BUTTON1_PRESSED:
      printk("Bluetooth transport AO Idle..\n");
      start_scan();
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
      // stop_scan();
      AO_TRANSITION(bluetoothTransportAO, Idle);
      // Transition to the IDLE state
      break;
   case BLE_ADV_PACKET_RECEIVED:
      printk("Got AD Packet from BT Stack:\n");
      // stop_scan();
      // Here we can check if the Service UUID is there, if it is connect.
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
      // stop_scan();
      //  Transition to the IDLE state
      break;
   default:
      break;
   }
}

void mtu_updated(struct bt_conn *conn, uint16_t tx, uint16_t rx)
{
   printk("Updated MTU: TX: %d RX: %d bytes\n", tx, rx);
}

static void device_found(const bt_addr_le_t *addr, int8_t rssi, uint8_t type,
                         struct net_buf_simple *ad)
{
   char dev[BT_ADDR_LE_STR_LEN];
   struct bt_conn *conn;
   int err;

   bt_addr_le_to_str(addr, dev, sizeof(dev));
   printk("[DEVICE]: %s, AD evt type %u, AD data len %u, RSSI %i\n",
          dev, type, ad->len, rssi);

   /* We're only interested in connectable events */
   if (type != BT_GAP_ADV_TYPE_ADV_IND &&
       type != BT_GAP_ADV_TYPE_ADV_DIRECT_IND)
   {
      return;
   }

   /* connect only to devices in close proximity */
   if (rssi < -50)
   {
      return;
   }

   err = bt_le_scan_stop();
   if (err)
   {
      printk("%s: Stop LE scan failed (err %d)\n", __func__, err);
      return;
   }

   err = bt_conn_le_create(addr, BT_CONN_LE_CREATE_CONN,
                           BT_LE_CONN_PARAM_DEFAULT, &conn);
   if (err)
   {
      printk("%s: Create conn failed (err %d)\n", __func__, err);
      start_scan();
   }
   else
   {
      bt_conn_unref(conn);
   }
}

int initialise_and_start_ble_transport_obj(void)
{
   int err = 0;
   zephyrAO_constructor(&bluetoothTransportAO, &bluetoothTransportAO_Idle);
   zephyrAO_start(&bluetoothTransportAO, my_msgq_buffer, bluetoothTransportAO_stack);

   err = bt_enable(NULL);
   if (err)
   {
      printk("Bluetooth init failed (err %d)\n", err);
      return 0;
   }

   bt_gatt_cb_register(&gatt_callbacks);

   return 0;
}