import { defineConfig } from 'vite'
import react from '@vitejs/plugin-react'
import eslint from 'vite-plugin-eslint'

export default defineConfig({
  base: '/',
  plugins: [react(), eslint({
    exclude: ['/virtual:/**', 'node_modules/**'],
    include: 'src/**/*.jsx',
    overrideConfigFile: './eslint.config.js',
  })],

  server: {
    open: true,
    port: 3000,
    proxy: {
      '/api': {
        target: 'http://localhost:8080'
      }
    }
  },
})
