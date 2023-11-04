import Image from "next/image";

export default function Description(): JSX.Element {
  return (
    <section className="w-full mt-10 flex justify-center items-center">
      <article className="w-11/12 lg:w-8/12 rounded-xl flex flex-col bg-green-300 pb-12">
        <div className="w-full flex flex-col md:flex-row justify-around">
          <div className="w-full md:w-1/2 flex justify-center">
            <Image
              src="/stumpt.png"
              alt="picture of stump"
              width={300}
              height={300}
              priority
            />
          </div>
          <div className="w-full md:w-1/2 text-center flex items-center flex-col md:pr-12 md:pl-0 pl-4 pr-4">
            <header className="font-bold text-green-600 text-2xl md:pt-10 pt-0">
              Did you know?
            </header>
            <p className="pt-4 text-lg tracking-wide">
              15.3 billion trees are cut down every year. Over the pass 12,000
              years, 46% of the earth trees have been cleared. Also, 1.5 billion
              tons of greenhouse gases are released into the environment every
              year.
            </p>
          </div>
        </div>
        <div className="w-full flex flex-col-reverse md:flex-row justify-around">
          <div className="w-full md:w-1/2 text-center flex items-center flex-col md:pr-12 pl-4 pr-4">
            <header className="font-bold text-green-600 text-2xl pt-10">
              Why does it matter?
            </header>
            <p className="pt-4 text-lg tracking-wide">
              Greenhouse gases contribute to global warming, making the Earth
              hotter over time. Trees can help slow the buildup of these gases.
              Vehicle emissions also affect our health and can cause breathing
              problems.
            </p>
          </div>
          <div className="w-full md:w-1/2 flex justify-center">
            <Image
              src="/carco2.png"
              alt="picture of car emissions"
              width={400}
              height={400}
              priority
            />
          </div>
        </div>
        <div className="w-full flex flex-col md:flex-row justify-around md:pt-10 pt-0">
          <div className="w-full md:w-1/2 flex justify-center md:pt-0 pt-8">
            <Image
              src="/earth.png"
              alt="picture of earth"
              width={230}
              height={230}
              priority
            />
          </div>
          <div className="w-full md:w-1/2 text-center flex items-center flex-col md:pr-12 md:pl-0 pl-4 pr-4">
            <header className="font-bold text-green-600 text-2xl pt-10">
              Why Vitatrax?
            </header>
            <p className="pt-4 text-lg tracking-wide">
              {
                "By walking instead of using vehicles, you reduce your carbon footprint. Keep yourself healthy! To relax after a stressful day. With vitatrax, you can walk to earn rewards and save the environment at the same time!"
              }
            </p>
          </div>
        </div>
      </article>
    </section>
  );
}
