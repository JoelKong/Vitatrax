import { useState, useEffect } from "react";
import { MdOutlineArrowDropDownCircle } from "react-icons/md";
import supabase from "@/utils/supabase";

export default function Progress(): JSX.Element {
  const [data, setData] = useState<any>();
  const [toggle, setToggle] = useState<any>({});

  async function getEcoData() {
    let { data: allData } = await supabase.from("eco").select("*");
    let ecoStats: any = [];

    for (let info of allData!) {
      let date = new Date(info.created_at);
      let year = date.getFullYear();
      let month = date.getMonth() + 1;
      let day = date.getDate();
      let formattedDate = `${year}-${month}-${day}`;
      let weight = info.weight;
      let step = info.step_progress;
      let emissions = info.emissions;
      let trees_saved = info.trees_saved;
      let car_miles = info.car_miles;
      ecoStats.push({
        date: formattedDate,
        weight: weight,
        step: step,
        emissions: emissions,
        trees_saved: trees_saved,
        car_miles: car_miles,
      });
    }

    setData(ecoStats);
  }

  const handleToggle = (index: number) => {
    setToggle((prevState: any) => ({
      ...prevState,
      [index]: !prevState[index],
    }));
  };

  useEffect(() => {
    getEcoData();

    const getEco = setInterval(() => {
      getEcoData;
    }, 3000);

    return () => {
      clearInterval(getEco);
    };
  }, []);

  return (
    <section className="w-full flex justify-center">
      <div className="w-11/12 md:w-8/12 flex flex-col items-center">
        <div className="font-bold text-2xl mb-4 text-green-700">
          Daily Progress 🌳
        </div>
        {data && data[0] ? (
          data.map((info: any, index: any) => {
            return (
              <div
                key={index}
                className={`w-full rounded-xl mb-10 flex flex-col justify-center items-center shadow-2xl ${
                  toggle[index] ? "h-fit pb-6 bg-green-300" : "h-14"
                }`}
              >
                <button
                  className="flex flex-row justify-center items-center w-full h-14 rounded-xl bg-blue-400 hover:bg-blue-500"
                  onClick={() => handleToggle(index)}
                >
                  <p className="font-semibold text-xl">{info.date}</p>
                  <div className="pt-1 pl-2 scale-125">
                    <MdOutlineArrowDropDownCircle />
                  </div>
                </button>
                {toggle[index] && (
                  <div className="flex flex-col items-center pt-2 font-semibold text-lg tracking-wide">
                    <p>Weight: {info.weight}</p>
                    <p>Total Steps: {info.step}</p>
                    <p>
                      Total CO2 emissions saved:{" "}
                      {Math.round(info.emissions * 1000) / 1000}
                    </p>
                    <p>Trees saved: {Math.floor(info.trees_saved)}</p>
                    <p>
                      Equivalent Car Miles travelled:{" "}
                      {Math.round(info.car_miles * 1000) / 1000}
                    </p>
                  </div>
                )}
              </div>
            );
          })
        ) : (
          <div className="text-xl font-semibold">
            No data recorded yet. Start walking!
          </div>
        )}
      </div>
    </section>
  );
}
