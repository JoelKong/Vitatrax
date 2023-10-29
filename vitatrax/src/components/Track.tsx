import { useState, useEffect } from "react";
import ProgressBar from "./Progressbar";
import supabase from "@/utils/supabase";

export default function Track({ setModal }: any): JSX.Element {
  const [server, setServer] = useState<any>();
  const [stepProgress, setStepProgress] = useState<any>({
    progress: 0,
    goal: 0,
  });
  const [attemptToConnect, setAttemptToConnect] = useState<boolean>(false);
  const [id, setID] = useState<any>({
    uartService: "6e400001-b5a3-f393-e0a9-e50e24dcca9e",
    rxCharacteristic: "6e400002-b5a3-f393-e0a9-e50e24dcca9e",
    txCharacteristic: "6e400003-b5a3-f393-e0a9-e50e24dcca9e",
  });
  const [result, setResult] = useState<string>("");
  const [data, setData] = useState({
    alarm: "1000",
    mood: "h",
    stepGoal: "10000",
    weight: "70",
  });
  const [loading, setLoading] = useState<boolean>(false);

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
      await readData();
      await writeToBluetooth(server);
      await readFromBluetooth(server);
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
    setLoading(true);
    try {
      const primaryService = await server.getPrimaryService(id.uartService);
      const rxCharacteristic = await primaryService.getCharacteristic(
        id.rxCharacteristic
      );
      const txCharacteristic = await primaryService.getCharacteristic(
        id.txCharacteristic
      );

      // Write data to the RX characteristic
      const encoder = new TextEncoder();
      const userDescription = encoder.encode(
        `${data.alarm}_${data.mood}_${data.stepGoal}_${data.weight}`
      );
      await rxCharacteristic.writeValue(userDescription);

      // Update db
      await supabase
        .from("settings")
        .update({
          alarm: data.alarm,
          mood: data.mood,
          step_goal: data.stepGoal,
          weight: data.weight,
        })
        .eq("id", "1")
        .select();

      setModal({
        active: true,
        type: "pass",
        message: "Successfully updated data!",
      });
      setLoading(false);
    } catch (error) {
      console.log(`${data.alarm}_${data.mood}_${data.stepGoal}_${data.weight}`);
      console.log(error);
      setLoading(false);
      setModal({
        active: true,
        type: "fail",
        message: "Something went wrong. Please try again.",
      });
    }
  }

  // Read from bluetooth
  async function readFromBluetooth(server: any) {
    try {
      const primaryService = await server.getPrimaryService(id.uartService);
      const txCharacteristic = await primaryService.getCharacteristic(
        id.txCharacteristic
      );

      // Read data from the TX characteristic (UART data from the peripheral)
      txCharacteristic.addEventListener(
        "characteristicvaluechanged",
        (event: any) => {
          const data = event.target.value;
          const newProgress = parseInt(data, 10);
          setStepProgress((prevStepProgress: any) => ({
            ...prevStepProgress,
            progress: newProgress,
          }));
        }
      );
      await txCharacteristic.startNotifications();
    } catch (error) {
      setModal({
        active: true,
        type: "fail",
        message: "Something went wrong. Please try again.",
      });
    }
  }

  // Generate motivational message
  async function generateMessage() {
    setLoading(true);
    const response = await fetch("/api/generate", {
      method: "POST",
      headers: {
        "Content-Type": "application/json",
      },
    });

    if (!response.ok) {
      const errorMessage = await response.text();
      setModal({
        active: true,
        type: "fail",
        message: "Something went wrong. Please try again.",
      });
      setLoading(false);
      return;
    }

    const data = response.body;
    if (!data) {
      setLoading(false);
      return;
    }

    const reader = data.getReader();
    const decoder = new TextDecoder();
    let done = false;
    let outputString = "";
    setResult("");

    while (!done) {
      const { value, done: doneReading } = await reader.read();
      done = doneReading;
      const chunkValue: any = decoder.decode(value);
      setResult((prev) => prev + chunkValue);
      outputString += chunkValue;
    }

    setLoading(false);
  }

  // Handle input change
  function handleChange(e: any) {
    const name = e.target.name;
    const value = e.target.value;
    setData({ ...data, [name]: value });
  }

  // Read from db
  async function readData() {
    let { data: settings, error } = await supabase.from("settings").select("*");
    setData({
      alarm: settings![0].alarm,
      mood: settings![0].mood,
      stepGoal: settings![0].step_goal,
      weight: settings![0].weight,
    });
    setStepProgress({
      progress: settings![0].step_progress,
      goal: settings![0].step_goal,
    });
  }

  // Update progress in db
  async function updateSupabaseWithProgress() {
    try {
      await supabase
        .from("settings")
        .update({
          step_progress: stepProgress.progress,
        })
        .eq("id", "1")
        .select();
    } catch (error) {
      console.log(error);
    }
  }

  useEffect(() => {
    readData();

    const settings = supabase
      .channel("custom-update-channel")
      .on(
        "postgres_changes",
        { event: "UPDATE", schema: "public", table: "settings" },
        (payload) => {
          console.log("Change received!", payload);
          setStepProgress((prevStepProgress: any) => ({
            ...prevStepProgress,
            goal: payload.new.step_goal,
          }));
        }
      )
      .subscribe();

    // Send updates to progress
    const updateInterval = setInterval(() => {
      updateSupabaseWithProgress();
    }, 5000);

    return () => {
      clearInterval(updateInterval);
      settings.unsubscribe();
    };
  }, []);

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
          <section className="relative flex flex-col items-center w-11/12 sm:w-7/12 h-fit pb-10 rounded-xl bg-white shadow-lg">
            <div className="flex flex-col justify-center w-full pl-10 pt-10">
              <div className="block lg:flex flex-row items-center">
                <span className="font-bold text-3xl text-blue-500 pr-5">
                  Step Progress
                </span>
                <ProgressBar
                  progress={stepProgress.progress}
                  goal={stepProgress.goal}
                />
              </div>
              <div className="pt-6">
                <p className="font-medium">Alarm</p>
                <input
                  className="w-11/12 h-12 outline-none border-2 rounded-xl pl-4 pr-4 mt-2 shadow-md focus:border-orange-500"
                  type="text"
                  name="alarm"
                  value={data.alarm}
                  onChange={(e) => handleChange(e)}
                  placeholder="Set your alarm in 24 hour format e.g 0800"
                />
              </div>
              <div className="pt-4">
                <p className="font-medium">Mood</p>
                <select
                  className="w-11/12 h-12 outline-none border-2 rounded-xl pl-2 pr-20 mt-2 shadow-md focus:border-orange-500"
                  placeholder="What is your mood today?"
                  name="mood"
                  value={data.mood}
                  onChange={(e) => handleChange(e)}
                >
                  <option value="h">Happy</option>
                  <option value="n">Neutral</option>
                  <option value="s">Sad</option>
                </select>
                <div className="pt-6">
                  <p className="font-medium">Step Goal</p>
                  <input
                    className="w-11/12 h-12 outline-none border-2 rounded-xl pl-4 pr-4 mt-2 shadow-md focus:border-orange-500"
                    type="text"
                    placeholder="Set your step goal"
                    name="stepGoal"
                    value={data.stepGoal}
                    onChange={(e) => handleChange(e)}
                  />

                  <div className="pt-6">
                    <p className="font-medium">Weight</p>
                    <input
                      className="w-11/12 h-12 outline-none border-2 rounded-xl pl-4 pr-4 mt-2 shadow-md focus:border-orange-500"
                      type="text"
                      name="weight"
                      value={data.weight}
                      onChange={(e) => handleChange(e)}
                      placeholder="Set your weight"
                    />
                  </div>
                  <button
                    disabled={loading}
                    onClick={() => writeToBluetooth(server)}
                    className="mt-12 rounded-lg w-11/12 h-12 disabled:bg-gray-400 disabled:cursor-not-allowed bg-blue-500 hover:bg-blue-600 transition-colors ease-in-out"
                  >
                    <span className="text-white font-medium">
                      {loading ? (
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
                        </div>
                      ) : (
                        "Send Data"
                      )}
                    </span>
                  </button>
                </div>
              </div>
              <button
                disabled={loading}
                onClick={() => generateMessage()}
                className="mt-8 rounded-lg w-11/12 h-12 disabled:bg-gray-400 disabled:cursor-not-allowed bg-blue-500 hover:bg-blue-600 transition-colors ease-in-out"
              >
                <span className="text-white font-medium">
                  {loading ? (
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
                    </div>
                  ) : (
                    "Generate Motivational Message"
                  )}
                </span>
              </button>
              <div className="w-11/12 pt-6 flex justify-center items-center">
                {result}
              </div>
            </div>
          </section>
        </div>
      )}
    </>
  );
}
