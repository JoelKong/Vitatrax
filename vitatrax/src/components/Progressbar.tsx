export default function ProgressBar({ progress, goal }: any): JSX.Element {
  const completionPercentage = (progress / goal) * 100;

  const clampedPercentage = Math.min(completionPercentage, 100);

  const innerDivStyle = {
    width: `${clampedPercentage}%`,
  };

  return (
    <div className="relative w-11/12 lg:w-8/12 h-8 rounded-full border-2 bg-gray-200 flex justify-center mt-4 lg:mt-0">
      <div
        className="absolute top-0 left-0 z-10 bg-green-400 h-full rounded-full border-2"
        style={innerDivStyle}
      ></div>
      <p className="relative z-20 text-lg font-semibold text-gray-500">{`${progress} / ${goal} Steps`}</p>
    </div>
  );
}
