import { useState, useEffect } from "react";

export default function Test(): JSX.Element {
  const [server, setServer] = useState();
  async function connectToBluetooth() {
    try {
      // @ts-ignore
      const device = await navigator.bluetooth.requestDevice({
        filters: [{ name: "BlueNRG" }],
      });

      const server = await device.gatt.connect();
      setServer(server);
      console.log("successfully connected");
    } catch (error) {
      console.log("Error connecting" + error);
    }
  }

  async function writeToBluetooth(server: any) {
    // Replace with the actual UUIDs for your characteristics
    const uartServiceUUID = "6E400001-B5A3-F393-E0A9-E50E24DCCA9E";
    const rxCharacteristicUUID = "6E400002-B5A3-F393-E0A9-E50E24DCCA9E";
    const txCharacteristicUUID = "6E400003-B5A3-F393-E0A9-E50E24DCCA9E";
    const clientConfigUUID = "2902";

    const rxCharacteristic = await server
      .getPrimaryService(uartServiceUUID)
      .getCharacteristic(rxCharacteristicUUID);

    const txCharacteristic = await server
      .getPrimaryService(uartServiceUUID)
      .getCharacteristic(txCharacteristicUUID);

    // Read data from the TX characteristic (UART data from the peripheral)
    txCharacteristic.addEventListener(
      "characteristicvaluechanged",
      (event: any) => {
        const data = event.target.value;
        // Process received data (data.buffer)
      }
    );
    await txCharacteristic.startNotifications();

    // Write data to the RX characteristic (e.g., sending commands)
    const dataToSend = new Uint8Array([8]);
    await rxCharacteristic.writeValue(dataToSend);
    server.disconnect();
  }

  return (
    <div>
      <button
        className="w-screen flex justify-center items-center border-2 bg-blue-200"
        onClick={() => connectToBluetooth()}
      >
        Connect to bluetooth
      </button>
      <button
        className="w-screen flex justify-center items-center border-2 bg-blue-200"
        onClick={() => writeToBluetooth(server)}
      >
        write
      </button>
    </div>
  );
}
