import { defineConfig } from "vite";
import { createRequire } from "module";
import fs from "fs";
import path from "path";

const _require = createRequire(import.meta.url);

// Custom middleware to serve wasm files with the correct MIME type
// See https://stackoverflow.com/questions/78095780/web-assembly-wasm-errors-in-a-vite-vue-app-using-realm-web-sdk
const wasmMiddleware = () => {
  return {
    name: "wasm-middleware",
    configureServer(server) {
      server.middlewares.use((req, res, next) => {
        if (req.url && req.url.endsWith(".wasm")) {
          const wasmPath = path.join(
            path.dirname(_require.resolve("f3d")),
            path.basename(req.url),
          );
          const wasmFile = fs.readFileSync(wasmPath);
          res.setHeader("Content-Type", "application/wasm");
          res.end(wasmFile);
          return;
        }
        next();
      });
    },
  };
};

export default defineConfig({
  plugins: [wasmMiddleware()],
  base: "./",
});
