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

  const { stepGoal, stepProgress } = (await req.json()) as {
    stepGoal?: number;
    stepProgress?: number;
  };

  systemContent =
    "You are a fitness instructor giving motivational messages for people to work hard depending on their step goal for the day and their current progress. Keep your message within 200 tokens";
  userContent = `My step goal for today is ${stepGoal} and my current progress is ${stepProgress}. Based off this, cheer me up and motivate me to reach the goal`;

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
    max_tokens: 200,
    stream: true,
  };

  const stream = await OpenAIStream(payload);
  return new Response(stream);
};

export default handler;
