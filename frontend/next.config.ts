import type { NextConfig } from "next";

const nextConfig: NextConfig = {
  output: "standalone",
  /* rest of your config */
  allowedDevOrigins: ['0.0.0.0', '192.168.1.18'],
};

export default nextConfig;
