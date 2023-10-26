export default function Modal({ modal }: any): JSX.Element {
  return (
    <section
      className={`absolute w-11/12 h-fit text-center top-5 z-10 flex justify-center rounded-md tracking-wide opacity-90 font-semibold text-gray-500 text-lg ${
        modal.type === "fail" ? "bg-red-400" : "bg-green-400"
      }`}
    >
      {modal.message}
    </section>
  );
}
