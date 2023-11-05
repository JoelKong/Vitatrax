import { useState, useEffect } from "react";
import supabase from "@/utils/supabase";

export default function History(): JSX.Element {
  const [time, setTime] = useState<any>();
  const [stopwatchTime, setStopwatchTime] = useState<any>();

  async function readTime() {
    let { data: dateTimings }: any = await supabase
      .from("timings")
      .select("created_at");

    let { data: watchTimings } = await supabase
      .from("timings")
      .select("timing");

    const extractedDateTimes = [];

    for (let i = 0; i < dateTimings.length; i++) {
      const timestamp = dateTimings[i].created_at;
      const date = new Date(timestamp);

      const year = date.getFullYear();
      const month = (date.getMonth() + 1).toString().padStart(2, "0");
      const day = date.getDate().toString().padStart(2, "0");

      // Get the time in the format HH:MM:SS
      const hours = date.getHours().toString().padStart(2, "0");
      const minutes = date.getMinutes().toString().padStart(2, "0");
      const seconds = date.getSeconds().toString().padStart(2, "0");

      const extractedDateTime = `${year}-${month}-${day} ${hours}:${minutes}:${seconds}`;

      extractedDateTimes.push(extractedDateTime);
    }

    setTime(extractedDateTimes);
    setStopwatchTime(watchTimings);
  }

  useEffect(() => {
    readTime();
    const interval = setInterval(() => {
      readTime();
    }, 5000);

    return () => {
      clearInterval(interval);
    };
  }, []);

  return (
    <section className="w-full h-full flex flex-col items-center justify-center">
      <div className="font-bold text-2xl mb-4 text-green-700">
        Stopwatch Timings
      </div>
      <div className="w-11/12 lg:w-5/12 flex justify-around rounded-3xl border-2 h-fit pt-2 pb-4 bg-blue-300">
        <div className="flex flex-col justify-center items-center h-fit">
          <span className="pb-4 font-bold text-xl text-green-700">Date</span>
          {time &&
            time
              .slice(0)
              .reverse()
              .map((dateTime: any, index: number) => {
                return (
                  <p
                    key={`time_${index}`}
                    className="tracking-widest font-medium pt-2"
                  >
                    {dateTime}
                  </p>
                );
              })}
        </div>
        <div className="flex flex-col justify-center items-center h-fit">
          <span className="pb-4 font-bold text-xl text-green-700">Timing</span>
          {stopwatchTime &&
            stopwatchTime
              .slice(0)
              .reverse()
              .map((stopwatch: any, index: number) => {
                return (
                  <p
                    key={`stopwatch_${index}`}
                    className="tracking-widest font-medium pt-2"
                  >
                    {stopwatch.timing}
                  </p>
                );
              })}
        </div>
      </div>
    </section>
  );
}
