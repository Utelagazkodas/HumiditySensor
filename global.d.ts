import type { Chart as ChartType } from "chart.js";

declare global {
  const Chart: typeof ChartType;
}

export {};
