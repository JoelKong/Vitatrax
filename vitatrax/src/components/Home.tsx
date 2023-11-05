import { useState, useEffect } from "react";
import Track from "./Track";
import History from "./History";
import Modal from "./Modal";
import Description from "./Description";
import Countdown from "./Countdown";
import supabase from "@/utils/supabase";
import Progress from "./Progress";
import Shop from "./Shop";

export default function Home(): JSX.Element {
  const [switchScreen, setSwitchScreen] = useState({
    connect: true,
    timings: false,
    progress: false,
    shop: false,
  });
  const [modal, setModal] = useState<any>({
    active: false,
    type: "fail",
    message: "",
  });
  const [currentDate, setCurrentDate] = useState<string>(getCurrentDate());

  // Function to get the current date
  function getCurrentDate() {
    const now = new Date();
    const year = now.getFullYear();
    const month = String(now.getMonth() + 1).padStart(2, "0");
    const day = String(now.getDate()).padStart(2, "0");
    return `${year}-${month}-${day}`;
  }

  // Check if the date has changed, and reset step progress
  async function checkAndResetStepProgressDate() {
    let { data: settings, error } = await supabase
      .from("settings")
      .select("current_date")
      .eq("id", "1");

    const currentDateWithoutTimeZone = new Date(settings![0].current_date);
    const formattedDate = currentDateWithoutTimeZone
      .toISOString()
      .split("T")[0];

    if (formattedDate !== currentDate) {
      let { data: allData } = await supabase.from("settings").select("*");

      let currentPoints = parseInt(allData![0].eco_points);
      let additionalEcoPoints =
        Math.floor(parseInt(allData![0].step_progress) / 1000) * 10;
      let totalPoints = currentPoints + additionalEcoPoints;

      await supabase
        .from("settings")
        .update({ eco_points: totalPoints })
        .eq("id", "1");

      const calories = (3.9 * allData![0].weight * 3.5) / 200;
      const emission = (calories / 100) * 2.2;
      const trees_saved = (allData![0].step_progress * emission) / 21.77;
      const car_miles = emission / 2.3;

      await supabase
        .from("eco")
        .insert([
          {
            weight: allData![0].weight,
            step_progress: allData![0].step_progress,
            emissions: emission,
            trees_saved: trees_saved,
            car_miles: car_miles,
          },
        ])
        .select();

      await supabase
        .from("settings")
        .update({ current_date: currentDate, step_progress: 0 })
        .eq("id", "1");
    }
  }

  // Turn off modal
  useEffect(() => {
    const timeout = setTimeout(() => {
      setModal({ active: false, type: "fail", message: "" });
    }, 3000);
    return () => clearTimeout(timeout);
  }, [modal]);

  useEffect(() => {
    const checkDate = setInterval(() => {
      const currentDateNow = getCurrentDate();
      setCurrentDate(currentDateNow);
      checkAndResetStepProgressDate();
    }, 3000);

    return () => {
      clearInterval(checkDate);
    };
  }, []);

  return (
    <>
      <header className="relative flex flex-col justify-center items-center w-full h-36 bg-green-200">
        {modal.active && <Modal modal={modal} />}
        <p className="font-bold text-5xl text-transparent bg-clip-text bg-gradient-to-l from-[#3e6442] to-[#378B29] drop-shadow-lg">
          VitaTrax
        </p>
        <p className="pt-4 font-semibold text-md md:text-xl text-green-600 drop-shadow-lg">
          Saving the environment, one step at a time
        </p>
      </header>
      <div className="flex justify-center items-center w-full h-24">
        <div className="flex justify-around items-center w-full lg:w-2/4 h-3/4">
          <div className="relative">
            <button
              className="font-semibold text-xl md:text-2xl"
              onClick={() => {
                setSwitchScreen({
                  connect: true,
                  timings: false,
                  progress: false,
                  shop: false,
                });
              }}
            >
              Connect
            </button>
            <div
              className={`absolute w-full h-2 ${
                switchScreen.connect &&
                "border-2 bg-green-600 transition ease-in-out"
              }`}
            ></div>
          </div>
          <div className="relative">
            <button
              className="font-semibold text-xl md:text-2xl"
              onClick={() => {
                setSwitchScreen({
                  connect: false,
                  timings: true,
                  progress: false,
                  shop: false,
                });
              }}
            >
              Timings
            </button>
            <div
              className={`absolute w-full h-2 ${
                switchScreen.timings &&
                "border-2 bg-green-600 transition ease-in-out"
              }`}
            ></div>
          </div>
          <div className="relative">
            <button
              className="font-semibold text-xl md:text-2xl"
              onClick={() => {
                setSwitchScreen({
                  connect: false,
                  timings: false,
                  progress: true,
                  shop: false,
                });
              }}
            >
              Progress
            </button>
            <div
              className={`absolute w-full h-2 ${
                switchScreen.progress &&
                "border-2 bg-green-600 transition ease-in-out"
              }`}
            ></div>
          </div>
          <div className="relative">
            <button
              className="font-semibold text-xl md:text-2xl"
              onClick={() => {
                setSwitchScreen({
                  connect: false,
                  timings: false,
                  progress: false,
                  shop: true,
                });
              }}
            >
              Shop
            </button>
            <div
              className={`absolute w-full h-2 ${
                switchScreen.shop &&
                "border-2 bg-green-600 transition ease-in-out"
              }`}
            ></div>
          </div>
        </div>
      </div>
      <div className="text-center w-full">
        <Countdown currentDate={currentDate} />
      </div>
      {switchScreen.connect && <Track setModal={setModal} />}
      {switchScreen.timings && <History />}
      {switchScreen.progress && <Progress />}
      {switchScreen.shop && <Shop setModal={setModal} />}
      <Description />
    </>
  );
}
