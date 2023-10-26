import { OpenAIStream, OpenAIStreamPayload } from "@/utils/stream";

if (!process.env.OPENAI_API_KEY) {
  throw new Error("Missing env var from OpenAI");
}

export const config = {
  runtime: "edge",
};

const handler = async (req: Request): Promise<Response> => {
  let systemContent;
  let userContent;

  systemContent =
    "You are a fitness instructor giving motivational messages for people to work hard";
  userContent =
    "Generate me a motivational message to get me going through the day";

  const payload: any = {
    model: "gpt-3.5-turbo",
    messages: [
      {
        role: "system",
        content: systemContent,
      },
      {
        role: "user",
        content: userContent,
      },
    ],
    max_tokens: 300,
    stream: true,
  };

  const stream = await OpenAIStream(payload);
  return new Response(stream);
};

export default handler;
