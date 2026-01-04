import { build } from 'esbuild';
import fs from 'fs';
import { minify as minifyHtml } from 'html-minifier-terser';

const result = await build({
  entryPoints: ['./main.ts'],
  bundle: true,
  minify: true,
  write: false,
  target: 'es2022',
  format: 'esm'
});

let js = result.outputFiles[0].text;
let html = fs.readFileSync('./index.html', 'utf8');

// Inline JS
html = html.replace(
  /<script type="module" src=".*"><\/script>/,
  `<script type="module">${js}</script>`
);


// Aggressive HTML minification
html = await minifyHtml(html, {
  collapseWhitespace: true,
  removeComments: true,
  removeRedundantAttributes: true,
  removeEmptyAttributes: true,
  removeOptionalTags: true,
  removeScriptTypeAttributes: true,
  removeStyleLinkTypeAttributes: true,
  useShortDoctype: true,
  minifyCSS: true,
  minifyJS: true,
  keepClosingSlash: false
});

fs.writeFileSync('./out.html', html);
