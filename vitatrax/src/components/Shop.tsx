import { useState, useEffect } from "react";
import Image from "next/image";
import supabase from "@/utils/supabase";

export default function Shop({ setModal }: any): JSX.Element {
  const [ecoPoints, setEcoPoints] = useState<number>(0);
  const [vouchers, setVouchers] = useState<any>([
    {
      index: 1,
      name: "FoodPanda Voucher",
      image: "/foodpanda.png",
      description: "$10 off on food delivery",
      price: "30",
    },
    ,
    {
      index: 2,
      name: "FoodPanda Voucher",
      image: "/foodpanda.png",
      description: "$20 off on food delivery",
      price: "50",
    },
    {
      index: 3,
      name: "Grab Voucher",
      image: "/grab.png",
      description: "$10 off on food delivery",
      price: "30",
    },
    {
      index: 4,
      name: "Grab Voucher",
      image: "/grab.png",
      description: "$15 off on food delivery, $5 off grab ride",
      price: "50",
    },
    {
      index: 5,
      name: "Amazon Gift Card",
      image: "/amazon.png",
      description: "$25 gift card from Amazon",
      price: "100",
    },
    {
      index: 6,
      name: "Amazon Gift Card",
      image: "/amazon.png",
      description: "$50 gift card from Amazon",
      price: "185",
    },
  ]);

  async function getEcoData() {
    let { data: eco, error } = await supabase
      .from("settings")
      .select("eco_points")
      .eq("id", "1");

    setEcoPoints(eco![0].eco_points);
  }

  async function buyVoucher(price: any, voucher: any) {
    if (ecoPoints < price) {
      setModal({
        active: true,
        type: "fail",
        message: "Not enough eco-points",
      });
      return;
    }

    const difference = ecoPoints - price;
    setEcoPoints(difference);

    await supabase
      .from("settings")
      .update({ eco_points: difference })
      .eq("id", "1")
      .select();

    setModal({
      active: true,
      type: "pass",
      message: `Successfully bought ${voucher}`,
    });
  }

  useEffect(() => {
    getEcoData();
  }, []);

  return (
    <section className="w-full flex justify-center">
      <div className="w-11/12 md:w-8/12 flex flex-col items-center">
        <div className="font-bold text-2xl mb-4 text-green-700">
          {ecoPoints} Eco-Points Available
        </div>
        <div className="w-full rounded-xl mb-10 flex flex-col justify-center items-center shadow-2x">
          <div className="flex flex-col w-full md:w-7/12 items-center">
            {vouchers.map((voucher: any, index: any) => {
              return (
                <div
                  key={index}
                  className="relative w-full rounded-lg h-36 bg-green-300 mb-6 flex flex-col pl-4 pt-2"
                >
                  <Image
                    src={voucher.image}
                    alt={voucher.name}
                    height={100}
                    width={100}
                  />
                  <p className="pt-2 font-semibold">{voucher.name}</p>
                  <p>{voucher.description}</p>
                  <button
                    className="absolute top-2 right-2 w-32 md:w-36 h-12 bg-blue-400 rounded-lg hover:bg-blue-500"
                    onClick={() => buyVoucher(voucher.price, voucher.name)}
                  >
                    {voucher.price} eco-points
                  </button>
                </div>
              );
            })}
          </div>
        </div>
      </div>
    </section>
  );
}
