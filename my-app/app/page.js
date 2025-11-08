'use client';
import DeviceHistoryChart from "@/components/DeviceHistoryChart";

export default function Home() {
  return (
    <main className="p-6 space-y-6">
      <h1 className="text-3xl font-bold text-center">⚽ Placar FIWARE</h1>
      <DeviceHistoryChart attr="gb" />
      <DeviceHistoryChart attr="gr" />
    </main>
  );
}