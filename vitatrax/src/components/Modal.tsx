export default function Modal({ modal }: any): JSX.Element {
  return (
    <section
      className={`absolute w-1/2 h-fit text-center top-5 z-10 flex justify-center rounded-lg tracking-wide opacity-90 font-semibold text-gray-600 text-lg ${
        modal.type === "fail" ? "bg-red-400" : "bg-green-400"
      }`}
    >
      {modal.message}
    </section>
  );
}
