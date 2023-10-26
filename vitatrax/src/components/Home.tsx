import { useState, useEffect } from "react";

export default function Home(): JSX.Element {
  const [server, setServer] = useState<any>();
  const [id, setID] = useState<any>({
    uartService: "6e400001-b5a3-f393-e0a9-e50e24dcca9e",
    rxCharacteristic: "6e400002-b5a3-f393-e0a9-e50e24dcca9e",
    txCharacteristic: "6e400003-b5a3-f393-e0a9-e50e24dcca9e",
  });
  const [modal, setModal] = useState<any>({
    active: false,
    type: "fail",
    message: "",
  });

  // Connect to Bluetooth
  async function connectToBluetooth() {
    try {
      // @ts-ignore
      const device = await navigator.bluetooth.requestDevice({
        filters: [{ name: "BlueNRG" }],
        optionalServices: [id.uartService],
      });

      const server = await device.gatt.connect();
      setServer(server);
      setModal({
        active: true,
        type: "pass",
        message: "Successfully Connected to Vitatrax",
      });

      console.log("successfully connected");
    } catch (error) {
      setModal({
        active: true,
        type: "pass",
        message: "Something went wrong. Please try again.",
      });
    }
  }

  // Write to TinyCircuit
  async function writeToBluetooth(server: any) {
    const primaryService = await server.getPrimaryService(id.uartService);
    const rxCharacteristic = await primaryService.getCharacteristic(
      id.rxCharacteristic
    );
    const txCharacteristic = await primaryService.getCharacteristic(
      id.txCharacteristic
    );

    // Read data from the TX characteristic (UART data from the peripheral)
    // txCharacteristic.addEventListener(
    //   "characteristicvaluechanged",
    //   (event: any) => {
    //     const data = event.target.value;
    //     // Process received data (data.buffer)
    //   }
    // );
    // await txCharacteristic.startNotifications();

    // Write data to the RX characteristic
    const encoder = new TextEncoder();
    const userDescription = encoder.encode("hello");
    await rxCharacteristic.writeValue(userDescription);
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
