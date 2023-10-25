import { useState, useEffect } from "react";

export default function Test(): JSX.Element {
  async function connectToBluetooth() {
    // @ts-ignore
    const bluetoothConnect = await navigator.bluetooth.requestDevice({
      filters: [{ name: "BlueNRG" }],
    });
  }

  return (
    <div>
      <button
        className="w-screen h-screen flex justify-center items-center border-2 bg-blue-200"
        onClick={() => connectToBluetooth()}
      >
        Connect to bluetooth
      </button>
    </div>
  );
}
