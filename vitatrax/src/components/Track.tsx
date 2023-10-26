import { useState } from "react";

export default function Track({ setModal }: any): JSX.Element {
  const [server, setServer] = useState<any>();
  const [attemptToConnect, setAttemptToConnect] = useState<boolean>(false);
  const [id, setID] = useState<any>({
    uartService: "6e400001-b5a3-f393-e0a9-e50e24dcca9e",
    rxCharacteristic: "6e400002-b5a3-f393-e0a9-e50e24dcca9e",
    txCharacteristic: "6e400003-b5a3-f393-e0a9-e50e24dcca9e",
  });

  // Connect to Bluetooth
  async function connectToBluetooth() {
    setAttemptToConnect(true);
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
        message: "Successfully Connected to Tiny Circuit",
      });
      setAttemptToConnect(false);
    } catch (error) {
      setModal({
        active: true,
        type: "fail",
        message: "Something went wrong. Please try again.",
      });
      setAttemptToConnect(false);
    }
  }

  // Disconnect from Bluetooth
  function disconnectFromBluetooth() {
    setAttemptToConnect(true);
    try {
      server.disconnect();
      setServer(null);
      setAttemptToConnect(false);
    } catch (error) {
      setModal({
        active: true,
        type: "fail",
        message: "Something went wrong. Please try again.",
      });
      setAttemptToConnect(false);
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
    <>
      <div className="w-full flex justify-center pt-4">
        {server ? (
          <button
            className="bg-blue-300 w-11/12 lg:w-1/4 h-12 rounded-2xl font-medium text-xl tracking-widest hover:bg-blue-400 disabled:cursor-not-allowed"
            onClick={() => {
              disconnectFromBluetooth();
            }}
            disabled={attemptToConnect}
          >
            {attemptToConnect ? (
              <div className="flex flex-row justify-center items-center">
                <svg
                  className="animate-spin -ml-1 mr-3 h-5 w-5 text-white"
                  xmlns="http://www.w3.org/2000/svg"
                  fill="none"
                  viewBox="0 0 24 24"
                >
                  <circle
                    className="opacity-25"
                    cx="12"
                    cy="12"
                    r="10"
                    stroke="currentColor"
                    stroke-width="4"
                  ></circle>
                  <path
                    className="opacity-75"
                    fill="currentColor"
                    d="M4 12a8 8 0 018-8V0C5.373 0 0 5.373 0 12h4zm2 5.291A7.962 7.962 0 014 12H0c0 3.042 1.135 5.824 3 7.938l3-2.647z"
                  ></path>
                </svg>
                <p className="pl-4">Disconnecting...</p>
              </div>
            ) : (
              <p>Disconnect from TinyCircuit</p>
            )}
          </button>
        ) : (
          <button
            className="bg-blue-300 w-11/12 lg:w-1/4 h-12 rounded-2xl font-medium text-xl tracking-widest hover:bg-blue-400 disabled:cursor-not-allowed"
            onClick={() => {
              connectToBluetooth();
            }}
            disabled={attemptToConnect}
          >
            {attemptToConnect ? (
              <div className="flex flex-row justify-center items-center">
                <svg
                  className="animate-spin -ml-1 mr-3 h-5 w-5 text-white"
                  xmlns="http://www.w3.org/2000/svg"
                  fill="none"
                  viewBox="0 0 24 24"
                >
                  <circle
                    className="opacity-25"
                    cx="12"
                    cy="12"
                    r="10"
                    stroke="currentColor"
                    stroke-width="4"
                  ></circle>
                  <path
                    className="opacity-75"
                    fill="currentColor"
                    d="M4 12a8 8 0 018-8V0C5.373 0 0 5.373 0 12h4zm2 5.291A7.962 7.962 0 014 12H0c0 3.042 1.135 5.824 3 7.938l3-2.647z"
                  ></path>
                </svg>
                <p className="pl-4">Connecting...</p>
              </div>
            ) : (
              <p>Connect to TinyCircuit</p>
            )}
          </button>
        )}
      </div>
      {server && (
        <div className="flex justify-center items-center w-full pt-6">
          <section className="relative flex flex-col items-center w-11/12 sm:w-9/12 h-fit pb-10 rounded-xl bg-white shadow-lg">
            <div className="flex flex-col justify-center w-full pl-10 pt-10">
              <span className="font-bold text-3xl text-blue-500">
                Step Progress
              </span>
              <div className="pt-6">
                <p className="font-medium">Alarm</p>
                <input
                  className="w-11/12 h-12 outline-none border-2 rounded-xl pl-4 pr-4 mt-2 shadow-md focus:border-orange-500"
                  type="text"
                  placeholder="Set your alarm in 24 hour format e.g 0800"
                />
                <button className="lg:ml-2 mt-2 bg-blue-300 rounded-lg w-11/12 lg:w-16 h-10 font-semibold">
                  Send
                </button>
              </div>
              <div className="pt-4">
                <p className="font-medium">Mood</p>
                <input
                  className="w-11/12 h-12 outline-none border-2 rounded-xl pl-4 pr-20 mt-2 shadow-md focus:border-orange-500"
                  placeholder="Enter your mood today"
                />
                <button className="lg:ml-2 mt-2 bg-blue-300 rounded-lg w-11/12 lg:w-16 h-10 font-semibold">
                  Send
                </button>
              </div>
              {/* <button
                type="submit"
                className="mt-6 rounded-lg w-11/12 h-12 disabled:bg-gray-400 disabled:cursor-not-allowed bg-blue-500 hover:bg-blue-600 transition-colors ease-in-out"
              >
                <span className="text-white font-medium">Login</span>
              </button>
              <div className="w-11/12 pt-6 flex justify-center items-center"></div> */}
            </div>
          </section>
        </div>
      )}
    </>
  );
}
