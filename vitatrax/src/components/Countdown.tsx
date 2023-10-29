import { useState, useEffect } from "react";

export default function Countdown({ currentDate }: any): JSX.Element {
  const [timeLeft, setTimeLeft] = useState("");

  useEffect(() => {
    // Update the time remaining every second
    const interval = setInterval(() => {
      const now: any = new Date();
      const [year, month, day] = currentDate.split("-").map(Number);

      const nextDay: any = new Date(year, month - 1, day + 1, 0, 0, 0, 0);
      const timeUntilNextDay = nextDay - now;
      const hours = Math.floor((timeUntilNextDay / 3600000) % 24);
      const minutes = Math.floor((timeUntilNextDay / 60000) % 60);
      const seconds = Math.floor((timeUntilNextDay / 1000) % 60);

      // Format the time as hours:minutes:seconds
      const formattedTime = `${String(hours).padStart(2, "0")}:${String(
        minutes
      ).padStart(2, "0")}:${String(seconds).padStart(2, "0")}`;

      setTimeLeft(formattedTime);
    }, 1000);

    // Clear the interval when the component unmounts
    return () => {
      clearInterval(interval);
    };
  }, [currentDate]);
  return (
    <div className="font-bold text-2xl drop-shadow-2xl text-green-600 pb-6">
      {timeLeft} till Step Reset
    </div>
  );
}
