import { defineConfig } from 'vite'
import { svelte } from '@sveltejs/vite-plugin-svelte'
import { fileURLToPath, URL } from 'node:url'

// https://vite.dev/config/
const repoRoot = fileURLToPath(new URL('..', import.meta.url))

export default defineConfig({
  plugins: [svelte()],
  // Use relative base so assets work when deployed under a subpath
  base: './',
  server: {
    fs: {
      // Allow importing assets from the repository root (../assets)
      allow: [repoRoot]
    }
  }
})
