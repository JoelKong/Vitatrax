import { useState, useEffect } from "react";
import Track from "./Track";
import History from "./History";
import Modal from "./Modal";
import Description from "./Description";

export default function Home(): JSX.Element {
  const [switchToHistory, setSwitchToHistory] = useState<boolean>(false);
  const [modal, setModal] = useState<any>({
    active: false,
    type: "fail",
    message: "",
  });

  // Turn off modal
  useEffect(() => {
    const timeout = setTimeout(() => {
      setModal({ active: false, type: "fail", message: "" });
    }, 3000);
    return () => clearTimeout(timeout);
  }, [modal]);

  return (
    <>
      <header className="relative flex flex-col justify-center items-center w-full h-36 bg-green-200">
        {modal.active && <Modal modal={modal} />}
        <p className="font-bold text-5xl text-transparent bg-clip-text bg-gradient-to-l from-[#3e6442] to-[#378B29] drop-shadow-lg">
          VitaTrax
        </p>
        <p className="pt-4 font-semibold text-xl text-green-600 drop-shadow-lg">
          Saving the environment, one step at a time
        </p>
      </header>
      <div className="flex justify-center items-center w-full h-24">
        <div className="flex justify-around items-center w-3/4 lg:w-2/4 h-3/4">
          <div className="relative">
            <button
              className="font-semibold text-2xl"
              onClick={() => {
                setSwitchToHistory(false);
              }}
            >
              Track
            </button>
            <div
              className={`absolute w-full h-2 ${
                switchToHistory ||
                "border-2 bg-green-600 transition ease-in-out"
              }`}
            ></div>
          </div>
          <div className="relative">
            <button
              className="font-semibold text-2xl"
              onClick={() => {
                setSwitchToHistory(true);
              }}
            >
              History
            </button>
            <div
              className={`absolute w-full h-2 ${
                switchToHistory &&
                "border-2 bg-green-600 transition ease-in-out"
              }`}
            ></div>
          </div>
        </div>
      </div>
      {switchToHistory ? <History /> : <Track setModal={setModal} />}
      <Description />
    </>
  );
}
